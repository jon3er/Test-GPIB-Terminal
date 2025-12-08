#pragma once

#include <wx/wx.h>
#include <thread>
#include "fkt_d2xx.h"
#include "ftd2xx.h"
#include <wx/textfile.h>

class sData
{
public:
    //Struktur für ein Datenobjekt
    struct sParam                                   //structure for 1D and 3D dataset parameters
    {
        wxString        File;
        wxString        Date;
        wxString        Time;
        wxString        Type;
        unsigned int    NoPoints_x;
        unsigned int    NoPoints_Y;
      //unsigned int  NoPoints_Z;
      //unsigned int  NoScans;
        wxString        ampUnit;
        unsigned int    startFreq;
        unsigned int    endFreq;

    };

    //Konstruktor
    
    sData(const char* type = "empty", unsigned int NoPoints = 0);
    //Destruktor
    ~sData();

    int SetData(sParam *par, std::vector<double> re, std::vector<double> im);
    sParam* GetParameter() { return(dsParam); };
    void GetData(std::vector<double>& re, std::vector<double>& im);
    bool saveToCsvFile(wxString& Filename);
    //virtual bool LoadFile(const wxString &name);

private:
    sParam*                 dsParam;                            //parameters
    std::vector<double>     dsR;                                //data
    std::vector<double>     dsI;

};

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
    sData::sParam getMesurmentData();

    void setFreqStartEnd(double FreqS, double FreqE);

private:
    std::vector<double> x_Data;
    std::vector<double> y_Data;
    double FreqStart;
    double FreqEnd; 
    wxString lastMesurementTime;
    unsigned int NoPoints_x;
    unsigned int NoPoints_y;

    sData tempData;
};

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
 
    FT_STATUS getStatus();
    FT_HANDLE getHandle();
    bool getConnected();
    std::string getLastMsgReseived();
    int getBaudrate();
    fsuMesurement getMesurement() { return Messung; };

    void setBaudrate(int BaudrateNew);
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

    fsuMesurement Messung;
};


wxString terminalTimestampOutput(wxString Text);

std::vector<char> checkAscii(std::string input);

void sleepMs(int timeMs);
