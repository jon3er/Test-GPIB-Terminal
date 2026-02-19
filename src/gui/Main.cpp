#include "main.h"
#include "cmdGpib.h"
#include <cstdio>


wxIMPLEMENT_APP(MainWin);

//-----MainWin Methodes-----
bool MainWin::OnInit()
{
    try
    {
        // Log to file for debugging
        FILE* debugLog = fopen("C:\\temp\\debug.log", "w");
        if (debugLog)
        {
            fprintf(debugLog, "TEST-GPIB-Terminal starting...\n");
            fflush(debugLog);
        }

        //Enable Debug output window
        wxLog::SetActiveTarget(new wxLogStderr());

        if (debugLog)
        {
            fprintf(debugLog, "Creating main frame...\n");
            fflush(debugLog);
        }

        MainProgrammWin *MainProgFrame = new MainProgrammWin(nullptr, new MainDocument());

        if (debugLog)
        {
            fprintf(debugLog, "Main frame created, showing...\n");
            fflush(debugLog);
        }

        MainProgFrame->Show();

        if (debugLog)
        {
            fprintf(debugLog, "Main frame shown, returning true\n");
            fclose(debugLog);
        }

        return true;
    }
    catch (const std::exception& e)
    {
        FILE* debugLog = fopen("C:\\temp\\debug.log", "a");
        if (debugLog)
        {
            fprintf(debugLog, "Exception caught: %s\n", e.what());
            fclose(debugLog);
        }
        wxLogError(wxT("Fatal error during initialization: %s"), wxString::FromUTF8(e.what()));
        return false;
    }
    catch (...)
    {
        FILE* debugLog = fopen("C:\\temp\\debug.log", "a");
        if (debugLog)
        {
            fprintf(debugLog, "Unknown exception caught\n");
            fclose(debugLog);
        }
        wxLogError(wxT("Unknown fatal error during initialization"));
        return false;
    }
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
    Bind(wxEVT_MENU, &MainProgrammWin::MenuMesurementNew,       this, MainMenuBar::ID_Main_Mesurement_New);
    Bind(wxEVT_MENU, &MainProgrammWin::MenuMesurementLoad,      this, MainMenuBar::ID_Main_Mesurement_Open);
    Bind(wxEVT_MENU, &MainProgrammWin::MenuMesurementLoad,      this, MainMenuBar::ID_Main_Mesurement_Load);
    Bind(wxEVT_MENU, &MainProgrammWin::MenuMesurementLoad,      this, MainMenuBar::ID_Main_Mesurement_Preset_1);
    Bind(wxEVT_MENU, &MainProgrammWin::MenuMesurementLoad,      this, MainMenuBar::ID_Main_Mesurement_Preset_2);
    Bind(wxEVT_MENU, &MainProgrammWin::MenuMesurementLoad,      this, MainMenuBar::ID_Main_Mesurement_Preset_3);
    Bind(wxEVT_MENU, &MainProgrammWin::MenuMesurement2DMess,    this, MainMenuBar::ID_Main_Mesurement_2D_Mess);
    Bind(wxEVT_MENU, &MainProgrammWin::MenuMesurementSetMarker, this, MainMenuBar::ID_Main_Mesurement_SetMarker);
    Bind(wxEVT_MENU, &MainProgrammWin::MenuMesurementSettings,  this, MainMenuBar::ID_Main_Mesurement_Settings);
    // Test Menu binds
    Bind(wxEVT_MENU, &MainProgrammWin::MenuTestTerminal,    this, MainMenuBar::ID_Main_Test_Terminal);
    Bind(wxEVT_MENU, &MainProgrammWin::MenuTestFunc,        this, MainMenuBar::ID_Main_Test_Func);
    Bind(wxEVT_MENU, &MainProgrammWin::MenuTestPloter,      this, MainMenuBar::ID_Main_Test_Ploter);
    // Help Menu binds
    Bind(wxEVT_MENU, &MainProgrammWin::MenuHelpAbout,       this, MainMenuBar::ID_Main_Help_About);


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

    // create submenu elemets
	m_menuMesure_Item_New           = new wxMenuItem( m_menu_Mesurement, ID_Main_Mesurement_New,        wxString( wxT("New Mesurement") ) , wxEmptyString, wxITEM_NORMAL );
	m_menuMesure_Item_Open          = new wxMenuItem( m_menu_Mesurement, ID_Main_Mesurement_Open,       wxString( wxT("Open Saved Mesurement") ) , wxEmptyString, wxITEM_NORMAL );
	m_menuMesure_Item_Load          = new wxMenuItem( m_menu_Mesurement, ID_Main_Mesurement_Load,       wxString( wxT("Load config") ) , wxEmptyString, wxITEM_NORMAL );
	m_menuMesure_Item_Preset_1      = new wxMenuItem( m_menu_Mesurement, ID_Main_Mesurement_Preset_1,   wxString( wxT("Preset 1") ) , wxEmptyString, wxITEM_NORMAL );
	m_menuMesure_Item_Preset_2      = new wxMenuItem( m_menu_Mesurement, ID_Main_Mesurement_Preset_2,   wxString( wxT("Preset 2") ) , wxEmptyString, wxITEM_NORMAL );
	m_menuMesure_Item_Preset_3      = new wxMenuItem( m_menu_Mesurement, ID_Main_Mesurement_Preset_3,   wxString( wxT("Preset 3") ) , wxEmptyString, wxITEM_NORMAL );
	m_menuMesure_Item_2DMesurment   = new wxMenuItem( m_menu_Mesurement, ID_Main_Mesurement_2D_Mess,    wxString( wxT("2D Plot Mesurment") ) , wxEmptyString, wxITEM_NORMAL );
	m_menuMesure_Item_SetMarker     = new wxMenuItem( m_menu_Mesurement, ID_Main_Mesurement_SetMarker,  wxString( wxT("Set Marker") ) , wxEmptyString, wxITEM_NORMAL );
	m_menuMesure_Item_Settings      = new wxMenuItem( m_menu_Mesurement, ID_Main_Mesurement_Settings,   wxString( wxT("Settings") ) , wxEmptyString, wxITEM_NORMAL );

    // set submenu order
    m_menu_Mesurement->Append( m_menuMesure_Item_New );
	m_menu_Mesurement->AppendSeparator();
	m_menu_Mesurement->Append( m_menuMesure_Item_Open );
	m_menu_Mesurement->AppendSeparator();
	m_menu_Mesurement->Append( m_menuMesure_Item_Load );
	m_menu_Mesurement->Append( m_menuMesure_Item_Preset_1 );
	m_menu_Mesurement->Append( m_menuMesure_Item_Preset_2 );
	m_menu_Mesurement->Append( m_menuMesure_Item_Preset_3 );
    m_menu_Mesurement->AppendSeparator();
	m_menu_Mesurement->Append( m_menuMesure_Item_2DMesurment );
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

    // set submenu order
    m_menu_Help->Append( m_menuHelp_Item_About );

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
    if (m_doc)
        m_doc->RemoveObserver(this);
    this->Destroy();
}
// Button functions
void MainProgrammWin::ButtonRefresh(wxCommandEvent& event)
{
    wxString Text = "";
    m_textCtrlAdapterStatus->SetValue(Text);
    m_textCtrlDeviceStatus->SetValue(Text);

    if (!Global::AdapterInstance.getConnected())
    {
        Global::AdapterInstance.connect();
    }

    if (Global::AdapterInstance.getConnected())
    {

        Text = Global::AdapterInstance.send(ProLogixCmdLookup.at(ProLogixCmd::VER));
        if (Text.substr(0,6) == "Failed")
        {
            m_textCtrlAdapterStatus->SetValue("Error Check Connection");
        }
        else
        {
            m_textCtrlAdapterStatus->SetValue(Text.substr(14));
        }

        Text = Global::AdapterInstance.send(ScpiCmdLookup.at(ScpiCmd::IDN));
        if (Text == "No Message to Read\n")
        {
            Text = "No GPIB Device Found Check Connection";
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
    this->Destroy();
}

void MainProgrammWin::MenuMesurementNew(wxCommandEvent& event)
{
    // Document owns hardware state, independent of the view
    MeasurementDocument measDoc(Global::AdapterInstance, Global::Messung);

    PlotWindow* PlotWin = new PlotWindow(this, m_doc);
    PlotWin->SetDocument(&measDoc);

    PlotWin->ShowModal();

    PlotWin->SetDocument(nullptr);
    PlotWin->Destroy();
    // measDoc goes out of scope: destructor handles disconnect + thread join
}
void MainProgrammWin::MenuMesurementLoad(wxCommandEvent& event)
{
    
}
void MainProgrammWin::MenuMesurementSettings(wxCommandEvent& event)
{
    // Document owns hardware state; created on the stack to match the lifecycle of the dialog
    SettingsDocument settingsDoc(Global::AdapterInstance);

    SettingsWindow* SettingsWin = new SettingsWindow(this);
    SettingsWin->SetDocument(&settingsDoc);   // register observer + initial refresh
    SettingsWin->ShowModal();
    SettingsWin->SetDocument(nullptr);        // deregister observer before destruction
    SettingsWin->Destroy();
}
void MainProgrammWin::MenuTestTerminal(wxCommandEvent& event)
{
    // Document owns hardware state and is independent of the view
    TerminalDocument termDoc(Global::AdapterInstance);

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
    FunctionDocument funcDoc(Global::AdapterInstance);

    FunctionWindow* FuncWin = new FunctionWindow(this);
    FuncWin->SetDocument(&funcDoc);
    FuncWin->ShowModal();
    FuncWin->SetDocument(nullptr);
    FuncWin->Destroy();
    // funcDoc goes out of scope here: destructor handles disconnect if needed.
}
void MainProgrammWin::MenuTestPloter(wxCommandEvent& event)
{
        PlotterFrame* Plotframe = new PlotterFrame();
        Plotframe->ShowModal();
        Plotframe->Destroy();
}

void MainProgrammWin::MenuMesurementSetMarker(wxCommandEvent& event)
{
    // Reuse the same adapter — marker dialog only needs to write SCPI commands
    MeasurementDocument markerDoc(Global::AdapterInstance, Global::Messung);

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

}


//Helper Functions
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