#include <wx/wx.h>
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


    }
private:
    void OnEnterTerminal(wxCommandEvent& event);

    //com Settings
    int BaudRate = 12000;
    //data recive var
    DWORD BytesToRead = 1024;
    DWORD BytesReturned;
    //com Var
    FT_HANDLE ftHandle;
    FT_STATUS ftStatus;

    bool DevConnected;
    wxStaticText* StaticTE;
    wxTextCtrl* TerminalDisplay;
};

class FunctionWindow : public wxDialog
{
public:
    FunctionWindow(wxWindow *parent) : wxDialog(parent, wxID_ANY, "Function Test Window", wxDefaultPosition, wxSize(400,600))
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


        wxStaticText* discFuncOutput = new wxStaticText(panelfunc,wxID_ANY,"Function output: ");
        // Text Log
        textFuncOutput = new wxTextCtrl(panelfunc, wxID_ANY,"",wxDefaultPosition,wxSize(300, 200), wxTE_MULTILINE);



        //sizer
        wxBoxSizer* sizerFunc = new wxBoxSizer(wxVERTICAL);
        sizerFunc->Add(discFuncInput, 0, wxEXPAND | wxALL , 10);
        sizerFunc->Add(writeFuncInput, 0, wxEXPAND | wxALL , 10);
        sizerFunc->Add(writeGpibButton, 0, wxEXPAND | wxALL , 10);
        sizerFunc->Add(readGpibButton, 0, wxEXPAND | wxALL , 10);
        sizerFunc->Add(readWriteGpibButton, 0, wxEXPAND | wxALL , 10);
        sizerFunc->Add(scanUsbButton, 0, wxEXPAND | wxALL , 10);
        sizerFunc->Add(devConfigButton, 0, wxEXPAND | wxALL , 10);
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
    //
    wxTextCtrl* textFuncOutput;
    wxTextCtrl* writeFuncInput;

    bool DeviceFound;
    FT_HANDLE ftHandle;
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

void TerminalWindow::OnEnterTerminal(wxCommandEvent& event)
{
    wxTextCtrl* Terminal = static_cast<wxTextCtrl*>(event.GetEventObject());
    wxString TText = Terminal->GetValue();
    Terminal->SetValue("");

    wxLogDebug("user entered: %s", TText.c_str());

    int testRes = checkCMD(TText);
    TText = TText + "\n";
    //Output to terminal
    TerminalDisplay->AppendText(terminalTimestampOutput(TText));

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
        DeviceFound = false;
    }
    else
    {
        textFuncOutput->AppendText(terminalTimestampOutput(deviceNumString));
        DeviceFound = true;
    }
}


void FunctionWindow::OnWriteGpib(wxCommandEvent& event)
{
    /*
    wxLogDebug("Writing entered text to device");

    wxString TextBox = FunctionWindow::writeFuncInput->GetValue();

    wxString Text = "trying to write: \"" + TextBox + "\" to the device \n" ;

    FunctionWindow::textFuncOutput->AppendText(terminalTimestampOutput(Text));
    */

    wxString Text;


    wxLogDebug("On Write Pressed");

    if (DeviceFound)
    {
        DWORD bytesWritten;

        wxString GPIBText = FunctionWindow::writeFuncInput->GetValue();

        std::string CheckText(GPIBText.ToUTF8());

        GPIBText = checkAscii(CheckText);

        wxLogDebug("Trying to write to Device...");
        FT_STATUS ftStatus =writeUsbDev(ftHandle, GPIBText, bytesWritten);

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
        wxLogDebug("No Device to send too");
        Text = "Failed to Connected to a Device\n";
        FunctionWindow::textFuncOutput->AppendText(terminalTimestampOutput(Text));
    }

}

void FunctionWindow::OnReadGpib(wxCommandEvent& event)
{
    wxString Text;

    wxLogDebug("On Read Pressed");

    if (DeviceFound)
    {
        char* Buffer;
        DWORD BufferSize;

        FT_SetTimeouts(ftHandle, 5000,0);
        wxLogDebug("Reading from Device...");
        FT_STATUS ftStatus = readUsbDev(ftHandle, Buffer, BufferSize);

        if (ftStatus == FT_OK)
        {
            Text = std::string(Buffer);
            Text = "Msg received: " + Text + "\n";
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

void FunctionWindow::OnReadWriteGpib(wxCommandEvent& event)
{
    char charInput[] = {104 ,01 ,02 ,13 ,96 ,10 ,100 ,27 ,05 ,43 ,06, 111};
    //char charInput[] = {104 ,97 ,27 ,108 ,108 ,111};
    int leng = sizeof(charInput);

    for (int i=0 ;i<leng ; i++ )
    {
        wxLogDebug(std::to_string(charInput[i]));
    }


    std::string inputString(charInput, sizeof(charInput));



    std::string checkedString = checkAscii("test+einfach");

    wxLogDebug(inputString);
    std::string Text = checkedString + " " + std::to_string(checkedString.length());
    wxLogDebug(Text);
}

void FunctionWindow::OnUsbConfig(wxCommandEvent& event)
{
 /*
    if (testRes)
    {
        wxString onlyCmd = sendGPIBcmd(TText,4);
        if (onlyCmd == "connect")
        {
            DWORD numDev = scanUsbDev();

                if (devices > 0)
                {
                    if (devices > 1)
                    {
                        wxLogDebug("More than one Device found!");
                    }
                    else
                    {
                        wxLogDebug("One Device found!");

                        ftStatus = configUsbDev(numDev, &ftHandle,BaudRate);
                    }

                    if (ftStatus != FT_OK)
                    {
                        wxLogDebug("Failed to open Device!");
                        ftStatus = FT_Close(ftHandle);
                        DevConnected = false;
                    }
                    else
                    {
                        wxLogDebug("Device Connected!");
                        TerminalWindow::DevConnected = true;
                    }
                }
        }
        sendGPIBcmd(TText,"cmd ");

        if (DevConnected && checkCMDinput(TText, "cmd write"))
        {
            wxString cmdText = sendGPIBcmd(TText,9);

            ftStatus = writeUsbDev(ftHandle, cmdText);

            if (ftStatus != FT_OK)
            {
                wxLogDebug("Failed to write to Device!");
                ftStatus = FT_Close(ftHandle);
                DevConnected = false;
            }
            else
            {
                wxLogDebug("msg Writen!")
                DevConnected = true;

                ftStatus = readUsbDev(ftHandle,*RPBuffer,BytesToRead,*BytesReturned);
                if (ftStatus != FT_OK)
                {
                    wxLogDebug("no msg Recived!");
                    ftStatus = FT_Close(ftHandle);
                    DevConnected = false;
                }
                else
                {
                    string msgRes = RPBuffer[BytesReturned];
                    wxLogDebug("msg resived: %s",msgRes);
                }
            }
        }

    }
    */

    DWORD numDev = 0;
    int BaudRate = 12000;
    wxString Text;

    FT_STATUS Status = configUsbDev(numDev, ftHandle, BaudRate);

    if (Status == FT_OK)
    {
        Text = "Set Device BaudRate to " + std::to_string(BaudRate);
        FunctionWindow::textFuncOutput->AppendText(terminalTimestampOutput(Text));
        wxLogDebug("Baudrate set to: " + std::to_string(BaudRate));
        //wxLogDebug(std::to_string(ftHandle);
        DeviceFound = true;
    }
    else
    {
        Text = "Failed to config device check if run as SU";
        FunctionWindow::textFuncOutput->AppendText(terminalTimestampOutput(Text));
        DeviceFound = false;
    }
}


