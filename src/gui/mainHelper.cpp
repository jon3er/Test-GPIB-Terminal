#include "mainHelper.h"


bool PlotterMesurement(sData* data, int measurementNumber)
{
    wxArrayString logAdapterReceived;
    CsvFile csvFile;

    std::vector<double> MessWerteReal;
    std::vector<double> MessWerteImag;

    int x;
    int y;

    auto fsu = &fsuMeasurement::get_instance();

    fsu->executeMeasurement();
    // Get last measuremtent results
    MessWerteReal = fsu->getX_Data();
    MessWerteImag = fsu->getY_Data();


    data->getXYCord(x, y, measurementNumber);
    // save lates data in 3d array
    data->set3DDataReal(MessWerteReal, x, y);
    data->set3DDataImag(MessWerteImag, x, y);

    // save data to a Csv file
    wxString fileName = System::filePathRoot + "LogFiles" + System::fileSystemSlash +
             data->GetFile() + "_" + data->GetDate() + "_" + data->GetTime() +".csv";
    if(!csvFile.saveCsvFile(fileName, *data, measurementNumber))
    {
        std::cout << "[Error] Failed to save data to CSV file" << std::endl;
    }

    std::cout << "Measuremnt done" << std::endl;

    return true; // Mesurement Finished
}

wxString terminalTimestampOutput(wxString Text)
{
    //Set Terminal Output Format
    wxDateTime zeitJetzt = wxDateTime::Now();
    wxString timestamp = zeitJetzt.Format("%H:%M:%S");
    wxString FormatText = "[" + timestamp + "] " + Text;

    return FormatText;
}

void sleepMs(int timeMs)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(timeMs));
}
