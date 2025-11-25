#pragma once

#include <wx/wx.h>
#include <thread>
#include "fkt_d2xx.h"
#include "ftd2xx.h"
#include <wx/textfile.h>

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
    void readScriptFile(const wxString& dirPath, const wxString& file, wxArrayString& logAdapterReceived);
    void seperateDataBlock(const wxString& receivedString, std::vector<double>& x);
    void calcYdata(double startY, double endY);

    FT_STATUS getStatus();
    FT_HANDLE getHandle();
    bool getConnected();
    std::string getLastMsgReseived();
    int getBaudrate();

    void setBaudrate(int BaudrateNew);

    std::vector<double> x_Data = {0};
    std::vector<double> y_Data = {0};

private:
    FT_HANDLE ftHandle = NULL;
    FT_STATUS ftStatus = FT_OK;
    DWORD numDev = 0;
    int BaudRate = 921600;
    bool Connected = false;
    bool configFin = false;
    int sendDelayMs = 100;
    std::string lastMsgReceived = "";
    DWORD BytesToRead = 0;
};

wxString terminalTimestampOutput(wxString Text);

std::vector<char> checkAscii(std::string input);

void sleepMs(int timeMs);
