#pragma once

#include <wx/wx.h>
#include <wx/notebook.h>
#include "SettingsDocument.h"


//------Subtabs (forward declarations)------
class SettingsTabDisplay;
class SettingsTabAdapter;
class SettingsTabGeneral;


//-----Settings Window-----
class SettingsWindow : public wxDialog
{
public:
    SettingsWindow(wxWindow *parent);

    /**
     * Attach/detach the document. Forwards to the Display tab.
     * Call SetDocument(&doc) before ShowModal() and
     * SetDocument(nullptr) before Destroy().
     */
    void SetDocument(SettingsDocument* doc);

private:
    SettingsTabDisplay* m_displayTab = nullptr;
};


//------Subtabs------
class SettingsTabDisplay : public wxPanel, public ISettingsObserver
{
public:
    SettingsTabDisplay(wxNotebook *parent, const wxString &label);

    // Document/View wiring
    void SetDocument(SettingsDocument* doc);

    // ISettingsObserver
    void OnDocumentChanged(const std::string& changeType) override;

private:
    // Event handlers
    void OnAnwenden(wxCommandEvent& event);
    void OnGetCurrent(wxCommandEvent& event);
    void OnToggleSelection(wxCommandEvent& event);

    // View helpers
    void RefreshFromDocument();
    void UpdateToggleWidgetState();

    // Non-owning pointer to the document (owned by main.cpp)
    SettingsDocument* m_doc = nullptr;

    //Elemente
    wxCheckBox* m_startEndeCheck;
    wxCheckBox* m_centerSpanCheck;

    wxChoice* m_freqEinheitAuswahl_1;
    wxChoice* m_freqEinheitAuswahl_2;
    wxChoice* m_freqEinheitAuswahl_3;
    wxChoice* m_freqEinheitAuswahl_4;
    wxChoice* m_pegelEinheitAuswahl;
    wxChoice* m_yScalingAuswahl;

    wxTextCtrl* m_inputText_1;
    wxTextCtrl* m_inputText_2;
    wxTextCtrl* m_inputText_3;
    wxTextCtrl* m_inputText_4;
    wxTextCtrl* m_inputText_5;
    wxTextCtrl* m_inputText_7;
};

class SettingsTabAdapter : public wxPanel
{
public:
    SettingsTabAdapter(wxNotebook *parent, const wxString &label);
};

class SettingsTabGeneral : public wxPanel
{
public:
    SettingsTabGeneral(wxNotebook *parent, const wxString &label);
};
//-----end Subtabs-----