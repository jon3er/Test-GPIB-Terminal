#pragma once

#include <wx/wx.h>
#include <wx/notebook.h>
#include <wx/valtext.h>
#include <wx/string.h>
#include <wx/dir.h>
#include <wx/textfile.h>
#include <wx/msgdlg.h>

#include <map>
#include <thread>
// Hardware header
#include "fkt_GPIB.h"
#include "fkt_d2xx.h"
// Gui header
#include "mathplot.h"
#include "Mesurement.h"
// Data
#include "dataManagement.h"
// info header
#include "systemInfo.h"
#include "cmdGpib.h"



//-----Global Varibles Start------

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

    // File menu elements
    wxMenuItem* m_menuFile_Item_Open;
    wxMenuItem* m_menuFile_Item_Close;
    wxMenuItem* m_menuFile_Item_Save;
    wxMenuItem* m_menuFile_Item_SaveAs;
    wxMenuItem* m_menuFile_Item_Exit;

    // Mesurement menu elements
    wxMenuItem* m_menuMesure_Item_New;
    wxMenuItem* m_menuMesure_Item_Open;
    wxMenuItem* m_menuMesure_Item_Load;
    wxMenuItem* m_menuMesure_Item_Preset_1;
    wxMenuItem* m_menuMesure_Item_Preset_2;
    wxMenuItem* m_menuMesure_Item_Preset_3;
    wxMenuItem* m_menuMesure_Item_2DMesurment;
    wxMenuItem* m_menuMesure_Item_SetMarker;
    wxMenuItem* m_menuMesure_Item_Settings;

    // Test menu elements
    wxMenuItem* m_menuTest_Item_Terminal;
    wxMenuItem* m_menuTest_Item_Func;
    wxMenuItem* m_menuHelp_Item_About;



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

    // Device managment
    void scanDevices(const std::string& args);
    void statusDevice(const std::string& args);
    void configDevice(const std::string& args);
    // connection
    void connectDevice(const std::string& args);
    void disconnectDevice(const std::string& args);
    // com
    wxString sendToDevice(const std::string& args);
    wxString readFromDevice(const std::string& args);
    void writeToDevice(const std::string& args);
    // test
    void testDevice(const std::string& args);

protected:
    wxTextCtrl* TerminalDisplay;

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
    void OnTestMultiMess(wxCommandEvent& event);
    void OnTest(wxCommandEvent& event);

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



