#include "mainHelper.h"


bool PlotterMesurement(sData* data, int mesurementNumber)
{
    wxArrayString logAdapterReceived;

    std::vector<double> MessWerteReal;
    std::vector<double> MessWerteImag;

    int x;
    int y;

    Global::AdapterInstance.readScriptFile(System::filePathSystem, "Plot_Messung.txt", logAdapterReceived);
    // Output received messages to debug log
    for (size_t i = 0; i < logAdapterReceived.GetCount(); i++)
    {
        wxString text = logAdapterReceived[i];

        std::cerr << text << std::endl;
    }

    
    MessWerteReal = Global::Messung.getX_Data();

    data->getXYCord(x, y, mesurementNumber);

    data->set3DDataReal(MessWerteReal, x, y);
    if (Global::Messung.isImagValues())
    {
        data->set3DDataImag(MessWerteImag, x, y);
    }

    // save data to a Csv file
    wxString fileName = System::filePathRoot + "LogFiles" + System::fileSystemSlash + "PlotterMessung.csv";
    if(!saveToCsvFile(fileName, *data, mesurementNumber))
    {
        std::cout << "[Error] Failed to save data to CSV file" << std::endl;
    }

    return true; // Mesurement Finished

}