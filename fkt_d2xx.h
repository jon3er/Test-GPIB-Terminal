#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <unistd.h>
#include <wx/wx.h>

#include "ftd2xx.h"


int printErr(FT_STATUS status, const std::string& msg);

DWORD scanUsbDev();

FT_STATUS configUsbDev(DWORD numDev, FT_HANDLE& ftHandle,int BaudRate);

FT_STATUS writeUsbDev(FT_HANDLE ftHandle, wxString cmdText, DWORD& bytesWritten);

FT_STATUS readUsbDev(FT_HANDLE ftHandle,char *RPBuffer, DWORD& BufferSize);



