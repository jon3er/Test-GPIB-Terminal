
#include "fkt_GPIB.h"
#include "cmdGpib.h"
#include "mainHelper.h"

//TODO Create Device Class and Create new read read and write funtions
PrologixUsbGpibAdapter::PrologixUsbGpibAdapter()
{

}

PrologixUsbGpibAdapter::~PrologixUsbGpibAdapter()
{
    if (m_Connected)
    {
        disconnect();
    }
}

std::string PrologixUsbGpibAdapter::read(int forceReadBytes)
{
    std::string Text;

    if (m_Connected)
    {
        std::vector<char> BigBuffer;
        DWORD BufferSize;
        FT_STATUS ftStatus;

        std::cerr << "Reading from Device..." << std::endl;

        ftStatus = readUsbDev(m_ftHandle, BigBuffer, BufferSize, forceReadBytes);

        if (ftStatus == FT_OK)
        {
            m_lastMsgReceived = std::string(BigBuffer.data(),BigBuffer.size());
            Text = m_lastMsgReceived + "\n";

            if (BigBuffer.size() == 0)
            {
                Text = "No Message to Read\n";
            }
        }
        else
        {
            Text = "Failed to Receive Data - TimeOut after 5s\n";
            m_Connected = false;
        }
    }
    else
    {
        std::cerr << "No Device to send too" << std::endl;
        Text = "Failed to Connect to a Device\n";
        m_Connected = false;
    }

    return Text;
}

std::string PrologixUsbGpibAdapter::write(std::string msg)
{
    std::string Text;

    std::cerr << "Write Command Entered" << std::endl;

    if (m_Connected)
    {
        DWORD bytesWritten;
        wxString GPIBText = msg;
        std::string CheckText(GPIBText.ToUTF8());
        //Check String if Adapter or GPIB Command and check for ASCII 10, 13, 27, 43
        std::vector<char> charArrWriteGpib = checkAscii(CheckText);

        std::cerr << "Trying to write to Device... " << std::string(charArrWriteGpib.begin(),charArrWriteGpib.end()) << std::endl;

        FT_STATUS ftStatus =writeUsbDev(m_ftHandle, charArrWriteGpib, bytesWritten);

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
    sleepMs(DelayMs);
    quaryBuffer();

    return read();
}

DWORD PrologixUsbGpibAdapter::quaryBuffer()
{
    //Get Number of bytes to read from receive queue
    m_ftStatus = FT_GetQueueStatus(m_ftHandle,&m_BytesToRead);
    std::cerr << "Bytes in Queue: " << m_BytesToRead << std::endl;
    printErr(m_ftStatus,"Failed to Get Queue Status");

    return m_BytesToRead;
}

void PrologixUsbGpibAdapter::connect(std::string args)
{
    if (!m_Connected)
    {
        m_ftStatus = FT_Open(m_numDev,&m_ftHandle);
        printErr(m_ftStatus,"Failed to Connect");
        write("SYST:DISP:UDP ON"); //Turn on monitor

        if (m_ftStatus == FT_OK)
        {
            std::cerr << "Connected to " << m_numDev << std::endl;
            m_Connected = true;
        }
    }
}
void PrologixUsbGpibAdapter::disconnect(std::string args)
{
    write(ProLogixCmdLookup.at(ProLogixCmd::AUTO) + " 0");
    write(ScpiCmdLookup.at(ScpiCmd::CLR));
    write(ProLogixCmdLookup.at(ProLogixCmd::LOC));
    write(ProLogixCmdLookup.at(ProLogixCmd::IFC));

    sleepMs(200);


    m_ftStatus = FT_Close(m_ftHandle);
    printErr(m_ftStatus,"Failed to Disconnect");
    if (m_ftStatus == FT_OK)
    {
        std::cerr << "Connected to " << m_numDev << std::endl;
        m_Connected = false;
    }
}
void PrologixUsbGpibAdapter::config()
{
    //set Baudrate
    m_ftStatus = FT_SetBaudRate(m_ftHandle,m_BaudRate);
    printErr(m_ftStatus,"Failed to set Baudrate");

    m_ftStatus = FT_SetDataCharacteristics(m_ftHandle, FT_BITS_8, FT_STOP_BITS_1, FT_PARITY_NONE);
    printErr(m_ftStatus,"Failed to set Data Characteristics");

    m_ftStatus = FT_SetFlowControl(m_ftHandle, FT_FLOW_NONE, 0, 0);
    printErr(m_ftStatus,"Failed to set flow Characteristics");

    m_ftStatus =  FT_SetTimeouts(m_ftHandle, 500,500);
    printErr(m_ftStatus,"Failed to set TimeOut");

    std::cerr << "FT-Config complete" << std::endl;

    if (m_ftStatus == FT_OK)
    {
        m_configFin = true;
    }
    else
    {
        m_configFin = false;
    }
}
void PrologixUsbGpibAdapter::readScriptFile(const wxString& dirPath, const wxString& file, wxArrayString& logAdapterReceived, const std::atomic<bool>* stopFlag)
{
    wxTextFile textFile;

    if (textFile.Open(dirPath + file))
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
                logAdapterReceived.Add(send(std::string(line.ToUTF8())));
                std::cerr << "responce: " << logAdapterReceived.Last() << std::endl;
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
                logAdapterReceived.Add(read());
                std::cerr << "responce: " << logAdapterReceived.Last() << std::endl;
            }
            else if(line.Contains("?") && line.substr(0,4) == "TRAC")
            {
                std::cerr << "line " << i << ": send: " << line << std::endl;

                write(std::string(line.ToUTF8()));
                write(ProLogixCmdLookup.at(ProLogixCmd::READ) + " eoi");
                sleepMs(300);
                logAdapterReceived.Add(read());
                std::cerr << "responce: " << logAdapterReceived.Last() << std::endl;
                fsuMesurement::get_instance().seperateDataBlock(logAdapterReceived.Last());
                fsuMesurement::get_instance().setFreqStartEnd(75'000'000,125'000'000);
                //Messung.calcYdata(); //start und end frequenz angeben

            }
            else if(line.Contains("?"))
            {
                std::cerr << "line " << i << ": send: " << line << std::endl;
                logAdapterReceived.Add(send(std::string(line.ToUTF8())));
                std::cerr << "responce: " << logAdapterReceived.Last() << std::endl;
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

    if (m_Connected)
    {
        Text = "Connected to a Device ";
    }
    else
    {
        Text = "Not connected to a device";
    }
    if (m_Connected && m_configFin)
    {
        Text = Text + " and ";
    }
    if (m_configFin)
    {
        Text = Text + "Device config set Baudrate to: " + std::to_string(m_BaudRate);
    }

    Text += "\nLast Status code:" + wxString(statusString(m_ftStatus)) + "\n";

    return Text;
}

FT_STATUS PrologixUsbGpibAdapter::getStatus()
{
    return m_ftStatus;
}

FT_HANDLE PrologixUsbGpibAdapter::getHandle()
{
    return m_ftHandle;
}

bool PrologixUsbGpibAdapter::getConnected()
{
    return m_Connected;
}

std::string PrologixUsbGpibAdapter::getLastMsgReseived()
{
    return m_lastMsgReceived;
}

int PrologixUsbGpibAdapter::getBaudrate()
{
    return m_BaudRate;
}

void PrologixUsbGpibAdapter::setBaudrate(int BaudrateNew)
{
    m_BaudRate = BaudrateNew;
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

    std::cerr << "Input str in ascii: " << OgString << std::endl;
    std::cerr << "Output str in ascii: " << ModString << std::endl;

    std::vector<char> vCharOutputGpib(charOutputBuffer,charOutputBuffer + strlen(charOutputBuffer));

    delete[] charInputBuffer;
    delete[] charOutputBuffer;

    return vCharOutputGpib;
}

//------fsuMesurement Beginn-----

fsuMesurement::fsuMesurement()
{
    /*
    m_x_Data = {0};
    m_y_Data = {0};

    m_FreqStart = 75'000'000;
    m_FreqEnd = 125'000'000;

    m_NoPoints_x = 0;
    m_NoPoints_y = 0;
    */
}

void fsuMesurement::seperateDataBlock(const wxString& receivedString)
{
    wxArrayString seperatedStrings = wxStringTokenize(receivedString, ",");

    double value;
    wxString data;
    std::vector<double> x;

    for (long unsigned int i = 0; i < seperatedStrings.Count(); i++)
    {
        data = seperatedStrings[i];

        if(data.ToCDouble(&value))
        {
             x.push_back(value);
             std::cerr << "seperated value: " << value << std::endl;
        }
        else
        {
            std::cerr << "Failed to convert" << std::endl;
        }
    }

    m_x_Data.resize(x.size());
    m_x_Data=x;
}
std::vector<double> fsuMesurement::calcYdata()
{
    int totalPoints = m_x_Data.size();
    m_NoPoints_x = totalPoints;
    m_y_Data.clear();

    std::cerr << "Total points: " << totalPoints << std::endl;
    double range = m_FreqEnd-m_FreqStart;

    double step = range/totalPoints;
    std::cerr << "Range: " << range << "   Step: " << step << std::endl;
    double newYPoint = m_FreqStart;

    for(int i = 0; i < totalPoints; i++)
    {
        newYPoint = newYPoint + step;
        m_y_Data.push_back(newYPoint);
        std::cerr << "Y Berechnet: " << m_y_Data[i] << std::endl;
    }

    m_NoPoints_y = m_y_Data.size();

    return m_y_Data;
}
void fsuMesurement::setFreqStartEnd(double FreqS, double FreqE)
{
    m_FreqStart = FreqS;
    m_FreqEnd = FreqE;
}
//------fsuMesurement Ende-----

// Helper functions



