#include <wx/wx.h>
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
    if (Connected)
    {
        disconnectDevice(" ");
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

void TerminalWindow::scanDevices(const std::string& args)
{
    DWORD devNum = scanUsbDev();
    wxString Text = "Number of devices: " + std::to_string(devNum) + "\n";

    TerminalDisplay->AppendText(terminalTimestampOutput(Text));
}

void TerminalWindow::statusDevice(const std::string& args)
{
    wxString Text;

    if (Connected)
    {
        Text = "Connected to a Device ";
    }
    else
    {
        Text = "Not connected to a device";
    }
    if (Connected && configFin)
    {
        Text = Text + " and ";
    }
    if (configFin)
    {
        Text = Text + "Device config set Baudrate to: " + std::to_string(BaudRate);
    }

    TerminalDisplay->AppendText(terminalTimestampOutput(Text +"\n"));
}

void TerminalWindow::connectDevice(const std::string& args)
{
    FT_STATUS ftStatus;

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

    if (!Connected)
    {
        ftStatus = FT_Open(dev,&ftHandle);
        printErr(ftStatus,"Failed to Connect");

        if (ftStatus == FT_OK)
        {
            TerminalDisplay->AppendText(terminalTimestampOutput("Connected to a device\n"));
            wxLogDebug("Connected to %i", dev);
            Connected = true;
        }
        else
        {
            wxLogDebug("Couldnt connect");
            TerminalDisplay->AppendText(terminalTimestampOutput("Couldnt connect to a device\n Is programm running as SU?\n Is the FTDI_SIO Driver unloaded?\n"));
        }

        ftStatus = FT_Purge(ftHandle, FT_PURGE_RX | FT_PURGE_TX);
        printErr(ftStatus,"Purge Failed");

    }
    else
    {
        TerminalDisplay->AppendText(terminalTimestampOutput("Device already connected\n"));
    }
}

void TerminalWindow::disconnectDevice(const std::string& args)
{
    FT_STATUS ftStatus;

    wxLogDebug("Command entered: disconnect with arg: %s", args);

    writeToDevice("++auto 0");
    writeToDevice("*CLS");
    writeToDevice("++loc");
    writeToDevice("++ifc");


    std::this_thread::sleep_for(std::chrono::microseconds(200000));


    ftStatus = FT_Close(ftHandle);
    printErr(ftStatus,"Failed to Disconnect");

    if (ftStatus == FT_OK)
    {
        TerminalDisplay->AppendText(terminalTimestampOutput("Disconnected from a device\n"));
        wxLogDebug("disconnected from current device");
        Connected = false;
        configFin = false;
    }
    else
    {
        TerminalDisplay->AppendText(terminalTimestampOutput("Coulnd't disconnect check if a device is connected with: status\n"));
    }

}

wxString TerminalWindow::sendToDevice(const std::string& args)
{
    wxString Text;
    wxString TextOut;

    wxLogDebug("terminal Command send %s Entered",args);

    if (Connected && configFin)
    {
        DWORD bytesWritten;
        wxString GPIBText = args;
        std::string CheckText(GPIBText.ToUTF8());
        //Check String if Adapter or GPIB Command and check for ASCII 10, 13, 27, 43
        std::vector<char> charArrWriteGpib = checkAscii(CheckText);

        wxLogDebug("Trying to write to Device... %s", std::string(charArrWriteGpib.begin(),charArrWriteGpib.end()));

        FT_STATUS ftStatus = writeUsbDev(ftHandle, charArrWriteGpib, bytesWritten);

        if (ftStatus == FT_OK)
        {
            Text = std::string(charArrWriteGpib.begin(),charArrWriteGpib.end());
            Text = "Msg sent: " + Text + " ; " + std::to_string(bytesWritten) + " Bytes Written to GPIB Device\n";
            TerminalWindow::TerminalDisplay->AppendText(terminalTimestampOutput(Text));
        }
        else
        {
            Text = "Failed to send Data\n";
            TerminalWindow::TerminalDisplay->AppendText(terminalTimestampOutput(Text));
        }

        //---- read ---
        // Wait for Responce
        std::this_thread::sleep_for(std::chrono::microseconds(100000));

        std::vector<char> BigBuffer;
        DWORD BufferSize;

        wxLogDebug("Reading from Device...");
        ftStatus = readUsbDev(ftHandle, BigBuffer,BufferSize);

        if (ftStatus == FT_OK)
        {
            TextOut = std::string(BigBuffer.begin(),BigBuffer.end());
            Text = "Msg received: " + TextOut + "\n";
            if (BigBuffer.size() == 0)
            {
                Text = "No Message to Read\n";
            }

            TerminalWindow::TerminalDisplay->AppendText(terminalTimestampOutput(Text));
        }
        else
        {
            Text = "Failed to Receive Data - TimeOut after 5s\n";
            TerminalWindow::TerminalDisplay->AppendText(terminalTimestampOutput(Text));
        }
    }
    else
    {
        wxLogDebug("No Device to send too or missing config");
        Text = "Failed to connect to a device or missing config\n";
        TerminalWindow::TerminalDisplay->AppendText(terminalTimestampOutput(Text));
    }

    return TextOut;
}

wxString TerminalWindow::readFromDevice(const std::string& args = "")
{
    wxString Text;
    wxString TextOut;

    wxLogDebug("command read entered with args: %s", args);

    if (Connected && configFin)
    {
        std::vector<char> BigBuffer;
        DWORD BufferSize;
        FT_STATUS ftStatus;

        wxLogDebug("Reading from Device...");

        ftStatus = readUsbDev(ftHandle, BigBuffer,BufferSize);

        if (ftStatus == FT_OK)
        {
            TextOut = std::string(BigBuffer.data(),BigBuffer.size());
            Text = "Msg received: " + TextOut + "\n";

            if (BigBuffer.size() == 0)
            {
                Text = "No Message to Read\n";
            }

            TerminalWindow::TerminalDisplay->AppendText(terminalTimestampOutput(Text));
        }
        else
        {
            Text = "Failed to Receive Data - TimeOut after 5s\n";
            TerminalWindow::TerminalDisplay->AppendText(terminalTimestampOutput(Text));
        }

    }
    else
    {
        wxLogDebug("No Device to send too");
        Text = "Failed to Connected to a Device\n";
        TerminalWindow::TerminalDisplay->AppendText(terminalTimestampOutput(Text));
    }

    return TextOut;
}

void TerminalWindow::writeToDevice(const std::string& args)
{
    wxString Text;

    wxLogDebug("Write Command Entered");

    if (TerminalWindow::Connected)
    {
        DWORD bytesWritten;
        wxString GPIBText = args;
        std::string CheckText(GPIBText.ToUTF8());
        //Check String if Adapter or GPIB Command and check for ASCII 10, 13, 27, 43
        std::vector<char> charArrWriteGpib = checkAscii(CheckText);

        wxLogDebug("Trying to write to Device... %s", std::string(charArrWriteGpib.begin(),charArrWriteGpib.end()));

        FT_STATUS ftStatus =writeUsbDev(ftHandle, charArrWriteGpib, bytesWritten);

        if (ftStatus == FT_OK)
        {
            Text = GPIBText;
            Text = "Msg sent: " + Text + " ; " + std::to_string(bytesWritten) + " Bytes Written to GPIB Device\n";
            TerminalWindow::TerminalDisplay->AppendText(terminalTimestampOutput(Text));
        }
        else
        {
            Text = "Failed to send Data\n";
            TerminalWindow::TerminalDisplay->AppendText(terminalTimestampOutput(Text));
        }

    }
    else
    {
        wxLogDebug("No Connection");
        Text = "Failed to Connect\n";
        TerminalWindow::TerminalDisplay->AppendText(terminalTimestampOutput(Text));
    }
}

void TerminalWindow::configDevice(const std::string& args)
{
    DWORD numDev = 0;
    wxString Text;

    if (args != "")
    {
        if (std::stoi(args) == std::clamp(std::stoi(args), 1, 1000000))
        {
            BaudRate = std::stoi(args);
            wxLogDebug("Set Baudrate to %i", BaudRate);
        }
        else
        {
            wxLogDebug("Using Default Baudrate: %i",BaudRate);
        }
    }
    else
    {
        wxLogDebug("Using Default Baudrate: %i",BaudRate);
    }

    FT_STATUS Status = configUsbDev(numDev, ftHandle, BaudRate);

    if (Status == FT_OK)
    {
        Text = "Set Device BaudRate to " + std::to_string(BaudRate) + "\n";
        TerminalWindow::TerminalDisplay->AppendText(terminalTimestampOutput(Text));
        wxLogDebug("Baudrate set to: %s", std::to_string(BaudRate));
        //wxLogDebug(std::to_string(ftHandle);
        configFin = true;
    }
    else
    {
        Text = "Failed to config device check if run as SU\n";
        TerminalWindow::TerminalDisplay->AppendText(terminalTimestampOutput(Text));
        configFin = false;
    }
}

void TerminalWindow::testDevice(const std::string& args)
{
    if (args == "")
    {
        connectDevice("");
        configDevice("");


        writeToDevice("++clr");
        std::this_thread::sleep_for(std::chrono::microseconds(200000));
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
        std::this_thread::sleep_for(std::chrono::microseconds(200000));

        writeToDevice("++mode 1");
        writeToDevice("++auto 0");
        writeToDevice("++addr 20");

        writeToDevice("*IDE?");
        writeToDevice("++read eoi");
        std::this_thread::sleep_for(std::chrono::microseconds(50000));
        readFromDevice("");
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
        std::this_thread::sleep_for(std::chrono::microseconds(200000));
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
        std::this_thread::sleep_for(std::chrono::microseconds(200'000));
        writeToDevice("SWE:TIME?");
        wxString swpTime = sendToDevice("++read");

        //float muS = std::stof(swpTime.c_str())*100;
        writeToDevice("INIT:IMM"); //Messung starten
        writeToDevice("*WAI");
        std::this_thread::sleep_for(std::chrono::microseconds(100*15000));

        wxString responce;
        int i = 0;
        while ((responce.substr(0,1) != "1") || (i == 20))
        {
            writeToDevice("*OPC?");
            std::this_thread::sleep_for(std::chrono::microseconds(100'000));
            responce = sendToDevice("++read eoi");
            i++;
        }

        writeToDevice("TRAC1:DATA?");
        writeToDevice("++read eoi");

        std::this_thread::sleep_for(std::chrono::microseconds(300'000));
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
        std::this_thread::sleep_for(std::chrono::microseconds(50000));
        readFromDevice("");
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
    if (Connected)
    {
        FT_STATUS ftStatus = FT_Close(ftHandle);
        printErr(ftStatus,"Failed to Disconnect");

        if (ftStatus == FT_OK)
        {
            textFuncOutput->AppendText(terminalTimestampOutput("Disconnected from a device\n"));
            wxLogDebug("Disconnected");
            Connected = false;
        }
    }
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
        configFin = false;
    }
    else
    {
        textFuncOutput->AppendText(terminalTimestampOutput(deviceNumString));
        configFin = true;
    }
}

void FunctionWindow::OnConDisconGpib(wxCommandEvent& event)
{
    FT_STATUS ftStatus;
    int dev = 0;

    if (!Connected)
    {
        ftStatus = FT_Open(dev,&ftHandle);
        printErr(ftStatus,"Failed to Connect");

        ftStatus = FT_Purge(ftHandle, FT_PURGE_RX | FT_PURGE_TX);
        printErr(ftStatus,"Purge Failed");

        if (ftStatus == FT_OK)
        {
            textFuncOutput->AppendText(terminalTimestampOutput("Connected to a device\n"));
            wxLogDebug("Connected to %i", dev);
            Connected = true;
        }
    }
    else
    {
        ftStatus = FT_Close(ftHandle);
        printErr(ftStatus,"Failed to Disconnect");

        if (ftStatus == FT_OK)
        {
            textFuncOutput->AppendText(terminalTimestampOutput("Disconnected from a device\n"));
            wxLogDebug("Connected to %i", dev);
            Connected = false;
        }
    }
}

void FunctionWindow::OnWriteGpib(wxCommandEvent& event)
{
    wxString Text;

    wxLogDebug("Write Pressed!");

    if (Connected)
    {
        DWORD bytesWritten;
        wxString GPIBText = FunctionWindow::writeFuncInput->GetValue();

        FunctionWindow::writeFuncInput->SetValue("");

        std::string CheckText(GPIBText.ToUTF8());
        std::vector<char> charArrWriteGpib = checkAscii(CheckText);

        wxLogDebug("Trying to write to Device... %s", std::string(charArrWriteGpib.begin(),charArrWriteGpib.end()));

        FT_STATUS ftStatus =writeUsbDev(ftHandle, charArrWriteGpib, bytesWritten);

        if (ftStatus == FT_OK)
        {
            Text = GPIBText;
            Text = "Msg sent: " + Text + " ; " + std::to_string(bytesWritten) + " Bytes Written to GPIB Device\n";
            FunctionWindow::textFuncOutput->AppendText(terminalTimestampOutput(Text));
        }
        else
        {
            Text = "Failed to send Data\n";
            FunctionWindow::textFuncOutput->AppendText(terminalTimestampOutput(Text));
        }

    }
    else
    {
        wxLogDebug("No Connection");
        Text = "Failed to Connect\n";
        FunctionWindow::textFuncOutput->AppendText(terminalTimestampOutput(Text));
    }

}

void FunctionWindow::OnReadGpib(wxCommandEvent& event)
{
    wxString Text;

    wxLogDebug("On Read Pressed");

    if (FunctionWindow::Connected)
    {
        std::vector<char> BigBuffer;
        DWORD BufferSize;
        FT_STATUS ftStatus;

        wxLogDebug("Reading from Device...");

        ftStatus = readUsbDev(ftHandle, BigBuffer,BufferSize);

        if (ftStatus == FT_OK)
        {
            Text = std::string(BigBuffer.data(),BigBuffer.size());
            Text = "Msg received: " + Text + "\n";

            if (BigBuffer.size() == 0)
            {
                Text = "No Message to Read\n";
            }

            FunctionWindow::textFuncOutput->AppendText(terminalTimestampOutput(Text));
        }
        else
        {
            Text = "Failed to Receive Data - TimeOut after 5s\n";
            FunctionWindow::textFuncOutput->AppendText(terminalTimestampOutput(Text));
        }

    }
    else
    {
        wxLogDebug("No Device to send too");
        Text = "Failed to Connect to a Device\n";
        FunctionWindow::textFuncOutput->AppendText(terminalTimestampOutput(Text));
    }
}

void FunctionWindow::OnReadWriteGpib(wxCommandEvent& event)
{
    wxString Text;

    wxLogDebug("On Write / Read Pressed");

    if (Connected && configFin)
    {
        DWORD bytesWritten;
        wxString GPIBText = FunctionWindow::writeFuncInput->GetValue();

        std::string CheckText(GPIBText.ToUTF8());
        std::vector<char> charArrWriteGpib = checkAscii(CheckText);

        wxLogDebug("Trying to write to Device... %s", std::string(charArrWriteGpib.begin(),charArrWriteGpib.end()));
        FT_STATUS ftStatus =writeUsbDev(ftHandle, charArrWriteGpib, bytesWritten);

        if (ftStatus == FT_OK)
        {
            Text = std::string(charArrWriteGpib.begin(),charArrWriteGpib.end());
            Text = "Msg sent: " + Text + " ; " + std::to_string(bytesWritten) + " Bytes Written to GPIB Device\n";
            FunctionWindow::textFuncOutput->AppendText(terminalTimestampOutput(Text));
        }
        else
        {
            Text = "Failed to send Data\n";
            FunctionWindow::textFuncOutput->AppendText(terminalTimestampOutput(Text));
        }

        //read
        std::this_thread::sleep_for(std::chrono::microseconds(50000));

        std::vector<char> BigBuffer;
        DWORD BufferSize;

        wxLogDebug("Reading from Device...");

        ftStatus = readUsbDev(ftHandle, BigBuffer,BufferSize);

        if (ftStatus == FT_OK)
        {
            Text = std::string(BigBuffer.data(),BigBuffer.size());
            Text = "Msg received:\n" + Text + "\n";

            if (BigBuffer.size() == 0)
            {
                Text = "No Message to Read\n";
            }

            FunctionWindow::textFuncOutput->AppendText(terminalTimestampOutput(Text));
        }
        else
        {
            Text = "Failed to Receive Data - TimeOut after 5s\n";
            FunctionWindow::textFuncOutput->AppendText(terminalTimestampOutput(Text));
        }

    }
    else
    {
        wxLogDebug("No Device to send too");
        Text = "Failed to Connect to a Device";
        FunctionWindow::textFuncOutput->AppendText(terminalTimestampOutput(Text));
    }

}

void FunctionWindow::OnUsbConfig(wxCommandEvent& event)
{
    DWORD numDev = 0;
    int BaudRate = 921600;
    wxString Text;

    FT_STATUS Status = configUsbDev(numDev, ftHandle, BaudRate);

    if (Status == FT_OK)
    {
        Text = "Set Device BaudRate to " + std::to_string(BaudRate) + "\n";
        FunctionWindow::textFuncOutput->AppendText(terminalTimestampOutput(Text));

        wxLogDebug("Baudrate set to: %s", std::to_string(BaudRate));

        configFin = true;
    }
    else
    {
        Text = "Failed to config device check if run as SU";
        FunctionWindow::textFuncOutput->AppendText(terminalTimestampOutput(Text));

        configFin = false;
    }
}
//-----Function Window Methodes End -----

