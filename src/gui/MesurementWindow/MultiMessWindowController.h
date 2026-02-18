#pragma once

#include <string>
#include <functional>

/**
 * MultiMessWindowController - Business Logic Layer
 * Handles all multi-point measurement operations completely independent from GUI (wxWidgets).
 * The GUI (MultiMessWindow) uses this controller to execute measurement operations.
 */
class MultiMessWindowController
{
public:
    // Callback types for notifications
    using OutputCallback = std::function<void(const std::string&)>;
    using ProgressCallback = std::function<void(int current, int total)>;
    using UpdateCallback = std::function<void()>;

    MultiMessWindowController();
    ~MultiMessWindowController();

    /**
     * Set the callback function that receives output messages.
     */
    void setOutputCallback(OutputCallback callback);

    /**
     * Set the callback function that receives progress updates.
     */
    void setProgressCallback(ProgressCallback callback);

    /**
     * Set the callback function for GUI updates.
     */
    void setUpdateCallback(UpdateCallback callback);

    // Measurement Point Configuration
    void setXMeasurementPoints(const std::string& value);
    void setYMeasurementPoints(const std::string& value);
    void setXStartCoordinate(const std::string& value);
    void setYStartCoordinate(const std::string& value);
    void setXMeasurementSpacing(const std::string& value);
    void setYMeasurementSpacing(const std::string& value);

    // Frequency Configuration
    void setStartFrequency(const std::string& value);
    void setStartFrequencyUnit(const std::string& value);
    void setStopFrequency(const std::string& value);
    void setStopFrequencyUnit(const std::string& value);
    void setNumberOfSweepPoints(const std::string& value);

    // Getters for current values
    std::string getXMeasurementPoints() const { return m_X_Messpunkte; }
    std::string getYMeasurementPoints() const { return m_Y_Messpunkte; }
    std::string getXStartCoordinate() const { return m_X_Cord; }
    std::string getYStartCoordinate() const { return m_Y_Cord; }
    std::string getXMeasurementSpacing() const { return m_X_MessAbstand; }
    std::string getYMeasurementSpacing() const { return m_Y_MessAbstand; }
    std::string getStartFrequency() const { return m_startFreq; }
    std::string getStartFrequencyUnit() const { return m_startFreqUnit; }
    std::string getStopFrequency() const { return m_stopFreq; }
    std::string getStopFrequencyUnit() const { return m_stopFreqUnit; }
    std::string getNumberOfSweepPoints() const { return m_AnzSweepMessPkt; }

    // Measurement Operations
    void start();
    void stop();
    void reset();
    void nextMeasurementPoint();

    int getTotalMeasurementPoints() const { return m_totalMeasurementPoints; }
    int getCurrentMeasurementPoint() const { return m_currentMeasurementPoint; }

    // Measurement Execution
    void executeMeasurement();

private:
    // Measurement point parameters
    std::string m_X_Messpunkte = "1";
    std::string m_Y_Messpunkte = "1";
    std::string m_X_Cord = "0";
    std::string m_Y_Cord = "0";
    std::string m_X_MessAbstand = "10";
    std::string m_Y_MessAbstand = "10";

    // Frequency parameters
    std::string m_startFreq = "50";
    std::string m_startFreqUnit = "kHz";
    std::string m_stopFreq = "100";
    std::string m_stopFreqUnit = "kHz";
    std::string m_AnzSweepMessPkt = "512";

    // State
    unsigned int m_currentMeasurementPoint = 0;
    unsigned int m_totalMeasurementPoints = 1;

    // Callbacks
    OutputCallback m_outputCallback;
    ProgressCallback m_progressCallback;
    UpdateCallback m_updateCallback;

    // Helper methods
    void output(const std::string& text);
    void updateProgress();
    void notifyUpdate();
    void calculateTotalMeasurementPoints();
};
