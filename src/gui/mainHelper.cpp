#include "mainHelper.h"


bool PlotterMesurement(sData* data, int mesurementNumber)
{
    wxArrayString logAdapterReceived;
    CsvFile csvFile;


    std::vector<double> MessWerteReal;
    std::vector<double> MessWerteImag;

    int x;
    int y;

    Global::AdapterInstance.readScriptFile(System::filePathSystem, "PlotMessung.txt", logAdapterReceived);
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
    if(!csvFile.saveToCsvFile(fileName, *data, mesurementNumber))
    {
        std::cout << "[Error] Failed to save data to CSV file" << std::endl;
    }

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
