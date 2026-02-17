
#include "TerminalController.h"
#include "systemInfo.h"
#include "cmdGpib.h"
#include "mainHelper.h"

#include <iostream>
#include <sstream>

TerminalController::TerminalController()
    : m_outputCallback(nullptr)
{
    setupCommands();
}

TerminalController::~TerminalController()
{
}

void TerminalController::setOutputCallback(OutputCallback callback)
{
    m_outputCallback = callback;
}

void TerminalController::output(const std::string& text)
{
    if (m_outputCallback)
    {
        m_outputCallback(text);
    }
    std::cerr << text; // Also log to stderr
}

void TerminalController::setupCommands()
{
    m_cmds["scan"]        = [this](const std::string& args) { return this->scanDevices(args); };
    m_cmds["status"]      = [this](const std::string& args) { return this->statusDevice(args); };
    m_cmds["config"]      = [this](const std::string& args) { return this->configDevice(args); };
    m_cmds["connect"]     = [this](const std::string& args) { return this->connectDevice(args); };
    m_cmds["disconnect"]  = [this](const std::string& args) { return this->disconnectDevice(args); };
    m_cmds["send"]        = [this](const std::string& args) { return this->sendToDevice(args); };
    m_cmds["read"]        = [this](const std::string& args) { return this->readFromDevice(args); };
    m_cmds["write"]       = [this](const std::string& args) { return this->writeToDevice(args); };
    m_cmds["test"]        = [this](const std::string& args) { return this->testDevice(args); };
}

void TerminalController::processCommand(const std::string& input)
{
    std::string strCmd;
    std::string args;

    size_t firstSpace = input.find(' ');

    // Parse command and arguments
    if (firstSpace == std::string::npos)
    {
        strCmd = input;
        args = "";
    }
    else
    {
        strCmd = input.substr(0, firstSpace);
        args = input.substr(firstSpace + 1);
    }

    std::cerr << "Processing command: " << strCmd << " with args: " << args << std::endl;

    // Find and execute command
    auto match = m_cmds.find(strCmd);

    if (match != m_cmds.end())
    {
        std::string result = match->second(args);
        output(result);
    }
    else
    {
        output("Unknown command!\n");
    }
}

std::string TerminalController::scanDevices(const std::string& args)
{
    DWORD devNum = scanUsbDev();
    std::ostringstream oss;
    oss << "Number of devices: " << devNum << "\n";
    return oss.str();
}

std::string TerminalController::statusDevice(const std::string& args)
{
    return Global::AdapterInstance.statusText() + "\n";
}

std::string TerminalController::connectDevice(const std::string& args)
{
    int dev = 0;
    std::ostringstream oss;

    std::cerr << "Command: connect with args: " << args << std::endl;

    if (args != "")
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

    if (!Global::AdapterInstance.getConnected())
    {
        Global::AdapterInstance.connect();

        if (Global::AdapterInstance.getStatus() == FT_OK)
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

        FT_STATUS ftStatus = FT_Purge(Global::AdapterInstance.getHandle(), FT_PURGE_RX | FT_PURGE_TX);
        if (ftStatus != FT_OK)
        {
            oss << "Purge Failed\n";
        }
    }
    else
    {
        oss << "Device already connected\n";
    }

    return oss.str();
}

std::string TerminalController::disconnectDevice(const std::string& args)
{
    std::ostringstream oss;

    std::cerr << "Command: disconnect with args: " << args << std::endl;

    Global::AdapterInstance.disconnect();

    if (Global::AdapterInstance.getStatus() == FT_OK)
    {
        oss << "Disconnected from device\n";
        std::cerr << "Disconnected from device" << std::endl;
    }
    else
    {
        oss << "Couldn't disconnect\nCheck if a device is connected with: status\n";
    }

    return oss.str();
}

std::string TerminalController::sendToDevice(const std::string& args)
{
    std::ostringstream oss;

    std::cerr << "Command: send " << args << std::endl;

    wxString gpibCmd(args);
    std::string checkText(gpibCmd.ToUTF8());

    wxString writeResult = Global::AdapterInstance.write(checkText);
    oss << writeResult.ToStdString();

    sleepMs(100); // Wait for response

    std::cerr << "Reading from device..." << std::endl;

    wxString readResult = Global::AdapterInstance.read();
    oss << readResult.ToStdString();

    return oss.str();
}

std::string TerminalController::readFromDevice(const std::string& args)
{
    std::ostringstream oss;

    std::cerr << "Command: read with args: " << args << std::endl;

    wxString readResult = Global::AdapterInstance.read();
    oss << readResult.ToStdString();

    return oss.str();
}

std::string TerminalController::writeToDevice(const std::string& args)
{
    std::ostringstream oss;

    std::cerr << "Command: write with args: " << args << std::endl;

    wxString writeResult = Global::AdapterInstance.write(args);
    oss << writeResult.ToStdString();

    return oss.str();
}

std::string TerminalController::configDevice(const std::string& args)
{
    std::ostringstream oss;

    if (args != "")
    {
        try
        {
            int baudRate = std::stoi(args);
            if (baudRate == std::clamp(baudRate, 1, 1'000'000))
            {
                Global::AdapterInstance.setBaudrate(baudRate);
                std::cerr << "Set Baudrate to " << Global::AdapterInstance.getBaudrate() << std::endl;
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
        std::cerr << "Using default baudrate: " << Global::AdapterInstance.getBaudrate() << std::endl;
    }

    Global::AdapterInstance.config();

    if (Global::AdapterInstance.getStatus() == FT_OK)
    {
        oss << "Device configured with baudrate: " << Global::AdapterInstance.getBaudrate() << "\n";
        std::cerr << "Baudrate set to: " << Global::AdapterInstance.getBaudrate() << std::endl;
    }
    else
    {
        oss << "Failed to configure device. Run as admin?\n";
    }

    return oss.str();
}

std::string TerminalController::testDevice(const std::string& args)
{
    std::ostringstream oss;

    if (args == "")
    {
        // Basic initialization test
        oss << connectDevice() << configDevice();

        writeToDevice(ProLogixCmdLookup.at(ProLogixCmd::CLR));
        sleepMs(200);
        writeToDevice(ProLogixCmdLookup.at(ProLogixCmd::MODE) + " 1");
        writeToDevice(ProLogixCmdLookup.at(ProLogixCmd::AUTO) + " 1");
        writeToDevice(ProLogixCmdLookup.at(ProLogixCmd::EOS) + " 2");
        writeToDevice(ProLogixCmdLookup.at(ProLogixCmd::EOI) + " 1");
        writeToDevice(ProLogixCmdLookup.at(ProLogixCmd::EOT_ENABLE) + " 0");
        writeToDevice(ProLogixCmdLookup.at(ProLogixCmd::EOT_CHAR) + " 10");
        writeToDevice(ProLogixCmdLookup.at(ProLogixCmd::ADDR) + " 20");
        oss << sendToDevice(ProLogixCmdLookup.at(ProLogixCmd::VER));
    }
    else if (args == "1")
    {
        // Test variant 1
        writeToDevice(ProLogixCmdLookup.at(ProLogixCmd::RST));
        sleepMs(200);

        writeToDevice(ProLogixCmdLookup.at(ProLogixCmd::MODE) + " 1");
        writeToDevice(ProLogixCmdLookup.at(ProLogixCmd::AUTO) + " 0");
        writeToDevice(ProLogixCmdLookup.at(ProLogixCmd::ADDR) + " 20");

        writeToDevice(ScpiCmdLookup.at(ScpiCmd::IDN));
        writeToDevice(ProLogixCmdLookup.at(ProLogixCmd::READ) + " eoi");
        sleepMs(50);
        oss << readFromDevice();
    }
    else if (args == "mess")
    {
        // Measurement test
        writeToDevice(ProLogixCmdLookup.at(ProLogixCmd::AUTO) + " 0");

        writeToDevice("RST");
        writeToDevice(ScpiCmdLookup.at(ScpiCmd::CLR));
        writeToDevice(ScpiCmdLookup.at(ScpiCmd::FREQ_CENT) + " 1 GHZ");
        writeToDevice(ScpiCmdLookup.at(ScpiCmd::FREQ_SPAN) + " 10 MHZ");
        writeToDevice(ScpiCmdLookup.at(ScpiCmd::BAND_RES) + " 100 KHZ ");
        writeToDevice(ScpiCmdLookup.at(ScpiCmd::CALC_MARK_MAX));
        writeToDevice(ScpiCmdLookup.at(ScpiCmd::WAI));

        writeToDevice(ProLogixCmdLookup.at(ProLogixCmd::AUTO) + " 1");
        sleepMs(200);
        oss << sendToDevice(ScpiCmdLookup.at(ScpiCmd::CALC_MARK1_Y));
        oss << sendToDevice(ScpiCmdLookup.at(ScpiCmd::CALC_MARK1_X));
    }
    else if (args == "big")
    {
        // Large trace measurement
        writeToDevice(ProLogixCmdLookup.at(ProLogixCmd::AUTO) + " 0");
        writeToDevice(ScpiCmdLookup.at(ScpiCmd::INIT_CONT) + " OFF");
        writeToDevice(ScpiCmdLookup.at(ScpiCmd::SWE_POIN) + " 10");
        writeToDevice(ScpiCmdLookup.at(ScpiCmd::FREQ_STAR) + " 80 MHZ");
        writeToDevice(ScpiCmdLookup.at(ScpiCmd::FREQ_STOP) + " 120 MHZ");
        writeToDevice(ScpiCmdLookup.at(ScpiCmd::BAND_RES) + " 100 KHZ ");

        writeToDevice(ScpiCmdLookup.at(ScpiCmd::FORM_DATA) + " ASC");
        writeToDevice(ScpiCmdLookup.at(ScpiCmd::FORM_BORD) + " NORM");
        writeToDevice(ScpiCmdLookup.at(ScpiCmd::SWE_TIME) + " AUTO");
        sleepMs(200);
        writeToDevice(ScpiCmdLookup.at(ScpiCmd::SWE_TIME_QUERY));
        oss << sendToDevice(ProLogixCmdLookup.at(ProLogixCmd::READ));

        writeToDevice(ScpiCmdLookup.at(ScpiCmd::INIT_IMM));
        writeToDevice(ScpiCmdLookup.at(ScpiCmd::WAI));
        sleepMs(300);

        std::string responce;
        int i = 0;
        while ((responce.substr(0, 1) != "1") && (i < 20))
        {
            writeToDevice(ScpiCmdLookup.at(ScpiCmd::OPC));
            sleepMs(100);
            responce = sendToDevice(ProLogixCmdLookup.at(ProLogixCmd::READ) + " eoi");
            i++;
        }

        writeToDevice(ScpiCmdLookup.at(ScpiCmd::TRAC_DATA) + " TRACE1");
        writeToDevice(ProLogixCmdLookup.at(ProLogixCmd::READ) + " eoi");

        sleepMs(100);
        std::string trace = readFromDevice();

        Global::Messung.seperateDataBlock(trace);

        wxFile file;
        if (file.Open("/home/jon3r/Documents/Code/CodeBlocks/Test_GPIB_Terminal/GpibScripts/Z_Log.txt", wxFile::write_append))
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

        writeToDevice(ScpiCmdLookup.at(ScpiCmd::INIT_CONT) + " ON");
    }
    else
    {
        oss << "Unknown test variant: " << args << "\n";
    }

    return oss.str();
}
