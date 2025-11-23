#pragma once

#include <wx/wx.h>


enum
{
    ID_Hello = 1,
    ID_OpenTerminal = 2
};

//-----MainWin-----
class MainWin : public wxApp
{
public:
    bool OnInit() override; //overrides function of Base Classe wxApp Function OnInit()
};

//-----MainWinFrame-----
class MainWinFrame : public wxFrame
{
public:
    MainWinFrame();

private:
    wxTextCtrl* ScanUsbDisplay;

    //menubar methodes
    void OnHello(wxCommandEvent& event);
    void OnExit(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);
    //buttons methodes
    void OnOpenTerminal(wxCommandEvent& event);
    void OnScanUsb(wxCommandEvent& event);
    void OnOpenFunctionTest(wxCommandEvent& event);
    void OnOpenPlot(wxCommandEvent& event);
    void OnOpenUploadScript(wxCommandEvent& event);
    void OnOpenSettings(wxCommandEvent& event);
};

//-----Terminal-----
class TerminalWindow : public wxDialog
{
public:
    TerminalWindow(wxWindow *parent);
    virtual ~TerminalWindow();

private:
    void OnEnterTerminal(wxCommandEvent& event);
    //Map Functions
    void setupCmds();
    //Command Functions
    using CommandMap = std::map<std::string, std::function<void(const std::string&)>>;
    CommandMap cmds;
    void scanDevices(const std::string& args);
    void statusDevice(const std::string& args);
    void configDevice(const std::string& args);
    void connectDevice(const std::string& args);
    void disconnectDevice(const std::string& args);
    wxString sendToDevice(const std::string& args);
    wxString readFromDevice(const std::string& args);
    void writeToDevice(const std::string& args);
    void testDevice(const std::string& args);

    //Terminal Output
    wxTextCtrl* TerminalDisplay;

    //device Class
    GpibDevice Adapter;
};

class PlotWindow : public wxDialog
{
public:
    PlotWindow(wxWindow *parent);
private:

};


class FunctionWindow : public wxDialog
{
public:
    FunctionWindow(wxWindow *parent);
    virtual ~FunctionWindow();

private:
    //Button Functions
    void OnWriteGpib(wxCommandEvent& event);
    void OnReadGpib(wxCommandEvent& event);
    void OnReadWriteGpib(wxCommandEvent& event);
    void OnUsbScan(wxCommandEvent& event);
    void OnUsbConfig(wxCommandEvent& event);
    void OnConDisconGpib(wxCommandEvent& event);

    //Text Boxes
    wxTextCtrl* textFuncOutput;
    wxTextCtrl* writeFuncInput;

    //device Class
    GpibDevice Adapter;
};
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
    //Adapter
    GpibDevice Adapter;
    //Units
    wxString FreqStartSetUnit;
    wxString FreqEndeSetUnit; 
    wxString FreqCenterSetUnit; 
    wxString FreqSpanSetUnit; 
    wxString pegelSetUnit;
    wxString scalingYSetUnit;
    //Text Input
    wxString FreqStartSet;
    wxString FreqEndeSet; 
    wxString FreqCenterSet; 
    wxString FreqSpanSet; 
    wxString pegelSet;
    wxString refPegelSet;
    //Bool
    bool useStartEnde = false;
    bool useCenterSpan = false;

    //Elemente
    wxCheckBox* startEndeCheck;
    wxCheckBox* centerSpanCheck;

    wxChoice* freqEinheitAuswahl_1;
    wxChoice* freqEinheitAuswahl_2;
    wxChoice* freqEinheitAuswahl_3;
    wxChoice* freqEinheitAuswahl_4;
    wxChoice* pegelEinheitAuswahl;
    wxChoice* yScalingAuswahl;

    wxTextCtrl* inputText_1;
    wxTextCtrl* inputText_2;
    wxTextCtrl* inputText_3;
    wxTextCtrl* inputText_4;
    wxTextCtrl* inputText_5;
    wxTextCtrl* inputText_7;

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