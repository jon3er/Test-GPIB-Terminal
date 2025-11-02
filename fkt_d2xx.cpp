#include "fkt_d2xx.h"

int printErr(FT_STATUS status, const std::string& msg)
{
    if (status != FT_OK)
    {
        wxLogDebug("Error:", msg, " (FT_Status Code: ", status, ")/n");
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

FT_STATUS configUsbDev(DWORD numDev, FT_HANDLE& ftHandle,int BaudRate)
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

}


FT_STATUS writeUsbDev(FT_HANDLE ftHandle, wxString cmdText,DWORD& bytesWritten)
{
    wxCharBuffer txBuffer = cmdText.c_str();

    DWORD dataSize = strlen(txBuffer.data()); // bei null terminatior +1 addieren

    FT_STATUS ftStatus = FT_Write(ftHandle, txBuffer.data(), dataSize, &bytesWritten);


    if (bytesWritten != dataSize)
    {
        int errorDetect = printErr(ftStatus,"Failed All of the Data");
    }
    else
    {
        wxLogDebug("Write Successful");
    }


    return ftStatus;
}


FT_STATUS readUsbDev(FT_HANDLE ftHandle,LPVOID RPBuffer,DWORD BytesToRead,DWORD* BytesReturned)
{
    DWORD ByteToRead;

    FT_STATUS ftStatus = FT_Read(ftHandle, RPBuffer, ByteToRead, BytesReturned);

    int errorDetect = printErr(ftStatus,"Failed to Write data");
    DWORD dataSize = sizeof(RPBuffer);

    if (*BytesReturned != dataSize)
    {
        errorDetect = printErr(ftStatus,"Failed to recive all of the Data");
    }
    else
    {
        wxLogDebug("Read Successful");
    }


    return ftStatus;
}

