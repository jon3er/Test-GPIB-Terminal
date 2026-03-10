#include "SweepSetDialog.h"
#include "fkt_GPIB.h"
#include "cmdGpib.h"
#include <wx/valnum.h>
#include <cmath>

wxBEGIN_EVENT_TABLE(SettingsDialog, wxDialog)
    EVT_BUTTON(wxID_APPLY, SettingsDialog::OnApply)
wxEND_EVENT_TABLE()

SettingsDialog::SettingsDialog(wxWindow* parent, MeasurementMode mode)
    : wxDialog(parent, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(420, 520)),
      m_mode(mode)
{
    // Titel je nach Modus
    switch (m_mode) {
        case MeasurementMode::SWEEP:       SetTitle("Sweep Einstellungen");       break;
        case MeasurementMode::IQ:          SetTitle("IQ Analyse Einstellungen");  break;
        case MeasurementMode::MARKER_PEAK: SetTitle("Marker Peak Einstellungen"); break;
    }

    wxFlexGridSizer* grid = new wxFlexGridSizer(2, wxSize(10, 10));
    grid->AddGrowableCol(1);
    wxFloatingPointValidator<double> floatVal;

    // ---- Gemeinsame Felder (alle Modi) ----
    grid->Add(new wxStaticText(this, wxID_ANY, "Ref. Pegel (dBm):"), 0, wxALIGN_CENTER_VERTICAL);
    m_txtRefLevel = new wxTextCtrl(this, wxID_ANY, "0", wxDefaultPosition, wxDefaultSize, 0, floatVal);
    grid->Add(m_txtRefLevel, 1, wxEXPAND);

    grid->Add(new wxStaticText(this, wxID_ANY, "HF-Daempfung (dB):"), 0, wxALIGN_CENTER_VERTICAL);
    m_spinAttenuation = new wxSpinCtrl(this, wxID_ANY);
    m_spinAttenuation->SetRange(0, 75);
    m_spinAttenuation->SetIncrement(5);
    grid->Add(m_spinAttenuation, 1, wxEXPAND);

    grid->Add(new wxStaticText(this, wxID_ANY, "Einheit:"), 0, wxALIGN_CENTER_VERTICAL);
    wxArrayString units = {"DBM", "V", "W", "DBUV"};
    m_choiceUnit = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, units);
    m_choiceUnit->SetSelection(0);
    grid->Add(m_choiceUnit, 1, wxEXPAND);

    // ---- Sweep + MarkerPeak Felder ----
    if (m_mode == MeasurementMode::SWEEP || m_mode == MeasurementMode::MARKER_PEAK) {
        grid->Add(new wxStaticText(this, wxID_ANY, "Start Frequenz (Hz):"), 0, wxALIGN_CENTER_VERTICAL);
        m_txtStartFreq = new wxTextCtrl(this, wxID_ANY, "1000000", wxDefaultPosition, wxDefaultSize, 0, floatVal);
        grid->Add(m_txtStartFreq, 1, wxEXPAND);

        grid->Add(new wxStaticText(this, wxID_ANY, "Stop Frequenz (Hz):"), 0, wxALIGN_CENTER_VERTICAL);
        m_txtStopFreq = new wxTextCtrl(this, wxID_ANY, "1000000000", wxDefaultPosition, wxDefaultSize, 0, floatVal);
        grid->Add(m_txtStopFreq, 1, wxEXPAND);

        grid->Add(new wxStaticText(this, wxID_ANY, "RBW (Hz):"), 0, wxALIGN_CENTER_VERTICAL);
        m_txtRBW = new wxTextCtrl(this, wxID_ANY, "10000", wxDefaultPosition, wxDefaultSize, 0, floatVal);
        grid->Add(m_txtRBW, 1, wxEXPAND);

        grid->Add(new wxStaticText(this, wxID_ANY, "VBW (Hz):"), 0, wxALIGN_CENTER_VERTICAL);
        m_txtVBW = new wxTextCtrl(this, wxID_ANY, "10000", wxDefaultPosition, wxDefaultSize, 0, floatVal);
        grid->Add(m_txtVBW, 1, wxEXPAND);

        grid->Add(new wxStaticText(this, wxID_ANY, "Detektor:"), 0, wxALIGN_CENTER_VERTICAL);
        wxArrayString detectors = {"APE", "POS", "NEG", "SAMP", "RMS", "AVER"};
        m_choiceDetector = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, detectors);
        m_choiceDetector->SetSelection(1);
        grid->Add(m_choiceDetector, 1, wxEXPAND);
    }

    // ---- Nur Sweep ----
    if (m_mode == MeasurementMode::SWEEP) {
        grid->Add(new wxStaticText(this, wxID_ANY, "Sweep Punkte:"), 0, wxALIGN_CENTER_VERTICAL);
        wxArrayString points = {"155", "313", "625", "1251", "1999", "2501", "5001", "10001", "20001", "30001"};
        m_choiceSweepPoints = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, points);
        m_choiceSweepPoints->SetSelection(2);
        grid->Add(m_choiceSweepPoints, 1, wxEXPAND);
    }

    // ---- Nur IQ ----
    if (m_mode == MeasurementMode::IQ) {
        grid->Add(new wxStaticText(this, wxID_ANY, "Center Frequenz (Hz):"), 0, wxALIGN_CENTER_VERTICAL);
        m_txtCenterFreq = new wxTextCtrl(this, wxID_ANY, "100000000", wxDefaultPosition, wxDefaultSize, 0, floatVal);
        grid->Add(m_txtCenterFreq, 1, wxEXPAND);

        grid->Add(new wxStaticText(this, wxID_ANY, "Sample Rate (Hz):"), 0, wxALIGN_CENTER_VERTICAL);
        m_txtSampleRate = new wxTextCtrl(this, wxID_ANY, "32000000", wxDefaultPosition, wxDefaultSize, 0, floatVal);
        grid->Add(m_txtSampleRate, 1, wxEXPAND);

        grid->Add(new wxStaticText(this, wxID_ANY, "Record Length (Samples):"), 0, wxALIGN_CENTER_VERTICAL);
        m_txtRecordLength = new wxTextCtrl(this, wxID_ANY, "1024", wxDefaultPosition, wxDefaultSize, 0, floatVal);
        grid->Add(m_txtRecordLength, 1, wxEXPAND);

        grid->Add(new wxStaticText(this, wxID_ANY, "IF Bandwidth (Hz):"), 0, wxALIGN_CENTER_VERTICAL);
        m_txtIfBandwidth = new wxTextCtrl(this, wxID_ANY, "10000000", wxDefaultPosition, wxDefaultSize, 0, floatVal);
        grid->Add(m_txtIfBandwidth, 1, wxEXPAND);

        grid->Add(new wxStaticText(this, wxID_ANY, "Trigger Quelle:"), 0, wxALIGN_CENTER_VERTICAL);
        wxArrayString trigSources = {"IMM", "EXT", "IFP"};
        m_choiceTriggerSource = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, trigSources);
        m_choiceTriggerSource->SetSelection(0);
        grid->Add(m_choiceTriggerSource, 1, wxEXPAND);

        grid->Add(new wxStaticText(this, wxID_ANY, "Trigger Level (dBm):"), 0, wxALIGN_CENTER_VERTICAL);
        m_txtTriggerLevel = new wxTextCtrl(this, wxID_ANY, "-20", wxDefaultPosition, wxDefaultSize, 0, floatVal);
        grid->Add(m_txtTriggerLevel, 1, wxEXPAND);

        grid->Add(new wxStaticText(this, wxID_ANY, "Trigger Delay (s):"), 0, wxALIGN_CENTER_VERTICAL);
        m_txtTriggerDelay = new wxTextCtrl(this, wxID_ANY, "0", wxDefaultPosition, wxDefaultSize, 0, floatVal);
        grid->Add(m_txtTriggerDelay, 1, wxEXPAND);
    }

    // Layout
    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
    mainSizer->Add(grid, 1, wxALL | wxEXPAND, 15);

    wxSizer* buttonSizer = CreateButtonSizer(wxAPPLY | wxCANCEL);
    mainSizer->Add(buttonSizer, 0, wxALL | wxALIGN_RIGHT, 10);

    SetSizer(mainSizer);
    RefreshData();
}

// ---- Event Handler ----
void SettingsDialog::OnApply(wxCommandEvent& event) {
    switch (m_mode) {
        case MeasurementMode::SWEEP:       ApplySweep();      break;
        case MeasurementMode::IQ:          ApplyIq();         break;
        case MeasurementMode::MARKER_PEAK: ApplyMarkerPeak(); break;
    }
}

// ---- Sweep Apply + Verifikation ----
void SettingsDialog::ApplySweep() {
    fsuMeasurement* fsu = &fsuMeasurement::get_instance();

    double startFreq, stopFreq, refLevel, rbw, vbw;
    m_txtStartFreq->GetValue().ToDouble(&startFreq);
    m_txtStopFreq->GetValue().ToDouble(&stopFreq);
    m_txtRefLevel->GetValue().ToDouble(&refLevel);
    m_txtRBW->GetValue().ToDouble(&rbw);
    m_txtVBW->GetValue().ToDouble(&vbw);
    int points = wxAtoi(m_choiceSweepPoints->GetStringSelection());
    std::string det = m_choiceDetector->GetStringSelection().ToStdString();
    int att = m_spinAttenuation->GetValue();
    std::string unit = m_choiceUnit->GetStringSelection().ToStdString();

    // Validierung
    if (!fsu->checkIfSettingsValidSweep(ScpiCommand::START_FREQUENCY, startFreq)) {
        wxMessageBox("Startfrequenz ausserhalb des Bereichs (0 - 26.5 GHz)!", "Validierungsfehler", wxOK | wxICON_ERROR); return;
    }
    if (!fsu->checkIfSettingsValidSweep(ScpiCommand::END_FREQUENCY, stopFreq)) {
        wxMessageBox("Stopfrequenz ausserhalb des Bereichs (0 - 26.5 GHz)!", "Validierungsfehler", wxOK | wxICON_ERROR); return;
    }
    if (!fsu->checkIfSettingsValidSweep(ScpiCommand::RF_ATTENUATION, att)) {
        wxMessageBox("Daempfung muss in 5-dB Schritten erfolgen (0-75 dB)!", "Validierungsfehler", wxOK | wxICON_ERROR); return;
    }
    if (!fsu->checkIfSettingsValidSweep(ScpiCommand::RBW, rbw)) {
        wxMessageBox("RBW ausserhalb des gueltigen Bereichs (1 Hz - 50 MHz)!", "Validierungsfehler", wxOK | wxICON_ERROR); return;
    }
    if (!fsu->checkIfSettingsValidSweep(ScpiCommand::VBW, vbw)) {
        wxMessageBox("VBW ausserhalb des gueltigen Bereichs (1 Hz - 50 MHz)!", "Validierungsfehler", wxOK | wxICON_ERROR); return;
    }
    if (!fsu->checkIfSettingsValidSweep(ScpiCommand::SWEEP_POINTS, points)) {
        wxMessageBox("Ungueltige Sweep Punkte!", "Validierungsfehler", wxOK | wxICON_ERROR); return;
    }

    fsuMeasurement::lastSweepSettings settings{};
    settings.startFreq = startFreq;
    settings.stopFreq  = stopFreq;
    settings.refLevel  = refLevel;
    settings.att       = att;
    settings.unit      = unit;
    settings.rbw       = static_cast<int>(rbw);
    settings.vbw       = static_cast<int>(vbw);
    settings.points    = points;
    settings.detector  = det;

    if (!fsu->writeSweepSettings(settings)) {
        wxMessageBox("Fehler beim Senden der Einstellungen!", "Fehler", wxOK | wxICON_ERROR); return;
    }

    // Ruecklesen und verifizieren
    if (!fsu->readSweepSettings()) {
        wxMessageBox("Einstellungen gesendet, aber Ruecklesen fehlgeschlagen!", "Warnung", wxOK | wxICON_WARNING);
        return;
    }

    auto rb = fsu->returnSweepSettings();
    wxString mismatches;
    bool ok = true;
    ok &= VerifyDouble("Start Frequenz", startFreq, rb.startFreq, mismatches);
    ok &= VerifyDouble("Stop Frequenz",  stopFreq,  rb.stopFreq,  mismatches);
    ok &= VerifyDouble("Ref. Pegel",     refLevel,  rb.refLevel,  mismatches);
    ok &= VerifyInt("Daempfung",         att,       rb.att,       mismatches);
    ok &= VerifyInt("RBW", static_cast<int>(rbw),   rb.rbw,       mismatches);
    ok &= VerifyInt("VBW", static_cast<int>(vbw),   rb.vbw,       mismatches);
    ok &= VerifyInt("Sweep Punkte",      points,    rb.points,    mismatches);
    ok &= VerifyString("Detektor",       det,       rb.detector,  mismatches);
    ok &= VerifyString("Einheit",        unit,      rb.unit,      mismatches);

    if (ok)
        wxMessageBox("Einstellungen erfolgreich uebernommen und verifiziert!", "Erfolg", wxOK | wxICON_INFORMATION);
    else
        wxMessageBox("Abweichungen festgestellt:\n\n" + mismatches, "Verifikation", wxOK | wxICON_WARNING);

    RefreshData();
}

// ---- IQ Apply + Verifikation ----
void SettingsDialog::ApplyIq() {
    fsuMeasurement* fsu = &fsuMeasurement::get_instance();

    double centerFreq, refLevel, sampleRate, ifBw, trigLevel, trigDelay;
    m_txtCenterFreq->GetValue().ToDouble(&centerFreq);
    m_txtRefLevel->GetValue().ToDouble(&refLevel);
    m_txtSampleRate->GetValue().ToDouble(&sampleRate);
    m_txtIfBandwidth->GetValue().ToDouble(&ifBw);
    m_txtTriggerLevel->GetValue().ToDouble(&trigLevel);
    m_txtTriggerDelay->GetValue().ToDouble(&trigDelay);
    int recordLen = wxAtoi(m_txtRecordLength->GetValue());
    int att = m_spinAttenuation->GetValue();
    std::string unit = m_choiceUnit->GetStringSelection().ToStdString();
    std::string trigSrc = m_choiceTriggerSource->GetStringSelection().ToStdString();

    // Validierung
    if (!fsu->checkIfSettingsValidSweep(ScpiCommand::CENTER_FREQUENCY, centerFreq)) {
        wxMessageBox("Center Frequenz ausserhalb des Bereichs (0 - 26.5 GHz)!", "Validierungsfehler", wxOK | wxICON_ERROR); return;
    }
    if (!fsu->checkIfSettingsValidSweep(ScpiCommand::RF_ATTENUATION, att)) {
        wxMessageBox("Daempfung muss in 5-dB Schritten erfolgen (0-75 dB)!", "Validierungsfehler", wxOK | wxICON_ERROR); return;
    }
    if (!fsu->checkIfSettingsValidSweep(ScpiCommand::IQ_SAMPLE_RATE, sampleRate)) {
        wxMessageBox("Sample Rate ausserhalb des Bereichs (10 kHz - 70.4 MHz)!", "Validierungsfehler", wxOK | wxICON_ERROR); return;
    }
    if (!fsu->checkIfSettingsValidSweep(ScpiCommand::IQ_RECORD_LENGTH, recordLen)) {
        wxMessageBox("Record Length ausserhalb des Bereichs (1 - 16M)!", "Validierungsfehler", wxOK | wxICON_ERROR); return;
    }
    if (!fsu->checkIfSettingsValidSweep(ScpiCommand::IQ_IF_BANDWIDTH, ifBw)) {
        wxMessageBox("IF Bandwidth ausserhalb des Bereichs (10 Hz - 50 MHz)!", "Validierungsfehler", wxOK | wxICON_ERROR); return;
    }
    if (!fsu->checkIfSettingsValidSweep(ScpiCommand::TRIGGER_SOURCE, trigSrc)) {
        wxMessageBox("Ungueltige Trigger Quelle!", "Validierungsfehler", wxOK | wxICON_ERROR); return;
    }
    if (!fsu->checkIfSettingsValidSweep(ScpiCommand::TRIGGER_LEVEL, trigLevel)) {
        wxMessageBox("Trigger Level ausserhalb des Bereichs (-130 - +30 dBm)!", "Validierungsfehler", wxOK | wxICON_ERROR); return;
    }
    if (!fsu->checkIfSettingsValidSweep(ScpiCommand::TRIGGER_DELAY, trigDelay)) {
        wxMessageBox("Trigger Delay ausserhalb des Bereichs (-1 - 65 s)!", "Validierungsfehler", wxOK | wxICON_ERROR); return;
    }

    fsuMeasurement::IqSettings settings{};
    settings.centerFreq    = centerFreq;
    settings.refLevel      = refLevel;
    settings.att           = att;
    settings.unit          = unit;
    settings.sampleRate    = sampleRate;
    settings.recordLength  = recordLen;
    settings.ifBandwidth   = ifBw;
    settings.triggerSource = trigSrc;
    settings.triggerLevel  = trigLevel;
    settings.triggerDelay  = trigDelay;

    if (!fsu->writeIqSettings(settings)) {
        wxMessageBox("Fehler beim Senden der IQ-Einstellungen!", "Fehler", wxOK | wxICON_ERROR); return;
    }

    if (!fsu->readIqSettings()) {
        wxMessageBox("IQ-Einstellungen gesendet, aber Ruecklesen fehlgeschlagen!", "Warnung", wxOK | wxICON_WARNING);
        return;
    }

    auto rb = fsu->returnIqSettings();
    wxString mismatches;
    bool ok = true;
    ok &= VerifyDouble("Center Frequenz", centerFreq,  rb.centerFreq,    mismatches);
    ok &= VerifyDouble("Ref. Pegel",      refLevel,    rb.refLevel,      mismatches);
    ok &= VerifyInt("Daempfung",          att,         rb.att,           mismatches);
    ok &= VerifyString("Einheit",         unit,        rb.unit,          mismatches);
    ok &= VerifyDouble("Sample Rate",     sampleRate,  rb.sampleRate,    mismatches);
    ok &= VerifyInt("Record Length",      recordLen,   rb.recordLength,  mismatches);
    ok &= VerifyDouble("IF Bandwidth",    ifBw,        rb.ifBandwidth,   mismatches);
    ok &= VerifyString("Trigger Quelle",  trigSrc,     rb.triggerSource, mismatches);
    ok &= VerifyDouble("Trigger Level",   trigLevel,   rb.triggerLevel,  mismatches);
    ok &= VerifyDouble("Trigger Delay",   trigDelay,   rb.triggerDelay,  mismatches);

    if (ok)
        wxMessageBox("IQ-Einstellungen erfolgreich uebernommen und verifiziert!", "Erfolg", wxOK | wxICON_INFORMATION);
    else
        wxMessageBox("Abweichungen festgestellt:\n\n" + mismatches, "Verifikation", wxOK | wxICON_WARNING);

    RefreshData();
}

// ---- MarkerPeak Apply + Verifikation ----
void SettingsDialog::ApplyMarkerPeak() {
    fsuMeasurement* fsu = &fsuMeasurement::get_instance();

    double startFreq, stopFreq, refLevel, rbw, vbw;
    m_txtStartFreq->GetValue().ToDouble(&startFreq);
    m_txtStopFreq->GetValue().ToDouble(&stopFreq);
    m_txtRefLevel->GetValue().ToDouble(&refLevel);
    m_txtRBW->GetValue().ToDouble(&rbw);
    m_txtVBW->GetValue().ToDouble(&vbw);
    std::string det = m_choiceDetector->GetStringSelection().ToStdString();
    int att = m_spinAttenuation->GetValue();
    std::string unit = m_choiceUnit->GetStringSelection().ToStdString();

    // Validierung
    if (!fsu->checkIfSettingsValidSweep(ScpiCommand::START_FREQUENCY, startFreq)) {
        wxMessageBox("Startfrequenz ausserhalb des Bereichs (0 - 26.5 GHz)!", "Validierungsfehler", wxOK | wxICON_ERROR); return;
    }
    if (!fsu->checkIfSettingsValidSweep(ScpiCommand::END_FREQUENCY, stopFreq)) {
        wxMessageBox("Stopfrequenz ausserhalb des Bereichs (0 - 26.5 GHz)!", "Validierungsfehler", wxOK | wxICON_ERROR); return;
    }
    if (!fsu->checkIfSettingsValidSweep(ScpiCommand::RF_ATTENUATION, att)) {
        wxMessageBox("Daempfung muss in 5-dB Schritten erfolgen (0-75 dB)!", "Validierungsfehler", wxOK | wxICON_ERROR); return;
    }
    if (!fsu->checkIfSettingsValidSweep(ScpiCommand::RBW, rbw)) {
        wxMessageBox("RBW ausserhalb des gueltigen Bereichs (1 Hz - 50 MHz)!", "Validierungsfehler", wxOK | wxICON_ERROR); return;
    }
    if (!fsu->checkIfSettingsValidSweep(ScpiCommand::VBW, vbw)) {
        wxMessageBox("VBW ausserhalb des gueltigen Bereichs (1 Hz - 50 MHz)!", "Validierungsfehler", wxOK | wxICON_ERROR); return;
    }

    fsuMeasurement::MarkerPeakSettings settings{};
    settings.startFreq = startFreq;
    settings.stopFreq  = stopFreq;
    settings.refLevel  = refLevel;
    settings.att       = att;
    settings.unit      = unit;
    settings.rbw       = rbw;
    settings.vbw       = vbw;
    settings.detector  = det;

    if (!fsu->writeMarkerPeakSettings(settings)) {
        wxMessageBox("Fehler beim Senden der MarkerPeak-Einstellungen!", "Fehler", wxOK | wxICON_ERROR); return;
    }

    if (!fsu->readMarkerPeakSettings()) {
        wxMessageBox("MarkerPeak-Einstellungen gesendet, aber Ruecklesen fehlgeschlagen!", "Warnung", wxOK | wxICON_WARNING);
        return;
    }

    auto rb = fsu->returnMarkerPeakSettings();
    wxString mismatches;
    bool ok = true;
    ok &= VerifyDouble("Start Frequenz", startFreq, rb.startFreq, mismatches);
    ok &= VerifyDouble("Stop Frequenz",  stopFreq,  rb.stopFreq,  mismatches);
    ok &= VerifyDouble("Ref. Pegel",     refLevel,  rb.refLevel,  mismatches);
    ok &= VerifyInt("Daempfung",         att,       rb.att,       mismatches);
    ok &= VerifyString("Einheit",        unit,      rb.unit,      mismatches);
    ok &= VerifyDouble("RBW",            rbw,       rb.rbw,       mismatches);
    ok &= VerifyDouble("VBW",            vbw,       rb.vbw,       mismatches);
    ok &= VerifyString("Detektor",       det,       rb.detector,  mismatches);

    if (ok)
        wxMessageBox("MarkerPeak-Einstellungen erfolgreich uebernommen und verifiziert!", "Erfolg", wxOK | wxICON_INFORMATION);
    else
        wxMessageBox("Abweichungen festgestellt:\n\n" + mismatches, "Verifikation", wxOK | wxICON_WARNING);

    RefreshData();
}

// ---- RefreshData: aktuelle Geraetewerte in GUI laden ----
void SettingsDialog::RefreshData()
{
    fsuMeasurement* fsu = &fsuMeasurement::get_instance();

    switch (m_mode) {
    case MeasurementMode::SWEEP: {
        auto s = fsu->returnSweepSettings();
        m_txtStartFreq     ->SetValue(std::to_string(s.startFreq));
        m_txtStopFreq      ->SetValue(std::to_string(s.stopFreq));
        m_txtRefLevel      ->SetValue(std::to_string(s.refLevel));
        m_spinAttenuation  ->SetValue(s.att);
        m_choiceUnit       ->SetStringSelection(s.unit);
        m_txtRBW           ->SetValue(std::to_string(s.rbw));
        m_txtVBW           ->SetValue(std::to_string(s.vbw));
        m_choiceSweepPoints->SetStringSelection(std::to_string(s.points));
        m_choiceDetector   ->SetStringSelection(s.detector);
        break;
    }
    case MeasurementMode::IQ: {
        auto s = fsu->returnIqSettings();
        m_txtRefLevel         ->SetValue(std::to_string(s.refLevel));
        m_spinAttenuation     ->SetValue(s.att);
        m_choiceUnit          ->SetStringSelection(s.unit);
        m_txtCenterFreq       ->SetValue(std::to_string(s.centerFreq));
        m_txtSampleRate       ->SetValue(std::to_string(s.sampleRate));
        m_txtRecordLength     ->SetValue(std::to_string(s.recordLength));
        m_txtIfBandwidth      ->SetValue(std::to_string(s.ifBandwidth));
        m_choiceTriggerSource ->SetStringSelection(s.triggerSource);
        m_txtTriggerLevel     ->SetValue(std::to_string(s.triggerLevel));
        m_txtTriggerDelay     ->SetValue(std::to_string(s.triggerDelay));
        break;
    }
    case MeasurementMode::MARKER_PEAK: {
        auto s = fsu->returnMarkerPeakSettings();
        m_txtStartFreq    ->SetValue(std::to_string(s.startFreq));
        m_txtStopFreq     ->SetValue(std::to_string(s.stopFreq));
        m_txtRefLevel     ->SetValue(std::to_string(s.refLevel));
        m_spinAttenuation ->SetValue(s.att);
        m_choiceUnit      ->SetStringSelection(s.unit);
        m_txtRBW          ->SetValue(std::to_string(s.rbw));
        m_txtVBW          ->SetValue(std::to_string(s.vbw));
        m_choiceDetector  ->SetStringSelection(s.detector);
        break;
    }
    }
}

// ---- Verifikations-Helfer ----
bool SettingsDialog::VerifyDouble(const wxString& name, double written, double readback, wxString& mismatches)
{
    double tol = std::max(std::abs(written) * 0.01, 1.0);
    if (std::abs(written - readback) > tol) {
        mismatches += wxString::Format("  %s: gesendet=%.4g, gelesen=%.4g\n", name, written, readback);
        return false;
    }
    return true;
}

bool SettingsDialog::VerifyInt(const wxString& name, int written, int readback, wxString& mismatches)
{
    if (written != readback) {
        mismatches += wxString::Format("  %s: gesendet=%d, gelesen=%d\n", name, written, readback);
        return false;
    }
    return true;
}

bool SettingsDialog::VerifyString(const wxString& name, const std::string& written, const std::string& readback, wxString& mismatches)
{
    if (written != readback) {
        mismatches += wxString::Format("  %s: gesendet=%s, gelesen=%s\n", name, written, readback);
        return false;
    }
    return true;
}