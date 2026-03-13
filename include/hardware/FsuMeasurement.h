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
        int startFreq = 10'000'000;
        int stopFreq = 50'000'000;
        int refLevel;
        int points = 625;
        int att;
        wxString unit;
        int rbw;
        int vbw;
        std::string sweepTime = "ON";
        std::string detector;
    };

    bool writeSweepSettings(lastSweepSettings settings);
    bool readSweepSettings();
    auto returnSweepSettings() { return m_lastSwpSettings; };

    // IQ Messung
    struct IqSettings
    {
        unsigned int centerFreq;
        std::string refLevel;
        unsigned int att;
        wxString unit;
        double sampleRate;
        int recordLength;
        double ifBandwidth;
        std::string triggerSource;
        double triggerLevel;
        double triggerDelay;
    };

    bool writeIqSettings(IqSettings settings);
    bool readIqSettings();
    auto returnIqSettings() { return m_lastIqSettings; };

    // Marker Peak Messung
    struct MarkerPeakSettings
    {
        unsigned int startFreq;
        unsigned int stopFreq;
        std::string refLevel;
        unsigned int att;
        wxString unit;
        unsigned int rbw;
        unsigned int vbw;
        std::string detector;
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

private:

    std::vector<double> m_x_Data;
    std::vector<double> m_y_Data;
    double m_FreqStart;
    double m_FreqEnd;
    wxString m_lastMesurementTime;
    unsigned int m_NoPoints_x;
    unsigned int m_NoPoints_y;

    bool m_ImagValues = false;

    // Custom Measurement file Paths
    std::string m_filePathCustomMeasurement;
    std::string m_fileNameCustomMeasurement;

    MeasurementMode m_lastMeasurementMode;
    // Settings
    lastSweepSettings m_lastSwpSettings;
    IqSettings m_lastIqSettings;
    MarkerPeakSettings m_lastMarkerPeakSettings;
};






