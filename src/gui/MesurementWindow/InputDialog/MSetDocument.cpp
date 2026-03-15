#include "MSetDocument.h"

#include <algorithm>
#include <cmath>
#include <sstream>

#include "fkt_d2xx.h"

MSetDocument::MSetDocument(PrologixUsbGpibAdapter& adapter, fsuMeasurement& fsu, MeasurementMode mode)
    : m_adapter(adapter)
    , m_fsu(fsu)
    , m_mode(mode)
{
    RefreshCachedSettings();
}

void MSetDocument::AddObserver(IMSetObserver* observer)
{
    if (observer && std::find(m_observers.begin(), m_observers.end(), observer) == m_observers.end())
        m_observers.push_back(observer);
}

void MSetDocument::RemoveObserver(IMSetObserver* observer)
{
    m_observers.erase(std::remove(m_observers.begin(), m_observers.end(), observer), m_observers.end());
}

void MSetDocument::NotifyObservers(const std::string& changeType)
{
    for (IMSetObserver* obs : m_observers)
    {
        if (obs)
            obs->OnDocumentChanged(changeType);
    }
}

void MSetDocument::SetMode(MeasurementMode mode)
{
    m_mode = mode;
    NotifyObservers("ModeChanged");
}

void MSetDocument::SetResult(bool success, bool startEnabled, const std::string& message)
{
    m_lastResult.success = success;
    m_lastResult.startEnabled = startEnabled;
    m_lastResult.message = message;
    NotifyObservers("ApplyResult");
}

bool MSetDocument::VerifyDouble(const std::string& name, double written, double readback, std::string& mismatches) const
{
    double tol = std::max(std::abs(written) * 0.01, 1.0);
    if (std::abs(written - readback) > tol)
    {
        std::ostringstream oss;
        oss << "  " << name << ": gesendet=" << written << ", gelesen=" << readback << "\n";
        mismatches += oss.str();
        return false;
    }
    return true;
}

bool MSetDocument::VerifyInt(const std::string& name, int written, int readback, std::string& mismatches) const
{
    if (written != readback)
    {
        std::ostringstream oss;
        oss << "  " << name << ": gesendet=" << written << ", gelesen=" << readback << "\n";
        mismatches += oss.str();
        return false;
    }
    return true;
}

bool MSetDocument::VerifyString(const std::string& name, const std::string& written, const std::string& readback, std::string& mismatches) const
{
    if (written != readback)
    {
        std::ostringstream oss;
        oss << "  " << name << ": gesendet=" << written << ", gelesen=" << readback << "\n";
        mismatches += oss.str();
        return false;
    }
    return true;
}

void MSetDocument::RefreshCachedSettings()
{
    m_sweep = m_fsu.returnSweepSettings();
    m_iq = m_fsu.returnIqSettings();
    m_marker = m_fsu.returnMarkerPeakSettings();
}

bool MSetDocument::ApplySweep(const fsuMeasurement::lastSweepSettings& settings)
{
    m_fsu.setMeasurementMode(MeasurementMode::SWEEP);

    if (!m_fsu.checkIfSettingsValidSweep(ScpiCommand::START_FREQUENCY, settings.startFreq))
    {
        SetResult(false, false, "Startfrequenz ausserhalb des Bereichs (0 - 26.5 GHz)!");
        return false;
    }
    if (!m_fsu.checkIfSettingsValidSweep(ScpiCommand::END_FREQUENCY, settings.stopFreq))
    {
        SetResult(false, false, "Stopfrequenz ausserhalb des Bereichs (0 - 26.5 GHz)!");
        return false;
    }
    if (!m_fsu.checkIfSettingsValidSweep(ScpiCommand::RF_ATTENUATION, settings.att))
    {
        SetResult(false, false, "Daempfung muss in 5-dB Schritten erfolgen (0-75 dB)!");
        return false;
    }
    if (!m_fsu.checkIfSettingsValidSweep(ScpiCommand::RBW, static_cast<double>(settings.rbw)))
    {
        SetResult(false, false, "RBW ausserhalb des gueltigen Bereichs (1 Hz - 50 MHz)!");
        return false;
    }
    if (!m_fsu.checkIfSettingsValidSweep(ScpiCommand::VBW, static_cast<double>(settings.vbw)))
    {
        SetResult(false, false, "VBW ausserhalb des gueltigen Bereichs (1 Hz - 50 MHz)!");
        return false;
    }
    if (!m_fsu.checkIfSettingsValidSweep(ScpiCommand::SWEEP_POINTS, settings.points))
    {
        SetResult(false, false, "Ungueltige Sweep Punkte!");
        return false;
    }

    if (!m_fsu.writeSweepSettings(settings))
    {
        SetResult(false, false, "Fehler beim Senden der Einstellungen!");
        return false;
    }

    if (!m_fsu.readSweepSettings())
    {
        SetResult(false, false, "Einstellungen gesendet, aber Ruecklesen fehlgeschlagen!");
        return false;
    }

    m_sweep = m_fsu.returnSweepSettings();

    std::string mismatches;
    bool ok = true;
    ok &= VerifyDouble("Start Frequenz", settings.startFreq, m_sweep.startFreq, mismatches);
    ok &= VerifyDouble("Stop Frequenz", settings.stopFreq, m_sweep.stopFreq, mismatches);
    ok &= VerifyDouble("Ref. Pegel", settings.refLevel, m_sweep.refLevel, mismatches);
    ok &= VerifyInt("Daempfung", settings.att, m_sweep.att, mismatches);
    ok &= VerifyInt("RBW", settings.rbw, m_sweep.rbw, mismatches);
    ok &= VerifyInt("VBW", settings.vbw, m_sweep.vbw, mismatches);
    ok &= VerifyInt("Sweep Punkte", settings.points, m_sweep.points, mismatches);
    ok &= VerifyString("Detektor", settings.detector, m_sweep.detector, mismatches);
    ok &= VerifyString("Einheit", settings.unit, m_sweep.unit, mismatches);

    NotifyObservers("SettingsChanged");

    if (ok)
    {
        SetResult(true, true, "Einstellungen erfolgreich uebernommen und verifiziert!");
        return true;
    }

    SetResult(false, false, "Abweichungen festgestellt:\n\n" + mismatches);
    return false;
}

bool MSetDocument::ApplyIq(const fsuMeasurement::IqSettings& settings)
{
    m_fsu.setMeasurementMode(MeasurementMode::IQ);

    if (!m_fsu.checkIfSettingsValidSweep(ScpiCommand::CENTER_FREQUENCY, settings.centerFreq))
    {
        SetResult(false, false, "Center Frequenz ausserhalb des Bereichs (0 - 26.5 GHz)!");
        return false;
    }
    if (!m_fsu.checkIfSettingsValidSweep(ScpiCommand::RF_ATTENUATION, settings.att))
    {
        SetResult(false, false, "Daempfung muss in 5-dB Schritten erfolgen (0-75 dB)!");
        return false;
    }
    if (!m_fsu.checkIfSettingsValidSweep(ScpiCommand::IQ_SAMPLE_RATE, settings.sampleRate))
    {
        SetResult(false, false, "Sample Rate ausserhalb des Bereichs (10 kHz - 70.4 MHz)!");
        return false;
    }
    if (!m_fsu.checkIfSettingsValidSweep(ScpiCommand::IQ_RECORD_LENGTH, settings.recordLength))
    {
        SetResult(false, false, "Record Length ausserhalb des Bereichs (1 - 16M)!");
        return false;
    }
    if (settings.pretriggerSamples < 0 || settings.pretriggerSamples > settings.recordLength)
    {
        SetResult(false, false, "Pretrigger Samples muss zwischen 0 und Record Length liegen!");
        return false;
    }
    if (!m_fsu.checkIfSettingsValidSweep(ScpiCommand::IQ_IF_BANDWIDTH, settings.ifBandwidth))
    {
        SetResult(false, false, "IF Bandwidth ausserhalb des Bereichs (10 Hz - 50 MHz)!");
        return false;
    }
    if (!m_fsu.checkIfSettingsValidSweep(ScpiCommand::TRIGGER_SOURCE, settings.triggerSource))
    {
        SetResult(false, false, "Ungueltige Trigger Quelle!");
        return false;
    }
    if (!m_fsu.checkIfSettingsValidSweep(ScpiCommand::TRIGGER_LEVEL, settings.triggerLevel))
    {
        SetResult(false, false, "Trigger Level ausserhalb des Bereichs (-130 - +30 dBm)!");
        return false;
    }
    if (!m_fsu.checkIfSettingsValidSweep(ScpiCommand::TRIGGER_DELAY, settings.triggerDelay))
    {
        SetResult(false, false, "Trigger Delay ausserhalb des Bereichs (-1 - 65 s)!");
        return false;
    }

    if (!m_fsu.writeIqSettings(settings))
    {
        SetResult(false, false, "Fehler beim Senden der IQ-Einstellungen!");
        return false;
    }

    if (!m_fsu.readIqSettings())
    {
        SetResult(false, false, "IQ-Einstellungen gesendet, aber Ruecklesen fehlgeschlagen!");
        return false;
    }

    m_iq = m_fsu.returnIqSettings();

    std::string mismatches;
    bool ok = true;
    ok &= VerifyDouble("Center Frequenz", settings.centerFreq, m_iq.centerFreq, mismatches);
    ok &= VerifyDouble("Ref. Pegel", settings.refLevel, m_iq.refLevel, mismatches);
    ok &= VerifyInt("Daempfung", settings.att, m_iq.att, mismatches);
    ok &= VerifyString("Einheit", settings.unit, m_iq.unit, mismatches);
    ok &= VerifyDouble("Sample Rate", settings.sampleRate, m_iq.sampleRate, mismatches);
    ok &= VerifyInt("Record Length", settings.recordLength, m_iq.recordLength, mismatches);
    ok &= VerifyDouble("IF Bandwidth", settings.ifBandwidth, m_iq.ifBandwidth, mismatches);
    ok &= VerifyString("Trigger Quelle", settings.triggerSource, m_iq.triggerSource, mismatches);
    ok &= VerifyString("Filter Type", settings.filterType, m_iq.filterType, mismatches);
    ok &= VerifyString("Trigger Slope", settings.triggerSlope, m_iq.triggerSlope, mismatches);
    ok &= VerifyInt("Pretrigger Samples", settings.pretriggerSamples, m_iq.pretriggerSamples, mismatches);
    ok &= VerifyDouble("Trigger Level", settings.triggerLevel, m_iq.triggerLevel, mismatches);
    ok &= VerifyDouble("Trigger Delay", settings.triggerDelay, m_iq.triggerDelay, mismatches);

    NotifyObservers("SettingsChanged");

    if (ok)
    {
        SetResult(true, true, "IQ-Einstellungen erfolgreich uebernommen und verifiziert!");
        return true;
    }

    SetResult(false, false, "Abweichungen festgestellt:\n\n" + mismatches);
    return false;
}

bool MSetDocument::ApplyMarkerPeak(const fsuMeasurement::MarkerPeakSettings& settings)
{
    m_fsu.setMeasurementMode(MeasurementMode::MARKER_PEAK);

    if (!m_fsu.checkIfSettingsValidSweep(ScpiCommand::START_FREQUENCY, settings.startFreq))
    {
        SetResult(false, false, "Startfrequenz ausserhalb des Bereichs (0 - 26.5 GHz)!");
        return false;
    }
    if (!m_fsu.checkIfSettingsValidSweep(ScpiCommand::END_FREQUENCY, settings.stopFreq))
    {
        SetResult(false, false, "Stopfrequenz ausserhalb des Bereichs (0 - 26.5 GHz)!");
        return false;
    }
    if (!m_fsu.checkIfSettingsValidSweep(ScpiCommand::RF_ATTENUATION, settings.att))
    {
        SetResult(false, false, "Daempfung muss in 5-dB Schritten erfolgen (0-75 dB)!");
        return false;
    }
    if (!m_fsu.checkIfSettingsValidSweep(ScpiCommand::RBW, static_cast<double>(settings.rbw)))
    {
        SetResult(false, false, "RBW ausserhalb des gueltigen Bereichs (1 Hz - 50 MHz)!");
        return false;
    }
    if (!m_fsu.checkIfSettingsValidSweep(ScpiCommand::VBW, static_cast<double>(settings.vbw)))
    {
        SetResult(false, false, "VBW ausserhalb des gueltigen Bereichs (1 Hz - 50 MHz)!");
        return false;
    }

    if (!m_fsu.writeMarkerPeakSettings(settings))
    {
        SetResult(false, false, "Fehler beim Senden der MarkerPeak-Einstellungen!");
        return false;
    }

    if (!m_fsu.readMarkerPeakSettings())
    {
        SetResult(false, false, "MarkerPeak-Einstellungen gesendet, aber Ruecklesen fehlgeschlagen!");
        return false;
    }

    m_marker = m_fsu.returnMarkerPeakSettings();

    std::string mismatches;
    bool ok = true;
    ok &= VerifyDouble("Start Frequenz", settings.startFreq, m_marker.startFreq, mismatches);
    ok &= VerifyDouble("Stop Frequenz", settings.stopFreq, m_marker.stopFreq, mismatches);
    ok &= VerifyDouble("Ref. Pegel", settings.refLevel, m_marker.refLevel, mismatches);
    ok &= VerifyInt("Daempfung", settings.att, m_marker.att, mismatches);
    ok &= VerifyString("Einheit", settings.unit, m_marker.unit, mismatches);
    ok &= VerifyDouble("RBW", static_cast<double>(settings.rbw), static_cast<double>(m_marker.rbw), mismatches);
    ok &= VerifyDouble("VBW", static_cast<double>(settings.vbw), static_cast<double>(m_marker.vbw), mismatches);
    ok &= VerifyString("Detektor", settings.detector, m_marker.detector, mismatches);

    NotifyObservers("SettingsChanged");

    if (ok)
    {
        SetResult(true, true, "MarkerPeak-Einstellungen erfolgreich uebernommen und verifiziert!");
        return true;
    }

    SetResult(false, false, "Abweichungen festgestellt:\n\n" + mismatches);
    return false;
}

bool MSetDocument::ReadCurrent()
{
    m_fsu.setMeasurementMode(m_mode);

    if (!m_fsu.readSettingsFromGpib())
    {
        SetResult(false, false, "Failed to read settings from instrument.");
        return false;
    }

    RefreshCachedSettings();
    NotifyObservers("SettingsChanged");
    SetResult(true, true, "Einstellungen vom Messgeraet geladen.");
    return true;
}
