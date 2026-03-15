#pragma once

#include <string>
#include <vector>

#include "cmdGpib.h"
#include "FsuMeasurement.h"

class PrologixUsbGpibAdapter;

class IMSetObserver
{
public:
    virtual ~IMSetObserver() = default;
    virtual void OnDocumentChanged(const std::string& changeType) = 0;
};

class MSetDocument
{
public:
    struct OperationResult
    {
        bool success = false;
        bool startEnabled = false;
        std::string message;
    };

    MSetDocument(PrologixUsbGpibAdapter& adapter, fsuMeasurement& fsu, MeasurementMode mode);

    void AddObserver(IMSetObserver* observer);
    void RemoveObserver(IMSetObserver* observer);

    MeasurementMode GetMode() const { return m_mode; }
    void SetMode(MeasurementMode mode);

    const fsuMeasurement::lastSweepSettings& GetSweepSettings() const { return m_sweep; }
    const fsuMeasurement::IqSettings& GetIqSettings() const { return m_iq; }
    const fsuMeasurement::MarkerPeakSettings& GetMarkerPeakSettings() const { return m_marker; }

    const OperationResult& GetLastResult() const { return m_lastResult; }

    bool ApplySweep(const fsuMeasurement::lastSweepSettings& settings);
    bool ApplyIq(const fsuMeasurement::IqSettings& settings);
    bool ApplyMarkerPeak(const fsuMeasurement::MarkerPeakSettings& settings);

    bool ReadCurrent();

private:
    void NotifyObservers(const std::string& changeType);

    bool VerifyDouble(const std::string& name, double written, double readback, std::string& mismatches) const;
    bool VerifyInt(const std::string& name, int written, int readback, std::string& mismatches) const;
    bool VerifyString(const std::string& name, const std::string& written, const std::string& readback, std::string& mismatches) const;

    void SetResult(bool success, bool startEnabled, const std::string& message);
    void RefreshCachedSettings();

    PrologixUsbGpibAdapter& m_adapter;
    fsuMeasurement& m_fsu;
    MeasurementMode m_mode;

    fsuMeasurement::lastSweepSettings m_sweep;
    fsuMeasurement::IqSettings m_iq;
    fsuMeasurement::MarkerPeakSettings m_marker;

    OperationResult m_lastResult;
    std::vector<IMSetObserver*> m_observers;
};
