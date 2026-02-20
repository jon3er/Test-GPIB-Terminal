#include "FunctionWindow.h"
#include "Mesurement.h"

//----- Function Window Constructor -----
FunctionWindow::FunctionWindow(wxWindow* parent)
    : wxDialog(parent, wxID_ANY, "Function Test Window", wxDefaultPosition, wxSize(500, 750))
{
    wxPanel* panelfunc = new wxPanel(this);

    // Input text label
    wxStaticText* discFuncInput = new wxStaticText(panelfunc, wxID_ANY, "Input text to write: ");

    // Function input textbox
    m_writeFuncInput = new wxTextCtrl(panelfunc, wxID_ANY, "", wxDefaultPosition, wxSize(300, 40));
    m_writeFuncInput->SetFocus();

    // Buttons
    wxButton* writeGpibButton       = new wxButton(panelfunc, wxID_ANY, "Write to GPIB",          wxPoint(10, 0));
    wxButton* readGpibButton        = new wxButton(panelfunc, wxID_ANY, "Read from GPIB",         wxPoint(10, 0));
    wxButton* readWriteGpibButton   = new wxButton(panelfunc, wxID_ANY, "Write and Read GPIB",    wxPoint(10, 0));
    wxButton* scanUsbButton         = new wxButton(panelfunc, wxID_ANY, "Scan For Device",        wxPoint(10, 0));
    wxButton* devConfigButton       = new wxButton(panelfunc, wxID_ANY, "Configure USB Device",   wxPoint(10, 0));
    wxButton* connectDevGpibButton  = new wxButton(panelfunc, wxID_ANY, "Connected / Disconnect", wxPoint(10, 0));
    wxButton* TestSaveFileButton    = new wxButton(panelfunc, wxID_ANY, "Test Save File",         wxPoint(10, 0));
    wxButton* TestMultiMessButton   = new wxButton(panelfunc, wxID_ANY, "Test Multi mesurement",  wxPoint(10, 0));
    wxButton* TestButton            = new wxButton(panelfunc, wxID_ANY, "Test Other",             wxPoint(10, 0));

    // Output area
    wxStaticText* discFuncOutput = new wxStaticText(panelfunc, wxID_ANY, "Function output: ");
    m_textFuncOutput = new wxTextCtrl(panelfunc, wxID_ANY, "", wxDefaultPosition, wxSize(300, 200), wxTE_MULTILINE);

    // Bindings
    writeGpibButton->      Bind(wxEVT_BUTTON, &FunctionWindow::OnWriteGpib,      this);
    readGpibButton->       Bind(wxEVT_BUTTON, &FunctionWindow::OnReadGpib,       this);
    readWriteGpibButton->  Bind(wxEVT_BUTTON, &FunctionWindow::OnReadWriteGpib,  this);
    scanUsbButton->        Bind(wxEVT_BUTTON, &FunctionWindow::OnUsbScan,        this);
    devConfigButton->      Bind(wxEVT_BUTTON, &FunctionWindow::OnUsbConfig,      this);
    connectDevGpibButton-> Bind(wxEVT_BUTTON, &FunctionWindow::OnConDisconGpib,  this);
    TestSaveFileButton->   Bind(wxEVT_BUTTON, &FunctionWindow::OnTestSaveFile,   this);
    TestMultiMessButton->  Bind(wxEVT_BUTTON, &FunctionWindow::OnTestMultiMess,  this);
    TestButton->           Bind(wxEVT_BUTTON, &FunctionWindow::OnTest,           this);

    // Layout
    wxBoxSizer* sizerFunc = new wxBoxSizer(wxVERTICAL);
    sizerFunc->Add(discFuncInput,        0, wxEXPAND | wxALL, 10);
    sizerFunc->Add(m_writeFuncInput,     0, wxEXPAND | wxALL, 10);
    sizerFunc->Add(scanUsbButton,        0, wxEXPAND | wxALL, 10);
    sizerFunc->Add(connectDevGpibButton, 0, wxEXPAND | wxALL, 10);
    sizerFunc->Add(devConfigButton,      0, wxEXPAND | wxALL, 10);
    sizerFunc->Add(writeGpibButton,      0, wxEXPAND | wxALL, 10);
    sizerFunc->Add(readGpibButton,       0, wxEXPAND | wxALL, 10);
    sizerFunc->Add(readWriteGpibButton,  0, wxEXPAND | wxALL, 10);
    sizerFunc->Add(TestSaveFileButton,   0, wxEXPAND | wxALL, 10);
    sizerFunc->Add(TestMultiMessButton,  0, wxEXPAND | wxALL, 10);
    sizerFunc->Add(TestButton,           0, wxEXPAND | wxALL, 10);
    sizerFunc->Add(discFuncOutput,       0, wxEXPAND | wxALL, 10);
    sizerFunc->Add(m_textFuncOutput,     0, wxEXPAND | wxALL, 10);
    panelfunc->SetSizerAndFit(sizerFunc);

    std::cerr << "Function Window Opened" << std::endl;
}

//----- Function Window Destructor -----
FunctionWindow::~FunctionWindow()
{
    // Disconnection is handled by FunctionDocument's destructor.
    std::cerr << "Function Window Closed" << std::endl;
}

//----- Document binding -----
void FunctionWindow::SetDocument(FunctionDocument* document)
{
    if (m_document)
        m_document->RemoveObserver(this);

    m_document = document;

    if (m_document)
    {
        m_document->AddObserver(this);
        UpdateView();
    }
}

//----- IFunctionObserver -----
void FunctionWindow::OnFunctionDocumentChanged(const std::string& changeType)
{
    if (changeType == "OutputAppended")
    {
        if (m_textFuncOutput && m_document)
            m_textFuncOutput->SetValue(wxString::FromUTF8(m_document->GetOutputLog()));
    }
    // "ConnectionChanged" — no dedicated UI element to update yet.
    // Future: update button label or status bar here.
}

void FunctionWindow::UpdateView()
{
    if (!m_document) return;
    if (m_textFuncOutput)
        m_textFuncOutput->SetValue(wxString::FromUTF8(m_document->GetOutputLog()));
}

//----- Event Handlers -----
void FunctionWindow::OnUsbScan(wxCommandEvent& event)
{
    if (m_document) m_document->ScanUsbDevices();
}

void FunctionWindow::OnConDisconGpib(wxCommandEvent& event)
{
    if (m_document) m_document->ConnectDisconnect();
}

void FunctionWindow::OnTestSaveFile(wxCommandEvent& event)
{
    if (m_document) m_document->TestSaveFile();
}

void FunctionWindow::OnWriteGpib(wxCommandEvent& event)
{
    if (!m_document) return;
    wxString GPIBText = m_writeFuncInput->GetValue();
    m_writeFuncInput->SetValue("");
    m_document->WriteToGpib(std::string(GPIBText.ToUTF8()));
}

void FunctionWindow::OnReadGpib(wxCommandEvent& event)
{
    if (m_document) m_document->ReadFromGpib();
}

void FunctionWindow::OnReadWriteGpib(wxCommandEvent& event)
{
    if (!m_document) return;
    wxString GPIBText = m_writeFuncInput->GetValue();
    m_writeFuncInput->SetValue("");
    m_document->ReadWriteGpib(std::string(GPIBText.ToUTF8()));
}

void FunctionWindow::OnUsbConfig(wxCommandEvent& event)
{
    if (m_document) m_document->ConfigureDevice();
}

void FunctionWindow::OnTestMultiMess(wxCommandEvent& event)
{
    // MultiMessDocument owns all multi-measurement state.
    MultiMessDocument multiDoc(PrologixUsbGpibAdapter::get_instance(), fsuMesurement::get_instance());

    MultiMessWindow* MultiMessWin = new MultiMessWindow(this);
    MultiMessWin->SetDocument(&multiDoc);
    MultiMessWin->ShowModal();
    MultiMessWin->SetDocument(nullptr);
    MultiMessWin->Destroy();

    // Log completion note in the function document.
    if (m_document) m_document->TestMultiMeasurement();
}

void FunctionWindow::OnTest(wxCommandEvent& event)
{
    if (m_document) m_document->Test();
}