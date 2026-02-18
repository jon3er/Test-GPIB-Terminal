#include "FunctionWindow.h"
#include "systemInfo.h"
#include "Mesurement.h"

//----- Function Window Constructor -----
FunctionWindow::FunctionWindow(wxWindow *parent)
    : wxDialog(parent, wxID_ANY, "Function Test Window", wxDefaultPosition, wxSize(500, 750))
{
    wxPanel* panelfunc = new wxPanel(this);

    // Input text label
    wxStaticText* discFuncInput = new wxStaticText(panelfunc, wxID_ANY, "Input text to write: ");

    // Function input textbox
    m_writeFuncInput = new wxTextCtrl(panelfunc, wxID_ANY, "", wxDefaultPosition, wxSize(300, 40));
    // set Cursor in writeFuncInput window
    m_writeFuncInput->SetFocus();

    // Create Button "Write to GPIB"
    wxButton* writeGpibButton = new wxButton(panelfunc, wxID_ANY, "Write to GPIB", wxPoint(10, 0));
    // Create Button "Read from GPIB"
    wxButton* readGpibButton = new wxButton(panelfunc, wxID_ANY, "Read from GPIB", wxPoint(10, 0));
    // Create Button "Write and Read GPIB"
    wxButton* readWriteGpibButton = new wxButton(panelfunc, wxID_ANY, "Write and Read GPIB", wxPoint(10, 0));
    // Create Button "Scan For Device"
    wxButton* scanUsbButton = new wxButton(panelfunc, wxID_ANY, "Scan For Device", wxPoint(10, 0));
    // Create Button "Configure USB Device"
    wxButton* devConfigButton = new wxButton(panelfunc, wxID_ANY, "Configure USB Device", wxPoint(10, 0));
    // Create Button "Connect / Disconnect"
    wxButton* connectDevGpibButton = new wxButton(panelfunc, wxID_ANY, "Connected / Disconnect", wxPoint(10, 0));
    // Create Button "Test Save File"
    wxButton* TestSaveFileButton = new wxButton(panelfunc, wxID_ANY, "Test Save File", wxPoint(10, 0));
    // Create Button "Test Multi Mesurement"
    wxButton* TestMultiMessButton = new wxButton(panelfunc, wxID_ANY, "Test Multi mesurement", wxPoint(10, 0));
    // Create Button "Test Other"
    wxButton* TestButton = new wxButton(panelfunc, wxID_ANY, "Test Other", wxPoint(10, 0));

    // Function Output Label
    wxStaticText* discFuncOutput = new wxStaticText(panelfunc, wxID_ANY, "Function output: ");
    // Function Output Text Box
    m_textFuncOutput = new wxTextCtrl(panelfunc, wxID_ANY, "", wxDefaultPosition, wxSize(300, 200), wxTE_MULTILINE);

    // Function bindings
    writeGpibButton->       Bind(wxEVT_BUTTON, &FunctionWindow::OnWriteGpib, this);
    readGpibButton->        Bind(wxEVT_BUTTON, &FunctionWindow::OnReadGpib, this);
    readWriteGpibButton->   Bind(wxEVT_BUTTON, &FunctionWindow::OnReadWriteGpib, this);
    scanUsbButton->         Bind(wxEVT_BUTTON, &FunctionWindow::OnUsbScan, this);
    devConfigButton->       Bind(wxEVT_BUTTON, &FunctionWindow::OnUsbConfig, this);
    connectDevGpibButton->  Bind(wxEVT_BUTTON, &FunctionWindow::OnConDisconGpib, this);
    TestSaveFileButton->    Bind(wxEVT_BUTTON, &FunctionWindow::OnTestSaveFile, this);
    TestMultiMessButton->   Bind(wxEVT_BUTTON, &FunctionWindow::OnTestMultiMess, this);
    TestButton->            Bind(wxEVT_BUTTON, &FunctionWindow::OnTest, this);

    // Sizer - Set Window Layout
    wxBoxSizer* sizerFunc = new wxBoxSizer(wxVERTICAL);
    sizerFunc->Add(discFuncInput, 0, wxEXPAND | wxALL, 10);
    sizerFunc->Add(m_writeFuncInput, 0, wxEXPAND | wxALL, 10);
    sizerFunc->Add(scanUsbButton, 0, wxEXPAND | wxALL, 10);
    sizerFunc->Add(connectDevGpibButton, 0, wxEXPAND | wxALL, 10);
    sizerFunc->Add(devConfigButton, 0, wxEXPAND | wxALL, 10);
    sizerFunc->Add(writeGpibButton, 0, wxEXPAND | wxALL, 10);
    sizerFunc->Add(readGpibButton, 0, wxEXPAND | wxALL, 10);
    sizerFunc->Add(readWriteGpibButton, 0, wxEXPAND | wxALL, 10);
    sizerFunc->Add(TestSaveFileButton, 0, wxEXPAND | wxALL, 10);
    sizerFunc->Add(TestMultiMessButton, 0, wxEXPAND | wxALL, 10);
    sizerFunc->Add(TestButton, 0, wxEXPAND | wxALL, 10);

    sizerFunc->Add(discFuncOutput, 0, wxEXPAND | wxALL, 10);
    sizerFunc->Add(m_textFuncOutput, 0, wxEXPAND | wxALL, 10);
    panelfunc->SetSizerAndFit(sizerFunc);

    std::cerr << "Function Window Opened" << std::endl;

    // Set controller's output callback to update display
    m_FunctionLogic.setOutputCallback([this](const std::string& output) {
        m_textFuncOutput->AppendText(formatOutput(output));
    });
}

//----- Function Window Destructor -----
FunctionWindow::~FunctionWindow()
{
    if (Global::AdapterInstance.getConnected() == true)
    {
        Global::AdapterInstance.disconnect();
    }
    std::cerr << "Function Window Closed" << std::endl;
}

//----- Helper Methods -----
wxString FunctionWindow::formatOutput(const std::string& text)
{
    return terminalTimestampOutput(text);
}

//----- Event Handlers -----
void FunctionWindow::OnUsbScan(wxCommandEvent& event)
{
    std::string result = m_FunctionLogic.scanUsbDevices();
    m_textFuncOutput->AppendText(formatOutput(result));
}
void FunctionWindow::OnConDisconGpib(wxCommandEvent& event)
{
    std::string result = m_FunctionLogic.connectDisconnect();
    m_textFuncOutput->AppendText(formatOutput(result));
}
void FunctionWindow::OnTestSaveFile(wxCommandEvent& event)
{
    std::string result = m_FunctionLogic.testSaveFile();
    m_textFuncOutput->AppendText(formatOutput(result));
}
void FunctionWindow::OnWriteGpib(wxCommandEvent& event)
{
    wxString GPIBText = m_writeFuncInput->GetValue();
    m_writeFuncInput->SetValue("");

    std::string input(GPIBText.ToUTF8());
    std::string result = m_FunctionLogic.writeToGpib(input);
    m_textFuncOutput->AppendText(formatOutput(result));
}
void FunctionWindow::OnReadGpib(wxCommandEvent& event)
{
    std::string result = m_FunctionLogic.readFromGpib();
    m_textFuncOutput->AppendText(formatOutput(result));
}
void FunctionWindow::OnReadWriteGpib(wxCommandEvent& event)
{
    wxString GPIBText = m_writeFuncInput->GetValue();
    m_writeFuncInput->SetValue("");

    std::string input(GPIBText.ToUTF8());
    std::string result = m_FunctionLogic.readWriteGpib(input);
    m_textFuncOutput->AppendText(formatOutput(result));
}
void FunctionWindow::OnUsbConfig(wxCommandEvent& event)
{
    std::string result = m_FunctionLogic.configureDevice();
    m_textFuncOutput->AppendText(formatOutput(result));
}
void FunctionWindow::OnTestMultiMess(wxCommandEvent& event)
{
    // Create new sub window
    MultiMessWindow *MultiMessWin = new MultiMessWindow(this);
    // open Window Pauses Main Window
    MultiMessWin->ShowModal();
    // Close Window
    MultiMessWin->Destroy();

    std::string result = m_FunctionLogic.testMultiMeasurement();
    m_textFuncOutput->AppendText(formatOutput(result));
}
void FunctionWindow::OnTest(wxCommandEvent& event)
{
    std::string result = m_FunctionLogic.test();
    m_textFuncOutput->AppendText(formatOutput(result));
}