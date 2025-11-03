#include "fkt_d2xx.h"

class FTDevice
{
public:
    FT_HANDLE ftHandle;
    FT_STATUS ftStatus;
    DWORD numDev;
    int BaudRate;
};


int printErr(FT_STATUS status, const std::string& msg)
{
    if (status != FT_OK)
    {
        wxString Text = "Error:" + msg + " (FT_Status Code: " + std::to_string(status) + ")";
        wxLogDebug(Text);
        return 1;
    }

    return 0;
}

DWORD scanUsbDev()
{
    DWORD numDevs;

    FT_STATUS ftStatus = FT_CreateDeviceInfoList(&numDevs);

    wxLogDebug("Number of Devices Found: %i", numDevs);

    wxLogDebug("FT Status Code: %i", ftStatus);

    return numDevs;
}

FT_STATUS configUsbDev(DWORD numDev, FT_HANDLE &ftHandle,int BaudRate)
{
        //open first Device
    FT_STATUS ftStatus = FT_Open(numDev, &ftHandle);

    int errorDetect = printErr(ftStatus,"FT Open Failed. Is programm Run as su? are ftdi_sio driver trunned off?");

    if (errorDetect)
    {
        return ftStatus;
    }


    //set Baud rate
    ftStatus = FT_SetBaudRate(ftHandle,BaudRate);

    errorDetect = printErr(ftStatus,"Failed to set Baudrate");

    if (errorDetect)
    {
        return ftStatus;
    }


    //set Baud rate
    ftStatus = FT_SetDataCharacteristics(ftHandle, FT_BITS_8, FT_STOP_BITS_1, FT_PARITY_NONE);

    errorDetect = printErr(ftStatus,"Failed to set Data Characteristics");

    if (errorDetect)
    {
        return ftStatus;
    }

    wxLogDebug("FT-Config complete");

    return ftStatus;

    FT_Close(ftHandle);

}


FT_STATUS writeUsbDev(FT_HANDLE ftHandle, wxString cmdText,DWORD& bytesWritten)
{

    //open first Device
    FT_STATUS ftStatus = FT_Open(0, &ftHandle);

    int errorDetect = printErr(ftStatus,"FT Open Failed. Is programm Run as su? are ftdi_sio driver trunned off?");

    if (errorDetect)
    {
        return ftStatus;
    }

    // TODO -- CR (ASCII 13), LF (ASCII 10), ESC (ASCII 27), ‘+’ (ASCII 43) – they must be escaped by preceding them with an ESC character.
    wxCharBuffer txBuffer = cmdText.c_str();

    DWORD dataSize = strlen(txBuffer.data()); // bei null terminatior +1 addieren

    ftStatus = FT_Write(ftHandle, txBuffer.data(), dataSize, &bytesWritten);

    errorDetect = printErr(ftStatus,"Failed to write all of the Data");

    if (bytesWritten != dataSize)
    {
        wxLogDebug("Write Failed");
    }
    else
    {
        wxLogDebug("Write Successful!");
    }

    FT_Close(ftHandle);
    return ftStatus;
}


FT_STATUS readUsbDev(FT_HANDLE ftHandle,char *RPBuffer, DWORD& BufferSize)
{
    DWORD BytesToRead;
    DWORD* BytesReturned;
    wxString Text;

    //open first Device
    FT_STATUS ftStatus = FT_Open(0, &ftHandle);

    int errorDetect = printErr(ftStatus,"FT Open Failed. Is programm Run as su? are ftdi_sio driver trunned off?");

    if (errorDetect)
    {
        return ftStatus;
    }


    //Get Number of bytes to read from receive queue
    ftStatus = FT_GetQueueStatus(ftHandle,&BytesToRead);

    Text = "Bytes to read from queue: " + std::to_string(BytesToRead);
    wxLogDebug(Text);

    if (BytesToRead <= 0)
    {
        FT_Close(ftHandle);
        return ftStatus;
    }

    ftStatus = FT_Read(ftHandle, RPBuffer, BytesToRead, BytesReturned);

    errorDetect = printErr(ftStatus,"Failed to Write data");
    DWORD dataSize = sizeof(*RPBuffer);

    BufferSize = *BytesReturned;

    if (*BytesReturned != dataSize)
    {
        errorDetect = printErr(ftStatus,"Failed to recive all of the Data");
    }
    else
    {
        wxLogDebug("Read Successful");
    }

    FT_Close(ftHandle);
    return ftStatus;
}

