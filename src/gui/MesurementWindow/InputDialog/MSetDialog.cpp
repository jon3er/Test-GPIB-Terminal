#include "MSetDialog.h"
#include "FsuMeasurement.h"
#include "cmdGpib.h"
#include "Mesurement.h"
#include "PlotterFrame.h"
#include <wx/valnum.h>
#include <cmath>

//----- Dialog Setup Helpers -----
void SettingsDialog::SetDialogTitleForMode()
{
    switch (m_mode) {
        case MeasurementMode::SWEEP:       SetTitle("Sweep Einstellungen");       break;
        case MeasurementMode::IQ:          SetTitle("IQ Analyse Einstellungen");  break;
        case MeasurementMode::MARKER_PEAK: SetTitle("Marker Peak Einstellungen"); break;
    }
}

void SettingsDialog::BuildCommonFields(wxWindow* parent, wxFlexGridSizer* grid, wxFloatingPointValidator<double>& floatVal)
{
    grid->Add(new wxStaticText(parent, wxID_ANY, "Ref. Pegel:"), 0, wxALIGN_CENTER_VERTICAL);
    floatVal.SetPrecision(2);
    m_txtRefLevel = new wxTextCtrl(parent, wxID_ANY, "0", wxDefaultPosition, wxDefaultSize, 0, floatVal);
    m_txtRefLevel->SetToolTip("Zulaessiger Bereich: -130.00 bis +30.00 dBm");

    wxArrayString units = {"DBM", "V", "W", "DBUV"};
    m_choiceUnit = new wxChoice(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, units);
    m_choiceUnit->SetSelection(0);
    m_choiceUnit->SetToolTip("Zulaessige Einheiten: DBM, V, W, DBUV");

    wxBoxSizer* refLevelSizer = new wxBoxSizer(wxHORIZONTAL);
    refLevelSizer->Add(m_txtRefLevel, 1, wxRIGHT, 6);
    refLevelSizer->Add(m_choiceUnit, 0);
    grid->Add(refLevelSizer, 1, wxEXPAND);

    grid->Add(new wxStaticText(parent, wxID_ANY, "HF-Daempfung (dB):"), 0, wxALIGN_CENTER_VERTICAL);
    m_spinAttenuation = new wxSpinCtrl(parent, wxID_ANY);
    m_spinAttenuation->SetRange(0, 75);
    m_spinAttenuation->SetIncrement(5);
    m_spinAttenuation->SetToolTip("Zulaessiger Bereich: 0 bis 75 dB in 5-dB Schritten");
    grid->Add(m_spinAttenuation, 1, wxEXPAND);
}

void SettingsDialog::BuildSweepMarkerFields(wxWindow* parent, wxFlexGridSizer* grid)
{
    grid->Add(new wxStaticText(parent, wxID_ANY, "Start Frequenz (Hz):"), 0, wxALIGN_CENTER_VERTICAL);
    m_txtStartFreq = new wxTextCtrl(parent, wxID_ANY, "1000000");
    m_txtStartFreq->SetToolTip("Zulaessiger Bereich: 0 bis 26.5 GHz (auch z. B. 1e6, 1 MHz, 0.1 GHz)");
    grid->Add(m_txtStartFreq, 1, wxEXPAND);

    grid->Add(new wxStaticText(parent, wxID_ANY, "Stop Frequenz (Hz):"), 0, wxALIGN_CENTER_VERTICAL);
    m_txtStopFreq = new wxTextCtrl(parent, wxID_ANY, "1000000000");
    m_txtStopFreq->SetToolTip("Zulaessiger Bereich: 0 bis 26.5 GHz (auch z. B. 2e6, 2 MHz, 0.2 GHz)");
    grid->Add(m_txtStopFreq, 1, wxEXPAND);

    grid->Add(new wxStaticText(parent, wxID_ANY, "RBW (Hz):"), 0, wxALIGN_CENTER_VERTICAL);
    m_txtRBW = new wxTextCtrl(parent, wxID_ANY, "10000");
    m_txtRBW->SetToolTip("Zulaessiger Bereich: 1 Hz bis 50 MHz (z. B. 10000, 10 kHz, 0.01 MHz)");
    grid->Add(m_txtRBW, 1, wxEXPAND);

    grid->Add(new wxStaticText(parent, wxID_ANY, "VBW (Hz):"), 0, wxALIGN_CENTER_VERTICAL);
    m_txtVBW = new wxTextCtrl(parent, wxID_ANY, "10000");
    m_txtVBW->SetToolTip("Zulaessiger Bereich: 1 Hz bis 50 MHz (z. B. 10000, 10 kHz, 0.01 MHz)");
    grid->Add(m_txtVBW, 1, wxEXPAND);

    grid->Add(new wxStaticText(parent, wxID_ANY, "Detektor:"), 0, wxALIGN_CENTER_VERTICAL);
    wxArrayString detectors = {"APE", "POS", "NEG", "SAMP", "RMS", "AVER"};
    m_choiceDetector = new wxChoice(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, detectors);
    m_choiceDetector->SetSelection(1);
    m_choiceDetector->SetToolTip("Zulaessige Detektoren: APE, POS, NEG, SAMP, RMS, AVER");
    grid->Add(m_choiceDetector, 1, wxEXPAND);
}

void SettingsDialog::BuildSweepFields(wxWindow* parent, wxFlexGridSizer* grid)
{
    grid->Add(new wxStaticText(parent, wxID_ANY, "Sweep Punkte:"), 0, wxALIGN_CENTER_VERTICAL);
    wxArrayString points = {"155", "313", "625", "1251", "1999", "2501", "5001", "10001", "20001", "30001"};
    m_choiceSweepPoints = new wxChoice(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, points);
    m_choiceSweepPoints->SetSelection(2);
    m_choiceSweepPoints->SetToolTip("Zulaessige Sweep-Punkte: 155, 313, 625, 1251, 1999, 2501, 5001, 10001, 20001, 30001");
    grid->Add(m_choiceSweepPoints, 1, wxEXPAND);
}

void SettingsDialog::BuildIqFields(wxWindow* parent, wxFlexGridSizer* grid, wxFloatingPointValidator<double>& floatVal)
{
    grid->Add(new wxStaticText(parent, wxID_ANY, "Center Frequenz (Hz):"), 0, wxALIGN_CENTER_VERTICAL);
    m_txtCenterFreq = new wxTextCtrl(parent, wxID_ANY, "100000000");
    m_txtCenterFreq->SetToolTip("Zulaessiger Bereich: 0 bis 26.5 GHz (auch z. B. 100e6, 100 MHz, 0.1 GHz)");
    grid->Add(m_txtCenterFreq, 1, wxEXPAND);

    grid->Add(new wxStaticText(parent, wxID_ANY, "Filter Type:"), 0, wxALIGN_CENTER_VERTICAL);
    wxArrayString filterTypes = {"NORM"};
    m_choiceFilterType = new wxChoice(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, filterTypes);
    m_choiceFilterType->SetSelection(0);
    grid->Add(m_choiceFilterType, 1, wxEXPAND);

    grid->Add(new wxStaticText(parent, wxID_ANY, "Sample Rate (Hz):"), 0, wxALIGN_CENTER_VERTICAL);
    m_txtSampleRate = new wxTextCtrl(parent, wxID_ANY, "32000000");
    m_txtSampleRate->SetToolTip("Zulaessiger Bereich: 10 kHz bis 70.4 MHz (z. B. 32e6, 32 MHz)");
    grid->Add(m_txtSampleRate, 1, wxEXPAND);

    grid->Add(new wxStaticText(parent, wxID_ANY, "Record Length (Samples):"), 0, wxALIGN_CENTER_VERTICAL);
    floatVal.SetPrecision(0);
    m_txtRecordLength = new wxTextCtrl(parent, wxID_ANY, "1024", wxDefaultPosition, wxDefaultSize, 0, floatVal);
    m_txtRecordLength->SetToolTip("Zulaessiger Bereich: 1 bis 16,000,000 Samples");
    grid->Add(m_txtRecordLength, 1, wxEXPAND);

    grid->Add(new wxStaticText(parent, wxID_ANY, "IF Bandwidth (Hz):"), 0, wxALIGN_CENTER_VERTICAL);
    m_txtIfBandwidth = new wxTextCtrl(parent, wxID_ANY, "10000000");
    m_txtIfBandwidth->SetToolTip("Zulaessiger Bereich: 10 Hz bis 50 MHz (z. B. 10e6, 10 MHz)");
    grid->Add(m_txtIfBandwidth, 1, wxEXPAND);

    grid->Add(new wxStaticText(parent, wxID_ANY, "Trigger Quelle:"), 0, wxALIGN_CENTER_VERTICAL);
    wxArrayString trigsources = {"IMM", "EXT", "IFP", "FREE"};
    m_choiceTriggerSource = new wxChoice(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, trigsources);
    m_choiceTriggerSource->SetSelection(0);
    m_choiceTriggerSource->SetToolTip("Zulaessige Trigger-Quellen: IMM, EXT, IFP, FREE");
    grid->Add(m_choiceTriggerSource, 1, wxEXPAND);

    grid->Add(new wxStaticText(parent, wxID_ANY, "Trigger Slope:"), 0, wxALIGN_CENTER_VERTICAL);
    wxArrayString triggerSlopes = {"POS", "NEG"};
    m_choiceTriggerSlope = new wxChoice(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, triggerSlopes);
    m_choiceTriggerSlope->SetSelection(0);
    grid->Add(m_choiceTriggerSlope, 1, wxEXPAND);

    grid->Add(new wxStaticText(parent, wxID_ANY, "Pretrigger Samples:"), 0, wxALIGN_CENTER_VERTICAL);
    m_txtPretriggerSamples = new wxTextCtrl(parent, wxID_ANY, "0");
    m_txtPretriggerSamples->SetToolTip("Ganzzahliger Wert, z. B. 0, 128, 512");
    grid->Add(m_txtPretriggerSamples, 1, wxEXPAND);

    grid->Add(new wxStaticText(parent, wxID_ANY, "Trigger Level (dBm):"), 0, wxALIGN_CENTER_VERTICAL);
    floatVal.SetPrecision(0);
    m_txtTriggerLevel = new wxTextCtrl(parent, wxID_ANY, "-20", wxDefaultPosition, wxDefaultSize, 0, floatVal);
    m_txtTriggerLevel->SetToolTip("Zulaessiger Bereich: -130 bis +30 dBm");
    grid->Add(m_txtTriggerLevel, 1, wxEXPAND);

    grid->Add(new wxStaticText(parent, wxID_ANY, "Trigger Delay (s):"), 0, wxALIGN_CENTER_VERTICAL);
    m_txtTriggerDelay = new wxTextCtrl(parent, wxID_ANY, "0");
    m_txtTriggerDelay->SetToolTip("Zulaessiger Bereich: -1 bis 65 s (auch z. B. 500 ms, 250 us)");
    grid->Add(m_txtTriggerDelay, 1, wxEXPAND);
}

void SettingsDialog::BuildBottomOptions(wxWindow* parent, wxFlexGridSizer* grid)
{
    grid->Add(new wxStaticText(parent, wxID_ANY, wxString::FromUTF8("Messung mit Plotter durchführen")), 0, wxALIGN_CENTER_VERTICAL);
    m_useMultipoint = new wxCheckBox(parent, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, 0);
    m_useMultipoint->SetValue(true);
    grid->Add(m_useMultipoint, 1, wxEXPAND);
}

void SettingsDialog::BuildActionButtons(wxWindow* parent, wxBoxSizer* buttonSizer, wxButton*& btnApply, wxButton*& btnCancel)
{
    m_btnGetCurrentSettings = new wxButton(parent, wxID_ANY, wxT("Get Settings"), wxDefaultPosition, wxDefaultSize, 0);
    btnApply = new wxButton(parent, wxID_APPLY, "Apply");
    m_btnStart = new wxButton(parent, wxID_ANY, "Start");
    btnCancel = new wxButton(parent, wxID_CANCEL, "Cancel");

    m_btnStart->Enable(false);   // TODO change back to False
    buttonSizer->Add(btnCancel, 0, wxRIGHT, 5);
    buttonSizer->Add(m_btnGetCurrentSettings, 0, wxRIGHT, 5);
    buttonSizer->Add(btnApply, 0, wxRIGHT, 5);
    buttonSizer->Add(m_btnStart, 0);
}

void SettingsDialog::BindActionEvents(wxButton* btnApply)
{
    btnApply->Bind(wxEVT_BUTTON, &SettingsDialog::OnApply, this);
    m_btnStart->Bind(wxEVT_BUTTON, &SettingsDialog::OnStart, this);
    m_btnGetCurrentSettings->Bind(wxEVT_BUTTON, &SettingsDialog::OnGetCurrent, this);
}

SettingsDialog::SettingsDialog(wxWindow* parent, MeasurementMode mode, const sData::sParam* preset)
    : wxDialog(parent, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(420, 620)),
      m_mode(mode)
{
    if (preset)
    {
        m_preset = *preset;
        m_hasPreset = true;
    }

    SetDialogTitleForMode();

    // Controls
    wxPanel* panel = new wxPanel(this);
    wxFlexGridSizer* grid = new wxFlexGridSizer(2, wxSize(10, 10));
    grid->AddGrowableCol(1);
    wxFloatingPointValidator<double> floatVal;

    BuildCommonFields(panel, grid, floatVal);
    if (m_mode == MeasurementMode::SWEEP || m_mode == MeasurementMode::MARKER_PEAK) {
        BuildSweepMarkerFields(panel, grid);
    }
    if (m_mode == MeasurementMode::SWEEP) {
        BuildSweepFields(panel, grid);
    }
    if (m_mode == MeasurementMode::IQ) {
        BuildIqFields(panel, grid, floatVal);
    }
    BuildBottomOptions(panel, grid);

    wxButton* btnApply = nullptr;
    wxButton* btnCancel = nullptr;

    // Layout
    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
    mainSizer->Add(grid, 1, wxALL | wxEXPAND, 15);

    wxBoxSizer* buttonSizer = new wxBoxSizer(wxHORIZONTAL);
    BuildActionButtons(panel, buttonSizer, btnApply, btnCancel);
    mainSizer->Add(buttonSizer, 0, wxALL | wxALIGN_RIGHT, 10);

    // Bindings
    BindActionEvents(btnApply);

    panel->SetSizerAndFit(mainSizer);

    wxBoxSizer* rootSizer = new wxBoxSizer(wxVERTICAL);
    rootSizer->Add(panel, 1, wxEXPAND);
    SetSizerAndFit(rootSizer);

    if (m_hasPreset)
        LoadPresetData();
}

SettingsDialog::~SettingsDialog()
{
    if (m_document)
        m_document->RemoveObserver(this);
}

void SettingsDialog::SetDocument(MSetDocument* document)
{
    if (m_document)
        m_document->RemoveObserver(this);

    m_document = document;

    if (m_document)
    {
        m_document->AddObserver(this);
        if (m_hasPreset)
            LoadPresetData();
        else
            RefreshData();
    }
}

void SettingsDialog::OnDocumentChanged(const std::string& changeType)
{
    if (!m_document)
        return;

    if (changeType == "SettingsChanged")
    {
        RefreshData();
    }
    else if (changeType == "ApplyResult")
    {
        const auto& result = m_document->GetLastResult();
        m_btnStart->Enable(result.startEnabled);

        wxMessageBox(wxString::FromUTF8(result.message.c_str()),
                     result.success ? "Erfolg" : "Fehler",
                     wxOK | (result.success ? wxICON_INFORMATION : wxICON_WARNING));
    }
}

//----- Event Handlers -----
void SettingsDialog::OnApply(wxCommandEvent& event) {
    switch (m_mode) {
        case MeasurementMode::SWEEP:       ApplySweep();      break;
        case MeasurementMode::IQ:          ApplyIq();         break;
        case MeasurementMode::MARKER_PEAK: ApplyMarkerPeak(); break;
    }

}

//----- Sweep Apply + Verification -----
void SettingsDialog::ApplySweep() {
    if (!m_document)
        return;

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

    // Number of points
    int points = wxAtoi(m_choiceSweepPoints->GetStringSelection());

    fsuMeasurement::lastSweepSettings settings{};
    settings.startFreq  = startFreq;
    settings.stopFreq   = stopFreq;
    settings.refLevel   = refLevel;
    settings.points     = points;
    settings.detector   = m_choiceDetector->GetStringSelection().ToStdString();
    settings.att        = m_spinAttenuation->GetValue();
    settings.rbw        = static_cast<int>(rbw);
    settings.vbw        = static_cast<int>(vbw);
    settings.unit       = m_choiceUnit->GetStringSelection().ToStdString();

    m_document->ApplySweep(settings);
}

//----- IQ Apply + Verification -----
void SettingsDialog::ApplyIq() {
    if (!m_document)
        return;

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
    int pretriggerSamples = wxAtoi(m_txtPretriggerSamples->GetValue());
    std::string trigSrc = m_choiceTriggerSource
        ? m_choiceTriggerSource->GetStringSelection().ToStdString()
        : "IMM";
    std::string filterType = m_choiceFilterType
        ? m_choiceFilterType->GetStringSelection().ToStdString()
        : "NORM";
    std::string trigSlope = m_choiceTriggerSlope
        ? m_choiceTriggerSlope->GetStringSelection().ToStdString()
        : "POS";

    fsuMeasurement::IqSettings settings{};
    settings.centerFreq    = centerFreq;
    settings.refLevel      = refLevel;
    settings.att           = m_spinAttenuation->GetValue();
    settings.unit          = m_choiceUnit->GetStringSelection().ToStdString();
    settings.filterType    = filterType;
    settings.sampleRate    = sampleRate;
    settings.recordLength  = recordLen;
    settings.ifBandwidth   = ifBw;
    settings.triggerSource = trigSrc;
    settings.triggerSlope  = trigSlope;
    settings.pretriggerSamples = pretriggerSamples;
    settings.triggerLevel  = trigLevel;
    settings.triggerDelay  = trigDelay;

    m_document->ApplyIq(settings);
}

//----- Marker Peak Apply + Verification -----
void SettingsDialog::ApplyMarkerPeak() {
    if (!m_document)
        return;

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

    fsuMeasurement::MarkerPeakSettings settings{};
    settings.startFreq = startFreq;
    settings.stopFreq  = stopFreq;
    settings.refLevel  = refLevel;
    settings.att       = m_spinAttenuation->GetValue();
    settings.unit      = m_choiceUnit->GetStringSelection().ToStdString();
    settings.rbw       = static_cast<int>(rbw);
    settings.vbw       = static_cast<int>(vbw);
    settings.detector  = m_choiceDetector->GetStringSelection().ToStdString();

    m_document->ApplyMarkerPeak(settings);
}

//----- Refresh Data -----
void SettingsDialog::RefreshData()
{
    if (!m_document)
        return;

    switch (m_mode) {
        case MeasurementMode::SWEEP: {
            auto s = m_document->GetSweepSettings();
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
            auto s = m_document->GetIqSettings();
            m_txtRefLevel         ->SetValue(wxString::Format(wxT("%.0f"),s.refLevel));
            m_spinAttenuation     ->SetValue(s.att);
            m_choiceUnit          ->SetStringSelection(s.unit);
            m_txtCenterFreq       ->SetValue(wxString::Format(wxT("%.0f"),s.centerFreq));
            m_txtCenterFreq       ->SetToolTip(FormatFrequencyAutoUnit(s.centerFreq));
            if (m_choiceFilterType) {
                m_choiceFilterType->SetStringSelection(s.filterType);
            }
            m_txtSampleRate       ->SetValue(wxString::Format(wxT("%.0f"),s.sampleRate));
            m_txtSampleRate       ->SetToolTip(FormatFrequencyAutoUnit(s.sampleRate));
            m_txtRecordLength     ->SetValue(wxString::Format(wxT("%i"),s.recordLength));
            m_txtIfBandwidth      ->SetValue(wxString::Format(wxT("%.0f"),s.ifBandwidth));
            m_txtIfBandwidth      ->SetToolTip(FormatFrequencyAutoUnit(s.ifBandwidth));
            if (m_choiceTriggerSource) {
                m_choiceTriggerSource->SetStringSelection(s.triggerSource);
            }
            if (m_choiceTriggerSlope) {
                m_choiceTriggerSlope->SetStringSelection(s.triggerSlope);
            }
            if (m_txtPretriggerSamples) {
                m_txtPretriggerSamples->SetValue(wxString::Format(wxT("%d"), s.pretriggerSamples));
            }
            m_txtTriggerLevel     ->SetValue(wxString::Format(wxT("%.0f"),s.triggerLevel));
            m_txtTriggerDelay     ->SetValue(wxString::Format(wxT("%.0f"),s.triggerDelay));
            break;
        }
        case MeasurementMode::MARKER_PEAK: {
            auto s = m_document->GetMarkerPeakSettings();
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

void SettingsDialog::LoadPresetData()
{
    const sData::sParam& s = m_preset;

    switch (m_mode)
    {
        case MeasurementMode::SWEEP:
        {
            if (m_txtStartFreq)      m_txtStartFreq->SetValue(wxString::Format(wxT("%.0f"), s.startFreq));
            if (m_txtStopFreq)       m_txtStopFreq->SetValue(wxString::Format(wxT("%.0f"), s.endFreq));
            if (m_txtRefLevel)       m_txtRefLevel->SetValue(wxString::Format(wxT("%.0f"), s.refPegel));
            if (m_spinAttenuation)   m_spinAttenuation->SetValue(s.HFDaempfung);
            if (m_choiceUnit && !s.ampUnit.empty())
            {
                wxString unit = wxString::FromUTF8(s.ampUnit.c_str());
                if (m_choiceUnit->FindString(unit) != wxNOT_FOUND)
                    m_choiceUnit->SetStringSelection(unit);
            }
            if (m_txtRBW)            m_txtRBW->SetValue(wxString::Format(wxT("%d"), s.RBW));
            if (m_txtVBW)            m_txtVBW->SetValue(wxString::Format(wxT("%d"), s.VBW));
            if (m_choiceSweepPoints)
                m_choiceSweepPoints->SetStringSelection(wxString::Format("%d", s.NoPoints_Array));
            if (m_choiceDetector && !s.detektor.empty())
            {
                wxString detector = wxString::FromUTF8(s.detektor.c_str());
                if (m_choiceDetector->FindString(detector) != wxNOT_FOUND)
                    m_choiceDetector->SetStringSelection(detector);
            }
            break;
        }
        case MeasurementMode::IQ:
        {
            if (m_txtRefLevel)       m_txtRefLevel->SetValue(wxString::Format(wxT("%.0f"), s.refPegel));
            if (m_spinAttenuation)   m_spinAttenuation->SetValue(s.HFDaempfung);
            if (m_choiceUnit && !s.ampUnit.empty())
            {
                wxString unit = wxString::FromUTF8(s.ampUnit.c_str());
                if (m_choiceUnit->FindString(unit) != wxNOT_FOUND)
                    m_choiceUnit->SetStringSelection(unit);
            }
            if (m_txtCenterFreq)     m_txtCenterFreq->SetValue(wxString::Format(wxT("%.0f"), s.centerFreq));
            if (m_txtSampleRate)     m_txtSampleRate->SetValue(wxString::Format(wxT("%.0f"), s.sampleRate));
            if (m_txtRecordLength)   m_txtRecordLength->SetValue(wxString::Format(wxT("%d"), s.recordLength));
            if (m_txtIfBandwidth)    m_txtIfBandwidth->SetValue(wxString::Format(wxT("%.0f"), s.ifBandwidth));
            if (m_choiceTriggerSource && !s.triggerSource.empty())
            {
                wxString triggerSource = wxString::FromUTF8(s.triggerSource.c_str());
                if (m_choiceTriggerSource->FindString(triggerSource) != wxNOT_FOUND)
                    m_choiceTriggerSource->SetStringSelection(triggerSource);
            }
            if (m_txtTriggerLevel)   m_txtTriggerLevel->SetValue(wxString::Format(wxT("%.0f"), s.triggerLevel));
            if (m_txtTriggerDelay)   m_txtTriggerDelay->SetValue(wxString::Format(wxT("%.0f"), s.triggerDelay));
            break;
        }
        case MeasurementMode::MARKER_PEAK:
        {
            if (m_txtStartFreq)      m_txtStartFreq->SetValue(wxString::Format(wxT("%.0f"), s.startFreq));
            if (m_txtStopFreq)       m_txtStopFreq->SetValue(wxString::Format(wxT("%.0f"), s.endFreq));
            if (m_txtRefLevel)       m_txtRefLevel->SetValue(wxString::Format(wxT("%.0f"), s.refPegel));
            if (m_spinAttenuation)   m_spinAttenuation->SetValue(s.HFDaempfung);
            if (m_choiceUnit && !s.ampUnit.empty())
            {
                wxString unit = wxString::FromUTF8(s.ampUnit.c_str());
                if (m_choiceUnit->FindString(unit) != wxNOT_FOUND)
                    m_choiceUnit->SetStringSelection(unit);
            }
            if (m_txtRBW)            m_txtRBW->SetValue(wxString::Format(wxT("%d"), s.RBW));
            if (m_txtVBW)            m_txtVBW->SetValue(wxString::Format(wxT("%d"), s.VBW));
            if (m_choiceDetector && !s.detektor.empty())
            {
                wxString detector = wxString::FromUTF8(s.detektor.c_str());
                if (m_choiceDetector->FindString(detector) != wxNOT_FOUND)
                    m_choiceDetector->SetStringSelection(detector);
            }
            break;
        }
        default:
            break;
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

//----- Start / Readback Handlers -----
void SettingsDialog::OnStart(wxCommandEvent& /*event*/)
{
    EndModal(wxID_OK);

    PrologixUsbGpibAdapter::get_instance().resetGpibBusBuffer();

    if (m_useMultipoint->GetValue()) {
        PlotterFrame::ShowOrRaise();
    } else {
        fsuMeasurement* fsu = &fsuMeasurement::get_instance();

        MeasurementDocument* measDoc = new MeasurementDocument(
            PrologixUsbGpibAdapter::get_instance(), fsuMeasurement::get_instance());

        PlotWindow* mw = new PlotWindow(GetParent());
        mw->SetDocument(measDoc);
        mw->SetOwnsDocument(true);
        mw->Show();
    }
}


void SettingsDialog::OnGetCurrent(wxCommandEvent& /*event*/)
{
    if (!m_document)
        return;

    m_document->ReadCurrent();

}

