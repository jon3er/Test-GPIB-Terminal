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

class PrologixUsbGpibAdapter
{
protected:
    PrologixUsbGpibAdapter();

    
public:
    // Singelton pattern to prevent multiple calls
    static PrologixUsbGpibAdapter& get_instance()
    {
        static PrologixUsbGpibAdapter instance;
        return instance;
    }
    // Overload to prevent second two instance
    PrologixUsbGpibAdapter(const PrologixUsbGpibAdapter&) = delete;
    PrologixUsbGpibAdapter(PrologixUsbGpibAdapter&&) = delete;
    PrologixUsbGpibAdapter operator=(const PrologixUsbGpibAdapter&) = delete;
    PrologixUsbGpibAdapter operator=(PrologixUsbGpibAdapter&&) = delete;

    ~PrologixUsbGpibAdapter();

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

    void setBaudrate(int BaudrateNew);
    
    fsuMesurement getMesurement() { return Messung; };

    
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

    // helper Functionen
    std::vector<char> checkAscii(std::string input);
};


wxString terminalTimestampOutput(wxString Text);


