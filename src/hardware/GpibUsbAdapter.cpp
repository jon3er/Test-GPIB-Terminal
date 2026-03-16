#include "GpibUsbAdapter.h"
#include "FsuMeasurement.h"
#include "mainHelper.h"


//TODO Create Device Class and Create new read read and write funtions
PrologixUsbGpibAdapter::PrologixUsbGpibAdapter()
{

}

PrologixUsbGpibAdapter::~PrologixUsbGpibAdapter()
{
    if (m_deviceInfo.Connected)
    {
        disconnect();
    }
}

std::string PrologixUsbGpibAdapter::read(unsigned int forceReadBytes)
{
    std::string Text;

    if (m_deviceInfo.Connected)
    {
        std::vector<char> BigBuffer;
        DWORD BufferSize;
        FT_STATUS ftStatus;

        std::cerr << "Reading from Device..." << std::endl;

        ftStatus = readUsbDev(m_deviceInfo.ftHandle, BigBuffer, BufferSize, forceReadBytes);

        if (ftStatus == FT_OK)
        {
            m_deviceInfo.lastMsgReceived = std::string(BigBuffer.data(),BigBuffer.size());
            Text = m_deviceInfo.lastMsgReceived + "\n";

            std::cout << "Read msg: "<< Text << std::endl;

            if (BigBuffer.size() == 0)
            {
                Text = "Failed to Receive Data - No Message to Read\n";
            }
        }
        else
        {
            Text = "Failed to Receive Data - TimeOut after 5s\n";
        }
    }
    else
    {
        std::cerr << "No Device to send too" << std::endl;
        Text = "Failed to Connect to a Device\n";
        m_deviceInfo.Connected = false;
    }

    return Text;
}

std::string PrologixUsbGpibAdapter::write(std::string msg)
{
    std::string Text;

    std::cout << "Write Command Entered" << std::endl;

    if (m_deviceInfo.Connected)
    {
        DWORD bytesWritten;
        wxString GPIBText = msg;
        std::string CheckText(GPIBText.ToUTF8());
        //Check String if Adapter or GPIB Command and check for ASCII 10, 13, 27, 43
        std::vector<char> charArrWriteGpib = checkAscii(CheckText);

        std::cout << "Trying to write to Device... " << std::string(charArrWriteGpib.begin(),charArrWriteGpib.end()) << std::endl;

        FT_STATUS ftStatus =writeUsbDev(m_deviceInfo.ftHandle, charArrWriteGpib, bytesWritten);

        if (ftStatus == FT_OK)
        {
            Text = GPIBText;
            Text = "Msg sent: " + Text + " ; " + std::to_string(bytesWritten) + " Bytes Written to GPIB Device\n";
        }
        else
        {
            Text = "Failed to send Data\n";
        }
    }
    else
    {
        std::cerr << "No Connection" << std::endl;
        Text = "Failed to Connect\n";
    }
    return Text;
}

std::string PrologixUsbGpibAdapter::send(std::string msg, int DelayMs)
{
    write(msg);
    int i = 0;
    if (!(msg.substr(0,2) == "++") && (msg.find("?") != std::string::npos))
        write("++read eoi");    // for reliable responce even when set to ++auto 0
    sleepMs(5);

    while ((quaryBuffer() < 1)&& (i < DelayMs))
    {
        sleepMs(5);
        i= i + 5;
        if (i % 100 == 0)
        {
            std::cerr << "Waiting for response... " << i << " ms elapsed" << std::endl;
        }
    }

    std::cout << "send Responce Time: " << 5 + i << " ms" << std::endl;

    return read();
}


std::string PrologixUsbGpibAdapter::sendForceDelay(std::string msg, int DelayMs)
{
    write(msg);

    sleepMs(DelayMs);
    std::cout << "write / read Time Delay forced: " << DelayMs << " ms" << std::endl;

    return read();
}

bool PrologixUsbGpibAdapter::checkIfMsgAvailable(int TimeOutMs)
{
    int elapsedMs = 0;
    int pollIntervalMs = 50; // Abfrage-Intervall (verhindert 100% CPU-Last)
    bool msgReceived = false;

    while (elapsedMs < TimeOutMs) {
        // Status-Byte abfragen
        write("++spoll");

        sleepMs(20);

        if(quaryBuffer() > 0)
        {
            std::string statusStr = read();

            try {
                int statusByte = std::stoi(statusStr);

                // check MAV-Bit (Message Available, Bit 4)
                if (statusByte & 0x10) {
                    msgReceived = true;
                    std::cout << "[GPIB BUS] "<<"Msg available" << std::endl;
                    break; // Msg available
                }
                else if (statusByte & 0x04) // Bit 2 Error Available
                {
                    std::string errorMsg = send("SYST:ERR?",200);

                    std::cout << "[GPIB BUS ERROR] "<< errorMsg << std::endl;
                    return msgReceived = false;
                }
            } catch (...) {

            }
        }
        sleepMs(pollIntervalMs-20);
        elapsedMs += (pollIntervalMs);
    }

    std::cerr << "[GPIB BUS ERROR] Timed out after " << elapsedMs << "ms waiting for message." << std::endl;

    return msgReceived;
}

DWORD PrologixUsbGpibAdapter::quaryBuffer()
{
    //Get Number of bytes to read from receive queue
    m_deviceInfo.ftStatus = FT_GetQueueStatus(m_deviceInfo.ftHandle,&m_deviceInfo.BytesToRead);
    std::cerr << "Bytes in Queue: " << m_deviceInfo.BytesToRead << std::endl;
    printErrD2XX(m_deviceInfo.ftStatus,"Failed to Get Queue Status");

    return m_deviceInfo.BytesToRead;
}

bool PrologixUsbGpibAdapter::checkIfAdapterAvailable()
{
    DWORD numDevs;
    FT_CreateDeviceInfoList(&numDevs);
    return (numDevs > 0);
}

bool PrologixUsbGpibAdapter::checkIfGpibDeviceAvailable()
{

    if (!checkIfAdapterAvailable())
    {
        return false;
    }

    if (!connect())
    {
        return false;
    }
    else
    {
        write("++spoll");
        sleepMs(20);
        std::string statusStr = read();
        int spollStatus;

        try
        {
            spollStatus = std::stoi(statusStr);
        }
        catch(...)
        {
            std::cout << "failed to convert String" << std::endl;
            return false;
        }


        if ((0x00 <= spollStatus) && (spollStatus<= 0xff))
        {
            std::cout << "GPIB Devcie found!" << std::endl;
            std::cout << "++spoll Status: " << statusStr << std::endl;
            return true;
        }
        else
        {
            std::cout << "Failed to find GPIB Devcie!" <<std::endl;
            std::cout << "++spoll Status: " << statusStr << std::endl;
            return false;
        }
    }
}

bool PrologixUsbGpibAdapter::connect()
{
    if (!getConnected())
    {
        m_deviceInfo.ftStatus = FT_Open(m_deviceInfo.numDev,&m_deviceInfo.ftHandle);
        printErrD2XX(m_deviceInfo.ftStatus,"Failed to Connect");
        write("SYST:DISP:UDP ON"); //Turn on monitor

        if (m_deviceInfo.ftStatus == FT_OK)
        {
            std::cerr << "Connected to " << m_deviceInfo.numDev << std::endl;
            m_deviceInfo.Connected = true;
            return true;
        }
        else
        {
            m_deviceInfo.Connected = false;
            return false;
        }
    }
    else
    {
        return true;
    }
}
bool PrologixUsbGpibAdapter::disconnect()
{
    write(ProLogixCmdLookup.at(ProLogixCmd::AUTO) + " 0");
    write(ScpiCmdLookup.at(ScpiCmd::CLR));
    write(ProLogixCmdLookup.at(ProLogixCmd::LOC));
    write(ProLogixCmdLookup.at(ProLogixCmd::IFC));

    sleepMs(200); // TODO Prüfen wieso


    m_deviceInfo.ftStatus = FT_Close(m_deviceInfo.ftHandle);
    printErrD2XX(m_deviceInfo.ftStatus,"Failed to Disconnect");
    if (m_deviceInfo.ftStatus == FT_OK)
    {
        std::cerr << "Connected to " << m_deviceInfo.numDev << std::endl;
        m_deviceInfo.Connected = false;
        return true;
    }
    else
    {
        return false;
    }
}
void PrologixUsbGpibAdapter::config()
{
    UCHAR LatencyTimer = 16;

    //set Baudrate
    m_deviceInfo.ftStatus = FT_SetBaudRate(m_deviceInfo.ftHandle,m_deviceInfo.BaudRate);
    printErrD2XX(m_deviceInfo.ftStatus,"Failed to set Baudrate");

    m_deviceInfo.ftStatus = FT_SetDataCharacteristics(m_deviceInfo.ftHandle, FT_BITS_8, FT_STOP_BITS_1, FT_PARITY_NONE);
    printErrD2XX(m_deviceInfo.ftStatus,"Failed to set Data Characteristics");

    m_deviceInfo.ftStatus = FT_SetFlowControl(m_deviceInfo.ftHandle, FT_FLOW_NONE, 0, 0);
    printErrD2XX(m_deviceInfo.ftStatus,"Failed to set flow Characteristics");

    m_deviceInfo.ftStatus =  FT_SetTimeouts(m_deviceInfo.ftHandle, 5000,5000);
    printErrD2XX(m_deviceInfo.ftStatus,"Failed to set TimeOut");

    m_deviceInfo.ftStatus = FT_SetLatencyTimer(m_deviceInfo.ftHandle, LatencyTimer);

    std::cerr << "FT-Config complete" << std::endl;

    // setup Adapter settings
    write(ProLogixCmdLookup.at(ProLogixCmd::CLR));
    //sleepMs(200);
    write(ProLogixCmdLookup.at(ProLogixCmd::MODE)       + " 1");
    write(ProLogixCmdLookup.at(ProLogixCmd::AUTO)       + " 1");
    write(ProLogixCmdLookup.at(ProLogixCmd::EOS)        + " 2");
    write(ProLogixCmdLookup.at(ProLogixCmd::EOI)        + " 1");
    write(ProLogixCmdLookup.at(ProLogixCmd::EOT_ENABLE) + " 0");
    write(ProLogixCmdLookup.at(ProLogixCmd::EOT_CHAR)   + " 10");
    write(ProLogixCmdLookup.at(ProLogixCmd::ADDR)       + " 20");
    write("SYST:DISP:UPD ON"); // turn Display on
    std::string responce = send("syst:err?", 1000);

    std::cout << "Config fin - status: " << responce << std::endl;


    if (m_deviceInfo.ftStatus == FT_OK)
    {
        m_deviceInfo.configFin = true;
    }
    else
    {
        m_deviceInfo.configFin = false;
    }
}
void PrologixUsbGpibAdapter::readScriptFile(const wxString& dirPath, const wxString& fileName, wxArrayString* logAdapterReceived, const std::atomic<bool>* stopFlag)
{
    wxTextFile textFile;

    if (textFile.Open(dirPath + fileName))
    {
        if (!getConnected())
        {
            connect();
            config();
        }

        for (size_t i = 0; i < textFile.GetLineCount(); i++)
        {
            // Check for stop signal from calling thread
            if (stopFlag && *stopFlag)
            {
                std::cout << "[GPIB] Measurement stopped by user" << std::endl;
                break;
            }

            wxString line = textFile.GetLine(i);
            if(line.IsEmpty())
            {
                std::cerr << "line " << i << ": Empty" << std::endl;
            }
            else if (line.substr(0,1) == "#")
            {
                std::cerr << "line " << i << ": Kommentar: " << line.substr(1) << std::endl;
            }
            else if (line.substr(0,5) == "wait ")
            {
                int wait;
                wxString strWait = line.substr(5);
                if (strWait.ToInt(&wait))
                {
                    std::cerr << "wait for " << wait << "ms" << std::endl;
                    sleepMs(wait);
                }
                else
                {
                    std::cerr << "Invalid wait Time input: " << strWait << std::endl;
                }
            }
            else if (line.substr(0,5) == "send ")
            {
                std::cerr << "line " << i << ": manuell send: " << line << std::endl;
                line = line.substr(5);
                logAdapterReceived->Add(send(std::string(line.ToUTF8())));
                std::cerr << "responce: " << logAdapterReceived->Last() << std::endl;
            }
            else if (line.substr(0,6) == "write ")
            {
                std::cerr << "line " << i << ": manuell write: " << line << std::endl;
                line = line.substr(6);
                write(std::string(line.ToUTF8()));
            }
            else if (line.substr(0,4) == "read")
            {
                std::cerr << "line " << i << ": manuell read" << std::endl;
                logAdapterReceived->Add(read());
                std::cerr << "responce: " << logAdapterReceived->Last() << std::endl;
            }
            else if(line.Contains("?") && line.substr(0,4) == "TRAC")
            {
                std::vector<double> bufferReal;
                std::vector<double> bufferImag;
                std::cerr << "line " << i << ": send: " << line << std::endl;

                write(std::string(line.ToUTF8()));
                write(ProLogixCmdLookup.at(ProLogixCmd::READ) + " eoi");
                sleepMs(300); // TODO change logic to be more efficent
                logAdapterReceived->Add(read());
                std::cerr << "responce: " << logAdapterReceived->Last() << std::endl;
                fsuMeasurement::get_instance().seperateDataBlock(logAdapterReceived->Last(), bufferReal, bufferImag);
                fsuMeasurement::get_instance().setX_Data(bufferReal);
                fsuMeasurement::get_instance().setY_Data(bufferImag);
                //Messung.calcYdata(); //start und end frequenz angeben

            }
            else if(line.Contains("?"))
            {
                std::cerr << "line " << i << ": send: " << line << std::endl;
                logAdapterReceived->Add(send(std::string(line.ToUTF8())));
                std::cerr << "responce: " << logAdapterReceived->Last() << std::endl;
            }
            else
            {
                std::cerr << "line " << i << ": write: " << line << std::endl;
                write(std::string(line.ToUTF8()));
            }
        }
    }
}

std::string PrologixUsbGpibAdapter::statusText()
{
    std::string Text;

    if (m_deviceInfo.Connected)
    {
        Text = "Connected to a Device \nCurrent Status: " + send("++spoll",300);
    }
    else
    {
        Text = "Not connected to a device";
    }
    if (m_deviceInfo.Connected && m_deviceInfo.configFin)
    {
        Text = Text + " and ";
    }
    if (m_deviceInfo.configFin)
    {
        Text = Text + "Device config set Baudrate to: " + std::to_string(m_deviceInfo.BaudRate);
    }

    Text += "\nLast Status code:" + wxString(statusString(m_deviceInfo.ftStatus)) + "\n";

    return Text;
}

bool PrologixUsbGpibAdapter::resetGpibBusBuffer()
{
    std::string responce = "1";
    int i = 0;

    if (m_deviceInfo.Connected)
    {
        write("++clr");

        sleepMs(20);

        if (quaryBuffer() > 0)
        {
            std::cout << "Old Buffered Msg: "<< read() << std::endl;
        }

        while (!(responce.substr(0,2) == "0,") && (i < 10))
        {
            responce = send("SYST:ERR?", 300);
            i++;
        }

        if (i >= 10)
        {
            std::cerr << "Failed to clear GPIB Bus Buffer" << std::endl;
            return false;
        }
        
        std::cout << "GPIB Bus Buffer cleared" << std::endl;
        
        return true;
    }
    return false;
}

FT_STATUS PrologixUsbGpibAdapter::getStatus()
{
    return m_deviceInfo.ftStatus;
}

FT_HANDLE PrologixUsbGpibAdapter::getHandle()
{
    return m_deviceInfo.ftHandle;
}

bool PrologixUsbGpibAdapter::getConnected()
{
    return m_deviceInfo.Connected;
}

std::string PrologixUsbGpibAdapter::getLastMsgReseived()
{
    return m_deviceInfo.lastMsgReceived;
}

int PrologixUsbGpibAdapter::getBaudrate()
{
    return m_deviceInfo.BaudRate;
}

void PrologixUsbGpibAdapter::setBaudrate(int BaudrateNew)
{
    m_deviceInfo.BaudRate = BaudrateNew;
}

std::vector<char> PrologixUsbGpibAdapter::checkAscii(std::string input)
{
    const char* charInput = input.c_str();
    int DataSize = strlen(input.c_str());

    std::cerr << "Length in function:" << std::endl;
    std::cerr << std::to_string(DataSize) << std::endl;

    char* charInputBuffer = new char[input.length()+1];
    strcpy(charInputBuffer, charInput);
    //allocate output for max possible length
    char* charOutputBuffer = new char[input.length()*2 + 2];

    if (input.substr(0,2) == "++")  //Adapter Command
    {
        strcpy(charOutputBuffer,charInputBuffer);
        size_t BufferSize = strlen(charInputBuffer);
        charOutputBuffer[BufferSize] = '\n';
        charOutputBuffer[BufferSize + 1] = '\0';
        std::vector<char> vCharOutputAdptr(charOutputBuffer,charOutputBuffer + strlen(charOutputBuffer));

        std::cerr << "Adapter Command: " << std::string(vCharOutputAdptr.begin(),vCharOutputAdptr.end()) << std::endl;

        delete[] charInputBuffer;
        delete[] charOutputBuffer;
        //return adapter command
        return vCharOutputAdptr;
    }

    //for debuging
    wxString OgString;
    wxString ModString;

    int j = 0;

    for (int i=0;i < DataSize;i++)
    {
        switch(charInputBuffer[i])
        {
            case 10:
            case 13:
            case 27:
            case 43:
                charOutputBuffer[j] = {27};
                ModString = ModString + std::to_string(charOutputBuffer[j]) + " ";
                j++;

                break;
            default:

                break;
        }
        charOutputBuffer[j]= charInputBuffer[i];

        OgString = OgString + std::to_string(charInputBuffer[i]) + " ";
        ModString = ModString + std::to_string(charOutputBuffer[j]) + " ";

        j++;
    }
    //Add LF to end "nicht notwendig wenn ++eos 2 und eigenglich müsste ascii 27 angehängt werden"
    charOutputBuffer[j] = '\n';
    charOutputBuffer[j+1] = '\0';
    ModString = ModString + std::to_string(charOutputBuffer[j]) + " ";
    ModString = ModString + std::to_string(charOutputBuffer[j + 1]);

    // std::cerr << "Input str in ascii: " << OgString << std::endl;
    // std::cerr << "Output str in ascii: " << ModString << std::endl;

    std::vector<char> vCharOutputGpib(charOutputBuffer,charOutputBuffer + strlen(charOutputBuffer));

    delete[] charInputBuffer;
    delete[] charOutputBuffer;

    return vCharOutputGpib;
}

void PrologixUsbGpibAdapter::prepareFTDIDevice() {
    std::cout << "Prüfe auf aktive VCP-Treiber (ftdi_sio)..." << std::endl;

    // check if module is loaded (lsmod | grep)
    // 0 means found
    if (std::system("lsmod | grep ftdi_sio > /dev/null") == 0) {
        std::cout << "Treiber ftdi_sio gefunden. Versuche zu entladen..." << std::endl;

        // unload module
        int res1 = std::system("sudo rmmod ftdi_sio");
        int res2 = std::system("sudo rmmod usbserial");

        if (res1 == 0) {
            std::cout << "VCP-Treiber erfolgreich entladen." << std::endl;
        } else {
            std::cerr << "Fehler beim Entladen. Wurde das Programm als SU gestartet?" << std::endl;
        }

        if (res2 == 0) {
            std::cout << "USBserial-Treiber erfolgreich entladen." << std::endl;
        } else {
            std::cerr << "Fehler beim Entladen. Wurde das Programm als SU gestartet?" << std::endl;
        }

    } else {
        std::cout << "Keine blockierenden Treiber aktiv. D2XX Zugriff bereit." << std::endl;
    }
}

void PrologixUsbGpibAdapter::checkForGpibBusError(wxWindow* parent)
{
    if (!getConnected()) return;
    // Check if SRQ-Line (Service Request) is activ

    std::string srqStatus = send("++srq");

    //  "1" signals Service Request
    if (srqStatus.find("1") != std::string::npos) {

        // Serial Poll  (R&S Analysator)
        std::string statusStr = send("++spoll");
        try {
            int statusByte = std::stoi(statusStr);

            // Event Status Bit (Bit 5 / Dec 32)
            if (statusByte & (1 << 5)) {

                // check error msg of R&S device
                std::string errorMsg = send("SYST:ERR?");

                // 5. wxWidgets warning window popup
                wxMessageBox(
                    wxString::Format("Spektrumanalysator meldet einen Fehler:\n%s", errorMsg),
                    "Gerätefehler",
                    wxOK | wxICON_WARNING | wxCENTRE,
                    parent
                );
            }
        } catch (const std::exception& e) {
            std::cerr << "Fehler beim Auslesen des Status-Bytes: " << e.what() << std::endl;
            wxMessageBox("Fehler beim Auslesen des Status-Bytes.", "Kommunikationsfehler", wxOK | wxICON_ERROR, parent);
        }
    }
}
