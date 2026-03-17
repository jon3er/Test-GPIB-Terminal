
#include "MeasurementDocument.h"
#include "FsuMeasurement.h"
#include "systemInfo.h"
#include "dataManagement.h"
#include "CsvManagement.h"
#include "cmdGpib.h"

#include <wx/wx.h>
#include <wx/textfile.h>

#include <iostream>
#include <algorithm>


// Construction / Destruction
MeasurementDocument::MeasurementDocument(PrologixUsbGpibAdapter& adapter,
                                         fsuMeasurement& messung)
    : m_adapter(adapter)
    , m_messung(messung)
{
}

MeasurementDocument::~MeasurementDocument()
{
    StopMeasurement();
    // Do NOT disconnect the shared adapter singleton here.
    // Multiple MeasurementDocuments may share the same adapter.
    // The adapter lifecycle is managed at the application level.
}

// Observer management

void MeasurementDocument::AddObserver(IMeasurementObserver* observer)
{
    if (observer &&
        std::find(m_observers.begin(), m_observers.end(), observer) == m_observers.end())
    {
        m_observers.push_back(observer);
    }
}

void MeasurementDocument::RemoveObserver(IMeasurementObserver* observer)
{
    m_observers.erase(
        std::remove(m_observers.begin(), m_observers.end(), observer),
        m_observers.end());
}

void MeasurementDocument::NotifyObservers(const std::string& changeType)
{
    for (IMeasurementObserver* obs : m_observers)
    {
        if (obs)
            obs->OnDocumentChanged(changeType);
    }
}

// Commands
// Only executes one measurement
void MeasurementDocument::StartMeasurement(const std::string& dirPath,
                                           const std::string& scriptName,
                                           int                measurementNumber)
{
    if (m_thread.joinable())
    {
        std::cerr << "MeasurementDocument: thread already running, ignoring StartMeasurement" << std::endl;
        return;
    }

    m_measurementNumber = measurementNumber;
    m_stopFlag  = false;
    m_measuring = true;

    NotifyObservers("MeasurementStarted");

    m_thread = std::thread(&MeasurementDocument::WorkerThread,
                           this, dirPath, scriptName, 1);
}

void MeasurementDocument::StopMeasurement()
{
    if (m_thread.joinable())
    {
        std::cerr << "MeasurementDocument: stopping thread..." << std::endl;
        m_stopFlag = true;
        m_thread.join();
        std::cerr << "MeasurementDocument: thread joined" << std::endl;
    }
    m_measuring = false;
}

void MeasurementDocument::SetResults(const sData& results)
{
    m_results = results;
    NotifyObservers("DataUpdated");
}

void MeasurementDocument::WriteMarker1(bool setToMax, const std::string& freqRaw)
{
    if (setToMax)
    {
        std::string cmd = ScpiCmdLookup.at(ScpiCmd::CALC_MARK_MAX);
        m_adapter.write(cmd);
        NotifyObservers("MarkerUpdated");
    }
    else if (!freqRaw.empty())
    {
        std::string cmd = ScpiCmdLookup.at(ScpiCmd::CALC_MARK_MAX) + " " + freqRaw;
        m_adapter.write(cmd);
        NotifyObservers("MarkerUpdated");
    }
}

void MeasurementDocument::WriteMarker2(bool setToMax, const std::string& freqRaw)
{
    if (setToMax)
    {
        m_adapter.write(std::string("CALC:MARK2:MAX"));
        NotifyObservers("MarkerUpdated");
    }
    else if (!freqRaw.empty())
    {
        m_adapter.write(std::string("CALC:MARK2:MAX ") + freqRaw);
        NotifyObservers("MarkerUpdated");
    }
}


// Worker thread
void MeasurementDocument::WorkerThread(const std::string& dirPath,
                                       const std::string& scriptName,
                                       int                measurementNumber)
{
    std::cout << "MeasurementDocument: worker thread started" << std::endl;
    CsvFile csvFile(m_csvSeparator);
    static wxString fileName;

    try
    {
        wxString wxDirPath    = wxString::FromUTF8(dirPath);
        wxString wxScriptName = wxString::FromUTF8(scriptName);

        if (measurementNumber == 1)
        {
            m_results.importFsuSettings();

            int pointsPerMeasurement = 1;
            wxString typeText = "Measurement";
            switch (m_messung.getMeasurementMode())
            {
            case MeasurementMode::SWEEP:
                pointsPerMeasurement = std::max(1, m_results.getNumberOfPts_Array());
                typeText = "Sweep";
                break;
            case MeasurementMode::IQ:
                pointsPerMeasurement = std::max(1, m_results.getRecordLength());
                typeText = "IQ";
                break;
            case MeasurementMode::MARKER_PEAK:
                pointsPerMeasurement = 1;
                typeText = "Marker";
                break;
            case MeasurementMode::COSTUM:
                pointsPerMeasurement = std::max(1, m_results.getNumberOfPts_Array());
                typeText = "Costum";
                break;
            }

            m_results.setNumberofPts_Array(pointsPerMeasurement);

            fileName = System::filePathRoot + "LogFiles" + System::fileSystemSlash
                     + m_results.GetFile() + "_" + "Single_Measurement_" + typeText + "_"
                     + m_results.GetDate() + "_" + m_results.GetTime() + ".csv";
        }

        // Get script name if custom
        if (!scriptName.empty())
        {
            m_messung.setFilePath(wxDirPath);
            m_messung.setFileName(wxScriptName);
        }

        if (!m_messung.executeMeasurement())
        {
            const std::string reason = m_messung.getLastError();
            throw std::runtime_error(reason.empty() ? "executeMeasurement failed" : reason);
        }

        // get Resultes
        std::vector<double> realValues = m_messung.getX_Data();
        std::vector<double> imagValues = m_messung.getY_Data();

        if (realValues.empty())
        {
            throw std::runtime_error("No measurement values returned");
        }

        sData::sParam* info = m_results.GetParameter();

        int xPos = 0, yPos = 0;
        m_results.getXYCord(xPos, yPos, measurementNumber);
        m_results.set3DDataReal(realValues, xPos, yPos);

        if (!imagValues.empty())
            m_results.set3DDataImag(imagValues, xPos, yPos);

        std::vector<double> xAxis;
        if (m_messung.getMeasurementMode() == MeasurementMode::IQ)
            xAxis = m_results.GetTimeIQStepVector();
        else
            xAxis = m_results.GetFreqStepVector();

        if (xAxis.empty())
        {
            xAxis.resize(realValues.size());
            for (size_t i = 0; i < xAxis.size(); ++i)
                xAxis[i] = static_cast<double>(i);
        }

        const size_t n = std::min(xAxis.size(), realValues.size());
        xAxis.resize(n);
        realValues.resize(n);

        // Update document state then notify observers
        // observers must use CallAfter for any wxWidgets GUI calls
        m_x = std::move(xAxis);
        m_y = std::move(realValues);

        NotifyObservers("DataUpdated");

        if (info && !m_includePlotterSettings)
            info->hasPlotterData = false;

        if (fileName.IsEmpty() && info)
        {
            fileName = System::filePathRoot + "LogFiles" + System::fileSystemSlash
                     + "Messung_" + info->Time + ".csv";
        }

        if (!csvFile.saveCsvFile(fileName, m_results, measurementNumber))
            std::cerr << "[MeasurementDocument] failed to save CSV" << std::endl;

        std::cout << "MeasurementDocument: worker thread completed" << std::endl;
    }
    catch (const std::exception& e)
    {
        std::cerr << "[MeasurementDocument THREAD ERROR] " << e.what() << std::endl;
    }
    catch (...)
    {
        std::cerr << "[MeasurementDocument THREAD ERROR] unknown exception" << std::endl;
    }

    m_measuring = false;
    NotifyObservers("MeasurementStopped");
}
