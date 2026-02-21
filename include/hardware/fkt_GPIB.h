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

/**
 * @brief single instance class to Performe frequency mesurements with a R&S instrument
 *
*/
class fsuMesurement
{
protected:
    // Constructor (Protected for Singleton)
    fsuMesurement();

public:

    /**
     * @brief get singleton instance of the Class
     */
    static fsuMesurement& get_instance()
    {
        static fsuMesurement instance;
        return instance;
    }
    // Overloads to prevent second instance
    fsuMesurement(const fsuMesurement&) = delete;
    fsuMesurement(fsuMesurement&&) = delete;
    fsuMesurement operator=(const fsuMesurement&) = delete;
    fsuMesurement operator=(fsuMesurement&&) = delete;

    ~fsuMesurement();

    /**
     * @brief Seperates comma seperated vaules
     * @param receivedString Input raw data
     * @param seperatedValues pass seperated values
     */
    void seperateDataBlock(const wxString& receivedString, std::vector<double>& seperatedValues);
    
    /**
     * @brief Caluculates Frequancy range array with set start and stop Frequancy and number of mesurement points
     * @return Frequancy range
     */
    std::vector<double> calcFreqData();

    /**
     * @brief To check if the last mesurement had complex values
     */
    bool isImagValues() { return m_ImagValues; };

    // Get Methodes

    std::vector<double> getX_Data() {return m_x_Data; };
    std::vector<double> getY_Data() {return m_y_Data; };

    wxString getMesurmentTime() { return m_lastMesurementTime; };
    unsigned int getNoPoints_x() { return m_NoPoints_x; };
    unsigned int getNoPoints_y() { return m_NoPoints_y; };

    // Set Methodes

    void setX_Data(std::vector<double> x) { m_x_Data = x; };
    void setY_Data(std::vector<double> y) { m_y_Data = y; };
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
    // Constructor (Protected for Singleton)
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

    void connect();
    void disconnect();
    void config();

    /**
     * @brief Reads avalible data from Adapter
     * @param forceReadBytes (optional) reads until set number of read bytes is reached 
     */
    std::string read(unsigned int forceReadBytes = 0);

    /**
     * @brief writes string to Adapter
     */
    std::string write(std::string msg);

    /**
     * @brief send string to Adapter and listens for a responce
     * @param msg Message to write to device
     * @param DelayMs (Optional) change wait time before expecting a responce
     */
    std::string send(std::string msg, int DelayMs = 100);

    /**
     * @brief retuns current Adapter status
     */
    std::string statusText();

    /**
     * @brief checks if Adapter buffer has data avalibe to read
     */
    DWORD       quaryBuffer();
    
    /**
     * @brief reads command from a .txt file
     */
    void readScriptFile(const wxString& dirPath, const wxString& file, wxArrayString& logAdapterReceived, const std::atomic<bool>* stopFlag = nullptr);
 
    // get methodes
    FT_STATUS getStatus();
    FT_HANDLE getHandle();

    bool getConnected();
    int getBaudrate();
    std::string getLastMsgReseived();

    // set methodes
    void setBaudrate(int BaudrateNew);
    
private:
    PrologixDeviceInfo m_deviceInfo;

    // helper Functionen

    /**
     * @brief Checks input string and modifies it to fix Adapter specifikation
     * @param input raw input text
     * @return modified string
     */
    std::vector<char> checkAscii(std::string input);
};

/**
 * @brief Appends current timestamp to text input
 */
wxString terminalTimestampOutput(wxString Text);


