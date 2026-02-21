#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <wx/wx.h>
#include <wx/tokenzr.h>

#include "ftd2xx.h"

/**
 * @brief Prints out D2XX communication related Errors to std::cerr
 * @param status input status to be printed
 * @param msg input reference message
 * @return True if a error is detected
 */
bool printErrD2XX(FT_STATUS status, const std::string& msg);

/**
 * @brief scans availble ports for D2XX devices
 * @return number of found devices
 */
DWORD scanUsbDev();

/**
 * @brief configures Basic device communication settings
 * @param numDev Device number
 * @param ftHandle Device Handle
 * @param BaudRate Baudrate to be set
 * @return Communication status
 */
FT_STATUS configUsbDev(DWORD numDev, FT_HANDLE& ftHandle, int BaudRate);

/**
 * @brief Writes message to a device, update numbe of sent Bytes
 * @return Communication status
 */
FT_STATUS writeUsbDev(FT_HANDLE ftHandle, std::vector<char> cmdText, DWORD& bytesWritten);

/**
 * @brief reads messages from the device buffer
 * @param ftHandle device handle
 * @param BRBuffer passes read message buffer
 * @param BytesReturned passes number total read bytes
 * @param forceReadBytes trys too read until set number of bytes is reached
 * @return Communication status
 */
FT_STATUS readUsbDev(FT_HANDLE ftHandle,std::vector<char>& RPBuffer,DWORD &BytesReturned, DWORD forceReadBytes = 0);

/**
 * @brief Translates D2XX error codes to text
 * @param status input D2XX FT_Status codes
 * @return error text 
 */
const char * statusString(FT_STATUS status);
