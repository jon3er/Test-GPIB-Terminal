#include <wx/wx.h>
#include <wx/notebook.h>
#include <map>
#include <thread>
#include "fkt_GPIB.h"
#include "fkt_d2xx.h"
#include "main.h"

wxIMPLEMENT_APP(MainWin);

//-----MainWin Methodes-----
bool MainWin::OnInit()
{
    //Enable Debug output window
    wxLog::SetActiveTarget(new wxLogStderr());

    MainWinFrame *frame = new MainWinFrame();
    frame->Show();

    return true;
}
//-----MainWin Methodes ende-----


//-----MainWinFrame + Methodes-----
MainWinFrame::MainWinFrame() : wxFrame(nullptr, wxID_ANY, "Main Window", wxDefaultPosition, wxSize(500,600))
{
    //Menu punkt 1 Hello
    wxMenu *menuFile = new wxMenu;
    menuFile->Append(ID_Hello,"&Hello \t Ctrl+H","Help item Hello");
    menuFile->AppendSeparator();
    menuFile->Append(wxID_EXIT);

    //Menu Punkt Help
    wxMenu* menuHelp = new wxMenu;
    menuHelp->Append(wxID_ABOUT);

    //menu Punkte an Menu Bar anbringen
    wxMenuBar* menuBar = new wxMenuBar;
    menuBar->Append(menuFile,"&File");
    menuBar->Append(menuHelp,"&Help");

    SetMenuBar(menuBar);

    //Create Status Bar
    CreateStatusBar();
    SetStatusText("Status Bar Text");

    Bind(wxEVT_MENU, &MainWinFrame::OnHello, this, ID_Hello);
    Bind(wxEVT_MENU, &MainWinFrame::OnAbout, this, wxID_ABOUT);
    Bind(wxEVT_MENU, &MainWinFrame::OnExit, this, wxID_EXIT);

    //Create Panel
    wxPanel* panelMain = new wxPanel(this);

    //Create Button "Open Terminal
    wxButton *terminalButton = new wxButton(panelMain, wxID_ANY, "Open Terminal",wxPoint(10,0));
    terminalButton->Bind(wxEVT_BUTTON, &MainWinFrame::OnOpenTerminal,this);

    //Create Button "Open Function Test"
    wxButton *testfunctionButton = new wxButton(panelMain, wxID_ANY, "Open Function Test",wxPoint(10,0));
    testfunctionButton->Bind(wxEVT_BUTTON, &MainWinFrame::OnOpenFunctionTest,this);


    //Create Button "Scan Usb"
    wxButton *scanUsbButton = new wxButton(panelMain, wxID_ANY, "Scan USB Devices",wxPoint(10,0));
    scanUsbButton->Bind(wxEVT_BUTTON, &MainWinFrame::OnScanUsb,this);

    //Create Button "Plot"
    wxButton *plotButton = new wxButton(panelMain, wxID_ANY, "Plot",wxPoint(10,0));
    plotButton->Bind(wxEVT_BUTTON, &MainWinFrame::OnOpenPlot,this);

    //Create Button "Upload GPIB Skript file"
    wxButton *uploadScriptButton = new wxButton(panelMain, wxID_ANY, "Upload GPIB Skript file",wxPoint(10,0));
    uploadScriptButton->Bind(wxEVT_BUTTON, &MainWinFrame::OnOpenUploadScript,this);

    //Create Button "Settings"
    wxButton *settingsButton = new wxButton(panelMain, wxID_ANY, "Settings",wxPoint(10,0));
    settingsButton->Bind(wxEVT_BUTTON, &MainWinFrame::OnOpenSettings,this);
   

    //Display Found Devices
    ScanUsbDisplay = new wxTextCtrl(panelMain,wxID_ANY,"Scan for avalible Devices...",wxDefaultPosition,wxSize(100, 40));
    //disable user input
    ScanUsbDisplay->SetEditable(false);


    //sizer     Window layout
    wxBoxSizer* sizerMain = new wxBoxSizer(wxVERTICAL);
    sizerMain->Add(terminalButton, 0, wxEXPAND | wxALL, 10);
    sizerMain->Add(ScanUsbDisplay, 0, wxEXPAND | wxALL, 10);
    sizerMain->Add(scanUsbButton, 0, wxALL | wxALIGN_RIGHT, 10);
    sizerMain->Add(testfunctionButton, 0, wxEXPAND | wxALL, 10);
    sizerMain->Add(plotButton, 0, wxEXPAND | wxALL, 10);
    sizerMain->Add(uploadScriptButton, 0, wxEXPAND | wxALL, 10);
    sizerMain->Add(settingsButton,0 , wxEXPAND | wxALL,10);
    panelMain->SetSizerAndFit(sizerMain);
}

void MainWinFrame::OnExit(wxCommandEvent& event)
{
    Close(true);
}

void MainWinFrame::OnAbout(wxCommandEvent& event)
{
    wxMessageBox("This is the About Page");
}

void MainWinFrame::OnHello(wxCommandEvent& event)
{
    wxLogMessage("Log Message with Hello");
}

void MainWinFrame::OnOpenTerminal(wxCommandEvent& event)
{
    //Create new sub window
    TerminalWindow *TWin = new TerminalWindow(this);
    //open Window Pauses Main Window
    TWin->ShowModal();
    //Close Window
    TWin->Destroy();
}

void MainWinFrame::OnOpenFunctionTest(wxCommandEvent& event)
{
    //Create new sub window
    FunctionWindow *FuncWin = new FunctionWindow(this);
    //open Window Pauses Main Window
    FuncWin->ShowModal();
    //Close Window
    FuncWin->Destroy();
}

void MainWinFrame::OnScanUsb(wxCommandEvent& event)
{
    wxLogDebug("Scan USB Devices");

    int devices = scanUsbDev();
    wxString deviceNumString = std::to_string(devices) + " Devices Found";

    if (devices <= 0)
    {
        MainWinFrame::ScanUsbDisplay->SetValue("no device found");
    }
    else
    {
        MainWinFrame::ScanUsbDisplay->SetValue(deviceNumString);

    }
}

void MainWinFrame::OnOpenPlot(wxCommandEvent& event)
{
    //Create new sub window
    //SettingsWindow *SettingsWin = new SettingsWindow(this);
    //open Window Pauses Main Window
    //SettingsWin->ShowModal();
    //Close Window
    //SettingsWin->Destroy();
}

void MainWinFrame::OnOpenUploadScript(wxCommandEvent& event)
{
    //Create new sub window
    //SettingsWindow *SettingsWin = new SettingsWindow(this);
    //open Window Pauses Main Window
    //SettingsWin->ShowModal();
    //Close Window
    //SettingsWin->Destroy();
}

void MainWinFrame::OnOpenSettings(wxCommandEvent& event)
{
    //Create new sub window
    SettingsWindow *SettingsWin = new SettingsWindow(this);
    //open Window Pauses Main Window
    SettingsWin->ShowModal();
    //Close Window
    SettingsWin->Destroy();
}
//-----MainWinFrame Methodes End -----

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
        writeToDevice("SWE:POIN 100"); //100 messpunkte über messbereich aufnehmen
        writeToDevice("FREQ:STAR 80 MHZ");
        writeToDevice("FREQ:STOP 120 MHZ");
        writeToDevice("BAND:RES 100 KHZ ");

        writeToDevice("FORM:DATA REAL,32");
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
        writeToDevice("TRAC1:DATA?");
        writeToDevice("++read eoi");

        sleepMs(100);
        readFromDevice();

        writeToDevice("INIT:CONT ON"); //Dauerhafter sweep an
    }
    else if ("swp")
    {

        writeToDevice("++auto 0");
        writeToDevice("*RST");
        writeToDevice("INIT:CONT OFF");                 //single sweep
        writeToDevice("SYST:DISP:UDP ON");              //Bildschrim an
        //Frequenzeinstellung
        writeToDevice("FREQ:STAR 85MHz;STOP 125MHz");   //Frequenz bereich
        //Pegeleinstellen
        writeToDevice("DISP:WIND:TRAC:Y:RLEV -20dBm"); //referenzpegel
        writeToDevice("INIT;*WAI");                     //sweep durchführen mit sync
        //TOI messen
        writeToDevice("CALC:MARK:PEXC 6DB");
        writeToDevice("CALC:MARK:FUNC:TOI ON");         //TOI messung an

        writeToDevice("CALC:MARK:FUNC:TOI:RES?");       //Ergebniss auslesen

        writeToDevice("++read eos");
        readFromDevice();

        writeToDevice("++auto 1");
    }
    else
    {
        wxLogDebug("manual read write");
        writeToDevice(args);
        writeToDevice("++read eos");
        sleepMs(50);
        readFromDevice();
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
    freqEinheiten.Add("mHz");
    freqEinheiten.Add("Hz");
    freqEinheiten.Add("kHz");
    freqEinheiten.Add("MHz");
    freqEinheiten.Add("GHz");

    wxArrayString pegelEinheiten;
    pegelEinheiten.Add("DBM");
    pegelEinheiten.Add("V");
    pegelEinheiten.Add("A");
    pegelEinheiten.Add("W");
    pegelEinheiten.Add("DBPW");
    pegelEinheiten.Add("DBUV");
    pegelEinheiten.Add("DBUA");

    wxArrayString scalingY;
    scalingY.Add("Logarthmmic");
    scalingY.Add("Linear");

    wxStaticText* labelText = new wxStaticText(this, wxID_ANY, label, wxPoint(10,10));   

    //Start-Ende Elemente-----Start
    wxCheckBox* startEndeCheck = new wxCheckBox(this, wxID_ANY, "Start - Ende Nutzen");
    wxStaticText* descriptionText_1 = new wxStaticText(this, wxID_ANY, "Start-Frequenz:", wxPoint(10,10));
    wxTextCtrl* inputText_1 = new wxTextCtrl(this, wxID_ANY, "75");
    wxChoice* freqEinheitAuswahl_1 = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, freqEinheiten);
    freqEinheitAuswahl_1->SetSelection(3);

    wxStaticText* descriptionText_2 = new wxStaticText(this, wxID_ANY, "End-Frequenz:", wxPoint(10,10));
    wxTextCtrl* inputText_2 = new wxTextCtrl(this, wxID_ANY,"125");
    wxChoice* freqEinheitAuswahl_2 = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, freqEinheiten);
    freqEinheitAuswahl_2->SetSelection(3);
    //Start-Ende Elemente-----Ende

    //Center-Spanne Elemente-----Start
    wxCheckBox* centerSpanCheck = new wxCheckBox(this, wxID_ANY, "Center - Spanne Nutzen");
    wxStaticText* descriptionText_3 = new wxStaticText(this, wxID_ANY, "Center-Frequenz:", wxPoint(10,10));
    wxTextCtrl* inputText_3 = new wxTextCtrl(this, wxID_ANY,"100");
    wxChoice* freqEinheitAuswahl_3 = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, freqEinheiten);
    freqEinheitAuswahl_3->SetSelection(3);

    wxStaticText* descriptionText_4 = new wxStaticText(this, wxID_ANY, "Span-Frequenz:", wxPoint(10,10));
    wxTextCtrl* inputText_4 = new wxTextCtrl(this, wxID_ANY, "50");
    wxChoice* freqEinheitAuswahl_4 = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, freqEinheiten);
    freqEinheitAuswahl_4->SetSelection(3);
    //Center-Spanne Elemente-----Ende

    //Y-Achsen Elemente-----Start
    wxStaticText* descriptionText_5 = new wxStaticText(this, wxID_ANY, "Y-Scaling:", wxPoint(10,10));
    wxTextCtrl* inputText_5 = new wxTextCtrl(this, wxID_ANY,"100");
    wxChoice* pegelEinheitAuswahl = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, pegelEinheiten);
    pegelEinheitAuswahl->SetSelection(0);

    wxStaticText* descriptionText_6 = new wxStaticText(this, wxID_ANY, "Y-Scale Spacing:", wxPoint(10,10));
    wxChoice* yScalingAuswahl = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, scalingY);
    yScalingAuswahl->SetSelection(0);

    wxStaticText* descriptionText_7 = new wxStaticText(this, wxID_ANY, "Referenzpegel in dB:", wxPoint(10,10));
    wxTextCtrl* inputText_7 = new wxTextCtrl(this, wxID_ANY, "-20");
    //Y-Achsen Elemente-----Ende

    //Knöpfe START
    wxButton* anwendenButton = new wxButton(this, wxID_ANY, "Anwenden");

    wxButton* getCurrentButton = new wxButton(this, wxID_ANY, wxString::FromUTF8("Messgerät Einstellungen Laden"));
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