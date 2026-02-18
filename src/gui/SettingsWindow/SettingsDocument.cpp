#include "SettingsDocument.h"
#include "fkt_GPIB.h"
#include "cmdGpib.h"

#include <algorithm>

// ---------------------------------------------------------------------------
// Construction
// ---------------------------------------------------------------------------

SettingsDocument::SettingsDocument(PrologixUsbGpibAdapter& adapter)
    : m_adapter(adapter)
{
    m_adapter.connect();
    m_adapter.config();
}

// ---------------------------------------------------------------------------
// Observer management
// ---------------------------------------------------------------------------

void SettingsDocument::AddObserver(ISettingsObserver* observer)
{
    if (observer && std::find(m_observers.begin(), m_observers.end(), observer) == m_observers.end())
        m_observers.push_back(observer);
}

void SettingsDocument::RemoveObserver(ISettingsObserver* observer)
{
    m_observers.erase(std::remove(m_observers.begin(), m_observers.end(), observer), m_observers.end());
}

void SettingsDocument::NotifyObservers(const std::string& changeType)
{
    for (ISettingsObserver* obs : m_observers)
        if (obs)
            obs->OnDocumentChanged(changeType);
}

// ---------------------------------------------------------------------------
// State
// ---------------------------------------------------------------------------

bool SettingsDocument::IsConnected() const
{
    return m_adapter.getConnected();
}

void SettingsDocument::SetFreqMode(bool useStartEnd)
{
    m_useStartEnd = useStartEnd;
    NotifyObservers("FreqModeChanged");
}

// ---------------------------------------------------------------------------
// ApplySettings — write all display settings to the device
// ---------------------------------------------------------------------------

void SettingsDocument::ApplySettings(
    const std::string& freqStart,     const std::string& freqStartUnit,
    const std::string& freqEnd,       const std::string& freqEndUnit,
    const std::string& freqCenter,    const std::string& freqCenterUnit,
    const std::string& freqSpan,      const std::string& freqSpanUnit,
    const std::string& pegelValue,    const std::string& pegelUnit,
    const std::string& ySpacing,
    const std::string& refLevel,
    bool               useStartEnd
)
{
    // Store
    m_freqStart      = freqStart;
    m_freqStartUnit  = freqStartUnit;
    m_freqEnd        = freqEnd;
    m_freqEndUnit    = freqEndUnit;
    m_freqCenter     = freqCenter;
    m_freqCenterUnit = freqCenterUnit;
    m_freqSpan       = freqSpan;
    m_freqSpanUnit   = freqSpanUnit;
    m_pegelValue     = pegelValue;
    m_pegelUnit      = pegelUnit;
    m_ySpacing       = ySpacing;
    m_refLevel       = refLevel;
    m_useStartEnd    = useStartEnd;

    if (!m_adapter.getConnected())
        return;

    std::string cmd;

    // Frequency range
    if (m_useStartEnd)
    {
        cmd = ScpiCmdLookup.at(ScpiCmd::FREQ_STAR) + " " + m_freqStart + m_freqStartUnit;
        m_adapter.write(cmd);
        cmd = ScpiCmdLookup.at(ScpiCmd::FREQ_STOP) + " " + m_freqEnd + m_freqEndUnit;
        m_adapter.write(cmd);
    }
    else
    {
        cmd = ScpiCmdLookup.at(ScpiCmd::FREQ_CENT) + " " + m_freqCenter + m_freqCenterUnit;
        m_adapter.write(cmd);
        cmd = ScpiCmdLookup.at(ScpiCmd::FREQ_SPAN) + " " + m_freqSpan + m_freqSpanUnit;
        m_adapter.write(cmd);
    }

    // Level unit
    cmd = "UNIT:POW " + m_pegelUnit;
    m_adapter.write(cmd);

    // Reference level
    cmd = ScpiCmdLookup.at(ScpiCmd::DISP_TRAC_Y_RLEV) + " " + m_refLevel;
    m_adapter.write(cmd);

    // Y-axis spacing (LOG / LIN)
    cmd = "DISP:TRAC:Y:SPAC " + m_ySpacing;
    m_adapter.write(cmd);

    NotifyObservers("SettingsApplied");
}

// ---------------------------------------------------------------------------
// QueryFromDevice — read all display settings from the device
// ---------------------------------------------------------------------------

void SettingsDocument::QueryFromDevice()
{
    if (!m_adapter.getConnected())
        return;

    // Each query result stored in its correct member (fixes original scrambled-assignment bug)
    m_freqStart  = m_adapter.send(ScpiQueryCmdLookup.at(ScpiQueryCmd::FREQ_STAR));
    m_freqEnd    = m_adapter.send(ScpiQueryCmdLookup.at(ScpiQueryCmd::FREQ_STOP));
    m_freqCenter = m_adapter.send(ScpiQueryCmdLookup.at(ScpiQueryCmd::FREQ_CENT));
    m_freqSpan   = m_adapter.send(ScpiQueryCmdLookup.at(ScpiQueryCmd::FREQ_SPAN));
    m_pegelUnit  = m_adapter.send("CALCulate:UNIT:POWer?");
    m_refLevel   = m_adapter.send(ScpiQueryCmdLookup.at(ScpiQueryCmd::DISP_TRAC_Y_RLEV));
    m_ySpacing   = m_adapter.send("DISP:TRAC:Y:SPAC?");

    NotifyObservers("SettingsLoaded");
}
