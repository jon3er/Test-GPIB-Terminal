#include "FunctionDocument.h"
#include "systemInfo.h"
#include "mainHelper.h"   // sleepMs, dataManagement; wx headers are transitive

#include <ctime>
#include <iostream>
#include <random>

// --------------------------------------------------------------------------
// Internal helpers
// --------------------------------------------------------------------------
namespace
{
    /** Returns a "[HH:MM:SS] " prefix using only <ctime>. */
    std::string funcTimestamp()
    {
        std::time_t t = std::time(nullptr);
        char buf[16] = {};
        std::strftime(buf, sizeof(buf), "[%H:%M:%S] ", std::localtime(&t));
        return std::string(buf);
    }
}

// --------------------------------------------------------------------------
// Construction / destruction
// --------------------------------------------------------------------------
FunctionDocument::FunctionDocument(PrologixUsbGpibAdapter& adapter)
    : m_adapter(adapter)
{
}

FunctionDocument::~FunctionDocument()
{
    if (m_adapter.getConnected())
    {
        m_adapter.disconnect();
    }
}

// --------------------------------------------------------------------------
// Observer management
// --------------------------------------------------------------------------
void FunctionDocument::AddObserver(IFunctionObserver* observer)
{
    m_observers.push_back(observer);
}

void FunctionDocument::RemoveObserver(IFunctionObserver* observer)
{
    m_observers.erase(
        std::remove(m_observers.begin(), m_observers.end(), observer),
        m_observers.end());
}

void FunctionDocument::NotifyObservers(const std::string& changeType)
{
    for (IFunctionObserver* obs : m_observers)
    {
        obs->OnFunctionDocumentChanged(changeType);
    }
}

// --------------------------------------------------------------------------
// State accessors
// --------------------------------------------------------------------------
bool FunctionDocument::IsConnected() const
{
    return m_adapter.getConnected();
}

// --------------------------------------------------------------------------
// Internal log helper
// --------------------------------------------------------------------------
void FunctionDocument::AppendToLog(const std::string& text)
{
    m_outputLog += funcTimestamp() + text;
}

// --------------------------------------------------------------------------
// GPIB command implementations
// --------------------------------------------------------------------------
void FunctionDocument::WriteToGpib(const std::string& cmd)
{
    m_lastCommand = cmd;
    std::cerr << "Write to GPIB" << std::endl;
    std::string result = m_adapter.write(cmd);
    AppendToLog(result);
    NotifyObservers("OutputAppended");
}

void FunctionDocument::ReadFromGpib()
{
    std::cerr << "Read from GPIB" << std::endl;
    std::string result = m_adapter.read();
    AppendToLog(result);
    NotifyObservers("OutputAppended");
}

void FunctionDocument::ReadWriteGpib(const std::string& cmd)
{
    m_lastCommand = cmd;
    std::cerr << "Read/Write GPIB operation started" << std::endl;

    std::cerr << "Writing to device..." << std::endl;
    AppendToLog(m_adapter.write(cmd));

    sleepMs(100);   // wait for response

    std::cerr << "Reading from device..." << std::endl;
    AppendToLog(m_adapter.read());

    NotifyObservers("OutputAppended");
}

void FunctionDocument::ScanUsbDevices()
{
    std::cerr << "Scan USB Devices" << std::endl;
    DWORD devices = scanUsbDev();

    if (devices <= 0)
        AppendToLog("no device found\n");
    else
        AppendToLog(std::to_string(devices) + " Devices Found\n");

    NotifyObservers("OutputAppended");
}

void FunctionDocument::ConfigureDevice()
{
    m_adapter.config();

    if (m_adapter.getStatus() == FT_OK)
    {
        AppendToLog("Set Default config\n");
        AppendToLog(m_adapter.statusText());
    }
    else
    {
        AppendToLog("Config failed\n");
    }
    NotifyObservers("OutputAppended");
}

void FunctionDocument::ConnectDisconnect()
{
    if (!m_adapter.getConnected())
    {
        m_adapter.connect();
        AppendToLog(m_adapter.getStatus() == FT_OK
                        ? "Connected to a device\n"
                        : "Connection operation failed\n");
    }
    else
    {
        m_adapter.disconnect();
        AppendToLog(m_adapter.getStatus() == FT_OK
                        ? "Disconnected from a device\n"
                        : "Connection operation failed\n");
    }
    NotifyObservers("OutputAppended");
    NotifyObservers("ConnectionChanged");
}

// --------------------------------------------------------------------------
// Test operations
// --------------------------------------------------------------------------
void FunctionDocument::TestSaveFile()
{
    std::cerr << "Test Save File operation started" << std::endl;

    sData TestObjekt;
    sData TestObjekt2;
    CsvFile csvFile1;

    int xpt      = 10;
    int ypt      = 10;
    int count    = 625;
    int endFreq  = 50'000;

    TestObjekt.setTimeAndDate();
    TestObjekt.setNumberOfPts_X(xpt);
    TestObjekt.setNumberOfPts_Y(ypt);
    TestObjekt.setEndFreq(endFreq);

    std::vector<double> TestArray;
    // Generates random values
    std::random_device rd; // obtain a random number from hardware
    std::mt19937 gen(rd()); // seed the generator
    std::uniform_real_distribution<> distr(0.0, 100.0); // define the range

    for (int i = 0; i < count; i++)
    {
        try   { TestArray.push_back(double(distr(gen))); }
        catch (const std::exception& e)
              { std::cerr << "pushback failed: " << e.what() << '\n'; }
    }

    std::cout << "TestArray Ok | count: " << count
              << " | size: " << TestArray.size() << std::endl;
    TestObjekt.setNumberofPts_Array(count);

    for (int i = 0; i < xpt; i++)
    {
        for (int j = 0; j < ypt; j++)
        {
            try { TestObjekt.set3DDataReal(TestArray, i, j); }
            catch (const std::exception& e)
            {
                std::cerr << "Set 3D Data failed: " << e.what()
                          << " at [" << i << "," << j << "]" << std::endl;
            }
        }
    }

    sData::sParam* TestData = TestObjekt.GetParameter();
    std::cerr << "Zeit: " << TestData->Time << std::endl;

    wxString Dateiname = "D:\\CodeProjects\\VSCode\\projects\\Diplom\\Test-GPIB-Terminal\\LogFiles\\TestCSVNeu";

    int messungen = TestObjekt.getNumberOfPts_X() * TestObjekt.getNumberOfPts_Y();
    for (int i = 1; i <= messungen; i++)
    {
        if (!csvFile1.saveToCsvFile(Dateiname, TestObjekt, i))
            std::cerr << "Failed to save file" << std::endl;
    }

    csvFile1.readCsvFile(Dateiname, TestObjekt2);

    wxString Dateiname2 = "D:\\CodeProjects\\VSCode\\projects\\Diplom\\Test-GPIB-Terminal\\LogFiles\\TestCSVNeuKopie";

    int totalpoints = TestObjekt2.getNumberOfPts_X() * TestObjekt2.getNumberOfPts_Y();
    std::cout << "[Debug] Totalpoints: " << totalpoints << std::endl;

    TestObjekt2.setFileName("kopie");
    TestObjekt2.setTimeAndDate();

    for (int i = 1; i <= totalpoints; i++)
    {
        if (!csvFile1.saveToCsvFile(Dateiname2, TestObjekt2, i))
            std::cerr << "Failed to save file" << std::endl;
    }

    AppendToLog("Test Save File completed\n");
    NotifyObservers("OutputAppended");
}

void FunctionDocument::TestMultiMeasurement()
{
    std::cerr << "Test Multi Measurement operation started" << std::endl;
    // MultiMessWindow is launched directly by the view; just log a note here.
    AppendToLog("Multi measurement window opened\n");
    NotifyObservers("OutputAppended");
}

void FunctionDocument::Test()
{
    std::cerr << "Test wxLogDebug" << std::endl;
    std::cerr << "Test cerr"       << std::endl;
    std::cout << "Test cout"       << std::endl;
    AppendToLog("Test operation completed\n");
    NotifyObservers("OutputAppended");
}
