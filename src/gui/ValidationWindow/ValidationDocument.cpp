#include "ValidationDocument.h"

#include "GpibUsbAdapter.h"
#include "FsuMeasurement.h"
#include "CsvManagement.h"
#include "dataManagement.h"
#include "systemInfo.h"

#include <algorithm>
#include <cmath>
#include <ctime>
#include <fstream>

namespace
{
    std::string validationTimestamp()
    {
        std::time_t currentTime = std::time(nullptr);
        char buffer[16] = {};
        std::strftime(buffer, sizeof(buffer), "[%H:%M:%S] ", std::localtime(&currentTime));
        return std::string(buffer);
    }

    std::string trimCopy(std::string text)
    {
        const size_t first = text.find_first_not_of(" \r\n\t");
        if (first == std::string::npos)
            return "";

        const size_t last = text.find_last_not_of(" \r\n\t");
        return text.substr(first, last - first + 1);
    }

    std::string singleLine(std::string text)
    {
        text.erase(std::remove(text.begin(), text.end(), '\r'), text.end());
        text.erase(std::remove(text.begin(), text.end(), '\n'), text.end());
        return text;
    }
}

ValidationDocument::ValidationDocument(PrologixUsbGpibAdapter& adapter)
    : m_adapter(adapter)
{
}

void ValidationDocument::AddObserver(IValidationObserver* observer)
{
    if (observer && std::find(m_observers.begin(), m_observers.end(), observer) == m_observers.end())
        m_observers.push_back(observer);
}

void ValidationDocument::RemoveObserver(IValidationObserver* observer)
{
    m_observers.erase(
        std::remove(m_observers.begin(), m_observers.end(), observer),
        m_observers.end());
}

void ValidationDocument::NotifyObservers(const std::string& changeType)
{
    for (IValidationObserver* observer : m_observers)
    {
        if (observer)
            observer->OnValidationDocumentChanged(changeType);
    }
}

void ValidationDocument::AppendToLog(const std::string& text)
{
    m_outputLog += validationTimestamp() + text;
}

bool ValidationDocument::AlmostEqual(double lhs, double rhs, double eps)
{
    return std::abs(lhs - rhs) <= eps;
}

bool ValidationDocument::EnsureAdapterReady()
{
    if (!m_adapter.checkIfAdapterAvailable())
    {
        AppendToLog("FAIL: Kein Prologix-Adapter erkannt.\n");
        return false;
    }

    if (!m_adapter.connect())
    {
        AppendToLog("FAIL: Verbindung zum Adapter fehlgeschlagen.\n");
        return false;
    }

    m_adapter.config();

    if (m_adapter.getStatus() != FT_OK)
    {
        AppendToLog("FAIL: Adapter konnte nicht konfiguriert werden.\n");
        return false;
    }

    return true;
}

void ValidationDocument::ClearOutput()
{
    m_outputLog.clear();
    NotifyObservers("OutputAppended");
}

bool ValidationDocument::ExportOutputLog(const std::string& filePath)
{
    std::ofstream output(filePath, std::ios::out | std::ios::trunc);
    if (!output.is_open())
        return false;

    output << m_outputLog;
    return output.good();
}

void ValidationDocument::RunAllValidations()
{
    AppendToLog("=== Validierung gestartet ===\n");
    ValidateAdapterPresence();
    ValidateAdapterStatus();
    ValidateGpibDevicePresence();
    ValidateDeviceStatus();
    ValidateScpiIdn();
    ValidateOpcQuery();
    ValidateSweepSettingsRoundtrip();
    ValidateCsvRoundtrip();
    AppendToLog("=== Validierung abgeschlossen ===\n");
    NotifyObservers("OutputAppended");
}

void ValidationDocument::ValidateAdapterPresence()
{
    AppendToLog("Pruefe Adaptererkennung...\n");

    const bool ok = m_adapter.checkIfAdapterAvailable();
    AppendToLog(ok
        ? "PASS: Adapter wurde erkannt.\n"
        : "FAIL: Adapter wurde nicht erkannt.\n");

    NotifyObservers("OutputAppended");
}

void ValidationDocument::ValidateAdapterStatus()
{
    AppendToLog("Lese Adapterstatus...\n");

    if (!EnsureAdapterReady())
    {
        NotifyObservers("OutputAppended");
        return;
    }

    AppendToLog("PASS: Adapterstatus -> " + singleLine(m_adapter.statusText()) + "\n");
    NotifyObservers("OutputAppended");
}

void ValidationDocument::ValidateGpibDevicePresence()
{
    AppendToLog("Pruefe GPIB-Geraeteerkennung...\n");

    const bool ok = m_adapter.checkIfGpibDeviceAvailable();
    AppendToLog(ok
        ? "PASS: GPIB-Geraet antwortet auf Polling.\n"
        : "FAIL: Kein GPIB-Geraet gefunden oder keine gueltige Polling-Antwort.\n");

    NotifyObservers("OutputAppended");
}

void ValidationDocument::ValidateDeviceStatus()
{
    AppendToLog("Pruefe Geraetestatus mit SYST:ERR?...\n");

    if (!EnsureAdapterReady())
    {
        NotifyObservers("OutputAppended");
        return;
    }

    const std::string response = singleLine(m_adapter.send("SYST:ERR?", 1000));
    const bool ok = !response.empty() && response.find("Failed") == std::string::npos;

    if (ok)
        AppendToLog("PASS: SYST:ERR? -> " + response + "\n");
    else
        AppendToLog("FAIL: Geraetestatus konnte nicht gelesen werden -> " + response + "\n");

    NotifyObservers("OutputAppended");
}

void ValidationDocument::ValidateScpiIdn()
{
    AppendToLog("Pruefe SCPI-Kommunikation mit *IDN?...\n");

    if (!EnsureAdapterReady())
    {
        NotifyObservers("OutputAppended");
        return;
    }

    const std::string response = singleLine(m_adapter.send("*IDN?", 1000));
    const bool ok = !response.empty()
        && response.find("Failed") == std::string::npos
        && std::count(response.begin(), response.end(), ',') >= 2;

    if (ok)
        AppendToLog("PASS: *IDN? -> " + response + "\n");
    else
        AppendToLog("FAIL: Ungueltige *IDN?-Antwort -> " + response + "\n");

    NotifyObservers("OutputAppended");
}

void ValidationDocument::ValidateOpcQuery()
{
    AppendToLog("Pruefe Synchronisation mit *OPC?...\n");

    if (!EnsureAdapterReady())
    {
        NotifyObservers("OutputAppended");
        return;
    }

    const std::string response = trimCopy(m_adapter.send("*OPC?", 5000));
    const bool ok = !response.empty()
        && response.find("Failed") == std::string::npos
        && response.find('1') != std::string::npos;

    if (ok)
        AppendToLog("PASS: *OPC? -> " + response + "\n");
    else
        AppendToLog("FAIL: *OPC? lieferte keine gueltige Abschlussmeldung -> " + response + "\n");

    NotifyObservers("OutputAppended");
}

void ValidationDocument::ValidateSweepSettingsRoundtrip()
{
    AppendToLog("Pruefe Sweep-Settings Roundtrip...\n");

    if (!EnsureAdapterReady())
    {
        NotifyObservers("OutputAppended");
        return;
    }

    auto& fsu = fsuMeasurement::get_instance();
    fsu.setMeasurementMode(MeasurementMode::SWEEP);

    if (!fsu.readSweepSettings())
    {
        AppendToLog("FAIL: Sweep-Einstellungen konnten nicht vom Geraet gelesen werden.\n");
        NotifyObservers("OutputAppended");
        return;
    }

    const auto before = fsu.returnSweepSettings();

    if (!fsu.writeSweepSettings(before))
    {
        AppendToLog("FAIL: Sweep-Einstellungen konnten nicht an das Geraet geschrieben werden.\n");
        NotifyObservers("OutputAppended");
        return;
    }

    if (!fsu.readSweepSettings())
    {
        AppendToLog("FAIL: Sweep-Einstellungen konnten nach dem Schreiben nicht erneut gelesen werden.\n");
        NotifyObservers("OutputAppended");
        return;
    }

    const auto after = fsu.returnSweepSettings();

    const bool ok =
        AlmostEqual(before.startFreq, after.startFreq) &&
        AlmostEqual(before.stopFreq, after.stopFreq) &&
        AlmostEqual(before.refLevel, after.refLevel) &&
        before.points == after.points &&
        before.att == after.att &&
        before.unit == after.unit &&
        before.rbw == after.rbw &&
        before.vbw == after.vbw &&
        before.sweepTime == after.sweepTime &&
        before.detector == after.detector;

    AppendToLog(ok
        ? "PASS: Sweep-Einstellungen konnten konsistent gelesen und verifiziert werden.\n"
        : "FAIL: Sweep-Einstellungen stimmen nach dem Roundtrip nicht ueberein.\n");

    NotifyObservers("OutputAppended");
}

void ValidationDocument::ValidateCsvRoundtrip()
{
    AppendToLog("Pruefe CSV-Roundtrip mit internen Testdaten...\n");

    sData source;
    source.setFileName("ValidationRoundtrip");
    source.setFileType("Line");
    source.setNumberOfPts_X(2);
    source.setNumberOfPts_Y(2);
    source.setTimeAndDate();

    auto& fsu = fsuMeasurement::get_instance();
    fsu.setMeasurementMode(MeasurementMode::MARKER_PEAK);
    source.importFsuSettings();

    for (int y = 0; y < source.getNumberOfPts_Y(); ++y)
    {
        for (int x = 0; x < source.getNumberOfPts_X(); ++x)
        {
            const double realValue = static_cast<double>((y * 10) + x + 1);
            const double imagValue = -realValue;

            if (!source.set3DDataReal(std::vector<double>{realValue}, x, y) ||
                !source.set3DDataImag(std::vector<double>{imagValue}, x, y))
            {
                AppendToLog("FAIL: Testdaten konnten nicht in die 3D-Struktur geschrieben werden.\n");
                NotifyObservers("OutputAppended");
                return;
            }
        }
    }

    wxString filePath = System::filePathRoot + "LogFiles" + System::fileSystemSlash + "ValidationRoundtrip";
    CsvFile writer(';');

    if (!writer.saveCsvFile(filePath, source, 0))
    {
        AppendToLog("FAIL: CSV-Datei konnte nicht geschrieben werden.\n");
        NotifyObservers("OutputAppended");
        return;
    }

    sData loaded;
    CsvFile reader(';');

    if (!reader.readCsvFile(filePath, loaded))
    {
        AppendToLog("FAIL: CSV-Datei konnte nicht erneut eingelesen werden.\n");
        NotifyObservers("OutputAppended");
        return;
    }

    const auto expectedReal = source.get3DDataReal(1, 1);
    const auto loadedReal = loaded.get3DDataReal(1, 1);
    const auto expectedImag = source.get3DDataImag(1, 1);
    const auto loadedImag = loaded.get3DDataImag(1, 1);

    bool ok =
        loaded.getNumberOfPts_X() == source.getNumberOfPts_X() &&
        loaded.getNumberOfPts_Y() == source.getNumberOfPts_Y() &&
        loaded.getNumberOfPts_Array() == source.getNumberOfPts_Array() &&
        expectedReal.size() == loadedReal.size() &&
        expectedImag.size() == loadedImag.size();

    if (ok)
    {
        for (size_t i = 0; i < expectedReal.size(); ++i)
        {
            if (!AlmostEqual(expectedReal[i], loadedReal[i]) ||
                !AlmostEqual(expectedImag[i], loadedImag[i]))
            {
                ok = false;
                break;
            }
        }
    }

    AppendToLog(ok
        ? "PASS: CSV-Roundtrip war konsistent.\n"
        : "FAIL: CSV-Roundtrip zeigte Abweichungen in Struktur oder Datenwerten.\n");

    NotifyObservers("OutputAppended");
}
