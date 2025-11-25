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

        wxLogDebug("Reading from Device...");

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
        }
    }
    else
    {
        wxLogDebug("No Device to send too");
        Text = "Failed to Connect to a Device\n";
    }

    return std::string(Text.ToUTF8());
}

std::string GpibDevice::write(std::string msg)
{
    std::string Text;

    wxLogDebug("Write Command Entered");

    if (Connected)
    {
        DWORD bytesWritten;
        wxString GPIBText = msg;
        std::string CheckText(GPIBText.ToUTF8());
        //Check String if Adapter or GPIB Command and check for ASCII 10, 13, 27, 43
        std::vector<char> charArrWriteGpib = checkAscii(CheckText);

        wxLogDebug("Trying to write to Device... %s", std::string(charArrWriteGpib.begin(),charArrWriteGpib.end()));

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
        wxLogDebug("No Connection");
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
    wxLogDebug("Bytes in Queue: %i", BytesToRead);
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
            wxLogDebug("Connected to %i", numDev);
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
        wxLogDebug("Connected to %i", numDev);
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

    wxLogDebug("FT-Config complete");

    if (ftStatus == FT_OK)
    {
        configFin = true;
    }
    else
    {
        configFin = false;
    }
}
void GpibDevice::readScriptFile(const wxString& dirPath, const wxString& file, wxArrayString& logAdapterReceived)
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
            wxString line = textFile.GetLine(i);
            if(line.IsEmpty())
            {
                wxLogDebug("line %i: Empty", (int)i, line);
            }
            else if (line.substr(0,1) == "#")
            {
                wxLogDebug("line %i: Kommentar: %s", (int)i, line.substr(1));
            }
            else if (line.substr(0,5) == "wait ")
            {
                int wait;
                wxString strWait = line.substr(5);
                if (strWait.ToInt(&wait))
                {
                    wxLogDebug("wait for %ims", wait);
                    sleepMs(wait);
                }
                else
                {
                    wxLogDebug("Invalid wait Time input: %s", strWait);
                }
            }
            else if (line.substr(0,5) == "send ")
            {
                wxLogDebug("line %i: manuell send: %s", (int)i, line);
                line = line.substr(5);
                logAdapterReceived.Add(send(std::string(line.ToUTF8())));
                wxLogDebug("responce: %s", logAdapterReceived.Last());
            }
            else if (line.substr(0,6) == "write ")
            {
                wxLogDebug("line %i: manuell write: %s", (int)i, line);
                line = line.substr(6);
                write(std::string(line.ToUTF8()));
            }
            else if (line.substr(0,4) == "read")
            {
                wxLogDebug("line %i: manuell read", (int)i);
                logAdapterReceived.Add(read());
                wxLogDebug("responce: %s", logAdapterReceived.Last());
            }
            else if(line.Contains("?") && line.substr(0,4) == "TRAC")
            {
                wxLogDebug("line %i: send: %s", (int)i, line);

                write(std::string(line.ToUTF8()));
                write("++read eoi");
                sleepMs(300);
                logAdapterReceived.Add(read());
                wxLogDebug("responce: %s", logAdapterReceived.Last());
                seperateDataBlock(logAdapterReceived.Last(),x_Data);
                calcYdata(75'000'000, 125'000'000); //start und end frequenz angeben

            }
            else if(line.Contains("?"))
            {
                wxLogDebug("line %i: send: %s", (int)i, line);
                logAdapterReceived.Add(send(std::string(line.ToUTF8())));
                wxLogDebug("responce: %s", logAdapterReceived.Last());
            }
            else
            {
                wxLogDebug("line %i: write: %s", (int)i, line);
                write(std::string(line.ToUTF8()));
            }
        }
    }
}
void GpibDevice::seperateDataBlock(const wxString& receivedString, std::vector<double>& x)
{
    wxArrayString seperatedStrings = wxStringTokenize(receivedString, ",");

    double value;
    wxString data;
    x.clear();

    for (long unsigned int i = 0; i < seperatedStrings.Count(); i++)
    {
        data = seperatedStrings[i];

        if(data.ToCDouble(&value))
        {
             x.push_back(value);
             wxLogDebug("seperated value: %3f", value);
        }
        else
        {
            wxLogDebug("Failed to convert");
        }
    }
}
void GpibDevice::calcYdata(double startY, double endY)
{
    int totalPoints = x_Data.size();
    y_Data.clear();

    wxLogDebug("Total points: %i", totalPoints);
    double range = endY-startY;

    double step = range/totalPoints;
    wxLogDebug("Range: %3f   Step: %3f",range,step);
    double newYPoint = startY;

    for(int i = 0; i < totalPoints; i++)
    {
        newYPoint = newYPoint + step;
        y_Data.push_back(newYPoint);
        wxLogDebug("Y Berechnet: %f", y_Data[i]);
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

    wxLogDebug("Length in function:");
    wxLogDebug("%s",std::to_string(DataSize));

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

        wxLogDebug("Adapter Command: %s",std::string(vCharOutputAdptr.begin(),vCharOutputAdptr.end()));

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

    wxLogDebug("Input str in ascii: %s", OgString);
    wxLogDebug("Output str in ascii: %s", ModString);

    std::vector<char> vCharOutputGpib(charOutputBuffer,charOutputBuffer + strlen(charOutputBuffer));

    delete[] charInputBuffer;
    delete[] charOutputBuffer;

    return vCharOutputGpib;
}

void sleepMs(int timeMs)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(timeMs));
}
