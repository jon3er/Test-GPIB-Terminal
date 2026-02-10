#pragma once

#include <fstream>
#include <iomanip>
#include <format>
#include <wx/wx.h>
#include <thread>
#include <atomic>
#include "fkt_d2xx.h"
#include "ftd2xx.h"
#include <wx/textfile.h>



class fsuMesurement
{
public:
    fsuMesurement();
    void seperateDataBlock(const wxString& receivedString);
    std::vector<double> calcYdata(); //TODO umbennen ist eigendlich x und ich erhalte vom gerät nur Y die amplitude.

    wxString getMesurmentTime() { return lastMesurementTime; };
    std::vector<double> getX_Data() {return x_Data; };
    std::vector<double> getY_Data() {return y_Data; };
    unsigned int getNoPoints_x() { return NoPoints_x; };
    unsigned int getNoPoints_y() { return NoPoints_y; };
    //sData::sParam getMesurmentData();
    bool isImagValues() { return ImagValues; };


    void setFreqStartEnd(double FreqS, double FreqE);

private:

    std::vector<double> x_Data;
    std::vector<double> y_Data;
    double FreqStart;
    double FreqEnd; 
    wxString lastMesurementTime;
    unsigned int NoPoints_x;
    unsigned int NoPoints_y;

    bool ImagValues = false;
    //sData tempData;
};

class GpibDevice
{
protected:
    GpibDevice();

    
public:
    // Singelton pattern to prevent multiple calls
    static GpibDevice& get_instance()
    {
        static GpibDevice instance;
        return instance;
    }
    // Overload to prevent second two instance
    GpibDevice(const GpibDevice&) = delete;
    GpibDevice(GpibDevice&&) = delete;
    GpibDevice operator=(const GpibDevice&) = delete;
    GpibDevice operator=(GpibDevice&&) = delete;

    ~GpibDevice();

    std::string read(int forceReadBytes = 0);
    std::string write(std::string msg);
    std::string send(std::string msg, int DelayMs = 100);
    std::string statusText();
    DWORD       quaryBuffer();
    

    void connect(std::string args = "");
    void disconnect(std::string args = "");
    void config();
    void readScriptFile(const wxString& dirPath, const wxString& file, wxArrayString& logAdapterReceived, const std::atomic<bool>* stopFlag = nullptr);
 
    FT_STATUS getStatus();
    FT_HANDLE getHandle();

    bool getConnected();
    int getBaudrate();
    std::string getLastMsgReseived();
    
    fsuMesurement getMesurement() { return Messung; };

    void setBaudrate(int BaudrateNew);
private:
    FT_HANDLE ftHandle  = NULL;
    FT_STATUS ftStatus  = FT_OK;
    DWORD numDev        = 0;
    int BaudRate        = 921600;
    bool Connected      = false;
    bool configFin      = false;
    int sendDelayMs     = 100;
    std::string lastMsgReceived = "";
    DWORD BytesToRead   = 0;

    fsuMesurement Messung;
};


wxString terminalTimestampOutput(wxString Text);

std::vector<char> checkAscii(std::string input);

void sleepMs(int timeMs);
