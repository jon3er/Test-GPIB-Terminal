#pragma once

#include <string>
#include <vector>
#include <functional>
#include "dataManagement.h"

/**
 * MeasurementController - Business Logic Layer
 * Handles all measurement operations completely independent from GUI (wxWidgets).
 * The GUI (PlotWindow etc) uses this controller to execute measurement operations.
 */
class MeasurementController
{
public:
    // Callback types for notifications
    using OutputCallback = std::function<void(const std::string&)>;
    using ProgressCallback = std::function<void(int current, int total)>;
    using DataUpdateCallback = std::function<void(const std::vector<double>&, const std::vector<double>&)>;

    MeasurementController();
    ~MeasurementController();

    /**
     * Set the callback function that receives output messages.
     * This allows the controller to communicate with GUI without wxWidgets dependency.
     */
    void setOutputCallback(OutputCallback callback);

    /**
     * Set the callback function that receives progress updates.
     */
    void setProgressCallback(ProgressCallback callback);

    /**
     * Set the callback function that receives plot data updates.
     */
    void setDataUpdateCallback(DataUpdateCallback callback);

    // Plot Window Operations
    std::vector<std::string> getAvailableScripts(const std::string& dirPath);
    void loadScriptData(const std::string& dirPath, const std::string& fileName, 
                        std::vector<double>& x, std::vector<double>& y);
    void startMeasurement(const std::string& dirPath, const std::string& scriptName, int measurementNumber);
    void stopMeasurement();

private:
    OutputCallback m_outputCallback;
    ProgressCallback m_progressCallback;
    DataUpdateCallback m_dataUpdateCallback;

    // Helper for output
    void output(const std::string& text);
    void updateProgress(int current, int total);
    void updateData(const std::vector<double>& x, const std::vector<double>& y);
};
