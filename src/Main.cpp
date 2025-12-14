#include <wx/wx.h>
#include <wx/notebook.h>
#include <wx/valtext.h>
#include <wx/string.h>
#include <wx/dir.h>
#include <wx/textfile.h>
#include <wx/msgdlg.h>
#include <map>
#include <thread>
#include "fkt_GPIB.h"
#include "fkt_d2xx.h"
#include "main.h"
#include "mathplot.h"

wxIMPLEMENT_APP(MainWin);

//-----MainWin Methodes-----
bool MainWin::OnInit()
{
    //Enable Debug output window
    wxLog::SetActiveTarget(new wxLogStderr());

    MainProgrammWin *frame = new MainProgrammWin(nullptr);
    frame->Show();

    return true;
}
//-----MainWin Methodes ende-----

//-----Main Programm Window-----
MainProgrammWin::MainProgrammWin( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) 
    : wxFrame( parent, id, title, pos, size, style )
{
    //TODO set new Functions

    //File Main binds
    Bind(wxEVT_MENU, &MainProgrammWin::MenuFileOpen, this, ID_Main_File_Open);
    Bind(wxEVT_MENU, &MainProgrammWin::MenuFileSave, this, ID_Main_File_Save);
    Bind(wxEVT_MENU, &MainProgrammWin::MenuFileSave, this, ID_Main_File_SaveAs);
    Bind(wxEVT_MENU, &MainProgrammWin::MenuFileClose, this, ID_Main_File_Exit);
    //Mesurement Menu binds
    Bind(wxEVT_MENU, &MainProgrammWin::MenuMesurementNew, this, ID_Main_Mesurement_New);
    Bind(wxEVT_MENU, &MainProgrammWin::MenuMesurementLoad, this, ID_Main_Mesurement_Open);
    Bind(wxEVT_MENU, &MainProgrammWin::MenuMesurementLoad, this, ID_Main_Mesurement_Load);
    Bind(wxEVT_MENU, &MainProgrammWin::MenuMesurementLoad, this, ID_Main_Mesurement_Preset_1);
    Bind(wxEVT_MENU, &MainProgrammWin::MenuMesurementLoad, this, ID_Main_Mesurement_Preset_2);
    Bind(wxEVT_MENU, &MainProgrammWin::MenuMesurementLoad, this, ID_Main_Mesurement_Preset_3);
    Bind(wxEVT_MENU, &MainProgrammWin::MenuMesurement2DMess, this, ID_Main_Mesurement_2D_Mess);
    Bind(wxEVT_MENU, &MainProgrammWin::MenuMesurementSetMarker, this, ID_Main_Mesurement_SetMarker);
    Bind(wxEVT_MENU, &MainProgrammWin::MenuMesurementSettings, this, ID_Main_Mesurement_Settings);
    //Test Menu binds
    Bind(wxEVT_MENU, &MainProgrammWin::MenuTestTerminal, this, ID_Main_Test_Terminal);
    Bind(wxEVT_MENU, &MainProgrammWin::MenuTestFunc, this, ID_Main_Test_Func);
    //Help Menu binds
    Bind(wxEVT_MENU, &MainProgrammWin::MenuHelpAbout, this, ID_Main_Help_About);


	this->SetSizeHints( wxDefaultSize, wxDefaultSize );

    //this->SetBackgroundColour(*wxLIGHT_GREY);

	m_menubarMainProg = new wxMenuBar( 0 );
    //file menu elements
	m_menu_File = new wxMenu();
	wxMenuItem* m_menuFile_Item_Open;
	m_menuFile_Item_Open = new wxMenuItem( m_menu_File, ID_Main_File_Open, wxString( wxT("Open") ) + wxT('\t') + wxT("CTRL + O"), wxEmptyString, wxITEM_NORMAL );
	m_menu_File->Append( m_menuFile_Item_Open );

    m_menu_File->AppendSeparator();
	
	wxMenuItem* m_menuFile_Item_Save;
	m_menuFile_Item_Save = new wxMenuItem( m_menu_File, ID_Main_File_Save, wxString( wxT("Save") ) + wxT('\t') + wxT("CTRL + S"), wxEmptyString, wxITEM_NORMAL );
	m_menu_File->Append( m_menuFile_Item_Save );
	
    wxMenuItem* m_menuFile_Item_SaveAs;
	m_menuFile_Item_SaveAs = new wxMenuItem( m_menu_File, ID_Main_File_Exit, wxString( wxT("Save as ...") ) , wxEmptyString, wxITEM_NORMAL );
	m_menu_File->Append( m_menuFile_Item_SaveAs );

	m_menu_File->AppendSeparator();

	wxMenuItem* m_menuFile_Item_Exit;
	m_menuFile_Item_Exit = new wxMenuItem( m_menu_File, ID_Main_File_Exit, wxString( wxT("Exit") ) , wxEmptyString, wxITEM_NORMAL );
	m_menu_File->Append( m_menuFile_Item_Exit );

    

	//File Menu
	m_menubarMainProg->Append( m_menu_File, wxT("File") ); 
	
    //Simutatuion menu elements
	m_menu_Sim = new wxMenu();

    //Simutatuin menu
	m_menubarMainProg->Append( m_menu_Sim, wxT("Simulation") ); 
	
    //Mesurement menu elements
	m_menu_Mesurement = new wxMenu();
	wxMenuItem* m_menuMesure_Item_New;
	m_menuMesure_Item_New = new wxMenuItem( m_menu_Mesurement, ID_Main_Mesurement_New, wxString( wxT("New Mesurement") ) , wxEmptyString, wxITEM_NORMAL );
	m_menu_Mesurement->Append( m_menuMesure_Item_New );
	
	m_menu_Mesurement->AppendSeparator();


    wxMenuItem* m_menuMesure_Item_Open;
	m_menuMesure_Item_Open = new wxMenuItem( m_menu_Mesurement, ID_Main_Mesurement_Open, wxString( wxT("Open Saved Mesurement") ) , wxEmptyString, wxITEM_NORMAL );
	m_menu_Mesurement->Append( m_menuMesure_Item_Open );
	
	m_menu_Mesurement->AppendSeparator();
	
	wxMenuItem* m_menuMesure_Item_Load;
	m_menuMesure_Item_Load = new wxMenuItem( m_menu_Mesurement, ID_Main_Mesurement_Load, wxString( wxT("Load config") ) , wxEmptyString, wxITEM_NORMAL );
	m_menu_Mesurement->Append( m_menuMesure_Item_Load );

    wxMenuItem* m_menuMesure_Item_Preset_1;
	m_menuMesure_Item_Preset_1 = new wxMenuItem( m_menu_Mesurement, ID_Main_Mesurement_Preset_1, wxString( wxT("Preset 1") ) , wxEmptyString, wxITEM_NORMAL );
	m_menu_Mesurement->Append( m_menuMesure_Item_Preset_1 );

    wxMenuItem* m_menuMesure_Item_Preset_2;
	m_menuMesure_Item_Preset_2 = new wxMenuItem( m_menu_Mesurement, ID_Main_Mesurement_Preset_2, wxString( wxT("Preset 2") ) , wxEmptyString, wxITEM_NORMAL );
	m_menu_Mesurement->Append( m_menuMesure_Item_Preset_2 );

    wxMenuItem* m_menuMesure_Item_Preset_3;
	m_menuMesure_Item_Preset_3 = new wxMenuItem( m_menu_Mesurement, ID_Main_Mesurement_Preset_3, wxString( wxT("Preset 3") ) , wxEmptyString, wxITEM_NORMAL );
	m_menu_Mesurement->Append( m_menuMesure_Item_Preset_3 );

    m_menu_Mesurement->AppendSeparator();

    wxMenuItem* m_menuMesure_Item_2DMesurment;
	m_menuMesure_Item_2DMesurment = new wxMenuItem( m_menu_Mesurement, ID_Main_Mesurement_2D_Mess, wxString( wxT("2D Plot Mesurment") ) , wxEmptyString, wxITEM_NORMAL );
	m_menu_Mesurement->Append( m_menuMesure_Item_2DMesurment );
	
	

    m_menu_Mesurement->AppendSeparator();

    wxMenuItem* m_menuMesure_Item_SetMarker;
	m_menuMesure_Item_SetMarker = new wxMenuItem( m_menu_Mesurement, ID_Main_Mesurement_SetMarker, wxString( wxT("Set Marker") ) , wxEmptyString, wxITEM_NORMAL );
	m_menu_Mesurement->Append( m_menuMesure_Item_SetMarker );
	
	m_menu_Mesurement->AppendSeparator();
	
	wxMenuItem* m_menuMesure_Item_Settings;
	m_menuMesure_Item_Settings = new wxMenuItem( m_menu_Mesurement, ID_Main_Mesurement_Settings, wxString( wxT("Settings") ) , wxEmptyString, wxITEM_NORMAL );
	m_menu_Mesurement->Append( m_menuMesure_Item_Settings );
	
    //Mesurement menu
	m_menubarMainProg->Append( m_menu_Mesurement, wxT("Mesurement") ); 
	
    //Processing menu elements
	m_menu_Processing = new wxMenu();

    //Processing menu
	m_menubarMainProg->Append( m_menu_Processing, wxT("Processing") ); 
	
    //Test menu elements
	m_menu_Test = new wxMenu();
	wxMenuItem* m_menuTest_Item_Terminal;
	m_menuTest_Item_Terminal = new wxMenuItem( m_menu_Test, ID_Main_Test_Terminal, wxString( wxT("Terminal") ) + wxT('\t') + wxT("CTRL + SHIFT +T"), wxEmptyString, wxITEM_NORMAL );
	m_menu_Test->Append( m_menuTest_Item_Terminal );
	
	wxMenuItem* m_menuTest_Item_Func;
	m_menuTest_Item_Func = new wxMenuItem( m_menu_Test, ID_Main_Test_Func, wxString( wxT("Function Test") ) + wxT('\t') + wxT("F1"), wxEmptyString, wxITEM_NORMAL );
	m_menu_Test->Append( m_menuTest_Item_Func );
	
    //Test menu 
	m_menubarMainProg->Append( m_menu_Test, wxT("Test") ); 
	
    //Help menu elements
	m_menu_Help = new wxMenu();
	wxMenuItem* m_menuHelp_Item_About;
	m_menuHelp_Item_About = new wxMenuItem( m_menu_Help, ID_Main_Help_About, wxString( wxT("About") ) , wxEmptyString, wxITEM_NORMAL );
	m_menu_Help->Append( m_menuHelp_Item_About );
	//Help menu
	m_menubarMainProg->Append( m_menu_Help, wxT("Help") ); 
	
	this->SetMenuBar( m_menubarMainProg );
	
	wxBoxSizer* bSizerMainProgV1;
	bSizerMainProgV1 = new wxBoxSizer( wxVERTICAL );
	
	m_panel21 = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	bSizerMainProgV1->Add( m_panel21, 1, wxEXPAND | wxALL, 5 );
	
	wxBoxSizer* bSizerMainProgH1;
	bSizerMainProgH1 = new wxBoxSizer( wxHORIZONTAL );
	
	m_staticText1 = new wxStaticText( this, wxID_ANY, wxT("GPIB Adapter: "), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText1->Wrap( -1 );
	bSizerMainProgH1->Add( m_staticText1, 1, wxALL, 5 );
	
	m_textCtrlAdapterStatus = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	bSizerMainProgH1->Add( m_textCtrlAdapterStatus, 2, wxALL, 5 );
	
	
	bSizerMainProgV1->Add( bSizerMainProgH1, 2, wxEXPAND, 5 );
	
	wxBoxSizer* bSizerMainProgH2;
	bSizerMainProgH2 = new wxBoxSizer( wxHORIZONTAL );
	
	bSizerMainProgH2->SetMinSize( wxSize( 0,0 ) ); 
	m_staticText11 = new wxStaticText( this, wxID_ANY, wxT("GPIB Device: "), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText11->Wrap( -1 );
	bSizerMainProgH2->Add( m_staticText11, 1, wxALL, 5 );
	
	m_textCtrlDeviceStatus = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	bSizerMainProgH2->Add( m_textCtrlDeviceStatus, 2, wxALL, 5 );
	
	
	bSizerMainProgV1->Add( bSizerMainProgH2, 2, wxEXPAND, 5 );
	
	m_panel2 = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	bSizerMainProgV1->Add( m_panel2, 2, wxEXPAND | wxALL, 5 );
	
	m_button1 = new wxButton( this, wxID_ANY, wxT("Refresh"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizerMainProgV1->Add( m_button1, 0, wxALIGN_RIGHT|wxALL, 10 );
	m_button1->Bind(wxEVT_BUTTON, &MainProgrammWin::ButtonRefresh, this);
	
	this->SetSizer( bSizerMainProgV1 );
	this->Layout();
	
	this->Centre( wxBOTH );
}
MainProgrammWin::~MainProgrammWin()
{

}

// Button functions
void MainProgrammWin::ButtonRefresh(wxCommandEvent& event)
{
    wxString Text = "";
    m_textCtrlAdapterStatus->SetValue(Text);
    m_textCtrlDeviceStatus->SetValue(Text);

    if (!Adapter.getConnected())
    {
        Adapter.connect();
    }

    if (Adapter.getConnected())
    {
        
        Text = Adapter.send("++ver");
        if (Text.substr(0,6) == "Failed")
        {
            m_textCtrlAdapterStatus->SetValue("Error Check Connection");
        }
        else
        {
            m_textCtrlAdapterStatus->SetValue(Text.substr(14));
        }

        Text = Adapter.send("*IDN?");
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
        filePathRoot,
        "", 
        "CSV Files (*.csv)|*.csv|All Files (*.*)|*.*",
        wxFD_OPEN);

    if (openFileDialog.ShowModal()== wxID_CANCEL)
    {
        return;
    }

    filePathCurrentFile = openFileDialog.GetPath();

    if (OpendData.openCsvFile(filePathCurrentFile))
    {
        fileOpen = true;
    }
    else
    {
        fileOpen = false;
    }

    wxLogDebug(filePathCurrentFile);    
}
void MainProgrammWin::MenuFileSave(wxCommandEvent& event)
{
    if (fileOpen)
    {
        OpendData.saveToCsvFile(filePathCurrentFile);
    }
    else
    {
        MenuFileSaveAs(event);
    }
}
void MainProgrammWin::MenuFileSaveAs(wxCommandEvent& event)
{
    wxFileDialog saveAsFileDialog(nullptr, _("File Save As..."), 
        "",//filePathRoot,
        "", 
        "CSV Files (*.csv)|*.csv|All Files (*.*)|*.*",
        wxFD_SAVE);

    if (saveAsFileDialog.ShowModal()== wxID_CANCEL)
    {
        return;
    }

    filePathCurrentFile = saveAsFileDialog.GetPath();
    if (!OpendData.saveToCsvFile(filePathCurrentFile))
    {
        wxLogDebug("failed to save");
        fileOpen = false;
        return;
    }
    else
    {
        fileOpen = true;
    }

    wxLogDebug(filePathCurrentFile);
    wxLogDebug("saved data");
}

void MainProgrammWin::MenuFileClose(wxCommandEvent& event)
{
    fileOpen = false;
    sData OpendDataTemp;
    OpendData = OpendDataTemp;
    OpendDataTemp.~sData();
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
    sizer->Add(StaticTE,0, wxALL, 20);
    sizer->Add(TerminalDisplay, 0, wxEXPAND | wxALL, 20);
    sizer->Add(StaticTEInput, 0 , wxALL, 20);
    sizer->Add(TerminalInput, 0, wxALL, 20);
    panelTerm->SetSizerAndFit(sizer);

    //debug msg
    wxLogDebug("Terminal Window Opened");

    TerminalInput->Bind(wxEVT_TEXT_ENTER, &TerminalWindow::OnEnterTerminal,this);
    //define Termianl Commands
    TerminalWindow::setupCmds();
}
//-----Terminal window Destructor -----
TerminalWindow::~TerminalWindow()
{
    if (Adapter.getConnected() == true)
    {
        Adapter.disconnect();
    }
    wxLogDebug("Terminal Window Closed");
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
    TerminalDisplay->AppendText(terminalTimestampOutput(Adapter.statusText()));
}

void TerminalWindow::connectDevice(const std::string& args = "")
{
    int dev = 0;

    wxLogDebug("Command entered: connected with args: %s", args);

    if (args != "")
    {
        dev = std::stoi(args);
        if (dev == std::clamp(dev, 1, 20))
        {
            wxLogDebug("Valid dev number: %i", dev);
            dev = dev - 1;
        }
        else
        {
            wxLogDebug("Invalid dev number: %i", dev);
            dev = 0;
        }
    }

    if (!Adapter.getConnected())
    {
        Adapter.connect();

        if (Adapter.getStatus() == FT_OK)
        {
            TerminalDisplay->AppendText(terminalTimestampOutput("Connected to a device\n"));
            wxLogDebug("Connected to %i", dev);
        }
        else
        {
            wxLogDebug("Couldnt connect");
            TerminalDisplay->AppendText(terminalTimestampOutput("Couldnt connect to a device\n Is programm running as SU?\n Is the FTDI_SIO Driver unloaded?\n"));
        }

        FT_STATUS ftStatus = FT_Purge(Adapter.getHandle(), FT_PURGE_RX | FT_PURGE_TX);
        printErr(ftStatus,"Purge Failed");

    }
    else
    {
        TerminalDisplay->AppendText(terminalTimestampOutput("Device already connected\n"));
    }
}

void TerminalWindow::disconnectDevice(const std::string& args = "")
{
    wxLogDebug("Command entered: disconnect with arg: %s", args);

    Adapter.disconnect();

    if (Adapter.getStatus() == FT_OK)
    {
        TerminalDisplay->AppendText(terminalTimestampOutput("Disconnected from a device\n"));
        wxLogDebug("disconnected from current device");
    }
    else
    {
        TerminalDisplay->AppendText(terminalTimestampOutput("Coulnd't disconnect check if a device is connected with: status\n"));
    }

}

wxString TerminalWindow::sendToDevice(const std::string& args)
{
    wxLogDebug("terminal Command send %s Entered",args);

    wxString GPIBText = args;
    std::string CheckText(GPIBText.ToUTF8());

    wxString Text = Adapter.write(CheckText);

    TerminalDisplay->AppendText(terminalTimestampOutput(Text));

    sleepMs(100);   //wait for responce

    wxLogDebug("Reading from device...");

    Text = Adapter.read();

    TerminalDisplay->AppendText(terminalTimestampOutput(Text));

    return Adapter.getLastMsgReseived();
}

wxString TerminalWindow::readFromDevice(const std::string& args = "")
{

    wxLogDebug("command read entered with args: %s", args);

    wxString Text = Adapter.read();

    TerminalWindow::TerminalDisplay->AppendText(terminalTimestampOutput(Text));

    return Adapter.getLastMsgReseived();
}

void TerminalWindow::writeToDevice(const std::string& args)
{
    wxLogDebug("Write Command Entered");

    wxString Text = Adapter.write(args);

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
            Adapter.setBaudrate(BaudRate);
            wxLogDebug("Set Baudrate to %i", Adapter.getBaudrate());

        }
        else
        {
            wxLogDebug("Using Default Baudrate: %i",Adapter.getBaudrate());
        }
    }
    else
    {
        wxLogDebug("Using Default Baudrate: %i",Adapter.getBaudrate());
    }

    Adapter.config();

    if (Adapter.getStatus() == FT_OK)
    {
        Text = "Set Device BaudRate to " + std::to_string(Adapter.getBaudrate()) + "\n";
        TerminalWindow::TerminalDisplay->AppendText(terminalTimestampOutput(Text));
        wxLogDebug("Baudrate set to: %s", std::to_string(Adapter.getBaudrate()));
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

        Messung.seperateDataBlock(Trace);

        wxFile file;

        // wxFile::write_append zum Anhängen oder wxFile::write zum Überschreiben
        if (file.Open("/home/jon3r/Documents/Code/CodeBlocks/Test_GPIB_Terminal/GpibScripts/Z_Log.txt", wxFile::write_append)) {
            file.Write(Trace + "\n");
            file.Close();
        } else {
            wxLogError("Konnte Datei nicht schreiben!");
        }

        wxLogDebug("Received Trace: %s", Trace);

        writeToDevice("INIT:CONT ON"); //Dauerhafter sweep an
    }
}

void TerminalWindow::OnEnterTerminal(wxCommandEvent& event)
{
    wxTextCtrl* Terminal = static_cast<wxTextCtrl*>(event.GetEventObject());
    wxString TText = Terminal->GetValue();
    Terminal->SetValue("");

    wxLogDebug("user entered: %s", TText.c_str());

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

//-----Plot Window BEGIN--------
PlotWindow::PlotWindow(wxWindow *parent) : wxDialog(parent, wxID_ANY, "Plot Window", wxDefaultPosition, wxSize(1000,750))
{
    getFileNames(filePath, fileNames);

    wxButton* executeMesurment = new wxButton(this, wxID_ANY, "Execute Mesurement");
    executeMesurment->Bind(wxEVT_BUTTON, &PlotWindow::executeScriptEvent,this);
    selectMesurement = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, fileNames);
    selectMesurement->SetSelection(0);
    // 1. mpWindow (Zeichenfläche) erstellen
    plot = new mpWindow(this, wxID_ANY);

    // Farbeinstellungen (Optional)
    plot->SetMargins(30, 30, 50, 50);

    // 2. Achsen als Layer hinzufügen
    // mpScaleX(Name, Ausrichtung, Ticks anzeigen, Typ)
    mpScaleX* xAxis = new mpScaleX("X-Achse", mpALIGN_BORDER_BOTTOM, true, mpX_NORMAL);
    mpScaleY* yAxis = new mpScaleY("Y-Achse", mpALIGN_BORDER_LEFT, true);
    plot->AddLayer(xAxis);
    plot->AddLayer(yAxis);

    // 3. Daten vorbereiten (std::vector laut Header Definition von mpFXYVector)
    x = {0.0, 1.0, 2.0, 3.0, 4.0, 5.0 };
    y = {0.0, 1.0, 4.0, 2.0, 5.0, 3.0 };

    // 4. Vektor-Layer erstellen
    vectorLayer = new mpFXYVector("Messdaten");
    vectorLayer->SetData(x, y);
    vectorLayer->SetContinuity(true); // True = Linie zeichnen
    vectorLayer->SetPen(wxPen(*wxBLUE, 2, wxPENSTYLE_SOLID));
    vectorLayer->ShowName(true);      // Wichtig für die Legende

    plot->AddLayer(vectorLayer);

    // 5. Legende hinzufügen (mpInfoLegend ist ein Layer)
    // wxRect definiert Startposition und ungefähre Größe
    mpInfoLegend* legend = new mpInfoLegend(wxRect(20, 20, 10, 10), wxTRANSPARENT_BRUSH);
    legend->SetItemMode(mpLEGEND_LINE); // Zeigt Linie statt Quadrat in der Legende
    plot->AddLayer(legend);

    // 6. Layout-Management
    wxBoxSizer* sizerButtons = new wxBoxSizer(wxHORIZONTAL);
    sizerButtons->Add(executeMesurment, 0, wxEXPAND | wxALL);
    sizerButtons->Add(selectMesurement, 0, wxEXPAND | wxALL);

    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    sizer->Add(plot, 1, wxEXPAND | wxALL, 5);
    sizer->Add(sizerButtons, 0, wxEXPAND| wxALL , 5);
    this->SetSizer(sizer);
    this->Layout();
    // 7. Zoom auf Daten anpassen
    plot->Fit();

}
PlotWindow::~PlotWindow()
{
    Adapter.disconnect();
}

void PlotWindow::getFileNames(const wxString& dirPath, wxArrayString& files)
{
    wxDir dir(dirPath);

    if (!dir.IsOpened())
    {
        return;
    }

    wxString filename;
    bool cont = dir.GetFirst(&filename, "*.txt", wxDIR_FILES);

    while (cont) {

        files.Add(filename);
        cont = dir.GetNext(&filename);
    }
}
void PlotWindow::executeScriptEvent(wxCommandEvent& event)
{
    wxArrayString logAdapterReceived;
    wxString fileName = selectMesurement->GetStringSelection();

    wxLogDebug("Reading Scriptfile...");
    Adapter.readScriptFile(filePath, fileName, logAdapterReceived);

    //output received msg
    for (size_t i = 0; i < logAdapterReceived.GetCount(); i++)
    {
        wxLogDebug(logAdapterReceived[i]);
    }
    y = Messung.getX_Data(); //zum test vertauscht
    x = Messung.getY_Data();
    //test
    updatePlotData();
}
void PlotWindow::updatePlotData()
{
    vectorLayer->SetData(x, y);
    plot->Fit();
}

//-----Plot Window ENDE--------

//-----Plot Window Set Marker-------
PlotWindowSetMarker::PlotWindowSetMarker( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
    wxArrayString freqEinheiten;
    freqEinheiten.Add("Hz");
    freqEinheiten.Add("kHz");
    freqEinheiten.Add("MHz");
    freqEinheiten.Add("GHz");

	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	
	wxBoxSizer* bSizer1;
	bSizer1 = new wxBoxSizer( wxVERTICAL );
	
	wxStaticBoxSizer* sbSizer1;
	sbSizer1 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Marker 1") ), wxVERTICAL );
	
	wxBoxSizer* bSizer2;
	bSizer2 = new wxBoxSizer( wxHORIZONTAL );
	
	m_checkBox1 = new wxCheckBox( sbSizer1->GetStaticBox(), wxID_ANY, wxT("Set to Frequency"), wxDefaultPosition, wxDefaultSize, 0 );
	m_checkBox1->Bind(wxEVT_CHECKBOX, &PlotWindowSetMarker::toggleSelection1, this);
    bSizer2->Add( m_checkBox1, 1, wxALL, 5 );
	
	m_textCtrl1 = new wxTextCtrl( sbSizer1->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	bSizer2->Add( m_textCtrl1, 1, wxALL, 5 );
	
	m_choice1 = new wxChoice( sbSizer1->GetStaticBox(), wxID_ANY, wxDefaultPosition, wxDefaultSize, freqEinheiten, 0 );
	m_choice1->SetSelection( 0 );
	bSizer2->Add( m_choice1, 1, wxALL, 5 );
	
	
	sbSizer1->Add( bSizer2, 1, wxEXPAND, 5 );
	
	wxBoxSizer* bSizer6;
	bSizer6 = new wxBoxSizer( wxVERTICAL );
	
	m_checkBox2 = new wxCheckBox( sbSizer1->GetStaticBox(), wxID_ANY, wxT("Set to highest freq"), wxDefaultPosition, wxDefaultSize, 0 );
	m_checkBox2->Bind(wxEVT_CHECKBOX, &PlotWindowSetMarker::toggleSelection1, this);
    bSizer6->Add( m_checkBox2, 0, wxALL, 5 );
	
	
	sbSizer1->Add( bSizer6, 1, wxEXPAND, 5 );
	
	m_button1 = new wxButton( sbSizer1->GetStaticBox(), wxID_ANY, wxT("Set"), wxDefaultPosition, wxDefaultSize, 0 );
	m_button1->Bind(wxEVT_BUTTON, &PlotWindowSetMarker::SetSelection1, this);
    sbSizer1->Add( m_button1, 1, wxALL, 5 );
	
	
	bSizer1->Add( sbSizer1, 1, wxEXPAND, 5 );
	
	wxStaticBoxSizer* sbSizer11;
	sbSizer11 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Marker 2") ), wxVERTICAL );
	
	wxBoxSizer* bSizer21;
	bSizer21 = new wxBoxSizer( wxHORIZONTAL );
	
	m_checkBox3 = new wxCheckBox( sbSizer11->GetStaticBox(), wxID_ANY, wxT("Set to Frequency"), wxDefaultPosition, wxDefaultSize, 0 );
	m_checkBox3->Bind(wxEVT_CHECKBOX, &PlotWindowSetMarker::toggleSelection2, this);
    bSizer21->Add( m_checkBox3, 1, wxALL, 5 );
	
	m_textCtrl2 = new wxTextCtrl( sbSizer11->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	bSizer21->Add( m_textCtrl2, 1, wxALL, 5 );
	
	
	m_choice2 = new wxChoice( sbSizer11->GetStaticBox(), wxID_ANY, wxDefaultPosition, wxDefaultSize, freqEinheiten, 0 );
	m_choice2->SetSelection( 0 );
	bSizer21->Add( m_choice2, 1, wxALL, 5 );
	
	
	sbSizer11->Add( bSizer21, 1, wxEXPAND, 5 );
	
	wxBoxSizer* bSizer5;
	bSizer5 = new wxBoxSizer( wxVERTICAL );
	
	m_checkBox4 = new wxCheckBox( sbSizer11->GetStaticBox(), wxID_ANY, wxT("Set to highest Freq"), wxDefaultPosition, wxDefaultSize, 0 );
	m_checkBox4->Bind(wxEVT_CHECKBOX, &PlotWindowSetMarker::toggleSelection2, this);
    bSizer5->Add( m_checkBox4, 1, wxALL, 5 );
	
	
	sbSizer11->Add( bSizer5, 1, wxEXPAND, 5 );
	
	m_button2 = new wxButton( sbSizer11->GetStaticBox(), wxID_ANY, wxT("Set"), wxDefaultPosition, wxDefaultSize, 0 );
	m_button2->Bind(wxEVT_BUTTON, &PlotWindowSetMarker::SetSelection2, this);
    sbSizer11->Add( m_button2, 1, wxALL, 5 );
	
	
	bSizer1->Add( sbSizer11, 1, wxEXPAND, 5 );
	
	
	this->SetSizer( bSizer1 );
	this->Layout();
	
	this->Centre( wxBOTH );

    GetValues();
    toggleSelection1fkt();
    toggleSelection2fkt();

}
PlotWindowSetMarker::~PlotWindowSetMarker()
{
}

void PlotWindowSetMarker::GetValues()
{
    Marker1FreqSet      = m_checkBox1->GetValue();
    Marker1MaxSet       = m_checkBox2->GetValue();

    Marker2FreqSet      = m_checkBox3->GetValue();
    Marker2MaxSet       = m_checkBox4->GetValue();

    Marker1Freq         = m_textCtrl1->GetValue();
    Marker1Unit         = m_choice1->GetStringSelection();
    Marker2Freq         = m_textCtrl2->GetValue();
    Marker2Unit         = m_choice2->GetStringSelection();

}

void PlotWindowSetMarker::toggleSelection1fkt()
{
    if (!Marker1FreqSet && !Marker1MaxSet)
    {
        m_checkBox1->SetValue(true);
        m_checkBox2->SetValue(false);
        m_textCtrl1->Enable(true);
        m_choice1->Enable(true);
    }
    else if (!Marker1FreqSet && Marker1MaxSet)
    {
        m_checkBox1->SetValue(true);
        m_checkBox2->SetValue(false);
        m_textCtrl1->Enable(true);
        m_choice1->Enable(true);
    }
    else if (Marker1FreqSet && !Marker1MaxSet)
    {
        m_checkBox1->SetValue(false);
        m_checkBox2->SetValue(true);
        m_textCtrl1->Enable(false);
        m_choice1->Enable(false);
    }

    GetValues();
}
void PlotWindowSetMarker::toggleSelection1(wxCommandEvent& event)
{
    toggleSelection1fkt();
}

void PlotWindowSetMarker::toggleSelection2fkt()
{
    if (!Marker2FreqSet && !Marker2MaxSet)
    {
        m_checkBox3->SetValue(true);
        m_checkBox4->SetValue(false);
        m_textCtrl2->Enable(true);
        m_choice2->Enable(true);
    }
    else if (!Marker2FreqSet && Marker2MaxSet)
    {
        m_checkBox3->SetValue(true);
        m_checkBox4->SetValue(false);
        m_textCtrl2->Enable(true);
        m_choice2->Enable(true);
    }
    else if (Marker2FreqSet && !Marker2MaxSet)
    {
        m_checkBox3->SetValue(false);
        m_checkBox4->SetValue(true);
        m_textCtrl2->Enable(false);
        m_choice2->Enable(false);
    }

    GetValues();
}
void PlotWindowSetMarker::toggleSelection2(wxCommandEvent& event)
{
    toggleSelection2fkt();
}
void PlotWindowSetMarker::GetSelectedValue1()
{
    const double multipliers[] = { 
    1.0,        // Index 0: Hz
    1000.0,     // Index 1: kHz
    1.0e6,      // Index 2: MHz
    1.0e9       // Index 3: GHz
    };

    int selection = m_choice1->GetSelection();
    double val;

    if (!Marker1Freq.ToDouble(&val))
    {
        wxLogDebug("Failed to convert input");
        return;
    }

    if (selection != wxNOT_FOUND) {
        double factor = multipliers[selection];
        double frequencyHz =  val * factor;
        FreqMarker1Raw = wxString::FromCDouble(frequencyHz);
    }
}
void PlotWindowSetMarker::GetSelectedValue2()
{
    GetValues();

    const double multipliers[] = { 
    1.0,        // Index 0: Hz
    1000.0,     // Index 1: kHz
    1.0e6,      // Index 2: MHz
    1.0e9       // Index 3: GHz
    };

    int selection = m_choice2->GetSelection();
    double val;

    if (!Marker1Freq.ToDouble(&val))
    {
        wxLogDebug("Failed to convert input");
        return;
    }

    if (selection != wxNOT_FOUND) {
        double factor = multipliers[selection];
        double frequencyHz =  val * factor;
        FreqMarker2Raw = wxString::FromCDouble(frequencyHz);
    }
}
void PlotWindowSetMarker::SetSelection1(wxCommandEvent& event)
{
    GetValues();    

    if (Marker1MaxSet)
    {
        std::string Text = "CALC:MARK:MAX";
        Adapter.write(Text);
    }
    else if (Marker1FreqSet && (!FreqMarker1Raw.IsNumber()));
    {
        GetSelectedValue1();

        std::string Text = "CALC:MARK:MAX " + FreqMarker1Raw;
        Adapter.write(Text);
    }
    //TODO Get X Y From Device and display in the Menu
}
void PlotWindowSetMarker::SetSelection2(wxCommandEvent& event)
{
    GetValues();    

    if (Marker2MaxSet)
    {
        std::string Text = "CALC:MARK2:MAX";
        Adapter.write(Text);
    }
    else if (Marker2FreqSet && (!FreqMarker2Raw.IsNumber()))
    {
        GetSelectedValue1();

        std::string Text = "CALC:MARK2:MAX " + FreqMarker1Raw;
        Adapter.write(Text);
    }
    //TODO Get X Y From Device and display in the Menu
}

Mesurement2D::Mesurement2D( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	
	wxBoxSizer* bSizer1;
	bSizer1 = new wxBoxSizer( wxVERTICAL );
	
	wxStaticBoxSizer* sbSizer1;
	sbSizer1 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Mesurment Settings") ), wxVERTICAL );
	
	m_staticText1 = new wxStaticText( sbSizer1->GetStaticBox(), wxID_ANY, wxT("Mesurement Points:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText1->Wrap( -1 );
	sbSizer1->Add( m_staticText1, 0, wxALL, 5 );
	
	wxBoxSizer* bSizer2;
	bSizer2 = new wxBoxSizer( wxHORIZONTAL );
	
	m_staticText2 = new wxStaticText( sbSizer1->GetStaticBox(), wxID_ANY, wxT("Y Points:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText2->Wrap( -1 );
	bSizer2->Add( m_staticText2, 1, wxALL, 5 );
	
	m_slider1 = new wxSlider( sbSizer1->GetStaticBox(), wxID_ANY, 10, 1, 100, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL );
	bSizer2->Add( m_slider1, 1, wxALL, 5 );
	
	
	sbSizer1->Add( bSizer2, 1, wxEXPAND, 5 );
	
	wxBoxSizer* bSizer21;
	bSizer21 = new wxBoxSizer( wxHORIZONTAL );
	
	m_staticText3 = new wxStaticText( sbSizer1->GetStaticBox(), wxID_ANY, wxT("X Points:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText3->Wrap( -1 );
	bSizer21->Add( m_staticText3, 1, wxALL, 5 );
	
	m_slider2 = new wxSlider( sbSizer1->GetStaticBox(), wxID_ANY, 10, 1, 100, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL );
	bSizer21->Add( m_slider2, 1, wxALL, 5 );
	
	
	sbSizer1->Add( bSizer21, 1, wxEXPAND, 5 );
	
	wxBoxSizer* bSizer211;
	bSizer211 = new wxBoxSizer( wxHORIZONTAL );
	
	m_staticText4 = new wxStaticText( sbSizer1->GetStaticBox(), wxID_ANY, wxT("Scale:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText4->Wrap( -1 );
	bSizer211->Add( m_staticText4, 1, wxALL, 5 );
	
	m_slider3 = new wxSlider( sbSizer1->GetStaticBox(), wxID_ANY, 100, 1, 100, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL );
	bSizer211->Add( m_slider3, 1, wxALL, 5 );
	
	
	sbSizer1->Add( bSizer211, 1, wxEXPAND, 5 );
	
	wxBoxSizer* bSizer9;
	bSizer9 = new wxBoxSizer( wxHORIZONTAL );
	
	m_staticText5 = new wxStaticText( sbSizer1->GetStaticBox(), wxID_ANY, wxT("Orientation point:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText5->Wrap( -1 );
	bSizer9->Add( m_staticText5, 1, wxALL, 5 );
	
	wxArrayString StartPosition;
    StartPosition.Add("Center");
    StartPosition.Add("Top Left");
    StartPosition.Add("Top Right");
    StartPosition.Add("Bottom Right");
    StartPosition.Add("Bottom Left");

	m_choice1 = new wxChoice( sbSizer1->GetStaticBox(), wxID_ANY, wxDefaultPosition, wxDefaultSize, StartPosition, 0 );
	m_choice1->SetSelection( 1 );
	bSizer9->Add( m_choice1, 1, wxALL, 5 );
	
	
	sbSizer1->Add( bSizer9, 1, wxEXPAND, 5 );
	
	wxBoxSizer* bSizer10;
	bSizer10 = new wxBoxSizer( wxHORIZONTAL );
	
	m_button2 = new wxButton( sbSizer1->GetStaticBox(), wxID_ANY, wxT("Reset"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer10->Add( m_button2, 1, wxALL, 5 );
	
	m_button1 = new wxButton( sbSizer1->GetStaticBox(), wxID_ANY, wxT("Open Device settings"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer10->Add( m_button1, 1, wxALIGN_RIGHT|wxALL, 5 );
	
	
	sbSizer1->Add( bSizer10, 1, wxEXPAND, 5 );
	
	
	bSizer1->Add( sbSizer1, 1, wxEXPAND, 5 );
	
	wxStaticBoxSizer* sbSizer2;
	sbSizer2 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Mesurment") ), wxVERTICAL );
	
	wxBoxSizer* bSizer14;
	bSizer14 = new wxBoxSizer( wxHORIZONTAL );
	
	m_staticText6 = new wxStaticText( sbSizer2->GetStaticBox(), wxID_ANY, wxT("Progress:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText6->Wrap( -1 );
	bSizer14->Add( m_staticText6, 1, wxALL, 5 );
	
	m_staticText7 = new wxStaticText( sbSizer2->GetStaticBox(), wxID_ANY, wxT("0/100"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText7->Wrap( -1 );
	bSizer14->Add( m_staticText7, 1, wxALL, 5 );
	
	m_gauge1 = new wxGauge( sbSizer2->GetStaticBox(), wxID_ANY, 100, wxDefaultPosition, wxDefaultSize, wxGA_HORIZONTAL );
	m_gauge1->SetValue( 0 ); 
	bSizer14->Add( m_gauge1, 5, wxALL, 5 );
	
	
	sbSizer2->Add( bSizer14, 1, wxEXPAND, 5 );
	
	wxBoxSizer* bSizer15;
	bSizer15 = new wxBoxSizer( wxHORIZONTAL );
	
	m_button3 = new wxButton( sbSizer2->GetStaticBox(), wxID_ANY, wxT("Start"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer15->Add( m_button3, 1, wxALL, 5 );
	
	m_button4 = new wxButton( sbSizer2->GetStaticBox(), wxID_ANY, wxT("Stop"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer15->Add( m_button4, 1, wxALL, 5 );
	
	m_button5 = new wxButton( sbSizer2->GetStaticBox(), wxID_ANY, wxT("Restart"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer15->Add( m_button5, 1, wxALL, 5 );
	
	
	sbSizer2->Add( bSizer15, 1, wxEXPAND, 5 );
	
	
	bSizer1->Add( sbSizer2, 1, wxEXPAND, 5 );
	
	
	this->SetSizer( bSizer1 );
	this->Layout();
	
	this->Centre( wxBOTH );
}
void Mesurement2D::GetValues()
{

}
Mesurement2D::~Mesurement2D()
{
}


//-----Plot Window Set Marker ENDE-------


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
    wxButton* writeGpibButton = new wxButton(panelfunc, wxID_ANY, "Write to GPIB",wxPoint(10,0));
    writeGpibButton->Bind(wxEVT_BUTTON, &FunctionWindow::OnWriteGpib,this);

    //Create Button "Read to GPIB"
    wxButton* readGpibButton = new wxButton(panelfunc, wxID_ANY, "Read from GPIB",wxPoint(10,0));
    readGpibButton->Bind(wxEVT_BUTTON, &FunctionWindow::OnReadGpib,this);

    //Create Button "Write and Read GPIB"
    wxButton* readWriteGpibButton = new wxButton(panelfunc, wxID_ANY, "Write and Read GPIB",wxPoint(10,0));
    readWriteGpibButton->Bind(wxEVT_BUTTON, &FunctionWindow::OnReadWriteGpib,this);

    //Create Button "Scan For Device"
    wxButton* scanUsbButton = new wxButton(panelfunc, wxID_ANY, "Scan For Device",wxPoint(10,0));
    scanUsbButton->Bind(wxEVT_BUTTON, &FunctionWindow::OnUsbScan,this);

    //Create Button "Configure USB Device"
    wxButton* devConfigButton = new wxButton(panelfunc, wxID_ANY, "Configure USB Device",wxPoint(10,0));
    devConfigButton->Bind(wxEVT_BUTTON, &FunctionWindow::OnUsbConfig,this);

    //Create Button "Connect / Disconnect"
    wxButton* connectDevGpibButton = new wxButton(panelfunc, wxID_ANY, "Connected / Disconnect",wxPoint(10,0));
    connectDevGpibButton->Bind(wxEVT_BUTTON, &FunctionWindow::OnConDisconGpib,this);

    //Create Button "Test Save File"
    wxButton* TestSaveFileButton = new wxButton(panelfunc, wxID_ANY, "Test Save File",wxPoint(10,0));
    TestSaveFileButton->Bind(wxEVT_BUTTON, &FunctionWindow::OnTestSaveFile,this);


    //Funtion Output Lable
    wxStaticText* discFuncOutput = new wxStaticText(panelfunc,wxID_ANY,"Function output: ");
    //Funtion Output Text Box
    textFuncOutput = new wxTextCtrl(panelfunc, wxID_ANY,"",wxDefaultPosition,wxSize(300, 200), wxTE_MULTILINE);

    //sizer     Set Window Layout
    wxBoxSizer* sizerFunc = new wxBoxSizer(wxVERTICAL);
    sizerFunc->Add(discFuncInput, 0, wxEXPAND | wxALL , 10);
    sizerFunc->Add(writeFuncInput, 0, wxEXPAND | wxALL , 10);
    sizerFunc->Add(scanUsbButton, 0, wxEXPAND | wxALL , 10);
    sizerFunc->Add(connectDevGpibButton, 0, wxEXPAND | wxALL , 10);
    sizerFunc->Add(devConfigButton, 0, wxEXPAND | wxALL , 10);
    sizerFunc->Add(writeGpibButton, 0, wxEXPAND | wxALL , 10);
    sizerFunc->Add(readGpibButton, 0, wxEXPAND | wxALL , 10);
    sizerFunc->Add(readWriteGpibButton, 0, wxEXPAND | wxALL , 10);
    sizerFunc->Add(TestSaveFileButton, 0, wxEXPAND | wxALL , 10);
    sizerFunc->Add(discFuncOutput, 0, wxEXPAND | wxALL , 10);
    sizerFunc->Add(textFuncOutput, 0, wxEXPAND | wxALL , 10);
    panelfunc->SetSizerAndFit(sizerFunc);


}
//-----Function Window Destructor-----
FunctionWindow::~FunctionWindow()
{
    Adapter.disconnect();
}
//-----Function Window Methodes-----
void FunctionWindow::OnUsbScan(wxCommandEvent& event)
{
    wxLogDebug("Scan USB Devices");

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
    if (Adapter.getConnected() == false)
    {
       Adapter.connect();

        if (Adapter.getStatus() == FT_OK)
        {
            textFuncOutput->AppendText(terminalTimestampOutput("Connected to a device\n"));
        }
    }
    else
    {
        Adapter.disconnect();

        if (Adapter.getStatus() == FT_OK)
        {
            textFuncOutput->AppendText(terminalTimestampOutput("Disconnected from a device\n"));
        }
    }
}
void FunctionWindow::OnTestSaveFile(wxCommandEvent& event)
{
    wxLogDebug("Pressed Test Save File");
    sData TestObjekt;

    TestObjekt.setTimeAndDate();
    sData::sParam* TestData = new sData::sParam;
    TestData = TestObjekt.GetParameter();

    wxLogDebug("Zeit: %s",TestData->Time);

    wxLogDebug("Schreib daten in CSV");

    wxString Dateiname = "D:\\CodeProjects\\VSCode\\projects\\Diplom\\Test-GPIB-Terminal\\LogFiles\\TestCSV";
    TestObjekt.saveToCsvFile(Dateiname);
}
void FunctionWindow::OnWriteGpib(wxCommandEvent& event)
{
    wxLogDebug("Write Pressed!");

    wxString GPIBText = FunctionWindow::writeFuncInput->GetValue();
    FunctionWindow::writeFuncInput->SetValue("");

    std::string CheckText(GPIBText.ToUTF8());

    wxString Text = Adapter.write(CheckText);

    FunctionWindow::textFuncOutput->AppendText(terminalTimestampOutput(Text));
}
void FunctionWindow::OnReadGpib(wxCommandEvent& event)
{
    wxLogDebug("On Read Pressed");

    wxString Text = Adapter.read();

    FunctionWindow::textFuncOutput->AppendText(terminalTimestampOutput(Text));
}
void FunctionWindow::OnReadWriteGpib(wxCommandEvent& event)
{
    wxLogDebug("Read / Write Pressed!");

    wxLogDebug("Writing to device...");

    wxString GPIBText = FunctionWindow::writeFuncInput->GetValue();
    std::string CheckText(GPIBText.ToUTF8());

    FunctionWindow::writeFuncInput->SetValue("");

    wxString Text = Adapter.write(CheckText);

    FunctionWindow::textFuncOutput->AppendText(terminalTimestampOutput(Text));

    sleepMs(100);   //wait for responce

    wxLogDebug("Reading from device...");

    Text = Adapter.read();

    FunctionWindow::textFuncOutput->AppendText(terminalTimestampOutput(Text));
}
void FunctionWindow::OnUsbConfig(wxCommandEvent& event)
{
    Adapter.config();

    if (Adapter.getStatus() == FT_OK)
    {
        FunctionWindow::textFuncOutput->AppendText(terminalTimestampOutput("Set Default config\n"));
    }
    else
    {
        FunctionWindow::textFuncOutput->AppendText(terminalTimestampOutput("Config failed\n"));
    }

    FunctionWindow::textFuncOutput->AppendText(terminalTimestampOutput(Adapter.statusText()));
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



    // 5. Layout-Management (Sizer) verwenden, damit das Notebook
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

    wxTextValidator val(wxFILTER_NUMERIC);

    wxStaticText* labelText = new wxStaticText(this, wxID_ANY, label, wxPoint(10,10));

    //Start-Ende Elemente-----Start
    startEndeCheck = new wxCheckBox(this, wxID_ANY, "Start - Ende Nutzen");
    startEndeCheck->Bind(wxEVT_CHECKBOX, &SettingsTabDisplay::toggleSelectionEvent, this);
    wxStaticText* descriptionText_1 = new wxStaticText(this, wxID_ANY, "Start-Frequenz:", wxPoint(10,10));
    inputText_1 = new wxTextCtrl(this, wxID_ANY, "75");
    freqEinheitAuswahl_1 = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, freqEinheiten);
    inputText_1->SetValidator(val);
    freqEinheitAuswahl_1->SetSelection(2);

    wxStaticText* descriptionText_2 = new wxStaticText(this, wxID_ANY, "End-Frequenz:", wxPoint(10,10));
    inputText_2 = new wxTextCtrl(this, wxID_ANY,"125");
    freqEinheitAuswahl_2 = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, freqEinheiten);
    inputText_2->SetValidator(val);
    freqEinheitAuswahl_2->SetSelection(2);
    //Start-Ende Elemente-----Ende

    //Center-Spanne Elemente-----Start
    centerSpanCheck = new wxCheckBox(this, wxID_ANY, "Center - Spanne Nutzen");
    centerSpanCheck->Bind(wxEVT_CHECKBOX, &SettingsTabDisplay::toggleSelectionEvent, this);
    wxStaticText* descriptionText_3 = new wxStaticText(this, wxID_ANY, "Center-Frequenz:", wxPoint(10,10));
    inputText_3 = new wxTextCtrl(this, wxID_ANY,"100");
    freqEinheitAuswahl_3 = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, freqEinheiten);
    inputText_3->SetValidator(val);
    freqEinheitAuswahl_3->SetSelection(2);

    wxStaticText* descriptionText_4 = new wxStaticText(this, wxID_ANY, "Span-Frequenz:", wxPoint(10,10));
    inputText_4 = new wxTextCtrl(this, wxID_ANY, "50");
    freqEinheitAuswahl_4 = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, freqEinheiten);
    inputText_4->SetValidator(val);
    freqEinheitAuswahl_4->SetSelection(2);
    //Center-Spanne Elemente-----Ende

    //Y-Achsen Elemente-----Start
    wxStaticText* descriptionText_5 = new wxStaticText(this, wxID_ANY, "Y-Scaling:", wxPoint(10,10));
    inputText_5 = new wxTextCtrl(this, wxID_ANY,"100");
    pegelEinheitAuswahl = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, pegelEinheiten);
    inputText_5->SetValidator(val);
    pegelEinheitAuswahl->SetSelection(0);

    wxStaticText* descriptionText_6 = new wxStaticText(this, wxID_ANY, "Y-Scale Spacing:", wxPoint(10,10));
    yScalingAuswahl = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, scalingY);
    yScalingAuswahl->SetSelection(0);

    wxStaticText* descriptionText_7 = new wxStaticText(this, wxID_ANY, "Referenzpegel in dB:", wxPoint(10,10));
    inputText_7 = new wxTextCtrl(this, wxID_ANY, "-20");
    inputText_7->SetValidator(val);
    //Y-Achsen Elemente-----Ende

    //Knöpfe START
    wxButton* anwendenButton = new wxButton(this, wxID_ANY, "Anwenden");
    anwendenButton->Bind(wxEVT_BUTTON, &SettingsTabDisplay::anwendenButton,this);

    wxButton* getCurrentButton = new wxButton(this, wxID_ANY, wxString::FromUTF8("Messgerät Einstellungen Laden"));
    getCurrentButton->Bind(wxEVT_BUTTON, &SettingsTabDisplay::getCurrentButton, this);
    //knöpfe ENDE


    //Start-Ende Sizer-----START
    wxBoxSizer* sizerHorizontal_1 = new wxBoxSizer(wxHORIZONTAL);
    sizerHorizontal_1->Add(descriptionText_1, 1, wxALL , 5);
    sizerHorizontal_1->Add(inputText_1, 1, wxALL , 5);
    sizerHorizontal_1->Add(freqEinheitAuswahl_1, 1, wxALL, 5);

    wxBoxSizer* sizerHorizontal_2 = new wxBoxSizer(wxHORIZONTAL);
    sizerHorizontal_2->Add(descriptionText_2, 1, wxALL , 5);
    sizerHorizontal_2->Add(inputText_2, 1, wxALL , 5);
    sizerHorizontal_2->Add(freqEinheitAuswahl_2, 1, wxALL, 5);

    //Rahmen um den abschnitt
    wxStaticBoxSizer* staticSizer_1 = new wxStaticBoxSizer(wxVERTICAL, this, "Anzeigebereich Start-Ende Frequenz");
    staticSizer_1->Add(startEndeCheck,0,wxALL | wxEXPAND,5);
    staticSizer_1->Add(sizerHorizontal_1,0, wxALL | wxEXPAND, 5);
    staticSizer_1->Add(sizerHorizontal_2,0, wxALL | wxEXPAND, 5);
    //Start-Ende Sizer-----ENDE

    //Center-Spann Sizer-----START
    wxBoxSizer* sizerHorizontal_3 = new wxBoxSizer(wxHORIZONTAL);
    sizerHorizontal_3->Add(descriptionText_3, 1, wxALL , 5);
    sizerHorizontal_3->Add(inputText_3, 1, wxALL , 5);
    sizerHorizontal_3->Add(freqEinheitAuswahl_3, 1, wxALL, 5);

    wxBoxSizer* sizerHorizontal_4 = new wxBoxSizer(wxHORIZONTAL);
    sizerHorizontal_4->Add(descriptionText_4, 1, wxALL , 5);
    sizerHorizontal_4->Add(inputText_4, 1, wxALL , 5);
    sizerHorizontal_4->Add(freqEinheitAuswahl_4, 1, wxALL, 5);

    //Rahmen um den abschnitt
    wxStaticBoxSizer* staticSizer_2 = new wxStaticBoxSizer(wxVERTICAL, this, "Anzeigebereich Center-Span Frequenz");
    staticSizer_2->Add(centerSpanCheck,0,wxALL | wxEXPAND,5);
    staticSizer_2->Add(sizerHorizontal_3,0, wxALL | wxEXPAND, 5);
    staticSizer_2->Add(sizerHorizontal_4,0, wxALL | wxEXPAND, 5);
    //Center-Spann Sizer-----ENDE

    //y-achsen einstellung Sizer-----START
    wxBoxSizer* sizerHorizontal_5 = new wxBoxSizer(wxHORIZONTAL);
    sizerHorizontal_5->Add(descriptionText_5, 1, wxALL | wxEXPAND , 5);
    sizerHorizontal_5->Add(inputText_5, 1, wxALL | wxEXPAND , 5);
    sizerHorizontal_5->Add(pegelEinheitAuswahl, 1, wxALL | wxEXPAND, 5);

    wxBoxSizer* sizerHorizontal_6 = new wxBoxSizer(wxHORIZONTAL);
    sizerHorizontal_6->Add(descriptionText_6,2,  wxALL | wxEXPAND,5);
    sizerHorizontal_6->Add(yScalingAuswahl,1,  wxALL | wxEXPAND,5);

    wxBoxSizer* sizerHorizontal_7 = new wxBoxSizer(wxHORIZONTAL);
    sizerHorizontal_7->Add(descriptionText_7,2 ,   wxALL | wxEXPAND,5);
    sizerHorizontal_7->Add(inputText_7,1 ,  wxALL | wxEXPAND,5);

    //Rahmen um den abschnitt
    wxStaticBoxSizer* staticSizer_3 = new wxStaticBoxSizer(wxVERTICAL, this, "Anzeigebereich Pegel-Skalierung");
    staticSizer_3->Add(sizerHorizontal_6,0, wxALL | wxEXPAND, 5);
    staticSizer_3->Add(sizerHorizontal_5,0, wxALL | wxEXPAND, 5);
    staticSizer_3->Add(sizerHorizontal_7,0, wxALL | wxEXPAND, 5);

    //y-achsen einstellung Sizer-----ENDE

    //Knöpfe
    wxBoxSizer* sizerHorizontal_8 = new wxBoxSizer(wxHORIZONTAL);
    sizerHorizontal_8->Add(getCurrentButton,1, wxALL | wxEXPAND, 5);
    sizerHorizontal_8->Add(anwendenButton,1, wxALL | wxEXPAND, 5);

    //knöpfe ENDE

    //Verticaler Sizer
    wxBoxSizer* sizerVertical = new wxBoxSizer(wxVERTICAL);
    sizerVertical->Add(labelText ,0 ,  wxALL | wxEXPAND, 5);
    sizerVertical->Add(staticSizer_1 ,0 , wxALL | wxEXPAND, 5);
    sizerVertical->Add(staticSizer_2 ,0 , wxALL | wxEXPAND, 5);
    sizerVertical->Add(staticSizer_3 ,0 , wxALL | wxEXPAND, 5);
    sizerVertical->Add(sizerHorizontal_8 ,0 , wxALL | wxEXPAND, 5);

    this->SetSizerAndFit(sizerVertical);

    toggleSelection(); //anfangszustand herstellen

    Adapter.connect();
    Adapter.config();
}
void SettingsTabDisplay::anwendenButton(wxCommandEvent& event)
{
    std::string cmdText;
    getValues();
    if (Adapter.getConnected())
    {
        if (!useCenterSpan)
        {
            cmdText = "FREQ:STAR " + FreqStartSet + FreqStartSetUnit;
            Adapter.write(cmdText);
            cmdText = "FREQ:STOP " + FreqEndeSet + FreqEndeSetUnit;
            Adapter.write(cmdText);
        }
        else if (!useStartEnde)
        {
            cmdText = "FREQ:CENT " + FreqCenterSet + FreqCenterSetUnit;
            Adapter.write(cmdText);
            cmdText = "FREQ:SPAN " + FreqSpanSet + FreqSpanSetUnit;
            Adapter.write(cmdText);
        }

        cmdText = "UNIT:POW " + pegelSet + pegelSetUnit;
        Adapter.write(cmdText);
        cmdText = "DISP:TRAC:Y:RLEV " + refPegelSet;
        Adapter.write(cmdText);
        cmdText = "DISP:TRAC:Y:SPAC " + refPegelSet;
        Adapter.write(cmdText);
    }
}
void SettingsTabDisplay::getCurrentButton(wxCommandEvent& event)
{

    if (Adapter.getConnected())
    {
        FreqStartSet    = Adapter.send("FREQ:STAR?");
        FreqStartSet    = Adapter.send("FREQ:STOP?");
        FreqEndeSet     = Adapter.send("FREQ:CENT?");
        FreqCenterSet   = Adapter.send("FREQ:SPAN?");
        FreqSpanSet     = Adapter.send("FREQ:POW?");
        pegelSet        = Adapter.send("DISP:TRAC:Y:RLEV?");
        refPegelSet     = Adapter.send("DISP:TRAC:Y:SPAC?");

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
