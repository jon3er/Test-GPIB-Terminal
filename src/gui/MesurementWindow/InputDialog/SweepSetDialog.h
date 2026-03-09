#pragma once
#include <wx/wx.h>
#include <wx/spinctrl.h>

class SettingsDialog : public wxDialog {
public:
    SettingsDialog(wxWindow* parent);

private:
    // UI Elemente
    wxTextCtrl* m_txtStartFreq;
    wxTextCtrl* m_txtStopFreq;
    wxTextCtrl* m_txtRefLevel;
    wxTextCtrl* m_txtRBW;
    wxTextCtrl* m_txtVBW;
    wxTextCtrl* m_txtSweepTime;
    
    wxSpinCtrl* m_spinAttenuation;
    
    wxChoice* m_choiceUnit;
    wxChoice* m_choiceSweepPoints;
    wxChoice* m_choiceDetector;

    // Event Handler
    void OnApply(wxCommandEvent& event);

    // helper
    void RefreshData();
    
    wxDECLARE_EVENT_TABLE();
};