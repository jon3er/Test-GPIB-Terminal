#pragma once

#include <fstream>
#include <iomanip>
#include <format>
#include <wx/wx.h>
#include <variant>
#include <set>
#include <thread>
#include <atomic>
#include "cmdGpib.h"
#include "fkt_d2xx.h"
#include "ftd2xx.h"
#include <wx/textfile.h>

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
    bool busy = false;
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
    // Overload to prevent second two instances
    PrologixUsbGpibAdapter(const PrologixUsbGpibAdapter&) = delete;
    PrologixUsbGpibAdapter(PrologixUsbGpibAdapter&&) = delete;
    PrologixUsbGpibAdapter operator=(const PrologixUsbGpibAdapter&) = delete;
    PrologixUsbGpibAdapter operator=(PrologixUsbGpibAdapter&&) = delete;

    ~PrologixUsbGpibAdapter();

    bool checkIfAdapterAvailable();
    bool checkIfGpibDeviceAvailable();


    bool connect();
    bool disconnect();
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
     * @brief Checks GPIB Bus Status bit-4 (message bit)
     * @returns True if Mesage is avaliable or timeOut is reached
     */
    bool checkIfMsgAvailable(int TimeOutMs = 100);

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
    void readScriptFile(const wxString& dirPath, const wxString& file, wxArrayString* logAdapterReceived = nullptr, const std::atomic<bool>* stopFlag = nullptr);
 
    /**
     * @brief unloads VCP drivers to solve drive conficts on linux/gnu
     */
    void prepareFTDIDevice();

    /**
     * @brief checks the Status register for errors and returns found errors
     */
    void checkForGpibBusError(wxWindow* parent);

    // get methodes
    FT_STATUS getStatus();
    FT_HANDLE getHandle();

    bool getConnected();
    int getBaudrate();
    std::string getLastMsgReseived();
    bool getBusy(){return m_deviceInfo.busy;};


    // set methodes
    void setBaudrate(int BaudrateNew);
    void setBusy(bool busy) {m_deviceInfo.busy = busy;};
    
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