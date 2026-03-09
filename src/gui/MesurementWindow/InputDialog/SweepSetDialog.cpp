#include "SweepSetDialog.h"
#include "fkt_GPIB.h"
#include "cmdGpib.h"
#include <wx/valnum.h> // Für numerische Validatoren auf GUI-Ebene

wxBEGIN_EVENT_TABLE(SettingsDialog, wxDialog)
    EVT_BUTTON(wxID_APPLY, SettingsDialog::OnApply)
wxEND_EVENT_TABLE()

SettingsDialog::SettingsDialog(wxWindow* parent) 
    : wxDialog(parent, wxID_ANY, "Spektrumanalysator Einstellungen", wxDefaultPosition, wxSize(400, 500)) 
{
    // Sizer für 2 Spalten: Label und Eingabefeld
    wxFlexGridSizer* grid = new wxFlexGridSizer(2, wxSize(10, 10));
    grid->AddGrowableCol(1);

    // Hilfsfunktion zum sauberen Erstellen von Text-Inputs mit Nummern-Filter
    wxFloatingPointValidator<double> floatVal;
    
    // Frequenzen & Pegel
    grid->Add(new wxStaticText(this, wxID_ANY, "Start Frequenz (Hz):"), 0, wxALIGN_CENTER_VERTICAL);
    m_txtStartFreq = new wxTextCtrl(this, wxID_ANY, "1000000", wxDefaultPosition, wxDefaultSize, 0, floatVal);
    grid->Add(m_txtStartFreq, 1, wxEXPAND);

    grid->Add(new wxStaticText(this, wxID_ANY, "Stop Frequenz (Hz):"), 0, wxALIGN_CENTER_VERTICAL);
    m_txtStopFreq = new wxTextCtrl(this, wxID_ANY, "1000000000", wxDefaultPosition, wxDefaultSize, 0, floatVal);
    grid->Add(m_txtStopFreq, 1, wxEXPAND);

    grid->Add(new wxStaticText(this, wxID_ANY, "Ref. Pegel (dBm):"), 0, wxALIGN_CENTER_VERTICAL);
    m_txtRefLevel = new wxTextCtrl(this, wxID_ANY, "0", wxDefaultPosition, wxDefaultSize, 0, floatVal);
    grid->Add(m_txtRefLevel, 1, wxEXPAND);

    // Dämpfung (Schritte via SpinCtrl vorfiltern)
    grid->Add(new wxStaticText(this, wxID_ANY, "HF-Dämpfung (dB):"), 0, wxALIGN_CENTER_VERTICAL);
    m_spinAttenuation = new wxSpinCtrl(this, wxID_ANY);
    m_spinAttenuation->SetRange(0, 75);
    m_spinAttenuation->SetIncrement(5); // 5-dB Schritte erzwingen
    grid->Add(m_spinAttenuation, 1, wxEXPAND);

    // Bandbreiten
    grid->Add(new wxStaticText(this, wxID_ANY, "RBW (Hz):"), 0, wxALIGN_CENTER_VERTICAL);
    m_txtRBW = new wxTextCtrl(this, wxID_ANY, "10000", wxDefaultPosition, wxDefaultSize, 0, floatVal);
    grid->Add(m_txtRBW, 1, wxEXPAND);

    grid->Add(new wxStaticText(this, wxID_ANY, "VBW (Hz):"), 0, wxALIGN_CENTER_VERTICAL);
    m_txtVBW = new wxTextCtrl(this, wxID_ANY, "10000", wxDefaultPosition, wxDefaultSize, 0, floatVal);
    grid->Add(m_txtVBW, 1, wxEXPAND);

    // Feste Auswahlmöglichkeiten für Einheiten, Punkte und Detektoren
    grid->Add(new wxStaticText(this, wxID_ANY, "Einheit:"), 0, wxALIGN_CENTER_VERTICAL);
    wxArrayString units = {"DBM", "V", "W", "DBUV"};
    m_choiceUnit = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, units);
    m_choiceUnit->SetSelection(0);
    grid->Add(m_choiceUnit, 1, wxEXPAND);

    grid->Add(new wxStaticText(this, wxID_ANY, "Sweep Punkte:"), 0, wxALIGN_CENTER_VERTICAL);
    wxArrayString points = {"155", "313", "625", "1251", "1999", "2501", "5001", "10001", "20001", "30001"};
    m_choiceSweepPoints = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, points);
    m_choiceSweepPoints->SetSelection(2); // 625 als Standard
    grid->Add(m_choiceSweepPoints, 1, wxEXPAND);

    grid->Add(new wxStaticText(this, wxID_ANY, "Detektor:"), 0, wxALIGN_CENTER_VERTICAL);
    wxArrayString detectors = {"APE", "POS", "NEG", "SAMP", "RMS", "AVER"};
    m_choiceDetector = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, detectors);
    m_choiceDetector->SetSelection(1);
    grid->Add(m_choiceDetector, 1, wxEXPAND);

    // Layout zusammensetzen
    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
    mainSizer->Add(grid, 1, wxALL | wxEXPAND, 15);

    // Buttons (Apply / Close)
    wxSizer* buttonSizer = CreateButtonSizer(wxAPPLY | wxCANCEL);
    mainSizer->Add(buttonSizer, 0, wxALL | wxALIGN_RIGHT, 10);

    SetSizer(mainSizer);

    // get current settings
    RefreshData();

}

void SettingsDialog::OnApply(wxCommandEvent& event) {
    // 1. Werte aus der GUI auslesen
    double startFreq;
    m_txtStartFreq->GetValue().ToDouble(&startFreq);
    double stopFreq;
    m_txtStopFreq->GetValue().ToDouble(&stopFreq);
    double refLevel;
    m_txtRefLevel->GetValue().ToDouble(&refLevel);
    double rbw;
    m_txtRBW->GetValue().ToDouble(&rbw);
    double vbw;
    m_txtVBW->GetValue().ToDouble(&vbw);
    
    int points = wxAtoi(m_choiceSweepPoints->GetStringSelection());
    std::string det = m_choiceDetector->GetStringSelection().ToStdString();
    int att = m_spinAttenuation->GetValue();
    fsuMeasurement* fsu = &fsuMeasurement::get_instance();

    // 2. Gegen deine Logik validieren (Beispiele)
    if (!fsu->checkIfSettingsValidSweep(ScpiCommand::START_FREQUENCY, startFreq)) {
        wxMessageBox("Startfrequenz ausserhalb des Bereichs (0 - 26.5 GHz)!", "Validierungsfehler", wxOK | wxICON_ERROR);
        return; 
    }

    if (!fsu->checkIfSettingsValidSweep(ScpiCommand::SWEEP_POINTS, points)) {
        wxMessageBox("Ungueltige Sweep Punkte!", "Validierungsfehler", wxOK | wxICON_ERROR);
        return; 
    }
    
    if (!fsu->checkIfSettingsValidSweep(ScpiCommand::RF_ATTENUATION, att)) {
        wxMessageBox("Daempfung muss in 5-dB Schritten erfolgen!", "Validierungsfehler", wxOK | wxICON_ERROR);
        return; 
    }

        if (!fsu->checkIfSettingsValidSweep(ScpiCommand::RBW, rbw)) {
        wxMessageBox("RWB Ausserhalb des glütigen Bereichs! (1- 50 MHZ)", "Validierungsfehler", wxOK | wxICON_ERROR);
        return; 
    }

        if (!fsu->checkIfSettingsValidSweep(ScpiCommand::VBW, vbw)) {
        wxMessageBox("VBW Ausserhalb des glütigen Bereichs! (1- 50 MHZ)", "Validierungsfehler", wxOK | wxICON_ERROR);
        return; 
    }

    fsuMeasurement::lastSweepSettings lastSettingsTemp;

    lastSettingsTemp.startFreq  = startFreq;
    lastSettingsTemp.stopFreq   = stopFreq;
    lastSettingsTemp.refLevel   = refLevel;
    lastSettingsTemp.points     = points;
    lastSettingsTemp.detector   = det;
    lastSettingsTemp.att        = att;
    lastSettingsTemp.rbw        = rbw;
    lastSettingsTemp.vbw        = vbw;
    lastSettingsTemp.unit       = m_choiceUnit->GetStringSelection().ToStdString();

    fsu->writeSweepSettings(lastSettingsTemp);

    // Wenn alles okay ist: Befehle in Liste packen und an Prologix senden...
    wxMessageBox("Alle Eingaben korrekt! Sende an Analysator...", "Erfolg", wxOK | wxICON_INFORMATION);
    
    // z.B. sendScpiCommand(scpiSetCommands[ScpiCommand::START_FREQUENCY] + std::to_string(startFreq));
}

void SettingsDialog::RefreshData()
{
    fsuMeasurement* fsu = &fsuMeasurement::get_instance();

    fsuMeasurement::lastSweepSettings lastSettings = fsu->returnSweepSettings();

    m_txtStartFreq      ->SetValue(std::to_string(lastSettings.startFreq));
    m_txtStopFreq       ->SetValue(std::to_string(lastSettings.stopFreq));
    m_txtRefLevel       ->SetValue(std::to_string(lastSettings.refLevel));
    m_spinAttenuation   ->SetValue(lastSettings.att);
    m_choiceUnit        ->SetStringSelection(lastSettings.unit);
    m_choiceSweepPoints ->SetStringSelection(std::to_string(lastSettings.points));
    m_choiceDetector    ->SetStringSelection(lastSettings.detector);
}