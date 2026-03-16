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
#include "GpibUsbAdapter.h"
#include <wx/textfile.h>

struct SweepTimeoutRef
{
    // Measured and tested ref values
    double spanHzRef = 80e6;          // z.B. stop-start der Referenz
    double rbwHzRef = 20e3;           // z.B. 20 kHz
    double vbwHzRef = 10e3;           // z.B. 20 kHz
    int pointsRef = 625;
    int checkTimeMsRef = 300;        // measured time untill MAV=true (Msg avaliable)
};

struct IqTimeoutRef
{
    // Measured and tested ref values
    int recordLengthRef = 128;
    double sampleRateHzRef = 100000.0;
    double ifBandwidthHzRef = 100000.0;
    int checkTimeMsRef = 200; // measured at the above reference settings
};

/**
 * @brief single instance class to Performe frequency mesurements with a R&S instrument
 *
*/
class fsuMeasurement
{
protected:
    // Constructor (Protected for Singleton)
    fsuMeasurement();

public:

    /**
     * @brief get singleton instance of the Class
     */
    static fsuMeasurement& get_instance()
    {
        static fsuMeasurement instance;
        return instance;
    }
    // Overloads to prevent second instance
    fsuMeasurement(const fsuMeasurement&) = delete;
    fsuMeasurement(fsuMeasurement&&) = delete;
    fsuMeasurement operator=(const fsuMeasurement&) = delete;
    fsuMeasurement operator=(fsuMeasurement&&) = delete;

    ~fsuMeasurement();

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
    void setNoPoints(unsigned int x, unsigned int y) { m_NoPoints_x = x; m_NoPoints_y = y; };

    // Set Methodes

    void setX_Data(std::vector<double> x) { m_x_Data = x; };
    void setY_Data(std::vector<double> y) { m_y_Data = y; };
    void setFreqStartEnd(unsigned int FreqS, unsigned int FreqE);


    // Measurement
    /**
     * @brief starts a measurement uses a default skript depending on the selected measurement
     * @returns when the measuremnt is complete
     */
    bool executeMeasurement(int TimeOutMs = 5000);


    /**
     * @brief Seperates comma seperated vaules
     * @param receivedString Input raw data
     * @param seperatedValues pass seperated values
     */
    void seperateDataBlock(const wxString& receivedString,
                std::vector<double>& seperatedValuesReal, std::vector<double>& seperatedValuesImag);

    /**
     * @brief Caluculates Frequancy range array with set start and stop Frequancy and number of mesurement points
     * @return Frequancy range
     */
    std::vector<double> calcFreqData();
    /**
     * @brief estimate Time a full (Sweep / Marker) measurement 
     */
    int estimateMeasurementTime(); 
    /**
     * @brief estimate Time a IQ measurement 
     */
    int estimateMeasurementTimeIQ();
    
    // Helper functions
    // Definition der unterstützten Datentypen für die Parameter

    using SettingValue = std::variant<double,unsigned int, int, std::string>;
    bool checkIfSettingsValidSweep(ScpiCommand command, const SettingValue& value);

    void setMeasurementMode(MeasurementMode mode) { m_lastMeasurementMode = mode; };
    MeasurementMode getMeasurementMode() const { return m_lastMeasurementMode; };

    // Helper for Measurement settings
    // Sweep Messung
    struct lastSweepSettings
    {
        double startFreq = 10'000'000;
        double stopFreq = 50'000'000;
        double refLevel = 0;
        int points = 625;
        int att = 0;
        std::string unit = "DBM";
        int rbw = 20000;
        int vbw = 20000;
        std::string sweepTime = "ON";
        std::string detector = "POS";
    };

    bool writeSweepSettings(lastSweepSettings settings);
    bool readSweepSettings();
    auto returnSweepSettings() { return m_lastSwpSettings; };

    // IQ Messung
    struct IqSettings
    {
        double centerFreq = 50'000'000;
        double refLevel = 0;
        int att = 0;
        std::string unit = "DBM";
        std::string filterType = "NORM";
        double sampleRate = 100'000;
        int recordLength = 128;
        double ifBandwidth = 100'000;
        std::string triggerSource = "IMM";
        std::string triggerSlope = "POS";
        int pretriggerSamples = 0;
        double triggerLevel = 0;
        double triggerDelay = 0;
    };

    bool writeIqSettings(IqSettings settings);
    bool readIqSettings();
    auto returnIqSettings() { return m_lastIqSettings; };

    // Marker Peak Messung
    struct MarkerPeakSettings
    {
        double startFreq = 10'000'000;
        double stopFreq = 50'000'000;
        double refLevel = 0;
        int att = 0;
        std::string unit = "DBM";
        int rbw = 10000;
        int vbw = 10000;
        std::string detector = "POS";
    };

    bool writeMarkerPeakSettings(MarkerPeakSettings settings);
    bool readMarkerPeakSettings();
    auto returnMarkerPeakSettings() { return m_lastMarkerPeakSettings; };

    // Custom Measurement
    void setFilePath(wxString path) { m_filePathCustomMeasurement = path; };
    void setFileName(wxString name) { m_fileNameCustomMeasurement = name; };
    std::string getFilePath() { return m_filePathCustomMeasurement; };
    std::string getFileName() { return m_fileNameCustomMeasurement; };

    /**
     * @brief Cached copy of fsuMeasurement settings for CSV I/O
     */
    struct FsuSettings
    {
        MeasurementMode mode{};
        fsuMeasurement::lastSweepSettings    sweep{};
        fsuMeasurement::IqSettings           iq{};
        fsuMeasurement::MarkerPeakSettings   marker{};
        wxString costumFile{};
    };

    // Write Current Settings to GPIB-BUS
    bool writeSettingsToGpib();
    bool readSettingsFromGpib();

    // Error Handeling
    std::string getLastError() {return m_lastError;}
    void setErrorMessage(std::string error);
    void clearLastError() {m_lastError = "";}


private:

    std::vector<double> m_x_Data;
    std::vector<double> m_y_Data;
    double m_FreqStart = 0;
    double m_FreqEnd = 100'000'000;
    wxString m_lastMesurementTime;
    int m_NoPoints_x = 1;
    int m_NoPoints_y = 1;

    bool m_ImagValues = false;

    // Error Handeling
    std::string m_lastError;

    // Custom Measurement file Paths
    std::string m_filePathCustomMeasurement;
    std::string m_fileNameCustomMeasurement;

    MeasurementMode m_lastMeasurementMode = MeasurementMode::SWEEP;
    // Settings
    lastSweepSettings m_lastSwpSettings;
    IqSettings m_lastIqSettings;
    MarkerPeakSettings m_lastMarkerPeakSettings;

    SweepTimeoutRef m_sweepTimeoutRef;
    IqTimeoutRef    m_iqTimeoutRef;
};






