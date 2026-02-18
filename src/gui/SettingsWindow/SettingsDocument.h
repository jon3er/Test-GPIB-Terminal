#pragma once

#include <string>
#include <vector>

// Forward declaration — no wxWidgets in the document
class PrologixUsbGpibAdapter;

/**
 * @brief Observer interface for SettingsDocument changes.
 *
 * Change types:
 *   "SettingsApplied"   — settings were written to the device
 *   "SettingsLoaded"    — settings were read from the device
 *   "FreqModeChanged"   — freq mode toggled (Start/End <-> Center/Span)
 */
class ISettingsObserver
{
public:
    virtual ~ISettingsObserver() = default;
    virtual void OnDocumentChanged(const std::string& changeType) = 0;
};

/**
 * @brief Document in the Document/View pattern for the Settings window.
 *
 * Responsibilities:
 *  - Own all display-settings state (frequencies, level, scaling)
 *  - Execute all GPIB hardware commands via the injected adapter reference
 *  - Notify registered observers on every state change
 *
 * This class is pure C++ — it has NO wxWidgets dependency.
 */
class SettingsDocument
{
public:
    explicit SettingsDocument(PrologixUsbGpibAdapter& adapter);

    // ---- Observer management ----
    void AddObserver(ISettingsObserver* observer);
    void RemoveObserver(ISettingsObserver* observer);

    // ---- Read-only state accessors ----
    const std::string& GetFreqStart()     const { return m_freqStart; }
    const std::string& GetFreqStartUnit() const { return m_freqStartUnit; }
    const std::string& GetFreqEnd()       const { return m_freqEnd; }
    const std::string& GetFreqEndUnit()   const { return m_freqEndUnit; }
    const std::string& GetFreqCenter()    const { return m_freqCenter; }
    const std::string& GetFreqCenterUnit()const { return m_freqCenterUnit; }
    const std::string& GetFreqSpan()      const { return m_freqSpan; }
    const std::string& GetFreqSpanUnit()  const { return m_freqSpanUnit; }
    const std::string& GetPegelValue()    const { return m_pegelValue; }
    const std::string& GetPegelUnit()     const { return m_pegelUnit; }
    const std::string& GetYSpacing()      const { return m_ySpacing; }
    const std::string& GetRefLevel()      const { return m_refLevel; }
    bool               IsUsingStartEnd()  const { return m_useStartEnd; }
    bool               IsConnected()      const;

    // ---- Command interface (called by the view's event handlers) ----

    /**
     * Store all provided values and write them to the device via SCPI commands.
     * Notifies observers with "SettingsApplied".
     */
    void ApplySettings(
        const std::string& freqStart,     const std::string& freqStartUnit,
        const std::string& freqEnd,       const std::string& freqEndUnit,
        const std::string& freqCenter,    const std::string& freqCenterUnit,
        const std::string& freqSpan,      const std::string& freqSpanUnit,
        const std::string& pegelValue,    const std::string& pegelUnit,
        const std::string& ySpacing,
        const std::string& refLevel,
        bool               useStartEnd
    );

    /**
     * Query all current settings from the device and store them.
     * Notifies observers with "SettingsLoaded".
     */
    void QueryFromDevice();

    /**
     * Toggle the frequency input mode.
     * Notifies observers with "FreqModeChanged".
     */
    void SetFreqMode(bool useStartEnd);

private:
    void NotifyObservers(const std::string& changeType);

    // ---- Hardware ----
    PrologixUsbGpibAdapter& m_adapter;

    // ---- State ----
    std::string m_freqStart      = "75";
    std::string m_freqStartUnit  = "MHz";
    std::string m_freqEnd        = "125";
    std::string m_freqEndUnit    = "MHz";
    std::string m_freqCenter     = "100";
    std::string m_freqCenterUnit = "MHz";
    std::string m_freqSpan       = "50";
    std::string m_freqSpanUnit   = "MHz";
    std::string m_pegelValue     = "100";
    std::string m_pegelUnit      = "DBM";
    std::string m_ySpacing       = "Logarithmic";
    std::string m_refLevel       = "-20";
    bool        m_useStartEnd    = true;

    std::vector<ISettingsObserver*> m_observers;
};


