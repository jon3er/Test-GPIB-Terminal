#pragma once

#include <wx/wx.h>


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

    //menubar methodes
    void OnHello(wxCommandEvent& event);
    void OnExit(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);
    //buttons methodes
    void OnOpenTerminal(wxCommandEvent& event);
    void OnScanUsb(wxCommandEvent& event);
    void OnOpenFunctionTest(wxCommandEvent& event);
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
    void scanDevices(const std::string& args);
    void statusDevice(const std::string& args);
    void configDevice(const std::string& args);
    void connectDevice(const std::string& args);
    void disconnectDevice(const std::string& args);
    wxString sendToDevice(const std::string& args);
    wxString readFromDevice(const std::string& args);
    void writeToDevice(const std::string& args);
    void testDevice(const std::string& args);

    //Terminal Output
    wxTextCtrl* TerminalDisplay;

    //com default settings
    int BaudRate = 921600;
    //Device Handle
    FT_HANDLE ftHandle = NULL;
    bool Connected = false;
    bool configFin = false;
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

    //Text Boxes
    wxTextCtrl* textFuncOutput;
    wxTextCtrl* writeFuncInput;

    //Class Variables
    bool configFin = false;
    bool Connected = false;
    FT_HANDLE ftHandle = NULL;
};
