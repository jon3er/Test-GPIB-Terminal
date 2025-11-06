#include <wx/wx.h>
#include <unistd.h>
#include <map>
#include "fkt_GPIB.h"
#include "fkt_d2xx.h"
//#include <Main.h>

enum
{
    ID_Hello = 1,
    ID_OpenTerminal = 2
};


//-----MainWin-----
class MainWin : public wxApp
{
public:
    bool OnInit() override; //overrides function of Base Classe wxApp Function OnInit()
};

//-----MainWinFrame-----
class MainWinFrame : public wxFrame
{
public:
    MainWinFrame();
private:
    wxTextCtrl* ScanUsbDisplay;

    //menubar
    void OnHello(wxCommandEvent& event);
    void OnExit(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);
    //bottons
    void OnOpenTerminal(wxCommandEvent& event);
    void OnScanUsb(wxCommandEvent& event);
    void OnOpenFunctionTest(wxCommandEvent& event);
};

//-----Terimal-----
class TerminalWindow : public wxDialog
{
public:
    TerminalWindow(wxWindow *parent) : wxDialog(parent, wxID_ANY, "GPIB Terminal Window", wxDefaultPosition, wxSize(1000,600))
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

        StaticTE = new wxStaticText(panelTerm, wxID_ANY,"GPIB Terminal log");

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

        cmds["scan"]        = [this](const std::string& args) { this->scanDevices(args); };
        cmds["status"]      = [this](const std::string& args) { this->statusDevice(args); };
        cmds["config"]      = [this](const std::string& args) { this->configDevice(args); };
        cmds["connect"]     = [this](const std::string& args) { this->connectDevice(args); };
        cmds["disconnect"]  = [this](const std::string& args) { this->disconnectDevice(args); };
        cmds["send"]        = [this](const std::string& args) { this->sendToDevice(args); };
        cmds["read"]        = [this](const std::string& args) { this->readFromDevice(args); };
        cmds["write"]       = [this](const std::string& args) { this->writeToDevice(args); };
    }


private:
    void OnEnterTerminal(wxCommandEvent& event);
    //Map Functions

    // Define a type for our command map for easier reading
    using CommandMap = std::map<std::string, std::function<void(const std::string&)>>;
    CommandMap cmds;
    void scanDevices(const std::string& args);
    void statusDevice(const std::string& args);
    void configDevice(const std::string& args);
    void connectDevice(const std::string& args);
    void disconnectDevice(const std::string& args);
    void sendToDevice(const std::string& args);
    void readFromDevice(const std::string& args);
    void writeToDevice(const std::string& args);


    //com Settings
    int BaudRate = 921600;
    //data recive var
    DWORD BytesToRead = 1024;
    DWORD BytesReturned;
    //com Var
    FT_HANDLE ftHandle;
    FT_STATUS ftStatus;

    bool Connected;
    bool configFin;
    wxStaticText* StaticTE;
    wxTextCtrl* TerminalDisplay;
};

class FunctionWindow : public wxDialog
{
public:
    FunctionWindow(wxWindow *parent) : wxDialog(parent, wxID_ANY, "Function Test Window", wxDefaultPosition, wxSize(500,750))
    {
        wxPanel* panelfunc = new wxPanel(this);


        //text to Write to Gpib

        wxStaticText* discFuncInput = new wxStaticText(panelfunc,wxID_ANY,"Input text to write: ");

        writeFuncInput = new wxTextCtrl(panelfunc, wxID_ANY,"",wxDefaultPosition,wxSize(300, 40));
        //set Cursor in input window
        writeFuncInput->SetFocus();

        //Create Button "Write to GPIB"
        wxButton* writeGpibButton = new wxButton(panelfunc, wxID_ANY, "Write to GPIB",wxPoint(10,0));
        // Trigger button Function when Pressing the button
        writeGpibButton->Bind(wxEVT_BUTTON, &FunctionWindow::OnWriteGpib,this);

        //Create Button "Read to GPIB"
        wxButton* readGpibButton = new wxButton(panelfunc, wxID_ANY, "Read from GPIB",wxPoint(10,0));
        // Trigger button Function when Pressing the button
        readGpibButton->Bind(wxEVT_BUTTON, &FunctionWindow::OnReadGpib,this);

        //Create Button "Read to GPIB"
        wxButton* readWriteGpibButton = new wxButton(panelfunc, wxID_ANY, "Write and Read from GPIB",wxPoint(10,0));
        // Trigger button Function when Pressing the button
        readWriteGpibButton->Bind(wxEVT_BUTTON, &FunctionWindow::OnReadWriteGpib,this);

        //Create Button "Read to GPIB"
        wxButton* scanUsbButton = new wxButton(panelfunc, wxID_ANY, "Scan For Device",wxPoint(10,0));
        // Trigger button Function when Pressing the button
        scanUsbButton->Bind(wxEVT_BUTTON, &FunctionWindow::OnUsbScan,this);

        //Create Button "Read to GPIB"
        wxButton* devConfigButton = new wxButton(panelfunc, wxID_ANY, "Configure USB Device",wxPoint(10,0));
        // Trigger button Function when Pressing the button
        devConfigButton->Bind(wxEVT_BUTTON, &FunctionWindow::OnUsbConfig,this);

        //Create Button "Read to GPIB"
        wxButton* connectDevGpibButton = new wxButton(panelfunc, wxID_ANY, "Connected / Disconnect",wxPoint(10,0));
        // Trigger button Function when Pressing the button
        connectDevGpibButton->Bind(wxEVT_BUTTON, &FunctionWindow::OnConDisconGpib,this);


        wxStaticText* discFuncOutput = new wxStaticText(panelfunc,wxID_ANY,"Function output: ");
        // Text Log
        textFuncOutput = new wxTextCtrl(panelfunc, wxID_ANY,"",wxDefaultPosition,wxSize(300, 200), wxTE_MULTILINE);



        //sizer
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
private:

    void OnWriteGpib(wxCommandEvent& event);
    void OnReadGpib(wxCommandEvent& event);
    void OnReadWriteGpib(wxCommandEvent& event);
    void OnUsbScan(wxCommandEvent& event);
    void OnUsbConfig(wxCommandEvent& event);
    void OnConDisconGpib(wxCommandEvent& event);
    //
    wxTextCtrl* textFuncOutput;
    wxTextCtrl* writeFuncInput;

    bool configFin;
    bool Connected;
    FT_HANDLE ftHandle = NULL;
};

wxIMPLEMENT_APP(MainWin);


//-----MainWin Methodes-----

bool MainWin::OnInit()
{
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

    //Create Button "Open Terminal in Position 10,10
    wxButton *terminalButton = new wxButton(panelMain, wxID_ANY, "Open Terminal",wxPoint(10,0));
    // Trigger OnOpenTerminal when Pressing the button
    terminalButton->Bind(wxEVT_BUTTON, &MainWinFrame::OnOpenTerminal,this);

    //Create Button "Open Function Test" in Position 10,10
    wxButton *testfunctionButton = new wxButton(panelMain, wxID_ANY, "Open Function Test",wxPoint(10,0));
    // Trigger OnOpenTerminal when Pressing the button
    testfunctionButton->Bind(wxEVT_BUTTON, &MainWinFrame::OnOpenFunctionTest,this);


    //Create Button "Scan Usb" in Position 10,10
    wxButton *scanUsbButton = new wxButton(panelMain, wxID_ANY, "Scan USB Devices",wxPoint(10,0));
    // Trigger OnScanUsb when Pressing the button
    scanUsbButton->Bind(wxEVT_BUTTON, &MainWinFrame::OnScanUsb,this);


    //Display Found Devices
    ScanUsbDisplay = new wxTextCtrl(panelMain,wxID_ANY,"Scan for avalible Devices...",wxDefaultPosition,wxSize(100, 40));
    //disable user input
    ScanUsbDisplay->SetEditable(false);


    //ass Sizer
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

//-----MainWinFrame Methodes ende-----


//-----TerminalWindow-----
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
        Text = Text + "Device config set";
    }

    TerminalDisplay->AppendText(terminalTimestampOutput(Text +"\n"));
}
void TerminalWindow::connectDevice(const std::string& args)
{
    FT_STATUS ftStatus;
    FT_HANDLE tempHandle = TerminalWindow::ftHandle;
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

    if (!TerminalWindow::Connected)
    {
        ftStatus = FT_Open(dev,&tempHandle);
        printErr(ftStatus,"Failed to Connect");

        ftStatus = FT_Purge(tempHandle, FT_PURGE_RX | FT_PURGE_TX);
        printErr(ftStatus,"Purge Failed");

        if (ftStatus == FT_OK)
        {
            TerminalDisplay->AppendText(terminalTimestampOutput("Connected to a device\n"));
            wxLogDebug("Connected to %i", dev);
            TerminalWindow::Connected = true;
        }
    }
    else
    {
        TerminalDisplay->AppendText(terminalTimestampOutput("Device Alreday connected\n"));
    }

    TerminalWindow::ftHandle = tempHandle;

}
void TerminalWindow::disconnectDevice(const std::string& args)
{
    FT_STATUS ftStatus;
    FT_HANDLE tempHandle = TerminalWindow::ftHandle;

    wxLogDebug("Command entered: disconnect with arg: %s", args);

    ftStatus = FT_Close(tempHandle);
    printErr(ftStatus,"Failed to Disconnect");

    if (ftStatus == FT_OK)
    {
        TerminalDisplay->AppendText(terminalTimestampOutput("Disconnected from a device\n"));
        wxLogDebug("disconnected from current device");
        TerminalWindow::Connected = false;
        TerminalWindow::configFin = false;
    }

}
void TerminalWindow::sendToDevice(const std::string& args)
{
    wxString Text;

    wxLogDebug("terminal Command send %s Entered",args);

    if (TerminalWindow::Connected && TerminalWindow::configFin)
    {
        DWORD bytesWritten;
        char* charArrWriteGpib;

        wxString GPIBText = args;

        std::string CheckText(GPIBText.ToUTF8());

        charArrWriteGpib = checkAscii(CheckText);

        wxLogDebug("Trying to write to Device... %s", charArrWriteGpib);
        FT_STATUS ftStatus =writeUsbDev(ftHandle, charArrWriteGpib, bytesWritten);

        if (ftStatus == FT_OK)
        {
            Text = charArrWriteGpib;
            Text = "Msg sent: " + Text + " \n " + std::to_string(bytesWritten) + " Bytes Written to GPIB Device\n";
            TerminalWindow::TerminalDisplay->AppendText(terminalTimestampOutput(Text));
        }
        else
        {
            Text = "Failed to send Data\n";
            TerminalWindow::TerminalDisplay->AppendText(terminalTimestampOutput(Text));
        }

        //read
        usleep(50000);

        //char Buffer[256];
        std::vector<char> BigBuffer;
        DWORD BufferSize;

        wxLogDebug("Reading from Device...");
        //FT_STATUS ftStatus = readUsbDev(ftHandle, Buffer, BufferSize);
        ftStatus = readUsbDevTest(ftHandle, BigBuffer,BufferSize);

        if (ftStatus == FT_OK)
        {
            //Text = std::string(Buffer);
            Text = std::string(BigBuffer.data(),BigBuffer.size());
            Text = "Msg received:\n" + Text + "\n";
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
        Text = "Failed to Connected to a Device";
        TerminalWindow::TerminalDisplay->AppendText(terminalTimestampOutput(Text));
    }

}
void TerminalWindow::readFromDevice(const std::string& args)
{
    wxString Text;

    wxLogDebug("command read entered with args: %s", args);

    if (TerminalWindow::Connected)
    {
        //char Buffer[256];
        std::vector<char> BigBuffer;
        DWORD BufferSize;
        FT_STATUS ftStatus;

        wxLogDebug("Reading from Device...");
        //FT_STATUS ftStatus = readUsbDev(ftHandle, Buffer, BufferSize);
        ftStatus = readUsbDevTest(ftHandle, BigBuffer,BufferSize);

        if (ftStatus == FT_OK)
        {
            //Text = std::string(Buffer);
            Text = std::string(BigBuffer.data(),BigBuffer.size());
            Text = "Msg received:\n" + Text + "\n";
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
}
void TerminalWindow::writeToDevice(const std::string& args)
{
    wxString Text;

    wxLogDebug("Write Command Entered");

    if (TerminalWindow::Connected)
    {
        DWORD bytesWritten;
        char* charArrWriteGpib;

        wxString GPIBText = args;

        std::string CheckText(GPIBText.ToUTF8());

        charArrWriteGpib = checkAscii(CheckText);

        wxLogDebug("Trying to write to Device... %s", charArrWriteGpib);
        FT_STATUS ftStatus =writeUsbDev(ftHandle, charArrWriteGpib, bytesWritten);

        if (ftStatus == FT_OK)
        {
            Text = GPIBText;
            Text = "Msg sent: " + Text + " \n " + std::to_string(bytesWritten) + " Bytes Written to GPIB Device\n";
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
        Text = "Failed to Connected";
        TerminalWindow::TerminalDisplay->AppendText(terminalTimestampOutput(Text));
    }
}

void TerminalWindow::configDevice(const std::string& args)
{
    DWORD numDev = 0;
    wxString Text;
    //FT_HANDLE tempHandle = TerminalWindow::ftHandle;
    //args => config Baurate

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
        wxLogDebug("Baudrate set to: " + std::to_string(BaudRate));
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

void TerminalWindow::OnEnterTerminal(wxCommandEvent& event)
{
    wxTextCtrl* Terminal = static_cast<wxTextCtrl*>(event.GetEventObject());
    wxString TText = Terminal->GetValue();
    Terminal->SetValue("");

    wxLogDebug("user entered: %s", TText.c_str());

    std::string rawText = std::string(TText.ToUTF8());
    //int testRes = checkCMD(TText);
    TText = TText + "\n";
    //Output to terminal
    TerminalDisplay->AppendText(terminalTimestampOutput(TText));

    //entspechende Funktionen finden und aufrufen
    std::string strCmd;
    std::string args;
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
        TerminalDisplay->AppendText(terminalTimestampOutput("Unknown command!"));
    }
}

//-----Terminal window Methodes endes-----

//-----Function Window-----
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
    FT_HANDLE tempHandle = FunctionWindow::ftHandle;
    int dev = 0;

    if (!FunctionWindow::Connected)
    {
        ftStatus = FT_Open(dev,&tempHandle);
        printErr(ftStatus,"Failed to Connect");

        ftStatus = FT_Purge(tempHandle, FT_PURGE_RX | FT_PURGE_TX);
        printErr(ftStatus,"Purge Failed");

        if (ftStatus == FT_OK)
        {
            textFuncOutput->AppendText(terminalTimestampOutput("Connected to a device\n"));
            wxLogDebug("Connected to %i", dev);
            FunctionWindow::Connected = true;
        }
    }
    else
    {
        ftStatus = FT_Close(tempHandle);
        printErr(ftStatus,"Failed to Disconnect");

        if (ftStatus == FT_OK)
        {
            textFuncOutput->AppendText(terminalTimestampOutput("Disconnected from a device\n"));
            wxLogDebug("Connected to %i", dev);
            FunctionWindow::Connected = false;
        }
    }

    FunctionWindow::ftHandle = tempHandle;
}

void FunctionWindow::OnWriteGpib(wxCommandEvent& event)
{

    wxString Text;


    wxLogDebug("On Write Pressed");

    if (FunctionWindow::Connected)
    {
        DWORD bytesWritten;
        char* charArrWriteGpib;

        wxString GPIBText = FunctionWindow::writeFuncInput->GetValue();

        std::string CheckText(GPIBText.ToUTF8());

        charArrWriteGpib = checkAscii(CheckText);

        wxLogDebug("Trying to write to Device... %s", charArrWriteGpib);
        FT_STATUS ftStatus =writeUsbDev(ftHandle, charArrWriteGpib, bytesWritten);

        if (ftStatus == FT_OK)
        {
            Text = GPIBText;
            Text = "Msg sent: " + Text + " \n " + std::to_string(bytesWritten) + " Bytes Written to GPIB Device\n";
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
        Text = "Failed to Connected";
        FunctionWindow::textFuncOutput->AppendText(terminalTimestampOutput(Text));
    }

}

void FunctionWindow::OnReadGpib(wxCommandEvent& event)
{
    wxString Text;

    wxLogDebug("On Read Pressed");

    if (FunctionWindow::Connected)
    {
        //char Buffer[256];
        std::vector<char> BigBuffer;
        DWORD BufferSize;
        FT_STATUS ftStatus;

        wxLogDebug("Reading from Device...");
        //FT_STATUS ftStatus = readUsbDev(ftHandle, Buffer, BufferSize);
        ftStatus = readUsbDevTest(ftHandle, BigBuffer,BufferSize);

        if (ftStatus == FT_OK)
        {
            //Text = std::string(Buffer);
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
        Text = "Failed to Connected to a Device\n";
        FunctionWindow::textFuncOutput->AppendText(terminalTimestampOutput(Text));
    }

}

void FunctionWindow::OnReadWriteGpib(wxCommandEvent& event)
{
    wxString Text;

    wxLogDebug("On Write / Read Pressed");

    if (FunctionWindow::Connected && FunctionWindow::configFin)
    {
        DWORD bytesWritten;
        char* charArrWriteGpib;

        wxString GPIBText = FunctionWindow::writeFuncInput->GetValue();

        std::string CheckText(GPIBText.ToUTF8());

        charArrWriteGpib = checkAscii(CheckText);

        wxLogDebug("Trying to write to Device... %s", charArrWriteGpib);
        FT_STATUS ftStatus =writeUsbDev(ftHandle, charArrWriteGpib, bytesWritten);

        if (ftStatus == FT_OK)
        {
            Text = charArrWriteGpib;
            Text = "Msg sent: " + Text + " \n " + std::to_string(bytesWritten) + " Bytes Written to GPIB Device\n";
            FunctionWindow::textFuncOutput->AppendText(terminalTimestampOutput(Text));
        }
        else
        {
            Text = "Failed to send Data\n";
            FunctionWindow::textFuncOutput->AppendText(terminalTimestampOutput(Text));
        }

        //read
        usleep(50000);

        //char Buffer[256];
        std::vector<char> BigBuffer;
        DWORD BufferSize;

        wxLogDebug("Reading from Device...");
        //FT_STATUS ftStatus = readUsbDev(ftHandle, Buffer, BufferSize);
        ftStatus = readUsbDevTest(ftHandle, BigBuffer,BufferSize);

        if (ftStatus == FT_OK)
        {
            //Text = std::string(Buffer);
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
        Text = "Failed to Connected to a Device";
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
        wxLogDebug("Baudrate set to: " + std::to_string(BaudRate));
        //wxLogDebug(std::to_string(ftHandle);
        configFin = true;
    }
    else
    {
        Text = "Failed to config device check if run as SU";
        FunctionWindow::textFuncOutput->AppendText(terminalTimestampOutput(Text));
        configFin = false;
    }
}


