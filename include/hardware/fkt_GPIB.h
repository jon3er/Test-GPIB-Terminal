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

/*
 * @brief fsuMesurement
 *  
*/

class fsuMesurement
{
protected:
    fsuMesurement();

public:

    // Singelton pattern to prevent multiple calls
    static fsuMesurement& get_instance()
    {
        static fsuMesurement instance;
        return instance;
    }
    // Overload to prevent second two instance
    fsuMesurement(const fsuMesurement&) = delete;
    fsuMesurement(fsuMesurement&&) = delete;
    fsuMesurement operator=(const fsuMesurement&) = delete;
    fsuMesurement operator=(fsuMesurement&&) = delete;

    void seperateDataBlock(const wxString& receivedString);
    
    std::vector<double> calcFreqData(); //TODO umbennen ist eigendlich x und ich erhalte vom gerät nur Y die amplitude.

    wxString getMesurmentTime() { return m_lastMesurementTime; };
    std::vector<double> getX_Data() {return m_x_Data; };
    std::vector<double> getY_Data() {return m_y_Data; };
    unsigned int getNoPoints_x() { return m_NoPoints_x; };
    unsigned int getNoPoints_y() { return m_NoPoints_y; };
    //sData::sParam getMesurmentData();
    bool isImagValues() { return m_ImagValues; };

    void setFreqStartEnd(double FreqS, double FreqE);

private:

    std::vector<double> m_x_Data;
    std::vector<double> m_y_Data;
    double m_FreqStart;
    double m_FreqEnd; 
    wxString m_lastMesurementTime;
    unsigned int m_NoPoints_x;
    unsigned int m_NoPoints_y;

    bool m_ImagValues = false;
    //sData tempData;
};

struct PrologixDeviceInfo {
    FT_HANDLE ftHandle  = NULL;
    FT_STATUS ftStatus  = FT_OK;
    DWORD numDev        = 0;
    int BaudRate        = 921600;
    bool Connected      = false;
    bool configFin      = false;
    int sendDelayMs     = 100;
    std::string lastMsgReceived = "";
    DWORD BytesToRead   = 0;
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

    
private:
    PrologixDeviceInfo m_deviceInfo;

    // helper Functionen
    std::vector<char> checkAscii(std::string input);
};


wxString terminalTimestampOutput(wxString Text);


