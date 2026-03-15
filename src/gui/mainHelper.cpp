#include "mainHelper.h"


bool PlotterMeasurement(sData* data, int measurementNumber)
{
    wxArrayString logAdapterReceived;
    static wxString fileName; // Keeps Filename the same;
    static CsvFile csvFile;   // Keeps Lookuptables persitient
    wxString TypeText;

    std::vector<double> MessWerteReal;
    std::vector<double> MessWerteImag;

    int x;
    int y;

    auto fsu = &fsuMeasurement::get_instance();

    std::cout << "current measurementNumber "<<measurementNumber << std::endl;

    data->setMeasurementNumb(measurementNumber);

    if (measurementNumber == 1)
    {
        //CsvFile csvFileRest;
        //csvFile = csvFileRest;

        data->importFsuSettings();

        int x = data->getNumberOfPts_X();
        int y = data->getNumberOfPts_Y();
        int anz = 625;

        switch (fsu->getMeasurementMode())
        {
        case MeasurementMode::SWEEP:
            anz = data->getNumberOfPts_Array();
            std::cout << " Mode: SWEEP" << std::endl;
            TypeText = "Sweep";
            break;
        case MeasurementMode::IQ:
            anz = data->getRecordLength();
            TypeText = "IQ";
            std::cout << " Mode: IQ" << std::endl;
            break;
        case MeasurementMode::MARKER_PEAK:
            anz = 1;
            // data->setNumberOfPts_Array(1);
            TypeText = "Marker";
            std::cout << " Mode: MARKER" << std::endl;
            break;
        case MeasurementMode::COSTUM:
            TypeText = "Costum";
            // TODO set the Lenght with skript
        default:
            break;
        }


        // save data to a Csv file
        fileName = System::filePathRoot + "LogFiles" + System::fileSystemSlash +
             data->GetFile() + "_" + TypeText + "_" + data->GetDate() + "_" + data->GetTime() +".csv";

        std::cout << "array size " << "x " <<x  <<"y "<< y << "anz "<< anz << std::endl;
        data->resize3DData(x,y,anz);
    }

    if (!fsu->executeMeasurement())
    {
        std::cout << "[Error] executeMeasurement failed" << std::endl;
        return false;
    }
    // Get last measuremtent results
    std::cout << "get Resultes from fsu" << std::endl;
    MessWerteReal = fsu->getX_Data();
    MessWerteImag = fsu->getY_Data();

    if (!MessWerteReal.empty())
    {
        std::cout << "passed resultes " << MessWerteReal.front() << std::endl;
    }
    else
    {
        std::cout << "[Error] No measurement values returned" << std::endl;
        return false;
    }


    data->getXYCord(x, y, measurementNumber);
    // save lates data in 3d array
    data->set3DDataReal(MessWerteReal, x, y);
    data->set3DDataImag(MessWerteImag, x, y);

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
