#pragma once

#include <wx/wx.h>
#include "mathplot.h"

#if defined(_WIN32)
    wxString filePathSytem = "D:\\CodeProjects\\VSCode\\projects\\Diplom\\Test-GPIB-Terminal\\GpibScripts\\";
    wxString filePathRoot = "D:\\CodeProjects\\VSCode\\projects\\Diplom\\Test-GPIB-Terminal\\";
    wxString fileSystemSlash = "\\";
#elif defined(__linux__)
    wxString filePathSytem = "/home/jon3r/Documents/Code/CodeBlocks/Test_GPIB_Terminal/GpibScripts/";
    wxString filePathRoot = "/home/jon3r/Documents/Code/CodeBlocks/Test_GPIB_Terminal/";
    wxString fileSystemSlash = "/";
#endif


enum
{
    ID_Hello = 01,
    ID_OpenTerminal = 02,

    ID_Main_File = 10,
    ID_Main_File_Open = 11,
    ID_Main_File_Close = 12,
    ID_Main_File_Save = 13,
    ID_Main_File_SaveAs = 14,
    ID_Main_File_Exit = 15,

    ID_Main_Simulation = 20,

    ID_Main_Mesurement = 30,
    ID_Main_Mesurement_New = 31,
    ID_Main_Mesurement_Open = 32,
    ID_Main_Mesurement_Load = 33,
    ID_Main_Mesurement_Preset_1 = 34,
    ID_Main_Mesurement_Preset_2 = 35,
    ID_Main_Mesurement_Preset_3 = 36,
    ID_Main_Mesurement_2D_Mess = 37,
    ID_Main_Mesurement_SetMarker = 38,
    ID_Main_Mesurement_Settings = 39,

    ID_Main_Processing = 40,

    ID_Main_Test = 50,
    ID_Main_Test_Terminal = 51,
    ID_Main_Test_Func = 52,

    ID_Main_Help = 60,
    ID_Main_Help_About = 61
};

//-----Global Varibles Start------
GpibDevice Adapter;
fsuMesurement Messung;

//-----Global Varibles Ende----

//-----MainWin-----
class MainWin : public wxApp
{
public:
    bool OnInit() override; //overrides function of Base Classe wxApp Function OnInit()
};


//-----Main Programm Window
class MainProgrammWin : public wxFrame
{
private:
    wxString filePathCurrentFile;
    sData OpendData;
    bool fileOpen;
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
    //PassData
    sData returnOpendData() { return OpendData; };
    bool isFileOpen() { return fileOpen; };
    // Button func
    void ButtonRefresh(wxCommandEvent& event);
    // Menubar items
    void MenuFileOpen(wxCommandEvent& event);
    void MenuFileClose(wxCommandEvent& event);
    void MenuFileSave(wxCommandEvent& event);
    void MenuFileSaveAs(wxCommandEvent& event);
    void MenuFileExit(wxCommandEvent& event);


    void MenuMesurementNew(wxCommandEvent& event);
	void MenuMesurementLoad(wxCommandEvent& event);
    void MenuMesurementSetMarker(wxCommandEvent& event);
    void MenuMesurement2DMess(wxCommandEvent& event);
    void MenuMesurementSettings(wxCommandEvent& event);

    void MenuTestTerminal(wxCommandEvent& event);
    void MenuTestFunc(wxCommandEvent& event);

    void MenuHelpAbout(wxCommandEvent& event);
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
protected:
    wxTextCtrl* TerminalDisplay;

    //device Class
    //GpibDevice Adapter;
};

class PlotWindow : public wxDialog
{
public:
    PlotWindow(wxWindow *parent);
    virtual ~PlotWindow();
protected:
    wxChoice* selectMesurement;
    //Plot var
    mpWindow* plot;
    mpFXYVector* vectorLayer;
private:

    wxString filePath = filePathSytem;
    wxArrayString fileNames;

    void getFileNames(const wxString& dirPath, wxArrayString& files);
    void executeScriptEvent(wxCommandEvent& event);
    void updatePlotData();

    std::vector<double> x;
    std::vector<double> y;

    //device Class
    //GpibDevice Adapter;
};

class PlotWindowSetMarker : public wxDialog
{
	private:

        bool Marker1FreqSet = false;
        bool Marker1MaxSet = false;

        wxString Marker1Freq;
        wxString Marker1Unit;
        wxString FreqMarker1Raw;

        bool Marker2FreqSet = false;
        bool Marker2MaxSet = false;

        wxString Marker2Freq;
        wxString Marker2Unit;
        wxString FreqMarker2Raw;

	protected:
		wxCheckBox* m_checkBox1;
		wxTextCtrl* m_textCtrl1;
		wxChoice* m_choice1;
		wxCheckBox* m_checkBox2;
		wxButton* m_button1;
		wxCheckBox* m_checkBox3;
		wxTextCtrl* m_textCtrl2;
		wxChoice* m_choice2;
		wxCheckBox* m_checkBox4;
		wxButton* m_button2;

	public:
		PlotWindowSetMarker( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("Set Marker"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 417,300 ), long style = wxDEFAULT_FRAME_STYLE|wxTAB_TRAVERSAL );
		~PlotWindowSetMarker();

        void toggleSelection1(wxCommandEvent& event);
        void toggleSelection2(wxCommandEvent& event);
        void SetSelection1(wxCommandEvent& event);
        void SetSelection2(wxCommandEvent& event);
        void toggleSelection1fkt();
        void toggleSelection2fkt();
        void GetSelectedValue1();
        void GetSelectedValue2();

        void GetValues();

};

class Mesurement2D : public wxDialog
{
	private:
        int sliderY;
        int sliderX;
        int sliderScale;
        int progressbar;
        int currentMesurmentPoint;
        int totalMesurmentPoints;
	protected:
		wxStaticText* m_staticText1;
		wxStaticText* m_staticText2;
		wxSlider* m_slider1;
		wxStaticText* m_staticText3;
		wxSlider* m_slider2;
		wxStaticText* m_staticText4;
		wxSlider* m_slider3;
		wxStaticText* m_staticText5;
		wxChoice* m_choice1;
		wxButton* m_button2;
		wxButton* m_button1;
		wxStaticText* m_staticText6;
		wxStaticText* m_staticText7;
		wxGauge* m_gauge1;
		wxButton* m_button3;
		wxButton* m_button4;
		wxButton* m_button5;

	public:

		Mesurement2D( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("2D Mesurement"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 421,345 ), long style = wxDEFAULT_DIALOG_STYLE );
		~Mesurement2D();
        //Gui Event Functions
        void OnSliderUpdate(wxCommandEvent& event);
        void OnReset(wxCommandEvent& event);
        void OnSettings(wxCommandEvent& event);
        void OnStart(wxCommandEvent& event);
        void OnRestart(wxCommandEvent& event);
        void OnStop(wxCommandEvent& event);
        //Gui Helper Functions
        void GetValues();
        void SetValues();
        void GetTotalMesurements();
        void incrementCurrentMesurmentPoint();
        void updateProgressBar();
        void resetProgressBar();
        void SetSliderValues();
        void resetGuiValues();

        //Mesurement Functions
        void singleMesurement();
        
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
    void OnTestSaveFile(wxCommandEvent& event);

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
