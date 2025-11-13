#pragma once

#include <wx/wx.h>
#include <thread>
#include "fkt_d2xx.h"
#include "ftd2xx.h"

class GpibDevice
{
public:
    GpibDevice();
    ~GpibDevice();

    std::string read(int forceReadBytes = 0);
    std::string write(std::string msg);
    std::string send(std::string msg, int DelayMs = 100);
    DWORD quaryBuffer();
    std::string statusText();
    
    void connect(std::string args = "");
    void disconnect(std::string args = "");
    void config();

    FT_STATUS getStatus();
    FT_HANDLE getHandle();
    
private:
    FT_HANDLE ftHandle = NULL;
    FT_STATUS ftStatus = FT_OK;
    DWORD numDev = 0;
    int BaudRate = 921600;
    bool Connected = false;
    bool configFin = false;
    int sendDelayMs = 100;

    DWORD BytesToRead = 0;
};

wxString terminalTimestampOutput(wxString Text);

std::vector<char> checkAscii(std::string input);

void sleepMs(int timeMs);
