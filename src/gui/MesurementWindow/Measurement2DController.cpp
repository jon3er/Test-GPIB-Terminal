#include "Measurement2DController.h"
#include "systemInfo.h"
#include <iostream>

Measurement2DController::Measurement2DController()
{
    calculateTotalMeasurements();
}

Measurement2DController::~Measurement2DController()
{
}

void Measurement2DController::setOutputCallback(OutputCallback callback)
{
    m_outputCallback = callback;
}

void Measurement2DController::setProgressCallback(ProgressCallback callback)
{
    m_progressCallback = callback;
}

void Measurement2DController::setUpdateCallback(UpdateCallback callback)
{
    m_updateCallback = callback;
}

void Measurement2DController::output(const std::string& text)
{
    if (m_outputCallback != nullptr)
    {
        m_outputCallback(text);
    }
}

void Measurement2DController::updateProgress()
{
    if (m_progressCallback != nullptr)
    {
        m_progressCallback(m_currentMeasurementPoint, m_totalMeasurementPoints);
    }
}

void Measurement2DController::notifyUpdate()
{
    if (m_updateCallback != nullptr)
    {
        m_updateCallback();
    }
}

void Measurement2DController::calculateTotalMeasurements()
{
    m_totalMeasurementPoints = m_sliderX * m_sliderY;
}

void Measurement2DController::setYPoints(int points)
{
    m_sliderY = points;
    calculateTotalMeasurements();
    notifyUpdate();
}

void Measurement2DController::setXPoints(int points)
{
    m_sliderX = points;
    calculateTotalMeasurements();
    notifyUpdate();
}

void Measurement2DController::setScale(int scale)
{
    m_sliderScale = scale;
    notifyUpdate();
}

void Measurement2DController::setOrientationPoint(int point)
{
    m_orientationPoint = point;
    notifyUpdate();
}

void Measurement2DController::reset()
{
    m_sliderY = 10;
    m_sliderX = 10;
    m_sliderScale = 100;
    m_orientationPoint = 0;
    m_currentMeasurementPoint = 0;
    calculateTotalMeasurements();
    output("2D measurement reset.\n");
    notifyUpdate();
}

void Measurement2DController::start()
{
    output("2D measurement started.\n");
    m_currentMeasurementPoint = 0;
    updateProgress();
}

void Measurement2DController::stop()
{
    output("2D measurement stopped.\n");
    m_currentMeasurementPoint = 0;
    updateProgress();
}

void Measurement2DController::incrementMeasurementPoint()
{
    if (m_currentMeasurementPoint < m_totalMeasurementPoints)
    {
        m_currentMeasurementPoint++;
        updateProgress();
    }
}

void Measurement2DController::performSingleMeasurement()
{
    output("Performing single measurement point...\n");
    // Logic for single measurement execution will go here
}
