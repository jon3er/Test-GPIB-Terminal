#include "MultiMessWindowController.h"
#include "systemInfo.h"
#include <iostream>

MultiMessWindowController::MultiMessWindowController()
{
    calculateTotalMeasurementPoints();
}

MultiMessWindowController::~MultiMessWindowController()
{
}

void MultiMessWindowController::setOutputCallback(OutputCallback callback)
{
    m_outputCallback = callback;
}

void MultiMessWindowController::setProgressCallback(ProgressCallback callback)
{
    m_progressCallback = callback;
}

void MultiMessWindowController::setUpdateCallback(UpdateCallback callback)
{
    m_updateCallback = callback;
}

void MultiMessWindowController::output(const std::string& text)
{
    if (m_outputCallback != nullptr)
    {
        m_outputCallback(text);
    }
}

void MultiMessWindowController::updateProgress()
{
    if (m_progressCallback != nullptr)
    {
        m_progressCallback(m_currentMeasurementPoint, m_totalMeasurementPoints);
    }
}

void MultiMessWindowController::notifyUpdate()
{
    if (m_updateCallback != nullptr)
    {
        m_updateCallback();
    }
}

void MultiMessWindowController::calculateTotalMeasurementPoints()
{
    try
    {
        int x = std::stoi(m_X_Messpunkte);
        int y = std::stoi(m_Y_Messpunkte);
        m_totalMeasurementPoints = x * y;
    }
    catch (const std::exception& e)
    {
        output("Error calculating total measurement points: " + std::string(e.what()) + "\n");
        m_totalMeasurementPoints = 1;
    }
}

void MultiMessWindowController::setXMeasurementPoints(const std::string& value)
{
    m_X_Messpunkte = value;
    calculateTotalMeasurementPoints();
    notifyUpdate();
}

void MultiMessWindowController::setYMeasurementPoints(const std::string& value)
{
    m_Y_Messpunkte = value;
    calculateTotalMeasurementPoints();
    notifyUpdate();
}

void MultiMessWindowController::setXStartCoordinate(const std::string& value)
{
    m_X_Cord = value;
    notifyUpdate();
}

void MultiMessWindowController::setYStartCoordinate(const std::string& value)
{
    m_Y_Cord = value;
    notifyUpdate();
}

void MultiMessWindowController::setXMeasurementSpacing(const std::string& value)
{
    m_X_MessAbstand = value;
    notifyUpdate();
}

void MultiMessWindowController::setYMeasurementSpacing(const std::string& value)
{
    m_Y_MessAbstand = value;
    notifyUpdate();
}

void MultiMessWindowController::setStartFrequency(const std::string& value)
{
    m_startFreq = value;
    notifyUpdate();
}

void MultiMessWindowController::setStartFrequencyUnit(const std::string& value)
{
    m_startFreqUnit = value;
    notifyUpdate();
}

void MultiMessWindowController::setStopFrequency(const std::string& value)
{
    m_stopFreq = value;
    notifyUpdate();
}

void MultiMessWindowController::setStopFrequencyUnit(const std::string& value)
{
    m_stopFreqUnit = value;
    notifyUpdate();
}

void MultiMessWindowController::setNumberOfSweepPoints(const std::string& value)
{
    m_AnzSweepMessPkt = value;
    notifyUpdate();
}

void MultiMessWindowController::start()
{
    calculateTotalMeasurementPoints();
    m_currentMeasurementPoint = 0;
    output("Multi-point measurement started.\n");
    updateProgress();
}

void MultiMessWindowController::stop()
{
    m_currentMeasurementPoint = 0;
    output("Multi-point measurement stopped.\n");
    updateProgress();
}

void MultiMessWindowController::reset()
{
    m_X_Messpunkte = "1";
    m_Y_Messpunkte = "1";
    m_X_Cord = "0";
    m_Y_Cord = "0";
    m_X_MessAbstand = "10";
    m_Y_MessAbstand = "10";
    m_startFreq = "50";
    m_startFreqUnit = "kHz";
    m_stopFreq = "100";
    m_stopFreqUnit = "kHz";
    m_AnzSweepMessPkt = "512";
    m_currentMeasurementPoint = 0;
    calculateTotalMeasurementPoints();
    output("Multi-point measurement reset.\n");
    updateProgress();
    notifyUpdate();
}

void MultiMessWindowController::nextMeasurementPoint()
{
    if (m_currentMeasurementPoint < m_totalMeasurementPoints)
    {
        m_currentMeasurementPoint++;
        updateProgress();
    }
}

void MultiMessWindowController::executeMeasurement()
{
    output("Executing measurement batch...\n");
    if (m_currentMeasurementPoint == 0)
    {
        // Initial measurement setup
        output("Starting first measurement point.\n");
    }

    if (m_totalMeasurementPoints == 1)
    {
        output("Single measurement point detected.\n");
        // Single measurement logic
        Global::AdapterInstance.getMesurement();
    }
    else
    {
        output("Multi-point measurement in progress.\n");
        // Multi-point measurement logic
    }
}
