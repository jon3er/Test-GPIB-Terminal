#include "main.h"
#include "cmdGpib.h"
#include <cstdio>


wxIMPLEMENT_APP(MainWin);

//-----MainWin Methodes-----
bool MainWin::OnInit()
{
    //Enable Debug output window
    wxLog::SetActiveTarget(new wxLogStderr());

    // Create main frame
    MainProgrammWin *MainProgFrame = new MainProgrammWin(nullptr, new MainDocument());

    SetTopWindow(MainProgFrame);

    MainProgFrame->Show();

    return true;
}
//-----MainWin Methodes ende-----

//-----Main Programm Window-----
MainProgrammWin::MainProgrammWin( wxWindow* parent, MainDocument* doc, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style )
    : wxFrame( parent, id, title, pos, size, style )
{
    // Store and observe document
    m_doc = doc;
    if (m_doc)
        m_doc->AddObserver(this);

    //TODO set new Functions

    // File Main binds
    Bind(wxEVT_MENU, &MainProgrammWin::MenuFileOpen,    this, MainMenuBar::ID_Main_File_Open);
    Bind(wxEVT_MENU, &MainProgrammWin::MenuFileClose,   this, MainMenuBar::ID_Main_File_Close);
    Bind(wxEVT_MENU, &MainProgrammWin::MenuFileSave,    this, MainMenuBar::ID_Main_File_Save);
    Bind(wxEVT_MENU, &MainProgrammWin::MenuFileSaveAs,  this, MainMenuBar::ID_Main_File_SaveAs);
    Bind(wxEVT_MENU, &MainProgrammWin::MenuFileExit,    this, MainMenuBar::ID_Main_File_Exit);
    // Mesurement Menu binds
    Bind(wxEVT_MENU, &MainProgrammWin::MenuMesurementLoad,      this, MainMenuBar::ID_Main_Mesurement_Open);
    Bind(wxEVT_MENU, &MainProgrammWin::MenuMesurementLoad,      this, MainMenuBar::ID_Main_Mesurement_Load);
    Bind(wxEVT_MENU, &MainProgrammWin::MenuMesurementSweep,     this, MainMenuBar::ID_Main_Mesurement_Sweep);
    Bind(wxEVT_MENU, &MainProgrammWin::MenuMesurementIQ,        this, MainMenuBar::ID_Main_Mesurement_IQ);
    Bind(wxEVT_MENU, &MainProgrammWin::MenuMesurementMarkerPeak,this, MainMenuBar::ID_Main_Mesurement_MarkerPeak);
    Bind(wxEVT_MENU, &MainProgrammWin::MenuMesurementSetMarker, this, MainMenuBar::ID_Main_Mesurement_SetMarker);
    Bind(wxEVT_MENU, &MainProgrammWin::MenuMesurementSettings,  this, MainMenuBar::ID_Main_Mesurement_Settings);
    Bind(wxEVT_MENU, &MainProgrammWin::MenuMesurementCustom,    this, MainMenuBar::ID_Main_Mesurement_Custom);
    // Test Menu binds
    Bind(wxEVT_MENU, &MainProgrammWin::MenuTestTerminal,    this, MainMenuBar::ID_Main_Test_Terminal);
    Bind(wxEVT_MENU, &MainProgrammWin::MenuTestFunc,        this, MainMenuBar::ID_Main_Test_Func);
    Bind(wxEVT_MENU, &MainProgrammWin::MenuTestPloter,      this, MainMenuBar::ID_Main_Test_Ploter);
    // Help Menu binds
    Bind(wxEVT_MENU, &MainProgrammWin::MenuHelpAbout,       this, MainMenuBar::ID_Main_Help_About);
    Bind(wxEVT_MENU, &MainProgrammWin::MenuHelpResetDevices, this, MainMenuBar::ID_Main_Help_ResetDevices);


	this->SetSizeHints( wxDefaultSize, wxDefaultSize );

    //this->SetBackgroundColour(*wxLIGHT_GREY);

    //------------------ Menubar --------------------
	m_menubarMainProg = new wxMenuBar( 0 );

    //------------------ file menu --------------------
	m_menu_File = new wxMenu();

    //file submenu
	m_menuFile_Item_Open    = new wxMenuItem( m_menu_File, MainMenuBar::ID_Main_File_Open,      wxString( wxT("Open") ) + wxT('\t') + wxT("Ctrl+O"), wxEmptyString, wxITEM_NORMAL );
	m_menuFile_Item_Close   = new wxMenuItem( m_menu_File, MainMenuBar::ID_Main_File_Close,     wxString( wxT("Close") ) , wxEmptyString, wxITEM_NORMAL );
    m_menuFile_Item_Save    = new wxMenuItem( m_menu_File, MainMenuBar::ID_Main_File_Save,      wxString( wxT("Save") ) + wxT('\t') + wxT("Ctrl+S"), wxEmptyString, wxITEM_NORMAL );
    m_menuFile_Item_SaveAs  = new wxMenuItem( m_menu_File, MainMenuBar::ID_Main_File_SaveAs,    wxString( wxT("Save as ...") ) , wxEmptyString, wxITEM_NORMAL );
	m_menuFile_Item_Exit    = new wxMenuItem( m_menu_File, MainMenuBar::ID_Main_File_Exit,      wxString( wxT("Exit") ) , wxEmptyString, wxITEM_NORMAL );

    // set file submenu order
    m_menu_File->Append( m_menuFile_Item_Open );
    m_menu_File->AppendSeparator();
    m_menu_File->Append( m_menuFile_Item_Close);
    m_menu_File->AppendSeparator();
	m_menu_File->Append( m_menuFile_Item_Save );
	m_menu_File->Append( m_menuFile_Item_SaveAs );
	m_menu_File->AppendSeparator();
	m_menu_File->Append( m_menuFile_Item_Exit );

    //------------------ Simulation menu --------------------
	m_menu_Sim = new wxMenu();

    // create submenu elemets


    // set submenu order


    //------------------ Mesurement menu --------------------
	m_menu_Mesurement = new wxMenu();

    // create submenu elements
	m_menuMesure_Item_New           = new wxMenuItem( m_menu_Mesurement, ID_Main_Mesurement_New,        wxString( wxT("New Mesurement") ) , wxEmptyString, wxITEM_NORMAL );
	m_menuMesure_Item_Open          = new wxMenuItem( m_menu_Mesurement, ID_Main_Mesurement_Open,       wxString( wxT("Open Saved Mesurement") ) , wxEmptyString, wxITEM_NORMAL );
	m_menuMesure_Item_Load          = new wxMenuItem( m_menu_Mesurement, ID_Main_Mesurement_Load,       wxString( wxT("Load config") ) , wxEmptyString, wxITEM_NORMAL );
	m_menuMesure_Item_Preset_1      = new wxMenuItem( m_menu_Mesurement, ID_Main_Mesurement_Sweep,      wxString( wxT("Sweep Measurement") ) , wxEmptyString, wxITEM_NORMAL );
	m_menuMesure_Item_Preset_2      = new wxMenuItem( m_menu_Mesurement, ID_Main_Mesurement_IQ,         wxString( wxT("IQ Measurement") ) , wxEmptyString, wxITEM_NORMAL );
	m_menuMesure_Item_Preset_3      = new wxMenuItem( m_menu_Mesurement, ID_Main_Mesurement_MarkerPeak, wxString( wxT("Marker Measurement") ) , wxEmptyString, wxITEM_NORMAL );
	m_menuMesure_Item_SetMarker     = new wxMenuItem( m_menu_Mesurement, ID_Main_Mesurement_SetMarker,  wxString( wxT("Set Marker") ) , wxEmptyString, wxITEM_NORMAL );
	m_menuMesure_Item_Settings      = new wxMenuItem( m_menu_Mesurement, ID_Main_Mesurement_Settings,   wxString( wxT("Settings") ) , wxEmptyString, wxITEM_NORMAL );
    m_menuMesure_Item_Custom        = new wxMenuItem( m_menu_Mesurement, ID_Main_Mesurement_Custom,     wxString( wxT("Load Custom Script") ) , wxEmptyString, wxITEM_NORMAL );


    // "New Mesurement" submenu with Load config and Presets
    wxMenu* m_submenu_NewMesurement = new wxMenu();


    m_submenu_NewMesurement->Append( m_menuMesure_Item_Load );
    m_submenu_NewMesurement->AppendSeparator();
    m_submenu_NewMesurement->Append( m_menuMesure_Item_Preset_1 );
    m_submenu_NewMesurement->Append( m_menuMesure_Item_Preset_2 );
    m_submenu_NewMesurement->Append( m_menuMesure_Item_Preset_3 );
    // m_submenu_NewMesurement->AppendSeparator();
    // m_submenu_NewMesurement->Append( m_menuMesure_Item_Custom );


    // set submenu order
    m_menu_Mesurement->AppendSubMenu( m_submenu_NewMesurement, wxT("New Mesurement") );
	m_menu_Mesurement->AppendSeparator();
	m_menu_Mesurement->Append( m_menuMesure_Item_Open );
    m_menu_Mesurement->AppendSeparator();
	m_menu_Mesurement->Append( m_menuMesure_Item_SetMarker );
	m_menu_Mesurement->AppendSeparator();
	m_menu_Mesurement->Append( m_menuMesure_Item_Settings );

    //------------------ Processing menu --------------------
	m_menu_Processing = new wxMenu();

    // create submenu elemets

    // set submenu order


    //------------------ Test menu --------------------
	m_menu_Test = new wxMenu();

    // create submenu elemets
	m_menuTest_Item_Terminal    = new wxMenuItem( m_menu_Test, ID_Main_Test_Terminal, wxString( wxT("Terminal") ) + wxT('\t') + wxT("Ctrl+Shift+T"), wxEmptyString, wxITEM_NORMAL );
	m_menuTest_Item_Func        = new wxMenuItem( m_menu_Test, ID_Main_Test_Func, wxString( wxT("Function Test") ) + wxT('\t') + wxT("F1"), wxEmptyString, wxITEM_NORMAL );
    m_menuTest_Item_Ploter      = new wxMenuItem( m_menu_Test, ID_Main_Test_Ploter, wxString( wxT("Ploter") ) + wxT('\t') + wxT("F2"), wxEmptyString, wxITEM_NORMAL );

    // set submenu order
    m_menu_Test->Append( m_menuTest_Item_Terminal );
	m_menu_Test->Append( m_menuTest_Item_Func );
    m_menu_Test->Append( m_menuTest_Item_Ploter );

    //------------------ Help menu --------------------
	m_menu_Help = new wxMenu();

    // create submenu elemets
	m_menuHelp_Item_About = new wxMenuItem( m_menu_Help, ID_Main_Help_About, wxString( wxT("About") ) , wxEmptyString, wxITEM_NORMAL );
    m_menuHelp_Item_ResetDevices = new wxMenuItem( m_menu_Help, ID_Main_Help_ResetDevices, wxString( wxT("Devices") ) , wxEmptyString, wxITEM_NORMAL );

    // set submenu order
    m_menu_Help->Append( m_menuHelp_Item_About );
    m_menu_Help->AppendSeparator();
    m_menu_Help->Append( m_menuHelp_Item_ResetDevices );

	//------------------  end Submenu --------------------


    //------------------ set Menubar order --------------------
    m_menubarMainProg->Append( m_menu_File,         wxT("File") );
    m_menubarMainProg->Append( m_menu_Sim,          wxT("Simulation") );
    m_menubarMainProg->Append( m_menu_Mesurement,   wxT("Mesurement") );
    m_menubarMainProg->Append( m_menu_Processing,   wxT("Processing") );
    m_menubarMainProg->Append( m_menu_Test,         wxT("Test") );
	m_menubarMainProg->Append( m_menu_Help,         wxT("Help") );

	this->SetMenuBar( m_menubarMainProg );

    //------------------  End of Menubar settings --------------------

    //------------------  Sizers --------------------
    wxBoxSizer* bSizerMainProgV1 = new wxBoxSizer( wxVERTICAL );;

    //----- 1. Horizontal Sizer -----
    wxBoxSizer* bSizerMainProgH1 = new wxBoxSizer( wxHORIZONTAL );;

    // Create Elements
    m_panel21 = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );

    m_staticText1 = new wxStaticText( this, wxID_ANY, wxT("GPIB Adapter: "), wxDefaultPosition, wxDefaultSize, 0 );
    m_staticText1->Wrap( -1 );

    m_textCtrlAdapterStatus = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );

    // Set elements to sizers
	bSizerMainProgV1->Add( m_panel21, 1, wxEXPAND | wxALL, 5 );

	bSizerMainProgH1->Add( m_staticText1, 1, wxALL, 5 );
	bSizerMainProgH1->Add( m_textCtrlAdapterStatus, 2, wxALL, 5 );


    //----- 2. Horizontal Sizer -----
	wxBoxSizer* bSizerMainProgH2 = new wxBoxSizer( wxHORIZONTAL );

	bSizerMainProgH2->SetMinSize( wxSize( 0,0 ) );

    // Create Elements
	m_staticText11 = new wxStaticText( this, wxID_ANY, wxT("GPIB Device: "), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText11->Wrap( -1 );

    m_textCtrlDeviceStatus = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );

    // Set elements to sizers
	bSizerMainProgH2->Add( m_staticText11, 1, wxALL, 5 );
	bSizerMainProgH2->Add( m_textCtrlDeviceStatus, 2, wxALL, 5 );

    // ----- Other elements -----
    m_panel2    = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
    m_button1   = new wxButton( this, wxID_ANY, wxT("Refresh"), wxDefaultPosition, wxDefaultSize, 0 );
    m_button1->Bind(wxEVT_BUTTON, &MainProgrammWin::ButtonRefresh, this);

    //----- Set to Vertical Sizer -----

    bSizerMainProgV1->Add( bSizerMainProgH1,    2, wxEXPAND, 5 );
	bSizerMainProgV1->Add( bSizerMainProgH2,    2, wxEXPAND, 5 );

	bSizerMainProgV1->Add( m_panel2,            2, wxEXPAND | wxALL, 5 );
	bSizerMainProgV1->Add( m_button1,           0, wxALIGN_RIGHT|wxALL, 10 );

	this->SetSizer( bSizerMainProgV1 );
	this->Layout();

	this->Centre( wxBOTH );
}
MainProgrammWin::~MainProgrammWin()
{
    // Close all open measurement windows
    for (PlotWindow* win : m_openMeasurementWindows)
    {
        if (win)
            win->Destroy();
    }
    m_openMeasurementWindows.clear();

    if (m_doc)
        m_doc->RemoveObserver(this);
}
// Button functions
void MainProgrammWin::ButtonRefresh(wxCommandEvent& event)
{
    wxString Text = "";
    m_textCtrlAdapterStatus->SetValue(Text);
    m_textCtrlDeviceStatus->SetValue(Text);
    auto& adapter = PrologixUsbGpibAdapter::get_instance();


    if (adapter.checkIfAdapterAvailable())
    {

        if (!adapter.connect())
        {
            m_textCtrlAdapterStatus->SetValue("Error! Check Connection");
        }
        else
        {
            m_textCtrlAdapterStatus->SetValue("Adapter Found");
        }

        if (!adapter.checkIfGpibDeviceAvailable())
        {
            Text = "No GPIB Device Found Check Connection";
        }
        else
        {
            Text = "Device Found";

            adapter.config();

        }

        m_textCtrlDeviceStatus->SetValue(Text);
    }
    else
    {
        m_textCtrlAdapterStatus->SetValue("No GPIB Adapter Found!");
        m_textCtrlDeviceStatus->SetValue("");
    }
}
// MenuBar functions

void MainProgrammWin::MenuFileOpen(wxCommandEvent& event)
{
    wxLogMessage("Open File");

    wxFileDialog openFileDialog(this, _("Open File"),
        System::filePathRoot,
        "",
        "CSV Files (*.csv)|*.csv|All Files (*.*)|*.*",
        wxFD_OPEN);

    if (openFileDialog.ShowModal() == wxID_CANCEL)
        return;

    m_doc->OpenFile(openFileDialog.GetPath());
}

void MainProgrammWin::MenuFileSave(wxCommandEvent& event)
{
    if (m_doc->IsFileOpen())
    {
        m_doc->SaveFile();
    }
    else
    {
        MenuFileSaveAs(event);
    }
}
void MainProgrammWin::MenuFileSaveAs(wxCommandEvent& event)
{
    std::cerr << "Try to Open save as dialog..." << std::endl;

    wxFileDialog saveAsFileDialog(nullptr, _("File Save As..."),
        "",
        "",
        "CSV Files (*.csv)|*.csv|All Files (*.*)|*.*",
        wxFD_SAVE);

    if (saveAsFileDialog.ShowModal() == wxID_CANCEL)
        return;

    m_doc->SaveFileAs(saveAsFileDialog.GetPath());
}

void MainProgrammWin::MenuFileClose(wxCommandEvent& event)
{
    m_doc->CloseFile();
}
void MainProgrammWin::MenuFileExit(wxCommandEvent& event)
{
    // Close all open measurement windows first
    for (PlotWindow* win : m_openMeasurementWindows)
    {
        if (win)
            win->Destroy();
    }
    m_openMeasurementWindows.clear();

    this->Destroy();
}

void MainProgrammWin::MenuMesurementLoad(wxCommandEvent& event)
{
    wxString filePath;
    sData importedData;

    // Reuse the already opened file from MainDocument if available.
    if (m_doc && m_doc->IsFileOpen())
    {
        importedData = m_doc->GetData();
        filePath = m_doc->GetFilePath();
    }
    else
    {
        wxFileDialog openFileDialog(this, _("Open CSV File"),
            System::filePathRoot,
            "",
            "CSV Files (*.csv)|*.csv|All Files (*.*)|*.*",
            wxFD_OPEN | wxFD_FILE_MUST_EXIST);

        if (openFileDialog.ShowModal() == wxID_CANCEL)
            return;

        filePath = openFileDialog.GetPath();

        // Read CSV into a temporary sData - does NOT affect the main document
        CsvFile csvFile;
        if (!csvFile.readCsvFile(filePath, importedData))
        {
            wxMessageBox("Failed to read CSV file:\n" + filePath,
                        "Import Error", wxOK | wxICON_ERROR, this);
            return;
        }
    }

    // Set File setting to Current settings
    importedData.exportFsuSettings();
    // send new Settings to Gpib device
    fsuMeasurement::get_instance().writeSettingsToGpib();


    // Create a new PlotWindow with its own MeasurementDocument
    MeasurementDocument* measDoc = new MeasurementDocument(
        PrologixUsbGpibAdapter::get_instance(), fsuMeasurement::get_instance());

    PlotWindow* PlotWin = new PlotWindow(this, nullptr);  // no mainDoc — data comes from import
    PlotWin->SetDocument(measDoc);
    PlotWin->SetOwnsDocument(true);

    if (!PlotWin->LoadImportedData(importedData, filePath))
    {
        wxMessageBox("Imported file does not contain plottable measurement data.",
                     "Import Error", wxOK | wxICON_ERROR, this);
        PlotWin->Destroy();
        return;
    }

    // Track and show
    m_openMeasurementWindows.insert(PlotWin);
    PlotWin->Show();
}
void MainProgrammWin::MenuMesurementSweep(wxCommandEvent& event)
{
    MSetDocument msetDoc(PrologixUsbGpibAdapter::get_instance(),
                         fsuMeasurement::get_instance(),
                         MeasurementMode::SWEEP);

    SettingsDialog dlg(this, MeasurementMode::SWEEP);
    dlg.SetDocument(&msetDoc);
    dlg.ShowModal();
    dlg.SetDocument(nullptr);
};
void MainProgrammWin::MenuMesurementIQ(wxCommandEvent& event)
{
    MSetDocument msetDoc(PrologixUsbGpibAdapter::get_instance(),
                         fsuMeasurement::get_instance(),
                         MeasurementMode::IQ);

    SettingsDialog dlg(this, MeasurementMode::IQ);
    dlg.SetDocument(&msetDoc);
    dlg.ShowModal();
    dlg.SetDocument(nullptr);
}
void MainProgrammWin::MenuMesurementMarkerPeak(wxCommandEvent& event)
{
    MSetDocument msetDoc(PrologixUsbGpibAdapter::get_instance(),
                         fsuMeasurement::get_instance(),
                         MeasurementMode::MARKER_PEAK);

    SettingsDialog dlg(this, MeasurementMode::MARKER_PEAK);
    dlg.SetDocument(&msetDoc);
    dlg.ShowModal();
    dlg.SetDocument(nullptr);
}

void MainProgrammWin::MenuMesurementCustom(wxCommandEvent& event)
{
    wxFileDialog openScriptDialog(
    this,
    _("GPIB Script öffnen"),
    System::filePathSystem,          // öffnet direkt im GpibScripts-Ordner
    wxEmptyString,
    "Text Files (*.txt)|*.txt|All Files (*.*)|*.*",
    wxFD_OPEN | wxFD_FILE_MUST_EXIST
    );

    if (openScriptDialog.ShowModal() == wxID_CANCEL)
    return;

    wxString scriptFileName = openScriptDialog.GetFilename();   // z.B. "TestScript.txt"
    wxString scriptFolderPath = openScriptDialog.GetDirectory(); // z.B. "D:\...\GpibScripts"

    // set Measurement Settings
    fsuMeasurement* fsu = &fsuMeasurement::get_instance();
    fsu->setMeasurementMode(MeasurementMode::COSTUM);
    fsu->setFilePath(scriptFolderPath);
    fsu->setFileName(scriptFileName);

    PlotterFrame::ShowOrRaise();

}

void MainProgrammWin::MenuMesurementSettings(wxCommandEvent& event)
{
    // Document owns hardware state; created on the stack to match the lifecycle of the dialog
    SettingsDocument settingsDoc(PrologixUsbGpibAdapter::get_instance());

    SettingsWindow* SettingsWin = new SettingsWindow(this);
    SettingsWin->SetDocument(&settingsDoc);   // register observer + initial refresh
    SettingsWin->ShowModal();
    SettingsWin->SetDocument(nullptr);        // deregister observer before destruction
    SettingsWin->Destroy();
}
void MainProgrammWin::MenuTestTerminal(wxCommandEvent& event)
{
    // Document owns hardware state and is independent of the view
    TerminalDocument termDoc(PrologixUsbGpibAdapter::get_instance());

    // Create view and attach document
    TerminalWindow* TWin = new TerminalWindow(this);
    TWin->SetDocument(&termDoc);

    // Open window — blocks until the user closes it
    TWin->ShowModal();

    // Detach document before destroying the view
    TWin->SetDocument(nullptr);
    TWin->Destroy();
    // termDoc goes out of scope here: destructor handles disconnect
}
void MainProgrammWin::MenuTestFunc(wxCommandEvent& event)
{
    // Document owns all function-test state and the adapter reference.
    FunctionDocument funcDoc(PrologixUsbGpibAdapter::get_instance());

    FunctionWindow* FuncWin = new FunctionWindow(this);
    FuncWin->SetDocument(&funcDoc);
    FuncWin->ShowModal();
    FuncWin->SetDocument(nullptr);
    FuncWin->Destroy();
    // funcDoc goes out of scope here: destructor handles disconnect if needed.
}
void MainProgrammWin::MenuTestPloter(wxCommandEvent& event)
{
    PlotterFrame::ShowOrRaise();
}

void MainProgrammWin::MenuMesurementSetMarker(wxCommandEvent& event)
{
    // Reuse the same adapter — marker dialog only needs to write SCPI commands
    MeasurementDocument markerDoc(PrologixUsbGpibAdapter::get_instance(), fsuMeasurement::get_instance());

    PlotWindowSetMarker* PlotWinMarker = new PlotWindowSetMarker(this);
    PlotWinMarker->SetDocument(&markerDoc);

    PlotWinMarker->ShowModal();

    PlotWinMarker->SetDocument(nullptr);
    PlotWinMarker->Destroy();
}
void MainProgrammWin::MenuMesurement2DMess(wxCommandEvent& event)
{

}

void MainProgrammWin::MenuHelpAbout(wxCommandEvent& event)
{
    wxString aboutText;
    aboutText << "GPIB-FSU-Measurement Tool\n"
              << "Version: 1.0\n"
              << "\n"
              << "GPIB measurement and control tool\n"
              << "based on wxWidgets and Prologix USB-GPIB adapter.\n"
              << "\n"
              << "GitHub: \n"
              << "https://github.com/jon3er/Test-GPIB-Terminal";

    wxMessageBox(
        aboutText,
        "About",
        wxOK | wxICON_INFORMATION,
        this
    );
}

void MainProgrammWin::MenuHelpResetDevices(wxCommandEvent& event)
{
    HelpDocument helpDoc(PrologixUsbGpibAdapter::get_instance());

    HelpWin* helpWin = new HelpWin(this);
    helpWin->SetDocument(&helpDoc);
    helpWin->ShowModal();
    helpWin->SetDocument(nullptr);
    helpWin->Destroy();
}


// -----------------------------------------------------------------------
// IMainObserver implementation
// -----------------------------------------------------------------------
void MainProgrammWin::OnFileChanged(const sData& data, bool isOpen)
{
    // Enable/disable file-menu items that require an open file
    if (m_menuFile_Item_Close)
        m_menuFile_Item_Close->Enable(isOpen);
    if (m_menuFile_Item_Save)
        m_menuFile_Item_Save->Enable(isOpen);
    if (m_menuFile_Item_SaveAs)
        m_menuFile_Item_SaveAs->Enable(isOpen);
}

void MainProgrammWin::OnFilePathChanged(const wxString& path)
{
    if (path.IsEmpty())
        SetTitle(wxT("GPIB Messurement"));
    else
        SetTitle(wxT("GPIB Messurement — ") + path.AfterLast('\\').AfterLast('/'));
}
