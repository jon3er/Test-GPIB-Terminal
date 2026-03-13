#include "mainHelper.h"


bool PlotterMesurement(sData* data, int measurementNumber)
{
    wxArrayString logAdapterReceived;
    static CsvFile csvFile;

    std::vector<double> MessWerteReal;
    std::vector<double> MessWerteImag;

    int x;
    int y;

    auto fsu = &fsuMeasurement::get_instance();

    std::cout << "current measurementNumber "<<measurementNumber << std::endl;

    if (measurementNumber == 1)
    {
        //CsvFile csvFileRest;
        //csvFile = csvFileRest;

        data->getFsuSettings();
        int x = data->getNumberOfPts_X();
        int y = data->getNumberOfPts_Y();
        int anz = 625;
        switch (fsu->getMeasurementMode())
        {
        case MeasurementMode::SWEEP:
            anz = data->getNumberOfPts_Array();
            break;
        case MeasurementMode::IQ:
            anz = data->getRecordLength();
            break;
        case MeasurementMode::MARKER_PEAK:
            anz = data->getNumberOfPts_Array();
            break;
        case MeasurementMode::COSTUM:
            // TODO set the Lenght with skript 
        default:
            break;
        }
        

        std::cout << "array size " << "x " <<x  <<"y "<< y << "anz "<< anz << std::endl;
        data->resize3DData(x,y,anz);
    }

    fsu->executeMeasurement();
    // Get last measuremtent results
    std::cout << "get Resultes from fsu" << std::endl;
    MessWerteReal = fsu->getX_Data();
    MessWerteImag = fsu->getY_Data();

    std::cout << "passed resultes" << MessWerteReal[1] << std::endl;


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
