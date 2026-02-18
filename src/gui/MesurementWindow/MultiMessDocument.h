#pragma once

#include <string>
#include <vector>
#include <functional>

// Forward declaration — keep this header wxWidgets-free
class PrologixUsbGpibAdapter;

/**
 * @brief Observer interface for MultiMessDocument.
 *
 * Change types emitted:
 *   "ConfigChanged"   — any measurement configuration parameter was updated
 *   "ProgressChanged" — current/total measurement point counters changed
 *   "Started"         — measurement sequence started
 *   "Stopped"         — measurement sequence stopped
 *   "Reset"           — configuration and progress reset to defaults
 */
class IMultiMessObserver
{
public:
    virtual ~IMultiMessObserver() = default;
    virtual void OnMultiMessDocumentChanged(const std::string& changeType) = 0;
};

/**
 * @brief Document in the Document/View pattern for MultiMessWindow.
 *
 * Owns:
 *  - Injected reference to the hardware adapter (not owned)
 *  - All multi-measurement configuration state
 *  - Progress counters (current / total measurement points)
 *
 * The document is wxWidgets-free and fully testable in isolation.
 */
class MultiMessDocument
{
public:
    explicit MultiMessDocument(PrologixUsbGpibAdapter& adapter);
    ~MultiMessDocument();

    // ---- Observer management ----
    void AddObserver(IMultiMessObserver* observer);
    void RemoveObserver(IMultiMessObserver* observer);

    // ---- Configuration setters (each notifies "ConfigChanged") ----
    void SetXMeasurementPoints(const std::string& value);
    void SetYMeasurementPoints(const std::string& value);
    void SetXStartCoordinate(const std::string& value);
    void SetYStartCoordinate(const std::string& value);
    void SetXMeasurementSpacing(const std::string& value);
    void SetYMeasurementSpacing(const std::string& value);
    void SetStartFrequency(const std::string& value);
    void SetStartFrequencyUnit(const std::string& value);
    void SetStopFrequency(const std::string& value);
    void SetStopFrequencyUnit(const std::string& value);
    void SetNumberOfSweepPoints(const std::string& value);

    // ---- Configuration read-only getters ----
    std::string GetXMeasurementPoints()  const { return m_X_Messpunkte; }
    std::string GetYMeasurementPoints()  const { return m_Y_Messpunkte; }
    std::string GetXStartCoordinate()    const { return m_X_Cord; }
    std::string GetYStartCoordinate()    const { return m_Y_Cord; }
    std::string GetXMeasurementSpacing() const { return m_X_MessAbstand; }
    std::string GetYMeasurementSpacing() const { return m_Y_MessAbstand; }
    std::string GetStartFrequency()      const { return m_startFreq; }
    std::string GetStartFrequencyUnit()  const { return m_startFreqUnit; }
    std::string GetStopFrequency()       const { return m_stopFreq; }
    std::string GetStopFrequencyUnit()   const { return m_stopFreqUnit; }
    std::string GetNumberOfSweepPoints() const { return m_AnzSweepMessPkt; }

    // ---- Progress accessors ----
    unsigned int GetCurrentPoint() const { return m_currentPoint; }
    unsigned int GetTotalPoints()  const { return m_totalPoints; }

    // ---- Commands ----
    void Start();
    void Stop();
    void Reset();
    void NextPoint();

    /**
     * Execute one measurement step via the hardware adapter.
     */
    void ExecuteMeasurement();

private:
    void NotifyObservers(const std::string& changeType);
    void RecalculateTotalPoints();

    // ---- Injected dependency ----
    PrologixUsbGpibAdapter& m_adapter;

    // ---- Configuration state ----
    std::string m_X_Messpunkte   = "1";
    std::string m_Y_Messpunkte   = "1";
    std::string m_X_Cord         = "0";
    std::string m_Y_Cord         = "0";
    std::string m_X_MessAbstand  = "10";
    std::string m_Y_MessAbstand  = "10";
    std::string m_startFreq      = "50";
    std::string m_startFreqUnit  = "kHz";
    std::string m_stopFreq       = "100";
    std::string m_stopFreqUnit   = "kHz";
    std::string m_AnzSweepMessPkt = "512";

    // ---- Progress state ----
    unsigned int m_currentPoint = 0;
    unsigned int m_totalPoints  = 1;

    std::vector<IMultiMessObserver*> m_observers;
};
