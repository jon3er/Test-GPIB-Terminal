#pragma once

#include <wx/wx.h>
#include <wx/notebook.h>


//-----Settings Window-----
class SettingsWindow : public wxDialog
{
public:
    SettingsWindow(wxWindow *parent);
    //virtual ~SettingsWindow();

private:

};


//------Subtabs------
class SettingsTabDisplay : public wxPanel
{
public:
    SettingsTabDisplay(wxNotebook *parent, const wxString &label);

private:
    void anwendenButton(wxCommandEvent& event);
    void getCurrentButton(wxCommandEvent& event);
    void toggleSelectionEvent(wxCommandEvent& event);
    void toggleSelection();
    void getValues();
    void setValues();

    std::string getGpibCmdFreq(wxString NumVal, wxString Selection);
    std::string getGpibCmdPegel(wxString NumVal, wxString Selection);

    //Units
    wxString m_FreqStartSetUnit;
    wxString m_FreqEndeSetUnit;
    wxString m_FreqCenterSetUnit;
    wxString m_FreqSpanSetUnit;
    wxString m_pegelSetUnit;
    wxString m_scalingYSetUnit;
    //Text Input
    wxString m_FreqStartSet;
    wxString m_FreqEndeSet;
    wxString m_FreqCenterSet;
    wxString m_FreqSpanSet;
    wxString m_pegelSet;
    wxString m_refPegelSet;
    //Bool
    bool m_useStartEnde = false;
    bool m_useCenterSpan = false;

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