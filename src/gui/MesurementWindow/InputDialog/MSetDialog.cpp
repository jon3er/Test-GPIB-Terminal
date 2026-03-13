#include "MSetDialog.h"
#include "FsuMeasurement.h"
#include "cmdGpib.h"
#include "Mesurement.h"
#include "PlotterFrame.h"
#include <wx/valnum.h>
#include <cmath>

enum { ID_BTN_START = wxID_HIGHEST + 1 };

wxBEGIN_EVENT_TABLE(SettingsDialog, wxDialog)
    EVT_BUTTON(wxID_APPLY,   SettingsDialog::OnApply)
    EVT_BUTTON(ID_BTN_START, SettingsDialog::OnStart)
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
    grid->Add(new wxStaticText(this, wxID_ANY, "Ref. Pegel:"), 0, wxALIGN_CENTER_VERTICAL);
    floatVal.SetPrecision(2);
    m_txtRefLevel = new wxTextCtrl(this, wxID_ANY, "0", wxDefaultPosition, wxDefaultSize, 0, floatVal);
    m_txtRefLevel->SetToolTip("Zulaessiger Bereich: -130.00 bis +30.00 dBm");

    wxArrayString units = {"DBM", "V", "W", "DBUV"};
    m_choiceUnit = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, units);
    m_choiceUnit->SetSelection(0);
    m_choiceUnit->SetToolTip("Zulaessige Einheiten: DBM, V, W, DBUV");

    wxBoxSizer* refLevelSizer = new wxBoxSizer(wxHORIZONTAL);
    refLevelSizer->Add(m_txtRefLevel, 1, wxRIGHT, 6);
    refLevelSizer->Add(m_choiceUnit, 0);
    grid->Add(refLevelSizer, 1, wxEXPAND);

    grid->Add(new wxStaticText(this, wxID_ANY, "HF-Daempfung (dB):"), 0, wxALIGN_CENTER_VERTICAL);
    m_spinAttenuation = new wxSpinCtrl(this, wxID_ANY);
    m_spinAttenuation->SetRange(0, 75);
    m_spinAttenuation->SetIncrement(5);
    m_spinAttenuation->SetToolTip("Zulaessiger Bereich: 0 bis 75 dB in 5-dB Schritten");
    grid->Add(m_spinAttenuation, 1, wxEXPAND);

    // ---- Sweep + MarkerPeak Felder ----
    if (m_mode == MeasurementMode::SWEEP || m_mode == MeasurementMode::MARKER_PEAK) {
        grid->Add(new wxStaticText(this, wxID_ANY, "Start Frequenz (Hz):"), 0, wxALIGN_CENTER_VERTICAL);
        m_txtStartFreq = new wxTextCtrl(this, wxID_ANY, "1000000");
        m_txtStartFreq->SetToolTip("Zulaessiger Bereich: 0 bis 26.5 GHz (auch z. B. 1e6, 1 MHz, 0.1 GHz)");
        grid->Add(m_txtStartFreq, 1, wxEXPAND);

        grid->Add(new wxStaticText(this, wxID_ANY, "Stop Frequenz (Hz):"), 0, wxALIGN_CENTER_VERTICAL);
        m_txtStopFreq = new wxTextCtrl(this, wxID_ANY, "1000000000");
        m_txtStopFreq->SetToolTip("Zulaessiger Bereich: 0 bis 26.5 GHz (auch z. B. 2e6, 2 MHz, 0.2 GHz)");
        grid->Add(m_txtStopFreq, 1, wxEXPAND);

        grid->Add(new wxStaticText(this, wxID_ANY, "RBW (Hz):"), 0, wxALIGN_CENTER_VERTICAL);
        m_txtRBW = new wxTextCtrl(this, wxID_ANY, "10000");
        m_txtRBW->SetToolTip("Zulaessiger Bereich: 1 Hz bis 50 MHz (z. B. 10000, 10 kHz, 0.01 MHz)");
        grid->Add(m_txtRBW, 1, wxEXPAND);

        grid->Add(new wxStaticText(this, wxID_ANY, "VBW (Hz):"), 0, wxALIGN_CENTER_VERTICAL);
        m_txtVBW = new wxTextCtrl(this, wxID_ANY, "10000");
        m_txtVBW->SetToolTip("Zulaessiger Bereich: 1 Hz bis 50 MHz (z. B. 10000, 10 kHz, 0.01 MHz)");
        grid->Add(m_txtVBW, 1, wxEXPAND);

        grid->Add(new wxStaticText(this, wxID_ANY, "Detektor:"), 0, wxALIGN_CENTER_VERTICAL);
        wxArrayString detectors = {"APE", "POS", "NEG", "SAMP", "RMS", "AVER"};
        m_choiceDetector = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, detectors);
        m_choiceDetector->SetSelection(1);
        m_choiceDetector->SetToolTip("Zulaessige Detektoren: APE, POS, NEG, SAMP, RMS, AVER");
        grid->Add(m_choiceDetector, 1, wxEXPAND);
    }

    // ---- Nur Sweep ----
    if (m_mode == MeasurementMode::SWEEP) {
        grid->Add(new wxStaticText(this, wxID_ANY, "Sweep Punkte:"), 0, wxALIGN_CENTER_VERTICAL);
        wxArrayString points = {"155", "313", "625", "1251", "1999", "2501", "5001", "10001", "20001", "30001"};
        m_choiceSweepPoints = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, points);
        m_choiceSweepPoints->SetSelection(2);
        m_choiceSweepPoints->SetToolTip("Zulaessige Sweep-Punkte: 155, 313, 625, 1251, 1999, 2501, 5001, 10001, 20001, 30001");
        grid->Add(m_choiceSweepPoints, 1, wxEXPAND);
    }

    // ---- Nur IQ ----
    if (m_mode == MeasurementMode::IQ) {
        grid->Add(new wxStaticText(this, wxID_ANY, "Center Frequenz (Hz):"), 0, wxALIGN_CENTER_VERTICAL);
        m_txtCenterFreq = new wxTextCtrl(this, wxID_ANY, "100000000");
        m_txtCenterFreq->SetToolTip("Zulaessiger Bereich: 0 bis 26.5 GHz (auch z. B. 100e6, 100 MHz, 0.1 GHz)");
        grid->Add(m_txtCenterFreq, 1, wxEXPAND);

        grid->Add(new wxStaticText(this, wxID_ANY, "Sample Rate (Hz):"), 0, wxALIGN_CENTER_VERTICAL);
        m_txtSampleRate = new wxTextCtrl(this, wxID_ANY, "32000000");
        m_txtSampleRate->SetToolTip("Zulaessiger Bereich: 10 kHz bis 70.4 MHz (z. B. 32e6, 32 MHz)");
        grid->Add(m_txtSampleRate, 1, wxEXPAND);

        grid->Add(new wxStaticText(this, wxID_ANY, "Record Length (Samples):"), 0, wxALIGN_CENTER_VERTICAL);
        floatVal.SetPrecision(0);
        m_txtRecordLength = new wxTextCtrl(this, wxID_ANY, "1024", wxDefaultPosition, wxDefaultSize, 0, floatVal);
        m_txtRecordLength->SetToolTip("Zulaessiger Bereich: 1 bis 16,000,000 Samples");
        grid->Add(m_txtRecordLength, 1, wxEXPAND);

        grid->Add(new wxStaticText(this, wxID_ANY, "IF Bandwidth (Hz):"), 0, wxALIGN_CENTER_VERTICAL);
        m_txtIfBandwidth = new wxTextCtrl(this, wxID_ANY, "10000000");
        m_txtIfBandwidth->SetToolTip("Zulaessiger Bereich: 10 Hz bis 50 MHz (z. B. 10e6, 10 MHz)");
        grid->Add(m_txtIfBandwidth, 1, wxEXPAND);

        // Trigger source Setting (currently disabled)
        // grid->Add(new wxStaticText(this, wxID_ANY, "Trigger Quelle:"), 0, wxALIGN_CENTER_VERTICAL);
        // wxArrayString trigsources = {"IMM", "EXT", "IFP"};
        // m_choiceTriggerSource = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, trigsources);
        // m_choiceTriggerSource->SetSelection(0);
        // m_choiceTriggerSource->SetToolTip("Zulaessige Trigger-Quellen: IMM, EXT, IFP");
        // grid->Add(m_choiceTriggerSource, 1, wxEXPAND);

        grid->Add(new wxStaticText(this, wxID_ANY, "Trigger Level (dBm):"), 0, wxALIGN_CENTER_VERTICAL);
        floatVal.SetPrecision(0);
        m_txtTriggerLevel = new wxTextCtrl(this, wxID_ANY, "-20", wxDefaultPosition, wxDefaultSize, 0, floatVal);
        m_txtTriggerLevel->SetToolTip("Zulaessiger Bereich: -130 bis +30 dBm");
        grid->Add(m_txtTriggerLevel, 1, wxEXPAND);

        grid->Add(new wxStaticText(this, wxID_ANY, "Trigger Delay (s):"), 0, wxALIGN_CENTER_VERTICAL);
        m_txtTriggerDelay = new wxTextCtrl(this, wxID_ANY, "0");
        m_txtTriggerDelay->SetToolTip("Zulaessiger Bereich: -1 bis 65 s (auch z. B. 500 ms, 250 us)");
        grid->Add(m_txtTriggerDelay, 1, wxEXPAND);
    }

    grid->Add(new wxStaticText(this, wxID_ANY, wxString::FromUTF8("Messung mit Plotter durchführen")), 0, wxALIGN_CENTER_VERTICAL);
    m_useMultipoint = new wxCheckBox(this, wxID_ANY,"", wxDefaultPosition, wxDefaultSize, 0);
    grid->Add(m_useMultipoint, 1, wxEXPAND);

    // Layout
    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
    mainSizer->Add(grid, 1, wxALL | wxEXPAND, 15);

    wxBoxSizer* buttonSizer = new wxBoxSizer(wxHORIZONTAL);
    wxButton* btnApply  = new wxButton(this, wxID_APPLY,   "Apply");
    m_btnStart          = new wxButton(this, ID_BTN_START, "Start");
    wxButton* btnCancel = new wxButton(this, wxID_CANCEL,  "Cancel");
    m_btnStart->Enable(true);   // TODO change back to False
    buttonSizer->Add(btnCancel, 0, wxRIGHT, 5);
    buttonSizer->Add(btnApply,  0, wxRIGHT, 5);
    buttonSizer->Add(m_btnStart, 0);
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
    // Set Mode for Measurement
    fsu->setMeasurementMode(MeasurementMode::SWEEP);

    double startFreq;
    if (!ParseFrequencyInputToHz(m_txtStartFreq->GetValue(), startFreq)) {
        wxMessageBox("Startfrequenz ungueltig! Beispiele: 1000000, 1 MHz, 1.5GHz", "Validierungsfehler", wxOK | wxICON_ERROR); return;
    }
    double stopFreq;
    if (!ParseFrequencyInputToHz(m_txtStopFreq->GetValue(), stopFreq)) {
        wxMessageBox("Stopfrequenz ungueltig! Beispiele: 2000000, 2 MHz, 2.5GHz", "Validierungsfehler", wxOK | wxICON_ERROR); return;
    }
    double refLevel;
    m_txtRefLevel->GetValue().ToDouble(&refLevel);
    double rbw;
    if (!ParseFrequencyInputToHz(m_txtRBW->GetValue(), rbw)) {
        wxMessageBox("RBW ungueltig! Beispiele: 10000, 10 kHz, 0.01 MHz", "Validierungsfehler", wxOK | wxICON_ERROR); return;
    }
    double vbw;
    if (!ParseFrequencyInputToHz(m_txtVBW->GetValue(), vbw)) {
        wxMessageBox("VBW ungueltig! Beispiele: 10000, 10 kHz, 0.01 MHz", "Validierungsfehler", wxOK | wxICON_ERROR); return;
    }

    // Number of Points
    int points = wxAtoi(m_choiceSweepPoints->GetStringSelection());
    // detector Selction
    std::string det = m_choiceDetector->GetStringSelection().ToStdString();
    // Attenuation
    int att = m_spinAttenuation->GetValue();
    // Unit
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
    settings.startFreq  = startFreq;
    settings.stopFreq   = stopFreq;
    settings.refLevel   = refLevel;
    settings.points     = points;
    settings.detector   = det;
    settings.att        = att;
    settings.rbw        = static_cast<int>(rbw);
    settings.vbw        = static_cast<int>(vbw);
    settings.unit      = unit;

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
    ok &= VerifyInt("VBW",  static_cast<int>(vbw),   rb.vbw,       mismatches);
    ok &= VerifyInt("Sweep Punkte",      points,    rb.points,    mismatches);
    ok &= VerifyString("Detektor",       det,       rb.detector,  mismatches);
    ok &= VerifyString("Einheit",        unit,      rb.unit,      mismatches);

    if (ok) {
        wxMessageBox("Einstellungen erfolgreich uebernommen und verifiziert!", "Erfolg", wxOK | wxICON_INFORMATION);
        m_btnStart->Enable(true);
    } else {
        wxMessageBox("Abweichungen festgestellt:\n\n" + mismatches, "Verifikation", wxOK | wxICON_WARNING);
        m_btnStart->Enable(false);
    }

    RefreshData();
}

// ---- IQ Apply + Verifikation ----
void SettingsDialog::ApplyIq() {
    fsuMeasurement* fsu = &fsuMeasurement::get_instance();
    // Set Mode for Measurement
    fsu->setMeasurementMode(MeasurementMode::IQ);

    double centerFreq, refLevel, sampleRate, ifBw, trigLevel, trigDelay;
    if (!ParseFrequencyInputToHz(m_txtCenterFreq->GetValue(), centerFreq)) {
        wxMessageBox("Center Frequenz ungueltig! Beispiele: 100000000, 100 MHz, 0.1 GHz", "Validierungsfehler", wxOK | wxICON_ERROR); return;
    }
    m_txtRefLevel->GetValue().ToDouble(&refLevel);
    if (!ParseFrequencyInputToHz(m_txtSampleRate->GetValue(), sampleRate)) {
        wxMessageBox("Sample Rate ungueltig! Beispiele: 32000000, 32 MHz", "Validierungsfehler", wxOK | wxICON_ERROR); return;
    }
    if (!ParseFrequencyInputToHz(m_txtIfBandwidth->GetValue(), ifBw)) {
        wxMessageBox("IF Bandwidth ungueltig! Beispiele: 10000000, 10 MHz", "Validierungsfehler", wxOK | wxICON_ERROR); return;
    }
    m_txtTriggerLevel->GetValue().ToDouble(&trigLevel);
    if (!ParseTimeInputToSeconds(m_txtTriggerDelay->GetValue(), trigDelay)) {
        wxMessageBox("Trigger Delay ungueltig! Beispiele: 0.5, 500 ms, 250 us", "Validierungsfehler", wxOK | wxICON_ERROR); return;
    }
    int recordLen = wxAtoi(m_txtRecordLength->GetValue());
    int att = m_spinAttenuation->GetValue();
    std::string unit = m_choiceUnit->GetStringSelection().ToStdString();
    std::string trigSrc = m_choiceTriggerSource
        ? m_choiceTriggerSource->GetStringSelection().ToStdString()
        : "IMM";

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

    if (ok) {
        wxMessageBox("IQ-Einstellungen erfolgreich uebernommen und verifiziert!", "Erfolg", wxOK | wxICON_INFORMATION);
        m_btnStart->Enable(true);
    } else {
        wxMessageBox("Abweichungen festgestellt:\n\n" + mismatches, "Verifikation", wxOK | wxICON_WARNING);
        m_btnStart->Enable(false);
    }

    RefreshData();
}

// ---- MarkerPeak Apply + Verifikation ----
void SettingsDialog::ApplyMarkerPeak() {
    fsuMeasurement* fsu = &fsuMeasurement::get_instance();
    // Set Mode for Measurement
    fsu->setMeasurementMode(MeasurementMode::MARKER_PEAK);

    double startFreq, stopFreq, refLevel, rbw, vbw;
    if (!ParseFrequencyInputToHz(m_txtStartFreq->GetValue(), startFreq)) {
        wxMessageBox("Startfrequenz ungueltig! Beispiele: 1000000, 1 MHz, 1.5GHz", "Validierungsfehler", wxOK | wxICON_ERROR); return;
    }
    if (!ParseFrequencyInputToHz(m_txtStopFreq->GetValue(), stopFreq)) {
        wxMessageBox("Stopfrequenz ungueltig! Beispiele: 2000000, 2 MHz, 2.5GHz", "Validierungsfehler", wxOK | wxICON_ERROR); return;
    }
    m_txtRefLevel->GetValue().ToDouble(&refLevel);
    if (!ParseFrequencyInputToHz(m_txtRBW->GetValue(), rbw)) {
        wxMessageBox("RBW ungueltig! Beispiele: 10000, 10 kHz, 0.01 MHz", "Validierungsfehler", wxOK | wxICON_ERROR); return;
    }
    if (!ParseFrequencyInputToHz(m_txtVBW->GetValue(), vbw)) {
        wxMessageBox("VBW ungueltig! Beispiele: 10000, 10 kHz, 0.01 MHz", "Validierungsfehler", wxOK | wxICON_ERROR); return;
    }
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

    if (ok) {
        wxMessageBox("MarkerPeak-Einstellungen erfolgreich uebernommen und verifiziert!", "Erfolg", wxOK | wxICON_INFORMATION);
        m_btnStart->Enable(true);
    } else {
        wxMessageBox("Abweichungen festgestellt:\n\n" + mismatches, "Verifikation", wxOK | wxICON_WARNING);
        m_btnStart->Enable(false);
    }

    RefreshData();
}

// ---- RefreshData: aktuelle Geraetewerte in GUI laden ----
void SettingsDialog::RefreshData()
{
    fsuMeasurement* fsu = &fsuMeasurement::get_instance();

    switch (m_mode) {
        case MeasurementMode::SWEEP: {
            auto s = fsu->returnSweepSettings();
            m_txtStartFreq     ->SetValue(wxString::Format(wxT("%.0f"),s.startFreq));
            m_txtStartFreq     ->SetToolTip(FormatFrequencyAutoUnit(s.startFreq));
            m_txtStopFreq      ->SetValue(wxString::Format(wxT("%.0f"),s.stopFreq));
            m_txtStopFreq      ->SetToolTip(FormatFrequencyAutoUnit(s.stopFreq));
            m_txtRefLevel      ->SetValue(wxString::Format(wxT("%.0f"),s.refLevel));
            m_spinAttenuation  ->SetValue(s.att);
            m_choiceUnit       ->SetStringSelection(s.unit);
            m_txtRBW           ->SetValue(wxString::Format(wxT("%i"),s.rbw));
            m_txtRBW           ->SetToolTip(FormatFrequencyAutoUnit(static_cast<double>(s.rbw)));
            m_txtVBW           ->SetValue(wxString::Format(wxT("%i"),s.vbw));
            m_txtVBW           ->SetToolTip(FormatFrequencyAutoUnit(static_cast<double>(s.vbw)));
            m_choiceSweepPoints->SetStringSelection(std::to_string(s.points));
            m_choiceDetector   ->SetStringSelection(s.detector);
            break;
        }
        case MeasurementMode::IQ: {
            auto s = fsu->returnIqSettings();
            m_txtRefLevel         ->SetValue(wxString::Format(wxT("%.0f"),s.refLevel));
            m_spinAttenuation     ->SetValue(s.att);
            m_choiceUnit          ->SetStringSelection(s.unit);
            m_txtCenterFreq       ->SetValue(wxString::Format(wxT("%.0f"),s.centerFreq));
            m_txtCenterFreq       ->SetToolTip(FormatFrequencyAutoUnit(s.centerFreq));
            m_txtSampleRate       ->SetValue(wxString::Format(wxT("%.0f"),s.sampleRate));
            m_txtSampleRate       ->SetToolTip(FormatFrequencyAutoUnit(s.sampleRate));
            m_txtRecordLength     ->SetValue(wxString::Format(wxT("%i"),s.recordLength));
            m_txtIfBandwidth      ->SetValue(wxString::Format(wxT("%.0f"),s.ifBandwidth));
            m_txtIfBandwidth      ->SetToolTip(FormatFrequencyAutoUnit(s.ifBandwidth));
            if (m_choiceTriggerSource) {
                m_choiceTriggerSource->SetStringSelection(s.triggerSource);
            }
            m_txtTriggerLevel     ->SetValue(wxString::Format(wxT("%.0f"),s.triggerLevel));
            m_txtTriggerDelay     ->SetValue(wxString::Format(wxT("%.0f"),s.triggerDelay));
            break;
        }
        case MeasurementMode::MARKER_PEAK: {
            auto s = fsu->returnMarkerPeakSettings();
            m_txtStartFreq    ->SetValue(wxString::Format(wxT("%.0f"),s.startFreq));
            m_txtStartFreq    ->SetToolTip(FormatFrequencyAutoUnit(s.startFreq));
            m_txtStopFreq     ->SetValue(wxString::Format(wxT("%.0f"),s.stopFreq));
            m_txtStopFreq     ->SetToolTip(FormatFrequencyAutoUnit(s.stopFreq));
            m_txtRefLevel     ->SetValue(wxString::Format(wxT("%.0f"),s.refLevel));
            m_spinAttenuation ->SetValue(s.att);
            m_choiceUnit      ->SetStringSelection(s.unit);
            m_txtRBW          ->SetValue(wxString::Format(wxT("%i"),s.rbw));
            m_txtRBW          ->SetToolTip(FormatFrequencyAutoUnit(s.rbw));
            m_txtVBW          ->SetValue(wxString::Format(wxT("%i"),s.vbw));
            m_txtVBW          ->SetToolTip(FormatFrequencyAutoUnit(s.vbw));
            m_choiceDetector  ->SetStringSelection(s.detector);
            break;
        }
    }
}

wxString SettingsDialog::FormatFrequencyAutoUnit(double hz) const
{
    double absHz = std::abs(hz);
    double scaled = hz;
    const char* unit = "Hz";

    if (absHz >= 1e9) {
        scaled = hz / 1e9;
        unit = "GHz";
    } else if (absHz >= 1e6) {
        scaled = hz / 1e6;
        unit = "MHz";
    } else if (absHz >= 1e3) {
        scaled = hz / 1e3;
        unit = "kHz";
    }

    return wxString::Format("%.3f %s", scaled, unit);
}

bool SettingsDialog::ParseFrequencyInputToHz(const wxString& input, double& hz) const
{
    wxString value = input;
    value.Trim(true);
    value.Trim(false);

    if (value.IsEmpty()) {
        return false;
    }

    value.Replace(",", ".");
    wxString lower = value.Lower();
    lower.Trim(true);
    lower.Trim(false);

    double factor = 1.0;
    wxString numberPart = lower;

    if (numberPart.EndsWith("ghz")) {
        factor = 1e9;
        numberPart = numberPart.Left(numberPart.Length() - 3);
    } else if (numberPart.EndsWith("mhz")) {
        factor = 1e6;
        numberPart = numberPart.Left(numberPart.Length() - 3);
    } else if (numberPart.EndsWith("khz")) {
        factor = 1e3;
        numberPart = numberPart.Left(numberPart.Length() - 3);
    } else if (numberPart.EndsWith("hz")) {
        factor = 1.0;
        numberPart = numberPart.Left(numberPart.Length() - 2);
    } else if (numberPart.EndsWith("g")) {
        factor = 1e9;
        numberPart = numberPart.Left(numberPart.Length() - 1);
    } else if (numberPart.EndsWith("m")) {
        factor = 1e6;
        numberPart = numberPart.Left(numberPart.Length() - 1);
    } else if (numberPart.EndsWith("k")) {
        factor = 1e3;
        numberPart = numberPart.Left(numberPart.Length() - 1);
    }

    numberPart.Trim(true);
    numberPart.Trim(false);

    double base = 0.0;
    if (!numberPart.ToDouble(&base)) {
        return false;
    }

    hz = base * factor;
    return std::isfinite(hz);
}

bool SettingsDialog::ParseTimeInputToSeconds(const wxString& input, double& seconds) const
{
    wxString value = input;
    value.Trim(true);
    value.Trim(false);

    if (value.IsEmpty()) {
        return false;
    }

    value.Replace(",", ".");
    wxString lower = value.Lower();
    lower.Trim(true);
    lower.Trim(false);

    double factor = 1.0;
    wxString numberPart = lower;

    if (numberPart.EndsWith("ms")) {
        factor = 1e-3;
        numberPart = numberPart.Left(numberPart.Length() - 2);
    } else if (numberPart.EndsWith("us")) {
        factor = 1e-6;
        numberPart = numberPart.Left(numberPart.Length() - 2);
    } else if (numberPart.EndsWith("s")) {
        factor = 1.0;
        numberPart = numberPart.Left(numberPart.Length() - 1);
    }

    numberPart.Trim(true);
    numberPart.Trim(false);

    double base = 0.0;
    if (!numberPart.ToDouble(&base)) {
        return false;
    }

    seconds = base * factor;
    return std::isfinite(seconds);
}

// ---- Verifikations-Helfer ----
void SettingsDialog::OnStart(wxCommandEvent& /*event*/)
{
    EndModal(wxID_OK);

    if (m_useMultipoint->GetValue()) {
        if (m_plotterWindow && m_plotterWindow->IsShown())
        {
            m_plotterWindow->Raise();  // move window to foreground
            return;
        }

        // open Plotter settings window
        m_plotterWindow = new PlotterFrame();
        m_plotterWindow->ShowModal();
        m_plotterWindow->Destroy();
        m_plotterWindow = nullptr;
    } else {
        fsuMeasurement* fsu = &fsuMeasurement::get_instance();

        PlotWindow* mw = new PlotWindow(GetParent());
        mw->Show();
    }
}

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
