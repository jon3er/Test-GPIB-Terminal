
#include "MultiMessDocument.h"
#include "fkt_GPIB.h"
#include "systemInfo.h"

#include <iostream>
#include <algorithm>

// ---------------------------------------------------------------------------
// Construction / Destruction
// ---------------------------------------------------------------------------

MultiMessDocument::MultiMessDocument(PrologixUsbGpibAdapter& adapter)
    : m_adapter(adapter)
{
    RecalculateTotalPoints();
}

MultiMessDocument::~MultiMessDocument()
{
}

// ---------------------------------------------------------------------------
// Observer management
// ---------------------------------------------------------------------------

void MultiMessDocument::AddObserver(IMultiMessObserver* observer)
{
    if (observer &&
        std::find(m_observers.begin(), m_observers.end(), observer) == m_observers.end())
    {
        m_observers.push_back(observer);
    }
}

void MultiMessDocument::RemoveObserver(IMultiMessObserver* observer)
{
    m_observers.erase(
        std::remove(m_observers.begin(), m_observers.end(), observer),
        m_observers.end());
}

void MultiMessDocument::NotifyObservers(const std::string& changeType)
{
    for (IMultiMessObserver* obs : m_observers)
    {
        if (obs)
            obs->OnMultiMessDocumentChanged(changeType);
    }
}

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

void MultiMessDocument::RecalculateTotalPoints()
{
    try
    {
        int x = std::stoi(m_X_Messpunkte);
        int y = std::stoi(m_Y_Messpunkte);
        m_totalPoints = static_cast<unsigned int>(x * y);
        if (m_totalPoints == 0)
            m_totalPoints = 1;
    }
    catch (...)
    {
        m_totalPoints = 1;
    }
}

// ---------------------------------------------------------------------------
// Configuration setters
// ---------------------------------------------------------------------------

void MultiMessDocument::SetXMeasurementPoints(const std::string& value)
{
    m_X_Messpunkte = value;
    RecalculateTotalPoints();
    NotifyObservers("ConfigChanged");
}

void MultiMessDocument::SetYMeasurementPoints(const std::string& value)
{
    m_Y_Messpunkte = value;
    RecalculateTotalPoints();
    NotifyObservers("ConfigChanged");
}

void MultiMessDocument::SetXStartCoordinate(const std::string& value)
{
    m_X_Cord = value;
    NotifyObservers("ConfigChanged");
}

void MultiMessDocument::SetYStartCoordinate(const std::string& value)
{
    m_Y_Cord = value;
    NotifyObservers("ConfigChanged");
}

void MultiMessDocument::SetXMeasurementSpacing(const std::string& value)
{
    m_X_MessAbstand = value;
    NotifyObservers("ConfigChanged");
}

void MultiMessDocument::SetYMeasurementSpacing(const std::string& value)
{
    m_Y_MessAbstand = value;
    NotifyObservers("ConfigChanged");
}

void MultiMessDocument::SetStartFrequency(const std::string& value)
{
    m_startFreq = value;
    NotifyObservers("ConfigChanged");
}

void MultiMessDocument::SetStartFrequencyUnit(const std::string& value)
{
    m_startFreqUnit = value;
    NotifyObservers("ConfigChanged");
}

void MultiMessDocument::SetStopFrequency(const std::string& value)
{
    m_stopFreq = value;
    NotifyObservers("ConfigChanged");
}

void MultiMessDocument::SetStopFrequencyUnit(const std::string& value)
{
    m_stopFreqUnit = value;
    NotifyObservers("ConfigChanged");
}

void MultiMessDocument::SetNumberOfSweepPoints(const std::string& value)
{
    m_AnzSweepMessPkt = value;
    NotifyObservers("ConfigChanged");
}

// ---------------------------------------------------------------------------
// Commands
// ---------------------------------------------------------------------------

void MultiMessDocument::Start()
{
    RecalculateTotalPoints();
    m_currentPoint = 0;
    NotifyObservers("Started");
    NotifyObservers("ProgressChanged");
}

void MultiMessDocument::Stop()
{
    m_currentPoint = 0;
    NotifyObservers("Stopped");
    NotifyObservers("ProgressChanged");
}

void MultiMessDocument::Reset()
{
    m_X_Messpunkte    = "1";
    m_Y_Messpunkte    = "1";
    m_X_Cord          = "0";
    m_Y_Cord          = "0";
    m_X_MessAbstand   = "10";
    m_Y_MessAbstand   = "10";
    m_startFreq       = "50";
    m_startFreqUnit   = "kHz";
    m_stopFreq        = "100";
    m_stopFreqUnit    = "kHz";
    m_AnzSweepMessPkt = "512";
    m_currentPoint    = 0;
    m_totalPoints     = 1;
    NotifyObservers("Reset");
    NotifyObservers("ProgressChanged");
}

void MultiMessDocument::NextPoint()
{
    if (m_currentPoint < m_totalPoints)
    {
        m_currentPoint++;
        NotifyObservers("ProgressChanged");
    }
}

void MultiMessDocument::ExecuteMeasurement()
{
    if (m_totalPoints == 1)
    {
        m_adapter.getMesurement();
        std::cerr << "MultiMessDocument: single-point measurement executed" << std::endl;
    }
    else
    {
        std::cerr << "MultiMessDocument: multi-point measurement not yet implemented" << std::endl;
    }
}
