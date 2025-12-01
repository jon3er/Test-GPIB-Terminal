#pragma once

#include <wx/wx.h>
#include "mathplot.h"

#if defined(_WIN32)
    wxString filePathSytem = "D:\\CodeProjects\\VSCode\\projects\\Diplom\\Test-GPIB-Terminal\\GpibScripts\\";
#elif defined(__linux__)
    wxString filePathSytem = "/home/jon3r/Documents/Code/CodeBlocks/Test_GPIB_Terminal/GpibScripts/";
#endif


enum
{
    ID_Hello = 01,
    ID_OpenTerminal = 02,

    ID_Main_File = 10,
    ID_Main_File_Open = 11,
    ID_Main_File_Save = 12,
    ID_Main_File_Close = 13,

    ID_Main_Simulation = 20,

    ID_Main_Mesurement = 30,
    ID_Main_Mesurement_New = 31,
    ID_Main_Mesurement_Load = 32,
    ID_Main_Mesurement_Settings = 33,

    ID_Main_Processing = 40,

    ID_Main_Test = 50,
    ID_Main_Test_Terminal = 51,
    ID_Main_Test_Func = 52,

    ID_Main_Help = 60,
    ID_Main_Help_About = 61
};

//-----Global Varibles Start------
GpibDevice Adapter;

//-----Global Varibles Ende----

//-----Main Programm Window
class MainProgrammWin : public wxFrame
{
private:

protected:
	wxMenuBar* m_menubarMainProg;
	wxMenu* m_menu_File;
	wxMenu* m_menu_Sim;
	wxMenu* m_menu_Mesurement;
	wxMenu* m_menu_Processing;
	wxMenu* m_menu_Test;
	wxMenu* m_menu_Help;
	wxPanel* m_panel21;
	wxStaticText* m_staticText1;
	wxTextCtrl* m_textCtrlAdapterStatus;
	wxStaticText* m_staticText11;
	wxTextCtrl* m_textCtrlDeviceStatus;
	wxPanel* m_panel2;
	wxButton* m_button1;

public:
	
	MainProgrammWin( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("GPIB Messurement"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 814,454 ), long style = wxDEFAULT_FRAME_STYLE|wxTAB_TRAVERSAL );
	~MainProgrammWin();
    // Button func
    void ButtonRefresh(wxCommandEvent& event);    
    // Menubar items
    void MenuFileOpen(wxCommandEvent& event);
    void MenuFileSave(wxCommandEvent& event);
    void MenuFileClose(wxCommandEvent& event);

    void MenuMesurementNew(wxCommandEvent& event);
	void MenuMesurementLoad(wxCommandEvent& event);
    void MenuMesurementSettings(wxCommandEvent& event);

    void MenuTestTerminal(wxCommandEvent& event);
    void MenuTestFunc(wxCommandEvent& event);

    void MenuHelpAbout(wxCommandEvent& event);
};


//-----MainWin-----
class MainWin : public wxApp
{
public:
    bool OnInit() override; //overrides function of Base Classe wxApp Function OnInit()
};
/*
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
*/

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
    //GpibDevice Adapter;
};

class PlotWindow : public wxDialog
{
public:
    PlotWindow(wxWindow *parent);
    virtual ~PlotWindow();
private:

    wxString filePath = filePathSytem;
    wxArrayString fileNames;

    void getFileNames(const wxString& dirPath, wxArrayString& files);
    void executeScriptEvent(wxCommandEvent& event);
    void updatePlotData();

    wxChoice* selectMesurement;
    //Plot var
    mpWindow* plot;
    mpFXYVector* vectorLayer;
    std::vector<double> x;
    std::vector<double> y;

    //device Class
    //GpibDevice Adapter;
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
    //GpibDevice Adapter;
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
    //GpibDevice Adapter;
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
