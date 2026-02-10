#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <wx/wx.h>
#include <wx/tokenzr.h>

#include "ftd2xx.h"


int printErr(FT_STATUS status, const std::string& msg);

DWORD scanUsbDev();

FT_STATUS configUsbDev(DWORD numDev, FT_HANDLE& ftHandle,int BaudRate);

FT_STATUS writeUsbDev(FT_HANDLE ftHandle, std::vector<char> cmdText, DWORD& bytesWritten);

FT_STATUS readUsbDev(FT_HANDLE ftHandle,std::vector<char>& RPBuffer,DWORD &BytesReturned, DWORD forceReadBytes = 0);

const char * statusString(FT_STATUS status);
