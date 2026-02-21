#include "fkt_d2xx.h"




bool printErrD2XX(FT_STATUS status, const std::string& msg) //Checks for Error and prints Error msg
{
    if (status != FT_OK)
    {

        wxString Text = "Error:" + msg + " (FT_Status Code: " + statusString(status) + ")";
        std::cerr << Text << std::endl;
        return true;
    }

    return false;
}

DWORD scanUsbDev()  //Scans for USB Devices
{
    DWORD numDevs;

    FT_STATUS ftStatus = FT_CreateDeviceInfoList(&numDevs);

    std::cerr << "Number of Devices Found: " << numDevs << std::endl;

    printErrD2XX(ftStatus, "Create Info List");

    return numDevs;
}

// TODO: expand settings
FT_STATUS configUsbDev(DWORD numDev, FT_HANDLE &ftHandle,int BaudRate)  //Sets Baudrate, Timeouts etc.
{
    FT_STATUS ftStatus;

    //set Baud rate
    ftStatus = FT_SetBaudRate(ftHandle,BaudRate);
    printErrD2XX(ftStatus,"Failed to set Baudrate");

    ftStatus = FT_SetDataCharacteristics(ftHandle, FT_BITS_8, FT_STOP_BITS_1, FT_PARITY_NONE);
    printErrD2XX(ftStatus,"Failed to set Data Characteristics");

    ftStatus = FT_SetFlowControl(ftHandle, FT_FLOW_NONE, 0, 0);
    printErrD2XX(ftStatus,"Failed to set Data Characteristics");

    ftStatus =  FT_SetTimeouts(ftHandle, 500,500);
    printErrD2XX(ftStatus,"Failed to set TimeOut");

    std::cerr << "FT-Config complete" << std::endl;

    return ftStatus;
}



FT_STATUS writeUsbDev(FT_HANDLE ftHandle, std::vector<char> cmdText,DWORD& bytesWritten)
{
    FT_STATUS ftStatus;

    char* charPtrCmdText = cmdText.data();
    DWORD dataSize = cmdText.size();
    std::cerr << "Write: " << std::string(cmdText.begin(),cmdText.end()) << " strlen: " << dataSize << std::endl;

    ftStatus = FT_Write(ftHandle, charPtrCmdText, dataSize, &bytesWritten);

    printErrD2XX(ftStatus,"Failed to write");

    if (bytesWritten != dataSize)
    {
        std::cerr << "Failed to write all data" << std::endl;
    }
    else
    {
        std::cerr << "Write Successful! Bytes Written: " << bytesWritten << std::endl;
    }

    return ftStatus;
}

FT_STATUS readUsbDev(FT_HANDLE ftHandle,std::vector<char>& RPBuffer,DWORD &BytesReturned, DWORD forceReadBytes)
{
    DWORD BytesToRead;
    wxString Text;
    FT_STATUS ftStatus;

    //Get Number of bytes to read from receive queue
    if (forceReadBytes == 0)
    {
        ftStatus = FT_GetQueueStatus(ftHandle,&BytesToRead);
        printErrD2XX(ftStatus,"Failed to Get Queue Status");
    }
    else
    {
        BytesToRead = forceReadBytes;
    }
    
    RPBuffer.clear();
    RPBuffer.resize(BytesToRead);
    char* ptrRPBuffer = RPBuffer.data();

    std::cerr << "Bytes to read from queue: " << std::to_string(BytesToRead) << std::endl;

    if (BytesToRead <= 0)
    {
        std::cerr << "No Data to read bytes to read: " << BytesToRead << std::endl;

        return ftStatus;
    }

    ftStatus = FT_Read(ftHandle, ptrRPBuffer, BytesToRead, &BytesReturned);

    printErrD2XX(ftStatus,"Failed to Read data");

    DWORD dataSize = RPBuffer.size();

    if (BytesReturned != dataSize)
    {
        printErrD2XX(ftStatus,"Failed to recive all of the Data");

        std::cerr << "Received data Size: " << dataSize << " Bytes Returned: " << BytesReturned << std::endl;
    }
    else
    {
        std::cerr << "Read Successful " << dataSize << " Bytes read" << std::endl;
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

