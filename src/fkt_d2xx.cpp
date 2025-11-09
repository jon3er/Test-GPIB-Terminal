#include "fkt_d2xx.h"

class FTDevice
{
public:
    FT_HANDLE ftHandle;
    FT_STATUS ftStatus;
    DWORD numDev;
    int BaudRate;
};


int printErr(FT_STATUS status, const std::string& msg) //Checks for Error and prints Error msg
{
    if (status != FT_OK)
    {

        wxString Text = "Error:" + msg + " (FT_Status Code: " + statusString(status) + ")";
        wxLogDebug(Text);
        return 1;
    }

    return 0;
}

DWORD scanUsbDev()  //Scans for USB Devices
{
    DWORD numDevs;

    FT_STATUS ftStatus = FT_CreateDeviceInfoList(&numDevs);

    wxLogDebug("Number of Devices Found: %i", numDevs);

    printErr(ftStatus, "Create Info List");

    return numDevs;
}

FT_STATUS configUsbDev(DWORD numDev, FT_HANDLE &ftHandle,int BaudRate)  //Sets Baudrate, Timeouts etc.
{
    FT_STATUS ftStatus;

    //set Baud rate
    ftStatus = FT_SetBaudRate(ftHandle,BaudRate);
    printErr(ftStatus,"Failed to set Baudrate");

    ftStatus = FT_SetDataCharacteristics(ftHandle, FT_BITS_8, FT_STOP_BITS_1, FT_PARITY_NONE);
    printErr(ftStatus,"Failed to set Data Characteristics");

    ftStatus = FT_SetFlowControl(ftHandle, FT_FLOW_NONE, 0, 0);
    printErr(ftStatus,"Failed to set Data Characteristics");

    ftStatus =  FT_SetTimeouts(ftHandle, 500,500);
    printErr(ftStatus,"Failed to set TimeOut");

    wxLogDebug("FT-Config complete");

    return ftStatus;
}



FT_STATUS writeUsbDev(FT_HANDLE ftHandle, std::vector<char> cmdText,DWORD& bytesWritten)
{
    FT_STATUS ftStatus;

    char* charPtrCmdText = cmdText.data();
    DWORD dataSize = cmdText.size();
    wxLogDebug("Write: %s \n strlen: %i",std::string(cmdText.begin(),cmdText.end()),(int)dataSize);

    ftStatus = FT_Write(ftHandle, charPtrCmdText, dataSize, &bytesWritten);

    printErr(ftStatus,"Failed to write");

    if (bytesWritten != dataSize)
    {
        wxLogDebug("Failed to write all data");
    }
    else
    {
        wxLogDebug("Write Successful! Bytes Written: %d",(int)bytesWritten);
    }

    return ftStatus;
}

FT_STATUS readUsbDev(FT_HANDLE ftHandle,std::vector<char>& RPBuffer,DWORD &BytesReturned)
{
    DWORD BytesToRead;
    wxString Text;
    FT_STATUS ftStatus;

    //Get Number of bytes to read from receive queue
    ftStatus = FT_GetQueueStatus(ftHandle,&BytesToRead);

    printErr(ftStatus,"Failed to Get Queue Status");
    
    RPBuffer.clear();
    RPBuffer.resize(BytesToRead);
    char* ptrRPBuffer = RPBuffer.data();

    wxLogDebug("Bytes to read from queue: %s", std::to_string(BytesToRead));

    if (BytesToRead <= 0)
    {
        wxLogDebug("No Data to read bytes to read: %d", (int)BytesToRead);

        return ftStatus;
    }

    ftStatus = FT_Read(ftHandle, ptrRPBuffer, BytesToRead, &BytesReturned);

    printErr(ftStatus,"Failed to Read data");

    DWORD dataSize = RPBuffer.size();

    if (BytesReturned != dataSize)
    {
        printErr(ftStatus,"Failed to recive all of the Data");

        wxLogDebug("Received data Size: %d \n Bytes Returned: %d",(int)dataSize,(int)BytesReturned);
    }
    else
    {
        wxLogDebug("Read Successful %d Bytes read",(int)dataSize);
    }

    return ftStatus;
}

const char * statusString(FT_STATUS status) //Error Codes
{
    switch (status)
    {
        case FT_OK:
            return "OK";
        case FT_INVALID_HANDLE:
            return "INVALID_HANDLE";
        case FT_DEVICE_NOT_FOUND:
            return "DEVICE_NOT_FOUND";
        case FT_DEVICE_NOT_OPENED:
            return "DEVICE_NOT_OPENED";
        case FT_IO_ERROR:
            return "IO_ERROR";
        case FT_INSUFFICIENT_RESOURCES:
            return "INSUFFICIENT_RESOURCES";
        case FT_INVALID_PARAMETER:
            return "INVALID_PARAMETER";
        case FT_INVALID_BAUD_RATE:
            return "INVALID_BAUD_RATE";
        case FT_DEVICE_NOT_OPENED_FOR_ERASE:
            return "DEVICE_NOT_OPENED_FOR_ERASE";
        case FT_DEVICE_NOT_OPENED_FOR_WRITE:
            return "DEVICE_NOT_OPENED_FOR_WRITE";
        case FT_FAILED_TO_WRITE_DEVICE:
            return "FAILED_TO_WRITE_DEVICE";
        case FT_EEPROM_READ_FAILED:
            return "EEPROM_READ_FAILED";
        case FT_EEPROM_WRITE_FAILED:
            return "EEPROM_WRITE_FAILED";
        case FT_EEPROM_ERASE_FAILED:
            return "EEPROM_ERASE_FAILED";
        case FT_EEPROM_NOT_PRESENT:
            return "EEPROM_NOT_PRESENT";
        case FT_EEPROM_NOT_PROGRAMMED:
            return "EEPROM_NOT_PROGRAMMED";
        case FT_INVALID_ARGS:
            return "INVALID_ARGS";
        case FT_NOT_SUPPORTED:
            return "NOT_SUPPORTED";
        case FT_OTHER_ERROR:
            return "OTHER_ERROR";
        case FT_DEVICE_LIST_NOT_READY:
            return "DEVICE_LIST_NOT_READY";
        default:
            return "unknown error";
    }
}

