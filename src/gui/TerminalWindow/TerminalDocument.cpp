
#include "TerminalDocument.h"
#include "fkt_GPIB.h"
#include "systemInfo.h"
#include "cmdGpib.h"
#include "mainHelper.h"

#include <iostream>
#include <sstream>
#include <algorithm>

// ---------------------------------------------------------------------------
// Construction / Destruction
// ---------------------------------------------------------------------------

TerminalDocument::TerminalDocument(PrologixUsbGpibAdapter& adapter)
    : m_adapter(adapter)
{
    SetupCommands();
}

TerminalDocument::~TerminalDocument()
{
    // Ensure we are disconnected when the document is destroyed
    if (m_adapter.getConnected())
    {
        m_adapter.disconnect();
        std::cerr << "TerminalDocument: adapter disconnected on destruction" << std::endl;
    }
}

// ---------------------------------------------------------------------------
// Observer management
// ---------------------------------------------------------------------------

void TerminalDocument::AddObserver(ITerminalObserver* observer)
{
    if (observer && std::find(m_observers.begin(), m_observers.end(), observer) == m_observers.end())
    {
        m_observers.push_back(observer);
    }
}

void TerminalDocument::RemoveObserver(ITerminalObserver* observer)
{
    m_observers.erase(std::remove(m_observers.begin(), m_observers.end(), observer), m_observers.end());
}

void TerminalDocument::NotifyObservers(const std::string& changeType)
{
    for (ITerminalObserver* obs : m_observers)
    {
        if (obs)
            obs->OnDocumentChanged(changeType);
    }
}

// ---------------------------------------------------------------------------
// State accessors
// ---------------------------------------------------------------------------

bool TerminalDocument::IsConnected() const
{
    return m_adapter.getConnected();
}

// ---------------------------------------------------------------------------
// Internal output helper
// ---------------------------------------------------------------------------

void TerminalDocument::AppendOutput(const std::string& text)
{
    m_lastOutput = text;
    std::cerr << text;
    NotifyObservers("OutputAppended");
}

// ---------------------------------------------------------------------------
// Command dispatch setup
// ---------------------------------------------------------------------------

void TerminalDocument::SetupCommands()
{
    m_cmds["scan"]       = [this](const std::string& args) { return ScanDevices(args); };
    m_cmds["status"]     = [this](const std::string& args) { return StatusDevice(args); };
    m_cmds["config"]     = [this](const std::string& args) { return ConfigDevice(args); };
    m_cmds["connect"]    = [this](const std::string& args) { return ConnectDevice(args); };
    m_cmds["disconnect"] = [this](const std::string& args) { return DisconnectDevice(args); };
    m_cmds["send"]       = [this](const std::string& args) { return SendToDevice(args); };
    m_cmds["read"]       = [this](const std::string& args) { return ReadFromDevice(args); };
    m_cmds["write"]      = [this](const std::string& args) { return WriteToDevice(args); };
    m_cmds["test"]       = [this](const std::string& args) { return TestDevice(args); };
}

// ---------------------------------------------------------------------------
// ProcessCommand — parse input and dispatch
// ---------------------------------------------------------------------------

void TerminalDocument::ProcessCommand(const std::string& input)
{
    m_commandHistory.push_back(input);

    std::string strCmd;
    std::string args;

    size_t firstSpace = input.find(' ');
    if (firstSpace == std::string::npos)
    {
        strCmd = input;
        args   = "";
    }
    else
    {
        strCmd = input.substr(0, firstSpace);
        args   = input.substr(firstSpace + 1);
    }

    std::cerr << "TerminalDocument: processing command '" << strCmd << "' args='" << args << "'" << std::endl;

    auto it = m_cmds.find(strCmd);
    if (it != m_cmds.end())
    {
        std::string result = it->second(args);
        AppendOutput(result);
    }
    else
    {
        AppendOutput("Unknown command!\n");
    }
}

// ---------------------------------------------------------------------------
// Command implementations
// ---------------------------------------------------------------------------

std::string TerminalDocument::ScanDevices(const std::string& /*args*/)
{
    DWORD devNum = scanUsbDev();
    std::ostringstream oss;
    oss << "Number of devices: " << devNum << "\n";
    return oss.str();
}

std::string TerminalDocument::StatusDevice(const std::string& /*args*/)
{
    return m_adapter.statusText() + "\n";
}

std::string TerminalDocument::ConnectDevice(const std::string& args)
{
    int dev = 0;
    std::ostringstream oss;

    std::cerr << "Command: connect with args: " << args << std::endl;

    if (!args.empty())
    {
        try
        {
            dev = std::stoi(args);
            if (dev == std::clamp(dev, 1, 20))
            {
                std::cerr << "Valid device number: " << dev << std::endl;
                dev = dev - 1;
            }
            else
            {
                std::cerr << "Invalid device number: " << dev << std::endl;
                dev = 0;
            }
        }
        catch (const std::exception& e)
        {
            oss << "Error parsing device number: " << e.what() << "\n";
            return oss.str();
        }
    }

    if (!m_adapter.getConnected())
    {
        m_adapter.connect();

        if (m_adapter.getStatus() == FT_OK)
        {
            oss << "Connected to device\n";
            std::cerr << "Connected to device " << dev << std::endl;
        }
        else
        {
            std::cerr << "Couldn't connect" << std::endl;
            oss << "Couldn't connect to device\n"
                << "Is program running as admin/SU?\n"
                << "Is the FTDI_SIO Driver unloaded?\n";
        }

        FT_STATUS ftStatus = FT_Purge(m_adapter.getHandle(), FT_PURGE_RX | FT_PURGE_TX);
        if (ftStatus != FT_OK)
            oss << "Purge Failed\n";
    }
    else
    {
        oss << "Device already connected\n";
    }

    NotifyObservers("ConnectionChanged");
    return oss.str();
}

std::string TerminalDocument::DisconnectDevice(const std::string& /*args*/)
{
    std::ostringstream oss;

    m_adapter.disconnect();

    if (m_adapter.getStatus() == FT_OK)
    {
        oss << "Disconnected from device\n";
        std::cerr << "Disconnected from device" << std::endl;
    }
    else
    {
        oss << "Couldn't disconnect\nCheck if a device is connected with: status\n";
    }

    NotifyObservers("ConnectionChanged");
    return oss.str();
}

std::string TerminalDocument::SendToDevice(const std::string& args)
{
    std::ostringstream oss;

    std::cerr << "Command: send " << args << std::endl;

    wxString writeResult = m_adapter.write(args);
    oss << writeResult.ToStdString();

    sleepMs(100);

    std::cerr << "Reading from device..." << std::endl;

    wxString readResult = m_adapter.read();
    oss << readResult.ToStdString();

    return oss.str();
}

std::string TerminalDocument::ReadFromDevice(const std::string& /*args*/)
{
    std::ostringstream oss;

    wxString readResult = m_adapter.read();
    oss << readResult.ToStdString();

    return oss.str();
}

std::string TerminalDocument::WriteToDevice(const std::string& args)
{
    std::ostringstream oss;

    wxString writeResult = m_adapter.write(args);
    oss << writeResult.ToStdString();

    return oss.str();
}

std::string TerminalDocument::ConfigDevice(const std::string& args)
{
    std::ostringstream oss;

    if (!args.empty())
    {
        try
        {
            int baudRate = std::stoi(args);
            if (baudRate == std::clamp(baudRate, 1, 1'000'000))
            {
                m_adapter.setBaudrate(baudRate);
                std::cerr << "Set Baudrate to " << m_adapter.getBaudrate() << std::endl;
            }
            else
            {
                std::cerr << "Baudrate out of range, using default" << std::endl;
            }
        }
        catch (const std::exception& e)
        {
            oss << "Error parsing baudrate: " << e.what() << "\n";
        }
    }
    else
    {
        std::cerr << "Using default baudrate: " << m_adapter.getBaudrate() << std::endl;
    }

    m_adapter.config();

    if (m_adapter.getStatus() == FT_OK)
        oss << "Device configured with baudrate: " << m_adapter.getBaudrate() << "\n";
    else
        oss << "Failed to configure device. Run as admin?\n";

    return oss.str();
}

std::string TerminalDocument::TestDevice(const std::string& args)
{
    std::ostringstream oss;

    if (args.empty())
    {
        oss << ConnectDevice() << ConfigDevice();

        WriteToDevice(ProLogixCmdLookup.at(ProLogixCmd::CLR));
        sleepMs(200);
        WriteToDevice(ProLogixCmdLookup.at(ProLogixCmd::MODE)       + " 1");
        WriteToDevice(ProLogixCmdLookup.at(ProLogixCmd::AUTO)       + " 1");
        WriteToDevice(ProLogixCmdLookup.at(ProLogixCmd::EOS)        + " 2");
        WriteToDevice(ProLogixCmdLookup.at(ProLogixCmd::EOI)        + " 1");
        WriteToDevice(ProLogixCmdLookup.at(ProLogixCmd::EOT_ENABLE) + " 0");
        WriteToDevice(ProLogixCmdLookup.at(ProLogixCmd::EOT_CHAR)   + " 10");
        WriteToDevice(ProLogixCmdLookup.at(ProLogixCmd::ADDR)       + " 20");
        oss << SendToDevice(ProLogixCmdLookup.at(ProLogixCmd::VER));
    }
    else if (args == "1")
    {
        WriteToDevice(ProLogixCmdLookup.at(ProLogixCmd::RST));
        sleepMs(200);

        WriteToDevice(ProLogixCmdLookup.at(ProLogixCmd::MODE) + " 1");
        WriteToDevice(ProLogixCmdLookup.at(ProLogixCmd::AUTO) + " 0");
        WriteToDevice(ProLogixCmdLookup.at(ProLogixCmd::ADDR) + " 20");

        WriteToDevice(ScpiCmdLookup.at(ScpiCmd::IDN));
        WriteToDevice(ProLogixCmdLookup.at(ProLogixCmd::READ) + " eoi");
        sleepMs(50);
        oss << ReadFromDevice();
    }
    else if (args == "mess")
    {
        WriteToDevice(ProLogixCmdLookup.at(ProLogixCmd::AUTO) + " 0");
        WriteToDevice(ProLogixCmdLookup.at(ProLogixCmd::RST));
        WriteToDevice(ScpiCmdLookup.at(ScpiCmd::CLR));
        WriteToDevice(ScpiCmdLookup.at(ScpiCmd::FREQ_CENT) + " 1 GHZ");
        WriteToDevice(ScpiCmdLookup.at(ScpiCmd::FREQ_SPAN) + " 10 MHZ");
        WriteToDevice(ScpiCmdLookup.at(ScpiCmd::BAND_RES)  + " 100 KHZ ");
        WriteToDevice(ScpiCmdLookup.at(ScpiCmd::CALC_MARK_MAX));
        WriteToDevice(ScpiCmdLookup.at(ScpiCmd::WAI));
        WriteToDevice(ProLogixCmdLookup.at(ProLogixCmd::AUTO) + " 1");
        sleepMs(200);
        oss << SendToDevice(ScpiQueryCmdLookup.at(ScpiQueryCmd::CALC_MARK1_Y));
        oss << SendToDevice(ScpiQueryCmdLookup.at(ScpiQueryCmd::CALC_MARK1_X));
    }
    else if (args == "big")
    {
        WriteToDevice(ProLogixCmdLookup.at(ProLogixCmd::AUTO) + " 0");
        WriteToDevice(ScpiCmdLookup.at(ScpiCmd::INIT_CONT) + " OFF");
        WriteToDevice(ScpiCmdLookup.at(ScpiCmd::SWE_POIN)  + " 10");
        WriteToDevice(ScpiCmdLookup.at(ScpiCmd::FREQ_STAR) + " 80 MHZ");
        WriteToDevice(ScpiCmdLookup.at(ScpiCmd::FREQ_STOP) + " 120 MHZ");
        WriteToDevice(ScpiCmdLookup.at(ScpiCmd::BAND_RES)  + " 100 KHZ ");
        WriteToDevice(ScpiCmdLookup.at(ScpiCmd::FORM_DATA) + " ASC");
        WriteToDevice(ScpiCmdLookup.at(ScpiCmd::FORM_BORD) + " NORM");
        WriteToDevice(ScpiCmdLookup.at(ScpiCmd::SWE_TIME)  + " AUTO");
        sleepMs(200);
        WriteToDevice(ScpiQueryCmdLookup.at(ScpiQueryCmd::SWE_TIME));
        oss << SendToDevice(ProLogixCmdLookup.at(ProLogixCmd::READ));

        WriteToDevice(ScpiCmdLookup.at(ScpiCmd::INIT_IMM));
        WriteToDevice(ScpiCmdLookup.at(ScpiCmd::WAI));
        sleepMs(300);

        std::string response;
        int i = 0;
        while ((response.substr(0, 1) != "1") && (i < 20))
        {
            WriteToDevice(ScpiCmdLookup.at(ScpiCmd::OPC));
            sleepMs(100);
            response = SendToDevice(ProLogixCmdLookup.at(ProLogixCmd::READ) + " eoi");
            i++;
        }

        WriteToDevice(ScpiQueryCmdLookup.at(ScpiQueryCmd::TRAC_DATA) + " TRACE1");
        WriteToDevice(ProLogixCmdLookup.at(ProLogixCmd::READ) + " eoi");
        sleepMs(100);
        std::string trace = ReadFromDevice();

        fsuMesurement::get_instance().seperateDataBlock(trace);

        wxFile file;
        if (file.Open("GpibScripts/Z_Log.txt", wxFile::write_append))
        {
            file.Write(trace);
            file.Close();
            oss << "Trace saved to file\n";
        }
        else
        {
            oss << "Error: Could not write trace to file\n";
        }

        std::cerr << "Received Trace: " << trace << std::endl;
        WriteToDevice(ScpiCmdLookup.at(ScpiCmd::INIT_CONT) + " ON");
    }
    else
    {
        oss << "Unknown test variant: " << args << "\n";
    }

    return oss.str();
}
