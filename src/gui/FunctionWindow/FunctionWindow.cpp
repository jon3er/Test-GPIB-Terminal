#include "FunctionWindow.h"
#include "systemInfo.h"
#include "Mesurement.h"

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