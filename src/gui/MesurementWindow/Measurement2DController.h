#pragma once

#include <string>
#include <functional>

/**
 * Measurement2DController - Business Logic Layer
 * Handles all 2D measurement operations completely independent from GUI (wxWidgets).
 * The GUI (Measurement2D) uses this controller to execute measurement operations.
 */
class Measurement2DController
{
public:
    // Callback types for notifications
    using OutputCallback = std::function<void(const std::string&)>;
    using ProgressCallback = std::function<void(int current, int total)>;
    using UpdateCallback = std::function<void()>;

    Measurement2DController();
    ~Measurement2DController();

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

    // Measurement Parameters
    void setYPoints(int points);
    void setXPoints(int points);
    void setScale(int scale);
    void setOrientationPoint(int point);

    int getYPoints() const { return m_sliderY; }
    int getXPoints() const { return m_sliderX; }
    int getScale() const { return m_sliderScale; }

    // Measurement Operations
    void reset();
    void start();
    void stop();
    void incrementMeasurementPoint();
    int getTotalMeasurements() const { return m_totalMeasurementPoints; }
    int getCurrentMeasurement() const { return m_currentMeasurementPoint; }

    // Measurement Execution
    void performSingleMeasurement();

private:
    // Internal state
    int m_sliderY = 10;
    int m_sliderX = 10;
    int m_sliderScale = 100;
    int m_orientationPoint = 0;
    int m_currentMeasurementPoint = 0;
    int m_totalMeasurementPoints = 100;

    // Callbacks
    OutputCallback m_outputCallback;
    ProgressCallback m_progressCallback;
    UpdateCallback m_updateCallback;

    // Helper methods
    void output(const std::string& text);
    void updateProgress();
    void notifyUpdate();
    void calculateTotalMeasurements();
};
