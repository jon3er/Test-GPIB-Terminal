#pragma once
#include <wx/wx.h>
#include <wx/spinctrl.h>
#include "cmdGpib.h"
#include "PlotterFrame.h"
#include "dataManagement.h"

class SettingsDialog : public wxDialog {
public:
    SettingsDialog(wxWindow* parent, MeasurementMode mode, const sData::sParam* preset = nullptr);

private:
    MeasurementMode m_mode;
    bool m_hasPreset = false;
    sData::sParam m_preset{};

    // Gemeinsame Widgets (alle Modi)
    wxTextCtrl* m_txtRefLevel      = nullptr;
    wxSpinCtrl* m_spinAttenuation  = nullptr;
    wxChoice*   m_choiceUnit       = nullptr;

    // Sweep + MarkerPeak
    wxTextCtrl* m_txtStartFreq     = nullptr;
    wxTextCtrl* m_txtStopFreq      = nullptr;
    wxTextCtrl* m_txtRBW           = nullptr;
    wxTextCtrl* m_txtVBW           = nullptr;
    wxChoice*   m_choiceDetector   = nullptr;

    // Nur Sweep
    wxChoice*   m_choiceSweepPoints = nullptr;

    // Nur IQ
    wxTextCtrl* m_txtCenterFreq    = nullptr;
    wxTextCtrl* m_txtSampleRate    = nullptr;
    wxTextCtrl* m_txtRecordLength  = nullptr;
    wxTextCtrl* m_txtIfBandwidth   = nullptr;
    wxChoice*   m_choiceTriggerSource = nullptr;
    wxTextCtrl* m_txtTriggerLevel  = nullptr;
    wxTextCtrl* m_txtTriggerDelay  = nullptr;

    // Options
    wxCheckBox* m_useMultipoint = nullptr;

    // Buttons
    wxButton* m_btnStart = nullptr;
    wxButton* m_btnGetCurrentSettings = nullptr;
    // Plotter window
    PlotterFrame* m_plotterWindow = nullptr;

    // Event Handler
    void OnApply(wxCommandEvent& event);
    void OnStart(wxCommandEvent& event);
    void OnGetCurrent(wxCommandEvent& event);

    // Mode-spezifische Apply + Verifikation
    void ApplySweep();
    void ApplyIq();
    void ApplyMarkerPeak();

    void RefreshData();
    void LoadPresetData();
    wxString FormatFrequencyAutoUnit(double hz) const;
    bool ParseFrequencyInputToHz(const wxString& input, double& hz) const;
    bool ParseTimeInputToSeconds(const wxString& input, double& seconds) const;

    // Verifikations-Helfer
    bool VerifyDouble(const wxString& name, double written, double readback, wxString& mismatches);
    bool VerifyInt(const wxString& name, int written, int readback, wxString& mismatches);
    bool VerifyString(const wxString& name, const std::string& written, const std::string& readback, wxString& mismatches);

    wxDECLARE_EVENT_TABLE();
};
