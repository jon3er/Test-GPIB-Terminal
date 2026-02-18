#include "MeasurementController.h"
#include "systemInfo.h"
#include <iostream>
#include <thread>

MeasurementController::MeasurementController()
{
}

MeasurementController::~MeasurementController()
{
}

void MeasurementController::setOutputCallback(OutputCallback callback)
{
    m_outputCallback = callback;
}

void MeasurementController::setProgressCallback(ProgressCallback callback)
{
    m_progressCallback = callback;
}

void MeasurementController::setDataUpdateCallback(DataUpdateCallback callback)
{
    m_dataUpdateCallback = callback;
}

void MeasurementController::output(const std::string& text)
{
    if (m_outputCallback != nullptr)
    {
        m_outputCallback(text);
    }
}

void MeasurementController::updateProgress(int current, int total)
{
    if (m_progressCallback != nullptr)
    {
        m_progressCallback(current, total);
    }
}

void MeasurementController::updateData(const std::vector<double>& x, const std::vector<double>& y)
{
    if (m_dataUpdateCallback != nullptr)
    {
        m_dataUpdateCallback(x, y);
    }
}

std::vector<std::string> MeasurementController::getAvailableScripts(const std::string& dirPath)
{
    std::vector<std::string> scripts;
    // Implementation for reading scripts from directory
    // This will be extracted from PlotWindow::getFileNames()
    return scripts;
}

void MeasurementController::loadScriptData(const std::string& dirPath, const std::string& fileName,
                                           std::vector<double>& x, std::vector<double>& y)
{
    // Implementation for loading script data
    // This will be extracted from PlotWindow::executeScriptEvent()
}

void MeasurementController::startMeasurement(const std::string& dirPath, const std::string& scriptName, int measurementNumber)
{
    // Implementation for starting measurement
    // This will be extracted from PlotWindow::MeasurementWorkerThread()
    output("Measurement started...\n");
}

void MeasurementController::stopMeasurement()
{
    // Implementation for stopping measurement
    output("Measurement stopped.\n");
}
