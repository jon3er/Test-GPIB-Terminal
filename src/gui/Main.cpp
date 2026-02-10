#include "main.h"

// Verion Wo Gui noch komplett in einer datei ist.

wxIMPLEMENT_APP(MainWin);

//-----MainWin Methodes-----
bool MainWin::OnInit()
{
    //Enable Debug output window
    wxLog::SetActiveTarget(new wxLogStderr());

    MainProgrammWin *MainProgFrame = new MainProgrammWin(nullptr);
    MainProgFrame->Show();

    return true;
}
//-----MainWin Methodes ende-----

//-----Main Programm Window-----
MainProgrammWin::MainProgrammWin( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style )
    : wxFrame( parent, id, title, pos, size, style )
{
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

        Text = Global::AdapterInstance.send("++ver");
        if (Text.substr(0,6) == "Failed")
        {
            m_textCtrlAdapterStatus->SetValue("Error Check Connection");
        }
        else
        {
            m_textCtrlAdapterStatus->SetValue(Text.substr(14));
        }

        Text = Global::AdapterInstance.send("*IDN?");
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

    if (openFileDialog.ShowModal()== wxID_CANCEL)
    {
        return;
    }

    filePathCurrentFile = openFileDialog.GetPath();

    if (readCsvFile(filePathCurrentFile, OpendData))
    {
        fileOpen = true;
    }
    else
    {
        fileOpen = false;
    }

    std::cerr << filePathCurrentFile << std::endl;
}

void MainProgrammWin::MenuFileSave(wxCommandEvent& event)
{
    if (fileOpen)
    {
        saveToCsvFile(filePathCurrentFile, OpendData, 0);
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
        "",//filePathRoot,
        "",
        "CSV Files (*.csv)|*.csv|All Files (*.*)|*.*",
        wxFD_SAVE);

    if (saveAsFileDialog.ShowModal()== wxID_CANCEL)
    {
        return;
    }

    std::cerr << "Opend save as window" << std::endl;

    filePathCurrentFile = saveAsFileDialog.GetPath();
    if (!saveToCsvFile(filePathCurrentFile, OpendData, 0))
    {
        std::cerr << "failed to save" << std::endl;
        fileOpen = false;
        return;
    }
    else
    {
        fileOpen = true;
    }

    std::cerr << filePathCurrentFile << std::endl;
    std::cerr << "saved data" << std::endl;
}

void MainProgrammWin::MenuFileClose(wxCommandEvent& event)
{
    fileOpen = false;
    sData OpendDataTemp;
    OpendData = OpendDataTemp;
}
void MainProgrammWin::MenuFileExit(wxCommandEvent& event)
{
    this->Destroy();
}

void MainProgrammWin::MenuMesurementNew(wxCommandEvent& event)
{
    //Create new sub window
    PlotWindow *PlotWin = new PlotWindow(this);
    //open Window Pauses Main Window
    PlotWin->ShowModal();
    //Close Window
    PlotWin->Destroy();
}
void MainProgrammWin::MenuMesurementLoad(wxCommandEvent& event)
{

}
void MainProgrammWin::MenuMesurementSettings(wxCommandEvent& event)
{
    //Create new sub window
    SettingsWindow *SettingsWin = new SettingsWindow(this);
    //open Window Pauses Main Window
    SettingsWin->ShowModal();
    //Close Window
    SettingsWin->Destroy();
}
void MainProgrammWin::MenuTestTerminal(wxCommandEvent& event)
{
    //Create new sub window
    TerminalWindow *TWin = new TerminalWindow(this);
    //open Window Pauses Main Window
    TWin->ShowModal();
    //Close Window
    TWin->Destroy();
}
void MainProgrammWin::MenuTestFunc(wxCommandEvent& event)
{
    //Create new sub window
    FunctionWindow *FuncWin = new FunctionWindow(this);
    //open Window Pauses Main Window
    FuncWin->ShowModal();
    //Close Window
    FuncWin->Destroy();
}
void MainProgrammWin::MenuTestPloter(wxCommandEvent& event)
{
        PlotterFrame* Plotframe = new PlotterFrame();
        Plotframe->ShowModal();
        Plotframe->Destroy();
}

void MainProgrammWin::MenuMesurementSetMarker(wxCommandEvent& event)
{
    //Create new sub window
    PlotWindowSetMarker *PlotWinMarker = new PlotWindowSetMarker(this);
    //open Window Pauses Main Window
    PlotWinMarker->ShowModal();
    //Close Window
    PlotWinMarker->Destroy();
}
void MainProgrammWin::MenuMesurement2DMess(wxCommandEvent& event)
{
    Mesurement2D *Mesurement2DWindow = new Mesurement2D(this);

    Mesurement2DWindow->ShowModal();

    Mesurement2DWindow->Destroy();
}

void MainProgrammWin::MenuHelpAbout(wxCommandEvent& event)
{

}

//----- Terminal Window Constructor -----
TerminalWindow::TerminalWindow(wxWindow *parent)
    : wxDialog(parent, wxID_ANY, "GPIB Terminal Window", wxDefaultPosition, wxSize(1000,600))
{
    wxPanel* panelTerm = new wxPanel(this);

    //text Output
    TerminalDisplay = new wxTextCtrl(panelTerm,wxID_ANY,"",wxDefaultPosition,wxSize(1000, 200), wxTE_MULTILINE);
    //disable user input
    TerminalDisplay->SetEditable(false);

    //text input
    wxTextCtrl* TerminalInput = new wxTextCtrl(panelTerm, wxID_ANY,"",wxDefaultPosition,wxSize(1000, 50), wxTE_MULTILINE | wxTE_PROCESS_ENTER);
    //set Cursor in input window
    TerminalInput->SetFocus();

    wxStaticText* StaticTE = new wxStaticText(panelTerm, wxID_ANY,"GPIB Terminal log");
    wxStaticText* StaticTEInput = new wxStaticText(panelTerm, wxID_ANY,"Input GPIB Commands:");

    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    sizer->Add(StaticTE,        0, wxALL, 20);
    sizer->Add(TerminalDisplay, 0, wxEXPAND | wxALL, 20);
    sizer->Add(StaticTEInput,   0 , wxALL, 20);
    sizer->Add(TerminalInput,   0, wxALL, 20);
    panelTerm->SetSizerAndFit(sizer);

    //debug msg
    std::cerr << "Terminal Window Opened" << std::endl;

    TerminalInput->Bind(wxEVT_TEXT_ENTER, &TerminalWindow::OnEnterTerminal,this);
    //define Termianl Commands
    TerminalWindow::setupCmds();
}
//-----Terminal window Destructor -----
TerminalWindow::~TerminalWindow()
{
    if (Global::AdapterInstance.getConnected() == true)
    {
        Global::AdapterInstance.disconnect();
    }
    std::cerr << "Terminal Window Closed" << std::endl;
}
//-----Terminal Window Methodes -----
void TerminalWindow::setupCmds()
{
    cmds["scan"]        = [this](const std::string& args) { this->scanDevices(args); };
    cmds["status"]      = [this](const std::string& args) { this->statusDevice(args); };
    cmds["config"]      = [this](const std::string& args) { this->configDevice(args); };
    cmds["connect"]     = [this](const std::string& args) { this->connectDevice(args); };
    cmds["disconnect"]  = [this](const std::string& args) { this->disconnectDevice(args); };
    cmds["send"]        = [this](const std::string& args) { this->sendToDevice(args); };
    cmds["read"]        = [this](const std::string& args) { this->readFromDevice(args); };
    cmds["write"]       = [this](const std::string& args) { this->writeToDevice(args); };
    cmds["test"]        = [this](const std::string& args) { this->testDevice(args); };
}

void TerminalWindow::scanDevices(const std::string& args = "")
{
    DWORD devNum = scanUsbDev();
    wxString Text = "Number of devices: " + std::to_string(devNum) + "\n";

    TerminalDisplay->AppendText(terminalTimestampOutput(Text));
}

void TerminalWindow::statusDevice(const std::string& args = "")
{
    TerminalDisplay->AppendText(terminalTimestampOutput(Global::AdapterInstance.statusText()));
}

void TerminalWindow::connectDevice(const std::string& args = "")
{
    int dev = 0;

    std::cerr << "Command entered: connected with args: " << args << std::endl;

    if (args != "")
    {
        dev = std::stoi(args);
        if (dev == std::clamp(dev, 1, 20))
        {
            std::cerr << "Valid dev number: " << dev << std::endl;
            dev = dev - 1;
        }
        else
        {
            std::cerr << "Invalid dev number: " << dev << std::endl;
            dev = 0;
        }
    }

    if (!Global::AdapterInstance.getConnected())
    {
        Global::AdapterInstance.connect();

        if (Global::AdapterInstance.getStatus() == FT_OK)
        {
            TerminalDisplay->AppendText(terminalTimestampOutput("Connected to a device\n"));
            std::cerr << "Connected to " << dev << std::endl;
        }
        else
        {
            std::cerr << "Couldnt connect" << std::endl;
            TerminalDisplay->AppendText(terminalTimestampOutput("Couldnt connect to a device\n Is programm running as SU?\n Is the FTDI_SIO Driver unloaded?\n"));
        }

        FT_STATUS ftStatus = FT_Purge(Global::AdapterInstance.getHandle(), FT_PURGE_RX | FT_PURGE_TX);
        printErr(ftStatus,"Purge Failed");

    }
    else
    {
        TerminalDisplay->AppendText(terminalTimestampOutput("Device already connected\n"));
    }
}

void TerminalWindow::disconnectDevice(const std::string& args = "")
{
    std::cerr << "Command entered: disconnect with arg: " << args << std::endl;

    Global::AdapterInstance.disconnect();

    if (Global::AdapterInstance.getStatus() == FT_OK)
    {
        TerminalDisplay->AppendText(terminalTimestampOutput("Disconnected from a device\n"));
        std::cerr << "disconnected from current device" << std::endl;
    }
    else
    {
        TerminalDisplay->AppendText(terminalTimestampOutput("Coulnd't disconnect check if a device is connected with: status\n"));
    }

}

wxString TerminalWindow::sendToDevice(const std::string& args)
{
    std::cerr << "terminal Command send " << args << " Entered" << std::endl;

    wxString GPIBText = args;
    std::string CheckText(GPIBText.ToUTF8());

    wxString Text = Global::AdapterInstance.write(CheckText);

    TerminalDisplay->AppendText(terminalTimestampOutput(Text));

    sleepMs(100);   //wait for responce

    std::cerr << "Reading from device..." << std::endl;

    Text = Global::AdapterInstance.read();

    TerminalDisplay->AppendText(terminalTimestampOutput(Text));

    return Global::AdapterInstance.getLastMsgReseived();
}

wxString TerminalWindow::readFromDevice(const std::string& args = "")
{

    std::cerr << "command read entered with args: " << args << std::endl;

    wxString Text = Global::AdapterInstance.read();

    TerminalWindow::TerminalDisplay->AppendText(terminalTimestampOutput(Text));

    return Global::AdapterInstance.getLastMsgReseived();
}

void TerminalWindow::writeToDevice(const std::string& args)
{
    std::cerr << "Write Command Entered" << std::endl;

    wxString Text = Global::AdapterInstance.write(args);

    TerminalWindow::TerminalDisplay->AppendText(terminalTimestampOutput(Text));
}

void TerminalWindow::configDevice(const std::string& args = "")
{
    wxString Text;

    if (args != "")
    {
        if (std::stoi(args) == std::clamp(std::stoi(args), 1, 1'000'000))
        {
            int BaudRate = std::stoi(args);
            Global::AdapterInstance.setBaudrate(BaudRate);
            std::cerr << "Set Baudrate to " << Global::AdapterInstance.getBaudrate() << std::endl;

        }
        else
        {
            std::cerr << "Using Default Baudrate: " << Global::AdapterInstance.getBaudrate() << std::endl;
        }
    }
    else
    {
        std::cerr << "Using Default Baudrate: " << Global::AdapterInstance.getBaudrate() << std::endl;
    }

    Global::AdapterInstance.config();

    if (Global::AdapterInstance.getStatus() == FT_OK)
    {
        Text = "Set Device BaudRate to " + std::to_string(Global::AdapterInstance.getBaudrate()) + "\n";
        TerminalWindow::TerminalDisplay->AppendText(terminalTimestampOutput(Text));
        std::cerr << "Baudrate set to: " << std::to_string(Global::AdapterInstance.getBaudrate()) << std::endl;
    }
    else
    {
        Text = "Failed to config device check if run as SU\n";
        TerminalWindow::TerminalDisplay->AppendText(terminalTimestampOutput(Text));
    }
}

void TerminalWindow::testDevice(const std::string& args = "")
{
    if (args == "")
    {
        connectDevice();
        configDevice();


        writeToDevice("++clr");
        sleepMs(200);
        writeToDevice("++mode 1");
        writeToDevice("++auto 1");
        writeToDevice("++eos 2"); //lf
        writeToDevice("++eoi 1");
        writeToDevice("++eot_enable 0");
        writeToDevice("++eot_char 10");
        writeToDevice("++addr 20");
        sendToDevice("++ver");

        //writeToDevice("IDE?");
        //writeToDevice("++read");
        //auf antwort warten
    }
    else if(args == "1")
    {

        writeToDevice("++rst");
        sleepMs(200);

        writeToDevice("++mode 1");
        writeToDevice("++auto 0");
        writeToDevice("++addr 20");

        writeToDevice("*IDE?");
        writeToDevice("++read eoi");
        sleepMs(50);
        readFromDevice();
    }
    else if(args == "mess")
    {
        writeToDevice("++auto 0");

        //Rest GPIB Device
        writeToDevice("RST");
        writeToDevice("*CLR");
        //Set Range and Maker
        writeToDevice("FREQ:CENT 1 GHZ");
        writeToDevice("FREQ:SPAN 10 MHZ");
        writeToDevice("BAND:RES 100 KHZ ");
        writeToDevice("CALC:MARK:MAX");
        //wait for fin
        writeToDevice("*WAI");
        //read makers

        //after setup set to auto 1
        writeToDevice("++auto 1");
        sleepMs(200);
        sendToDevice("CALC:MARK1:Y?");
        sendToDevice("CALC:MARK1:X?");


    }
    else if ("big")
    {

        writeToDevice("++auto 0");
        writeToDevice("INIT:CONT OFF"); //Dauerhafter sweep aus
        writeToDevice("SWE:POIN 10"); //100 messpunkte über messbereich aufnehmen
        writeToDevice("FREQ:STAR 80 MHZ");
        writeToDevice("FREQ:STOP 120 MHZ");
        writeToDevice("BAND:RES 100 KHZ ");

        writeToDevice("FORM:DATA ASC");
        writeToDevice("FORM:BORD NORM");
        writeToDevice("SWE:TIME AUTO");
        sleepMs(200);
        writeToDevice("SWE:TIME?");
        wxString swpTime = sendToDevice("++read");

        //float muS = std::stof(swpTime.c_str())*100;
        writeToDevice("INIT:IMM"); //Messung starten
        writeToDevice("*WAI");
        sleepMs(300);

        wxString responce;
        int i = 0;
        while ((responce.substr(0,1) != "1") || (i == 20))
        {
            writeToDevice("*OPC?");
            sleepMs(100);
            responce = sendToDevice("++read eoi");
            i++;
        }

        writeToDevice("TRAC:DATA? TRACE1");
        //writeToDevice("TRAC1:DATA?");
        writeToDevice("++read eoi");

        sleepMs(100);
        wxString Trace = readFromDevice();
        std::vector<double> x_werte;

        Global::Messung.seperateDataBlock(Trace);

        wxFile file;

        // wxFile::write_append zum Anhängen oder wxFile::write zum Überschreiben
        if (file.Open("/home/jon3r/Documents/Code/CodeBlocks/Test_GPIB_Terminal/GpibScripts/Z_Log.txt", wxFile::write_append)) {
            file.Write(Trace + "\n");
            file.Close();
        } else {
            wxLogError("Konnte Datei nicht schreiben!");
        }

        std::cerr << "Received Trace: " << Trace << std::endl;

        writeToDevice("INIT:CONT ON"); //Dauerhafter sweep an
    }
}

void TerminalWindow::OnEnterTerminal(wxCommandEvent& event)
{
    wxTextCtrl* Terminal = static_cast<wxTextCtrl*>(event.GetEventObject());
    wxString TText = Terminal->GetValue();
    Terminal->SetValue("");

    std::cerr << "user entered: " << TText.c_str() << std::endl;

    //Output to terminal
    TerminalDisplay->AppendText(terminalTimestampOutput(TText + "\n"));

    //entspechende Funktionen finden und aufrufen
    std::string strCmd;
    std::string args;
    std::string rawText(TText.ToUTF8());

    size_t firstSpace = rawText.find(' ');

    //seperiert Befehl und argument
    if (firstSpace == std::string::npos)
    {
        strCmd = rawText;
        args = "";
    }
    else
    {
        strCmd = rawText.substr(0, firstSpace);
        args = rawText.substr(firstSpace + 1);
    }

    //findet passenden command und führt die funktion aus
    auto match = cmds.find(strCmd);

    if (match != cmds.end())
    {
        match->second(args);
    }
    else
    {
        TerminalDisplay->AppendText(terminalTimestampOutput("Unknown command!\n"));
    }
}
//-----Terminal Window Methodes End -----

//-----Function Window Constructor-----
FunctionWindow::FunctionWindow(wxWindow *parent)
    : wxDialog(parent, wxID_ANY, "Function Test Window", wxDefaultPosition, wxSize(500,750))
{
    wxPanel* panelfunc = new wxPanel(this);

    //Input text lable
    wxStaticText* discFuncInput = new wxStaticText(panelfunc,wxID_ANY,"Input text to write: ");

    //Function input textbox
    writeFuncInput = new wxTextCtrl(panelfunc, wxID_ANY,"",wxDefaultPosition,wxSize(300, 40));
    //set Cursor in writeFuncInput window
    writeFuncInput->SetFocus();


    //Create Button "Write to GPIB"
    wxButton* writeGpibButton       = new wxButton(panelfunc, wxID_ANY, "Write to GPIB",            wxPoint(10,0));
    //Create Button "Read to GPIB"
    wxButton* readGpibButton        = new wxButton(panelfunc, wxID_ANY, "Read from GPIB",           wxPoint(10,0));
    //Create Button "Write and Read GPIB"
    wxButton* readWriteGpibButton   = new wxButton(panelfunc, wxID_ANY, "Write and Read GPIB",      wxPoint(10,0));
    //Create Button "Scan For Device"
    wxButton* scanUsbButton         = new wxButton(panelfunc, wxID_ANY, "Scan For Device",          wxPoint(10,0));
    //Create Button "Configure USB Device"
    wxButton* devConfigButton       = new wxButton(panelfunc, wxID_ANY, "Configure USB Device",     wxPoint(10,0));
    //Create Button "Connect / Disconnect"
    wxButton* connectDevGpibButton  = new wxButton(panelfunc, wxID_ANY, "Connected / Disconnect",   wxPoint(10,0));
    //Create Button "Test Save File"
    wxButton* TestSaveFileButton    = new wxButton(panelfunc, wxID_ANY, "Test Save File",           wxPoint(10,0));
    //Create Button "Test Multi Mesurement"
    wxButton* TestMultiMessButton   = new wxButton(panelfunc, wxID_ANY, "Test Multi mesurement",    wxPoint(10,0));

    wxButton* TestButton            = new wxButton(panelfunc, wxID_ANY, "Test Other",               wxPoint(10,0));


    //Funtion Output Lable
    wxStaticText* discFuncOutput = new wxStaticText(panelfunc,wxID_ANY,"Function output: ");
    //Funtion Output Text Box
    textFuncOutput = new wxTextCtrl(panelfunc, wxID_ANY,"",wxDefaultPosition,wxSize(300, 200), wxTE_MULTILINE);

    // Function bindes
    writeGpibButton     ->Bind(wxEVT_BUTTON, &FunctionWindow::OnWriteGpib,      this);
    readGpibButton      ->Bind(wxEVT_BUTTON, &FunctionWindow::OnReadGpib,       this);
    readWriteGpibButton ->Bind(wxEVT_BUTTON, &FunctionWindow::OnReadWriteGpib,  this);
    scanUsbButton       ->Bind(wxEVT_BUTTON, &FunctionWindow::OnUsbScan,        this);
    devConfigButton     ->Bind(wxEVT_BUTTON, &FunctionWindow::OnUsbConfig,      this);
    connectDevGpibButton->Bind(wxEVT_BUTTON, &FunctionWindow::OnConDisconGpib,  this);
    TestSaveFileButton  ->Bind(wxEVT_BUTTON, &FunctionWindow::OnTestSaveFile,   this);
    TestMultiMessButton ->Bind(wxEVT_BUTTON, &FunctionWindow::OnTestMultiMess,  this);
    TestButton          ->Bind(wxEVT_BUTTON, &FunctionWindow::OnTest,           this);
    //sizer     Set Window Layout
    wxBoxSizer* sizerFunc = new wxBoxSizer(wxVERTICAL);
    sizerFunc->Add(discFuncInput,       0, wxEXPAND | wxALL , 10);
    sizerFunc->Add(writeFuncInput,      0, wxEXPAND | wxALL , 10);
    sizerFunc->Add(scanUsbButton,       0, wxEXPAND | wxALL , 10);
    sizerFunc->Add(connectDevGpibButton,0, wxEXPAND | wxALL , 10);
    sizerFunc->Add(devConfigButton,     0, wxEXPAND | wxALL , 10);
    sizerFunc->Add(writeGpibButton,     0, wxEXPAND | wxALL , 10);
    sizerFunc->Add(readGpibButton,      0, wxEXPAND | wxALL , 10);
    sizerFunc->Add(readWriteGpibButton, 0, wxEXPAND | wxALL , 10);
    sizerFunc->Add(TestSaveFileButton,  0, wxEXPAND | wxALL , 10);
    sizerFunc->Add(TestMultiMessButton, 0, wxEXPAND | wxALL , 10);
    sizerFunc->Add(TestButton,          0, wxEXPAND | wxALL , 10);

    sizerFunc->Add(discFuncOutput,      0, wxEXPAND | wxALL , 10);
    sizerFunc->Add(textFuncOutput,      0, wxEXPAND | wxALL , 10);
    panelfunc->SetSizerAndFit(sizerFunc);
}
//-----Function Window Destructor-----
FunctionWindow::~FunctionWindow()
{
    Global::AdapterInstance.disconnect();
}
//-----Function Window Methodes-----
void FunctionWindow::OnUsbScan(wxCommandEvent& event)
{
    std::cerr << "Scan USB Devices" << std::endl;

    DWORD devices = scanUsbDev();
    wxString deviceNumString = std::to_string(devices) + " Devices Found" + "\n";

    if (devices <= 0)
    {
        textFuncOutput->AppendText(terminalTimestampOutput("no device found \n"));
    }
    else
    {
        textFuncOutput->AppendText(terminalTimestampOutput(deviceNumString));
    }
}
void FunctionWindow::OnConDisconGpib(wxCommandEvent& event)
{
    if (Global::AdapterInstance.getConnected() == false)
    {
       Global::AdapterInstance.connect();

        if (Global::AdapterInstance.getStatus() == FT_OK)
        {
            textFuncOutput->AppendText(terminalTimestampOutput("Connected to a device\n"));
        }
    }
    else
    {
        Global::AdapterInstance.disconnect();

        if (Global::AdapterInstance.getStatus() == FT_OK)
        {
            textFuncOutput->AppendText(terminalTimestampOutput("Disconnected from a device\n"));
        }
    }
}
void FunctionWindow::OnTestSaveFile(wxCommandEvent& event)
{
    std::thread CsvThread;
    std::cerr << "Pressed Test Save File" << std::endl;
    sData TestObjekt;
    sData TestObjekt2;

    int xpt = 1;
    int ypt = 1;
    int count = 10000;
    int endFreq = 50'000;
    // set Mesurement Header
    TestObjekt.setTimeAndDate();
    TestObjekt.setNumberOfPts_X(xpt);
    TestObjekt.setNumberOfPts_Y(ypt);
    TestObjekt.setEndFreq(endFreq);

    std::vector<double> TestArray;

    for (int i = 0; i < count; i++)
    {
        try
        {
            TestArray.push_back(double(i));
        }
        catch(const std::exception& e)
        {
            std::cerr << "pushback failed" << e.what() << '\n';
        }
    }
    std::cout << "TestArray Ok" << std::endl;
    std::cout << "count: " << count << std::endl;
    std::cout << "real size :" << TestArray.size() << std::endl;
    TestObjekt.setNumberofPts_Array(count);
    std::cout << "setNumberofPts_Array Ok: " << TestObjekt.getNumberOfPts_Array()<< std::endl;
    for (int i = 0; i < xpt; i++)
    {
        for (int j = 0; j < ypt; j++)
        {
            try
            {
                std::cout << "x: " << i << " y: " << j << std::endl;

                TestObjekt.set3DDataReal(TestArray,i,j);
            }
            catch(const std::exception& e)
            {
                std::cerr << "Set 3D Data failed: " << e.what() << '\n';
                std::cerr << i << " " << j << std::endl;
            }
        }
    }
    std::cout << "Set 3D Data Ok" << std::endl;



    sData::sParam* TestData = TestObjekt.GetParameter();

    std::cerr << "Zeit: " << TestData->Time << std::endl;

    std::cerr << "Schreib daten in CSV" << std::endl;

    wxString Dateiname = "D:\\CodeProjects\\VSCode\\projects\\Diplom\\Test-GPIB-Terminal\\LogFiles\\TestCSVNeu";

    int messungen = TestObjekt.getNumberOfPts_X()* TestObjekt.getNumberOfPts_Y();
    for (int i = 1; i <= messungen; i++)
    {
        if (!saveToCsvFile(Dateiname, TestObjekt, i))
        {
            std::cerr << "Failed to save file" << std::endl;
        }
    }

    // bis hier alles ok

    readCsvFile(Dateiname, TestObjekt2);

    Dateiname = "D:\\CodeProjects\\VSCode\\projects\\Diplom\\Test-GPIB-Terminal\\LogFiles\\TestCSVNeuKopie";

    int totalpoints = TestObjekt2.getNumberOfPts_X()* TestObjekt2.getNumberOfPts_Y();
    std::cout << "[Debug] Totalpoints: " << totalpoints << std::endl;

    TestObjekt2.setFileName("kopie");
    TestObjekt2.setTimeAndDate();

    for (int i = 1; i <= totalpoints; i++)
    {
        if (!saveToCsvFile(Dateiname, TestObjekt2, i))
        {
            std::cerr << "Failed to save file" << std::endl;
        }
    }
}
void FunctionWindow::OnWriteGpib(wxCommandEvent& event)
{
    std::cerr << "Write Pressed!" << std::endl;

    wxString GPIBText = FunctionWindow::writeFuncInput->GetValue();
    FunctionWindow::writeFuncInput->SetValue("");

    std::string CheckText(GPIBText.ToUTF8());

    wxString Text = Global::AdapterInstance.write(CheckText);

    FunctionWindow::textFuncOutput->AppendText(terminalTimestampOutput(Text));
}
void FunctionWindow::OnReadGpib(wxCommandEvent& event)
{
    std::cerr << "On Read Pressed" << std::endl;

    wxString Text = Global::AdapterInstance.read();

    FunctionWindow::textFuncOutput->AppendText(terminalTimestampOutput(Text));
}
void FunctionWindow::OnReadWriteGpib(wxCommandEvent& event)
{
    std::cerr << "Read / Write Pressed!" << std::endl;

    std::cerr << "Writing to device..." << std::endl;

    wxString GPIBText = FunctionWindow::writeFuncInput->GetValue();
    std::string CheckText(GPIBText.ToUTF8());

    FunctionWindow::writeFuncInput->SetValue("");

    wxString Text = Global::AdapterInstance.write(CheckText);

    FunctionWindow::textFuncOutput->AppendText(terminalTimestampOutput(Text));

    sleepMs(100);   //wait for responce

    std::cerr << "Reading from device..." << std::endl;

    Text = Global::AdapterInstance.read();

    FunctionWindow::textFuncOutput->AppendText(terminalTimestampOutput(Text));
}
void FunctionWindow::OnUsbConfig(wxCommandEvent& event)
{
    Global::AdapterInstance.config();

    if (Global::AdapterInstance.getStatus() == FT_OK)
    {
        FunctionWindow::textFuncOutput->AppendText(terminalTimestampOutput("Set Default config\n"));
    }
    else
    {
        FunctionWindow::textFuncOutput->AppendText(terminalTimestampOutput("Config failed\n"));
    }

    FunctionWindow::textFuncOutput->AppendText(terminalTimestampOutput(Global::AdapterInstance.statusText()));
}
void FunctionWindow::OnTestMultiMess(wxCommandEvent& event)
{
    //Create new sub window
    MultiMessWindow *MultiMessWin = new MultiMessWindow(this);
    //open Window Pauses Main Window
    MultiMessWin->ShowModal();
    //Close Window
    MultiMessWin->Destroy();

}
void FunctionWindow::OnTest(wxCommandEvent& event)
{
    std::cerr << "Test wxLogDebug" << std::endl;
    std::cerr << "Test cerr" << std::endl;
    std::cout << "Test cout" << std::endl;
}
//-----Function Window Methodes End -----

//-----Settings Window--------
SettingsWindow::SettingsWindow(wxWindow *parent)
    : wxDialog(parent, wxID_ANY, "Settings", wxDefaultPosition, wxSize(500,750))
{
    wxPanel* mainPanel = new wxPanel(this, wxID_ANY);

    //Main settingswindow elements
    wxStaticText* infoText = new wxStaticText(mainPanel, wxID_ANY, "Settings for FSU Display, Adapter and General programm enviroment");
    wxButton* resetButton = new wxButton(mainPanel,wxID_ANY,"reset all");
    //Subtab elements
    wxNotebook* notebook = new wxNotebook(mainPanel, wxID_ANY);

    SettingsTabDisplay* displayTab = new SettingsTabDisplay(notebook, wxString::FromUTF8("Inhalt für 'display'"));
    SettingsTabAdapter* adapterTab = new SettingsTabAdapter(notebook, wxString::FromUTF8("Inhalt für 'adapter'"));
    SettingsTabGeneral* generalTab = new SettingsTabGeneral(notebook, wxString::FromUTF8("Inhalt für 'adapter'"));

    notebook->AddPage(displayTab, "Display");
    notebook->AddPage(adapterTab, "Adapter");
    notebook->AddPage(generalTab, "General");

    //  Layout-Management (Sizer) verwenden, damit das Notebook
    //    das Hauptfenster ausfüllt
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    sizer->Add(infoText, 1, wxEXPAND | wxALL, 5);
    sizer->Add(notebook, 18, wxEXPAND | wxALL, 5); // 1 = dehnbar, wxEXPAND = ausfüllen
    sizer->Add(resetButton,1 ,wxEXPAND | wxALL, 5);
    mainPanel->SetSizer(sizer);

}
//-----settings window subtabs------
SettingsTabDisplay::SettingsTabDisplay(wxNotebook *parent, const wxString &label)
    : wxPanel(parent, wxID_ANY)
{
    wxArrayString freqEinheiten;
    freqEinheiten.Add("Hz");
    freqEinheiten.Add("kHz");
    freqEinheiten.Add("MHz");
    freqEinheiten.Add("GHz");

    wxArrayString pegelEinheiten;
    pegelEinheiten.Add("DBM");
    pegelEinheiten.Add("DBMU");
    pegelEinheiten.Add("DBUV");
    pegelEinheiten.Add("DBUA");
    pegelEinheiten.Add("DBPW");
    pegelEinheiten.Add("VOLT");
    pegelEinheiten.Add("AMPERE");
    pegelEinheiten.Add("WATT");


    wxArrayString scalingY;
    scalingY.Add("Logarthmmic");
    scalingY.Add("Linear");

    // Check if input is number
    wxTextValidator val(wxFILTER_NUMERIC);

    wxStaticText* labelText = new wxStaticText(this, wxID_ANY, label, wxPoint(10,10));

    // ----- Start-Ende Elemente-----Start
    startEndeCheck = new wxCheckBox(this, wxID_ANY, "Start - Ende Nutzen");
    // Frequenz Start
    wxStaticText* descriptionText_1 = new wxStaticText(this, wxID_ANY, "Start-Frequenz:", wxPoint(10,10));
    inputText_1             = new wxTextCtrl(this, wxID_ANY, "75");
    freqEinheitAuswahl_1    = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, freqEinheiten);

    inputText_1         ->SetValidator(val);
    freqEinheitAuswahl_1->SetSelection(2);

    // Frequenz Ende
    wxStaticText* descriptionText_2 = new wxStaticText(this, wxID_ANY, "End-Frequenz:", wxPoint(10,10));
    inputText_2             = new wxTextCtrl(this, wxID_ANY,"125");
    freqEinheitAuswahl_2    = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, freqEinheiten);

    inputText_2         ->SetValidator(val);
    freqEinheitAuswahl_2->SetSelection(2);
    // ----- Start-Ende Elemente-----Ende

    // ----- Center-Spanne Elemente-----Start
    centerSpanCheck = new wxCheckBox(this, wxID_ANY, "Center - Spanne Nutzen");
    // Frequenz Center
    wxStaticText* descriptionText_3 = new wxStaticText(this, wxID_ANY, "Center-Frequenz:", wxPoint(10,10));
    inputText_3             = new wxTextCtrl(this, wxID_ANY,"100");
    freqEinheitAuswahl_3    = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, freqEinheiten);

    inputText_3         ->SetValidator(val);
    freqEinheitAuswahl_3->SetSelection(2);

    // Frequenz Spanne
    wxStaticText* descriptionText_4 = new wxStaticText(this, wxID_ANY, "Span-Frequenz:", wxPoint(10,10));
    inputText_4             = new wxTextCtrl(this, wxID_ANY, "50");
    freqEinheitAuswahl_4    = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, freqEinheiten);

    inputText_4         ->SetValidator(val);
    freqEinheitAuswahl_4->SetSelection(2);
    // ----- Center-Spanne Elemente-----Ende

    //Y-Achsen Elemente-----Start
    wxStaticText* descriptionText_5 = new wxStaticText(this, wxID_ANY, "Y-Scaling:", wxPoint(10,10));

    inputText_5             = new wxTextCtrl(this, wxID_ANY,"100");
    pegelEinheitAuswahl     = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, pegelEinheiten);

    inputText_5         ->SetValidator(val);
    pegelEinheitAuswahl ->SetSelection(0);


    wxStaticText* descriptionText_6 = new wxStaticText(this, wxID_ANY, "Y-Scale Spacing:", wxPoint(10,10));

    yScalingAuswahl = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, scalingY);
    yScalingAuswahl ->SetSelection(0);

    wxStaticText* descriptionText_7 = new wxStaticText(this, wxID_ANY, "Referenzpegel in dB:", wxPoint(10,10));

    inputText_7 = new wxTextCtrl(this, wxID_ANY, "-20");
    inputText_7 ->SetValidator(val);
    //Y-Achsen Elemente-----Ende

    //Knöpfe START
    wxButton* anwendenButton    = new wxButton(this, wxID_ANY, "Anwenden");
    wxButton* getCurrentButton  = new wxButton(this, wxID_ANY, wxString::FromUTF8("Messgerät Einstellungen Laden"));

    startEndeCheck  ->Bind(wxEVT_CHECKBOX,  &SettingsTabDisplay::toggleSelectionEvent,  this);
    centerSpanCheck ->Bind(wxEVT_CHECKBOX,  &SettingsTabDisplay::toggleSelectionEvent,  this);
    anwendenButton  ->Bind(wxEVT_BUTTON,    &SettingsTabDisplay::anwendenButton,        this);
    getCurrentButton->Bind(wxEVT_BUTTON,    &SettingsTabDisplay::getCurrentButton,      this);
    //knöpfe ENDE


    //Start-Ende Sizer-----START
    wxBoxSizer* sizerHorizontal_1 = new wxBoxSizer(wxHORIZONTAL);
    sizerHorizontal_1->Add(descriptionText_1,   1, wxALL, 5);
    sizerHorizontal_1->Add(inputText_1,         1, wxALL, 5);
    sizerHorizontal_1->Add(freqEinheitAuswahl_1,1, wxALL, 5);

    wxBoxSizer* sizerHorizontal_2 = new wxBoxSizer(wxHORIZONTAL);
    sizerHorizontal_2->Add(descriptionText_2,   1, wxALL, 5);
    sizerHorizontal_2->Add(inputText_2,         1, wxALL, 5);
    sizerHorizontal_2->Add(freqEinheitAuswahl_2,1, wxALL, 5);

    //Rahmen um den abschnitt
    wxStaticBoxSizer* staticSizer_1 = new wxStaticBoxSizer(wxVERTICAL, this, "Anzeigebereich Start-Ende Frequenz");
    staticSizer_1->Add(startEndeCheck,      0, wxALL | wxEXPAND, 5);
    staticSizer_1->Add(sizerHorizontal_1,   0, wxALL | wxEXPAND, 5);
    staticSizer_1->Add(sizerHorizontal_2,   0, wxALL | wxEXPAND, 5);
    //Start-Ende Sizer-----ENDE

    //Center-Spann Sizer-----START
    wxBoxSizer* sizerHorizontal_3 = new wxBoxSizer(wxHORIZONTAL);
    sizerHorizontal_3->Add(descriptionText_3,   1, wxALL, 5);
    sizerHorizontal_3->Add(inputText_3,         1, wxALL, 5);
    sizerHorizontal_3->Add(freqEinheitAuswahl_3,1, wxALL, 5);

    wxBoxSizer* sizerHorizontal_4 = new wxBoxSizer(wxHORIZONTAL);
    sizerHorizontal_4->Add(descriptionText_4,   1, wxALL, 5);
    sizerHorizontal_4->Add(inputText_4,         1, wxALL, 5);
    sizerHorizontal_4->Add(freqEinheitAuswahl_4,1, wxALL, 5);

    //Rahmen um den abschnitt
    wxStaticBoxSizer* staticSizer_2 = new wxStaticBoxSizer(wxVERTICAL, this, "Anzeigebereich Center-Span Frequenz");
    staticSizer_2->Add(centerSpanCheck,     0, wxALL | wxEXPAND, 5);
    staticSizer_2->Add(sizerHorizontal_3,   0, wxALL | wxEXPAND, 5);
    staticSizer_2->Add(sizerHorizontal_4,   0, wxALL | wxEXPAND, 5);
    //Center-Spann Sizer-----ENDE

    //y-achsen einstellung Sizer-----START
    wxBoxSizer* sizerHorizontal_5 = new wxBoxSizer(wxHORIZONTAL);
    sizerHorizontal_5->Add(descriptionText_5,   1, wxALL | wxEXPAND, 5);
    sizerHorizontal_5->Add(inputText_5,         1, wxALL | wxEXPAND, 5);
    sizerHorizontal_5->Add(pegelEinheitAuswahl, 1, wxALL | wxEXPAND, 5);

    wxBoxSizer* sizerHorizontal_6 = new wxBoxSizer(wxHORIZONTAL);
    sizerHorizontal_6->Add(descriptionText_6,   2,  wxALL | wxEXPAND, 5);
    sizerHorizontal_6->Add(yScalingAuswahl,     1,  wxALL | wxEXPAND, 5);

    wxBoxSizer* sizerHorizontal_7 = new wxBoxSizer(wxHORIZONTAL);
    sizerHorizontal_7->Add(descriptionText_7,   2 , wxALL | wxEXPAND, 5);
    sizerHorizontal_7->Add(inputText_7,         1 , wxALL | wxEXPAND, 5);

    //Rahmen um den abschnitt
    wxStaticBoxSizer* staticSizer_3 = new wxStaticBoxSizer(wxVERTICAL, this, "Anzeigebereich Pegel-Skalierung");
    staticSizer_3->Add(sizerHorizontal_6, 0, wxALL | wxEXPAND, 5);
    staticSizer_3->Add(sizerHorizontal_5, 0, wxALL | wxEXPAND, 5);
    staticSizer_3->Add(sizerHorizontal_7, 0, wxALL | wxEXPAND, 5);

    //y-achsen einstellung Sizer-----ENDE

    //Knöpfe
    wxBoxSizer* sizerHorizontal_8 = new wxBoxSizer(wxHORIZONTAL);
    sizerHorizontal_8->Add(getCurrentButton,1, wxALL | wxEXPAND, 5);
    sizerHorizontal_8->Add(anwendenButton,  1, wxALL | wxEXPAND, 5);

    //knöpfe ENDE

    //Verticaler Sizer
    wxBoxSizer* sizerVertical = new wxBoxSizer(wxVERTICAL);
    sizerVertical->Add(labelText ,          0 , wxALL | wxEXPAND, 5);
    sizerVertical->Add(staticSizer_1 ,      0 , wxALL | wxEXPAND, 5);
    sizerVertical->Add(staticSizer_2 ,      0 , wxALL | wxEXPAND, 5);
    sizerVertical->Add(staticSizer_3 ,      0 , wxALL | wxEXPAND, 5);
    sizerVertical->Add(sizerHorizontal_8 ,  0 , wxALL | wxEXPAND, 5);

    this->SetSizerAndFit(sizerVertical);

    toggleSelection(); //anfangszustand herstellen

    Global::AdapterInstance.connect();
    Global::AdapterInstance.config();
}
void SettingsTabDisplay::anwendenButton(wxCommandEvent& event)
{
    std::string cmdText;
    getValues();
    if (Global::AdapterInstance.getConnected())
    {
        if (!useCenterSpan)
        {
            cmdText = "FREQ:STAR " + FreqStartSet + FreqStartSetUnit;
            Global::AdapterInstance.write(cmdText);
            cmdText = "FREQ:STOP " + FreqEndeSet + FreqEndeSetUnit;
            Global::AdapterInstance.write(cmdText);
        }
        else if (!useStartEnde)
        {
            cmdText = "FREQ:CENT " + FreqCenterSet + FreqCenterSetUnit;
            Global::AdapterInstance.write(cmdText);
            cmdText = "FREQ:SPAN " + FreqSpanSet + FreqSpanSetUnit;
            Global::AdapterInstance.write(cmdText);
        }

        cmdText = "UNIT:POW " + pegelSet + pegelSetUnit;
        Global::AdapterInstance.write(cmdText);
        cmdText = "DISP:TRAC:Y:RLEV " + refPegelSet;
        Global::AdapterInstance.write(cmdText);
        cmdText = "DISP:TRAC:Y:SPAC " + refPegelSet;
        Global::AdapterInstance.write(cmdText);
    }
}
void SettingsTabDisplay::getCurrentButton(wxCommandEvent& event)
{

    if (Global::AdapterInstance.getConnected())
    {
        FreqStartSet    = Global::AdapterInstance.send("FREQ:STAR?");
        FreqStartSet    = Global::AdapterInstance.send("FREQ:STOP?");
        FreqEndeSet     = Global::AdapterInstance.send("FREQ:CENT?");
        FreqCenterSet   = Global::AdapterInstance.send("FREQ:SPAN?");
        FreqSpanSet     = Global::AdapterInstance.send("CALCulate:UNIT:POWer?");
        pegelSet        = Global::AdapterInstance.send("DISP:TRAC:Y:RLEV?");
        refPegelSet     = Global::AdapterInstance.send("DISP:TRAC:Y:SPAC?");

        setValues();
    }
}
void SettingsTabDisplay::toggleSelectionEvent(wxCommandEvent& event)
{
    toggleSelection();
}
void SettingsTabDisplay::getValues()
{
    FreqStartSetUnit    = freqEinheitAuswahl_1->GetStringSelection();
    FreqEndeSetUnit     = freqEinheitAuswahl_2->GetStringSelection();
    FreqCenterSetUnit   = freqEinheitAuswahl_3->GetStringSelection();
    FreqSpanSetUnit     = freqEinheitAuswahl_4->GetStringSelection();
    pegelSetUnit        = pegelEinheitAuswahl->GetStringSelection();
    scalingYSetUnit     = yScalingAuswahl->GetStringSelection();

    FreqStartSet        = inputText_1->GetValue();
    FreqEndeSet         = inputText_2->GetValue();
    FreqCenterSet       = inputText_3->GetValue();
    FreqSpanSet         = inputText_4->GetValue();
    pegelSet            = inputText_5->GetValue();
    refPegelSet         = inputText_7->GetValue();

    useStartEnde        = startEndeCheck->GetValue();
    useCenterSpan       = centerSpanCheck->GetValue();
}
void SettingsTabDisplay::setValues()
{
    freqEinheitAuswahl_1->SetStringSelection(FreqStartSetUnit);
    freqEinheitAuswahl_2->SetStringSelection(FreqEndeSetUnit);
    freqEinheitAuswahl_3->SetStringSelection(FreqCenterSetUnit);
    freqEinheitAuswahl_4->SetStringSelection(FreqSpanSetUnit);
    pegelEinheitAuswahl ->SetStringSelection(pegelSetUnit);
    yScalingAuswahl     ->SetStringSelection(scalingYSetUnit);

    inputText_1         ->SetValue(FreqStartSet);
    inputText_2         ->SetValue(FreqEndeSet);
    inputText_3         ->SetValue(FreqCenterSet);
    inputText_4         ->SetValue(FreqSpanSet);
    inputText_5         ->SetValue(pegelSet);
    inputText_7         ->SetValue(refPegelSet);

    startEndeCheck      ->SetValue(useStartEnde);
    centerSpanCheck     ->SetValue(useCenterSpan);
}
void SettingsTabDisplay::toggleSelection()
{
    if (!useStartEnde && !useCenterSpan)
    {   //für setup
        centerSpanCheck->SetValue(false);
        startEndeCheck->SetValue(true);
        inputText_1->Enable(true);
        inputText_2->Enable(true);
        inputText_3->Enable(false);
        inputText_4->Enable(false);
        freqEinheitAuswahl_1->Enable(true);
        freqEinheitAuswahl_2->Enable(true);
        freqEinheitAuswahl_3->Enable(false);
        freqEinheitAuswahl_4->Enable(false);
    }
    else if(!useStartEnde && useCenterSpan)
    {
        startEndeCheck->SetValue(true);
        centerSpanCheck->SetValue(false);
        inputText_1->Enable(true);
        inputText_2->Enable(true);
        inputText_3->Enable(false);
        inputText_4->Enable(false);
        freqEinheitAuswahl_1->Enable(true);
        freqEinheitAuswahl_2->Enable(true);
        freqEinheitAuswahl_3->Enable(false);
        freqEinheitAuswahl_4->Enable(false);
    }
    else if (!useCenterSpan && useStartEnde)
    {
        startEndeCheck->SetValue(false);
        centerSpanCheck->SetValue(true);
        inputText_1->Enable(false);
        inputText_2->Enable(false);
        inputText_3->Enable(true);
        inputText_4->Enable(true);
        freqEinheitAuswahl_1->Enable(false);
        freqEinheitAuswahl_2->Enable(false);
        freqEinheitAuswahl_3->Enable(true);
        freqEinheitAuswahl_4->Enable(true);
    }

    getValues();
}


SettingsTabAdapter::SettingsTabAdapter(wxNotebook *parent, const wxString &label)
    : wxPanel(parent, wxID_ANY)
{
    new wxStaticText(this, wxID_ANY, label, wxPoint(10,10));
}

SettingsTabGeneral::SettingsTabGeneral(wxNotebook *parent, const wxString &label)
    : wxPanel(parent, wxID_ANY)
{
    new wxStaticText(this, wxID_ANY, label, wxPoint(10,10));
}
//------Settings window subtab end-----


//Helper Functions
