#include "FunctionController.h"
#include "systemInfo.h"
#include "Mesurement.h"
#include <thread>

FunctionController::FunctionController()
{
}

FunctionController::~FunctionController()
{
}

void FunctionController::setOutputCallback(OutputCallback callback)
{
    m_outputCallback = callback;
}

void FunctionController::output(const std::string& text)
{
    if (m_outputCallback != nullptr)
    {
        // updates text display and appends line
        m_outputCallback(text);
    }
}

std::string FunctionController::writeToGpib(const std::string& data)
{
    std::cerr << "Write to GPIB" << std::endl;
    std::string text = Global::AdapterInstance.write(data);
    return text;
}

std::string FunctionController::readFromGpib()
{
    std::cerr << "Read from GPIB" << std::endl;
    std::string text = Global::AdapterInstance.read();
    return text;
}

std::string FunctionController::readWriteGpib(const std::string& data)
{
    std::cerr << "Read/Write GPIB operation started" << std::endl;
    
    std::cerr << "Writing to device..." << std::endl;
    std::string writeResult = Global::AdapterInstance.write(data);
    output(writeResult);
    
    sleepMs(100);   // wait for response
    
    std::cerr << "Reading from device..." << std::endl;
    std::string readResult = Global::AdapterInstance.read();
    
    return readResult;
}

std::string FunctionController::scanUsbDevices()
{
    std::cerr << "Scan USB Devices" << std::endl;
    DWORD devices = scanUsbDev();
    
    if (devices <= 0)
    {
        return "no device found\n";
    }
    else
    {
        return std::to_string(devices) + " Devices Found\n";
    }
}

std::string FunctionController::configureDevice()
{
    Global::AdapterInstance.config();

    if (Global::AdapterInstance.getStatus() == FT_OK)
    {
        output("Set Default config\n");
        return Global::AdapterInstance.statusText();
    }
    else
    {
        return "Config failed\n";
    }
}

std::string FunctionController::connectDisconnect()
{
    if (Global::AdapterInstance.getConnected() == false)
    {
        Global::AdapterInstance.connect();

        if (Global::AdapterInstance.getStatus() == FT_OK)
        {
            return "Connected to a device\n";
        }
    }
    else
    {
        Global::AdapterInstance.disconnect();

        if (Global::AdapterInstance.getStatus() == FT_OK)
        {
            return "Disconnected from a device\n";
        }
    }
    return "Connection operation failed\n";
}

std::string FunctionController::testSaveFile()
{
    std::cerr << "Test Save File operation started" << std::endl;
    
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
                TestObjekt.set3DDataReal(TestArray, i, j);
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
    
    return "Test Save File completed\n";
}

std::string FunctionController::testMultiMeasurement()
{
    std::cerr << "Test Multi Measurement operation started" << std::endl;
    // This is handled by GUI directly with MultiMessWindow dialog
    return "Multi measurement window opened\n";
}

std::string FunctionController::test()
{
    std::cerr << "Test wxLogDebug" << std::endl;
    std::cerr << "Test cerr" << std::endl;
    std::cout << "Test cout" << std::endl;
    return "Test operation completed\n";
}
