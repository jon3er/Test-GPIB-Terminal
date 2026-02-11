
#include "fkt_GPIB.h"

//TODO Create Device Class and Create new read read and write funtions
GpibDevice::GpibDevice()
{

}

GpibDevice::~GpibDevice()
{
    disconnect();
}

std::string GpibDevice::read(int forceReadBytes)
{
    wxString Text;

    if (Connected)
    {
        std::vector<char> BigBuffer;
        DWORD BufferSize;
        FT_STATUS ftStatus;

        std::cerr << "Reading from Device..." << std::endl;

        ftStatus = readUsbDev(ftHandle, BigBuffer, BufferSize, forceReadBytes);

        if (ftStatus == FT_OK)
        {
            lastMsgReceived = std::string(BigBuffer.data(),BigBuffer.size());
            Text = "Msg received: " + lastMsgReceived + "\n";

            if (BigBuffer.size() == 0)
            {
                Text = "No Message to Read\n";
            }
        }
        else
        {
            Text = "Failed to Receive Data - TimeOut after 5s\n";
            Connected = false;
        }
    }
    else
    {
        std::cerr << "No Device to send too" << std::endl;
        Text = "Failed to Connect to a Device\n";
        Connected = false;
    }

    return std::string(Text.ToUTF8());
}

std::string GpibDevice::write(std::string msg)
{
    std::string Text;

    std::cerr << "Write Command Entered" << std::endl;

    if (Connected)
    {
        DWORD bytesWritten;
        wxString GPIBText = msg;
        std::string CheckText(GPIBText.ToUTF8());
        //Check String if Adapter or GPIB Command and check for ASCII 10, 13, 27, 43
        std::vector<char> charArrWriteGpib = checkAscii(CheckText);

        std::cerr << "Trying to write to Device... " << std::string(charArrWriteGpib.begin(),charArrWriteGpib.end()) << std::endl;

        FT_STATUS ftStatus =writeUsbDev(ftHandle, charArrWriteGpib, bytesWritten);

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

std::string GpibDevice::send(std::string msg, int DelayMs)
{
    write(msg);
    sleepMs(DelayMs);
    quaryBuffer();

    return read();
}

DWORD GpibDevice::quaryBuffer()
{
    //Get Number of bytes to read from receive queue
    ftStatus = FT_GetQueueStatus(ftHandle,&BytesToRead);
    std::cerr << "Bytes in Queue: " << BytesToRead << std::endl;
    printErr(ftStatus,"Failed to Get Queue Status");

    return BytesToRead;
}

void GpibDevice::connect(std::string args)
{
    if (!Connected)
    {
        ftStatus = FT_Open(numDev,&ftHandle);
        printErr(ftStatus,"Failed to Connect");
        write("SYST:DISP:UDP ON"); //Turn on monitor

        if (ftStatus == FT_OK)
        {
            std::cerr << "Connected to " << numDev << std::endl;
            Connected = true;
        }
    }
}
void GpibDevice::disconnect(std::string args)
{
    write("++auto 0");
    write("*CLS");
    write("++loc");
    write("++ifc");

    sleepMs(200);


    ftStatus = FT_Close(ftHandle);
    printErr(ftStatus,"Failed to Disconnect");
    if (ftStatus == FT_OK)
    {
        std::cerr << "Connected to " << numDev << std::endl;
        Connected = false;
    }
}
void GpibDevice::config()
{
    //set Baudrate
    ftStatus = FT_SetBaudRate(ftHandle,BaudRate);
    printErr(ftStatus,"Failed to set Baudrate");

    ftStatus = FT_SetDataCharacteristics(ftHandle, FT_BITS_8, FT_STOP_BITS_1, FT_PARITY_NONE);
    printErr(ftStatus,"Failed to set Data Characteristics");

    ftStatus = FT_SetFlowControl(ftHandle, FT_FLOW_NONE, 0, 0);
    printErr(ftStatus,"Failed to set flow Characteristics");

    ftStatus =  FT_SetTimeouts(ftHandle, 500,500);
    printErr(ftStatus,"Failed to set TimeOut");

    std::cerr << "FT-Config complete" << std::endl;

    if (ftStatus == FT_OK)
    {
        configFin = true;
    }
    else
    {
        configFin = false;
    }
}
void GpibDevice::readScriptFile(const wxString& dirPath, const wxString& file, wxArrayString& logAdapterReceived, const std::atomic<bool>* stopFlag)
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
                write("++read eoi");
                sleepMs(300);
                logAdapterReceived.Add(read());
                std::cerr << "responce: " << logAdapterReceived.Last() << std::endl;
                Messung.seperateDataBlock(logAdapterReceived.Last());
                Messung.setFreqStartEnd(75'000'000,125'000'000);
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

std::string GpibDevice::statusText()
{
    std::string Text;

    if (Connected)
    {
        Text = "Connected to a Device ";
    }
    else
    {
        Text = "Not connected to a device";
    }
    if (Connected && configFin)
    {
        Text = Text + " and ";
    }
    if (configFin)
    {
        Text = Text + "Device config set Baudrate to: " + std::to_string(BaudRate);
    }

    Text += "\nLast Status code:" + wxString(statusString(ftStatus)) + "\n";

    return Text;
}

FT_STATUS GpibDevice::getStatus()
{
    return ftStatus;
}

FT_HANDLE GpibDevice::getHandle()
{
    return ftHandle;
}

bool GpibDevice::getConnected()
{
    return Connected;
}

std::string GpibDevice::getLastMsgReseived()
{
    return lastMsgReceived;
}

int GpibDevice::getBaudrate()
{
    return BaudRate;
}

void GpibDevice::setBaudrate(int BaudrateNew)
{
    BaudRate = BaudrateNew;
}
//------fsuMesurement Beginn-----
fsuMesurement::fsuMesurement()
{
    x_Data = {0};
    y_Data = {0};

    FreqStart = 75'000'000;
    FreqEnd = 125'000'000;

    NoPoints_x = 0;
    NoPoints_y = 0;
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

    x_Data.resize(x.size());
    x_Data=x;
}
std::vector<double> fsuMesurement::calcYdata()
{
    int totalPoints = x_Data.size();
    NoPoints_x = totalPoints;
    y_Data.clear();

    std::cerr << "Total points: " << totalPoints << std::endl;
    double range = FreqEnd-FreqStart;

    double step = range/totalPoints;
    std::cerr << "Range: " << range << "   Step: " << step << std::endl;
    double newYPoint = FreqStart;

    for(int i = 0; i < totalPoints; i++)
    {
        newYPoint = newYPoint + step;
        y_Data.push_back(newYPoint);
        std::cerr << "Y Berechnet: " << y_Data[i] << std::endl;
    }

    NoPoints_y = y_Data.size();

    return y_Data;
}
void fsuMesurement::setFreqStartEnd(double FreqS, double FreqE)
{
    FreqStart = FreqS;
    FreqEnd = FreqE;
}
/*
sData::sParam fsuMesurement::getMesurmentData()
{
    sData::sParam *tempOld = tempData.GetParameter();
    return *tempOld;

}
*/

//------fsuMesurement Ende-----

// Helper functions
wxString terminalTimestampOutput(wxString Text)
{
    //Set Terminal Output Format
    wxDateTime zeitJetzt = wxDateTime::Now();
    wxString timestamp = zeitJetzt.Format("%H:%M:%S");
    wxString FormatText = "[" + timestamp + "] " + Text;

    return FormatText;
}
std::vector<char> checkAscii(std::string input)
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
void sleepMs(int timeMs)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(timeMs));
}
