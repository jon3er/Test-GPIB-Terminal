#pragma once

#include <wx/wx.h>
#include <wx/notebook.h>
#include <wx/valtext.h>
#include <wx/string.h>
#include <wx/dir.h>
#include <wx/textfile.h>
#include <wx/msgdlg.h>

#include <map>
#include <set>
#include <thread>
// Hardware header
#include "FsuMeasurement.h"
#include "fkt_d2xx.h"
// Gui header
#include "mathplot.h"
#include "Mesurement.h"
#include "TerminalWindow.h"
#include "FunctionWindow.h"
#include "ValidationWindow/ValidationWindow.h"
#include "SettingsWindow.h"
#include "MSetDialog.h"
#include "helpTab/helpWin.h"
// document header
#include "MeasurementDocument.h"
#include "TerminalDocument.h"
#include "ValidationWindow/ValidationDocument.h"
#include "SettingsDocument.h"
#include "helpTab/helpDocument.h"
// Data
#include "dataManagement.h"
#include "CsvManagement.h"
// Doc/View
#include "MainDocument.h"
// info header
#include "systemInfo.h"
#include "cmdGpib.h"
// Plotter
#include "PlotterFrame.h"
//#include "PlotView.h"


//-----MainWin-----
class MainWin : public wxApp
{
public:
    bool OnInit() override; //overrides function of Base Classe wxApp Function OnInit()
};


//-----Main Programm Window
class MainProgrammWin : public wxFrame, public IMainObserver
{
private:
    // Non-owning pointer to application-level document (owned by MainWin::OnInit)
    MainDocument* m_doc = nullptr;

    // Track all open measurement windows for proper cleanup
    std::set<PlotWindow*> m_openMeasurementWindows;
public:
    /** Called by PlotWindow to unregister itself when closing */
    void UnregisterMeasurementWindow(PlotWindow* win) { m_openMeasurementWindows.erase(win); }

private:

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
    wxMenuItem* m_menuMesure_Item_Custom;


    // Test menu elements
    wxMenuItem* m_menuTest_Item_Terminal;
    wxMenuItem* m_menuTest_Item_Func;
    wxMenuItem* m_menuTest_Item_Ploter;
    wxMenuItem* m_menuTest_Item_Validation;

    wxMenuItem* m_menuHelp_Item_About;
    wxMenuItem* m_menuHelp_Item_ResetDevices;



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

	MainProgrammWin( wxWindow* parent, MainDocument* doc, wxWindowID id = wxID_ANY, const wxString& title = wxT("GPIB Messurement Tool"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 500,400 ), long style = wxDEFAULT_FRAME_STYLE|wxTAB_TRAVERSAL );
	~MainProgrammWin();

    // Document access
    MainDocument* GetDocument() const { return m_doc; }

    // Legacy convenience delegates (keep callers compiling)
    sData& returnOpendData() { return m_doc->GetData(); }
    bool   isFileOpen()      { return m_doc->IsFileOpen(); }

    // IMainObserver — updates window title and menu-item enabled states
    void OnFileChanged(const sData& data, bool isOpen) override;
    void OnFilePathChanged(const wxString& path) override;
    // Button func
    void ButtonRefresh(wxCommandEvent& event);
    // Menubar items
    void MenuFileOpen(wxCommandEvent& event);
    void MenuFileClose(wxCommandEvent& event);
    void MenuFileSave(wxCommandEvent& event);
    void MenuFileSaveAs(wxCommandEvent& event);
    void MenuFileExit(wxCommandEvent& event);


	void MenuMesurementLoad(wxCommandEvent& event);
    void MenuMesurementSweep(wxCommandEvent& event);
    void MenuMesurementIQ(wxCommandEvent& event);
    void MenuMesurementMarkerPeak(wxCommandEvent& event);
    void MenuMesurementCustom(wxCommandEvent& event);
    void MenuMesurementSetMarker(wxCommandEvent& event);
    void MenuMesurement2DMess(wxCommandEvent& event);
    void MenuMesurementSettings(wxCommandEvent& event);

    void MenuTestTerminal(wxCommandEvent& event);
    void MenuTestFunc(wxCommandEvent& event);
    void MenuTestPloter(wxCommandEvent& event);
    void MenuTestValidation(wxCommandEvent& event);


    void MenuHelpAbout(wxCommandEvent& event);
    void MenuHelpResetDevices(wxCommandEvent& event);
};







