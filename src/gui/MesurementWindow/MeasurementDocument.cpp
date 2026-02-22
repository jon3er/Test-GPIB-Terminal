
#include "MeasurementDocument.h"
#include "fkt_GPIB.h"
#include "systemInfo.h"
#include "dataManagement.h"
#include "cmdGpib.h"

#include <wx/wx.h>
#include <wx/textfile.h>

#include <iostream>
#include <algorithm>

// ---------------------------------------------------------------------------
// Construction / Destruction
// ---------------------------------------------------------------------------

MeasurementDocument::MeasurementDocument(PrologixUsbGpibAdapter& adapter,
                                         fsuMesurement& messung)
    : m_adapter(adapter)
    , m_messung(messung)
{
}

MeasurementDocument::~MeasurementDocument()
{
    StopMeasurement();
    // NOTE: Do NOT disconnect the shared adapter singleton here.
    // Multiple MeasurementDocuments may share the same adapter.
    // The adapter lifecycle is managed at the application level.
}

// ---------------------------------------------------------------------------
// Observer management
// ---------------------------------------------------------------------------

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

// ---------------------------------------------------------------------------
// Commands
// ---------------------------------------------------------------------------

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
                           this, dirPath, scriptName, measurementNumber);
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

void MeasurementDocument::WriteMarker1(bool setToMax, const std::string& freqRaw)
{
    if (setToMax)
    {
        std::string cmd = ScpiCmdLookup.at(ScpiCmd::CALC_MARK_MAX);
        m_adapter.write(cmd);
    }
    else if (!freqRaw.empty())
    {
        std::string cmd = ScpiCmdLookup.at(ScpiCmd::CALC_MARK_MAX) + " " + freqRaw;
        m_adapter.write(cmd);
    }
}

void MeasurementDocument::WriteMarker2(bool setToMax, const std::string& freqRaw)
{
    if (setToMax)
    {
        m_adapter.write(std::string("CALC:MARK2:MAX"));
    }
    else if (!freqRaw.empty())
    {
        m_adapter.write(std::string("CALC:MARK2:MAX ") + freqRaw);
    }
}

// ---------------------------------------------------------------------------
// Worker thread — replaces PlotWindow::MeasurementWorkerThread
// ---------------------------------------------------------------------------

void MeasurementDocument::WorkerThread(const std::string& dirPath,
                                       const std::string& scriptName,
                                       int                measurementNumber)
{
    std::cout << "MeasurementDocument: worker thread started" << std::endl;
    CsvFile csvFile;

    try
    {
        wxArrayString logAdapterReceived;
        wxString wxDirPath    = wxString::FromUTF8(dirPath);
        wxString wxScriptName = wxString::FromUTF8(scriptName);

        // Execute the GPIB script — blocks until completed or stopped
        m_adapter.readScriptFile(wxDirPath, wxScriptName, logAdapterReceived, &m_stopFlag);

        for (size_t i = 0; i < logAdapterReceived.GetCount(); i++)
            std::cerr << logAdapterReceived[i] << std::endl;

        // Collect results from the shared fsuMesurement sink
        std::vector<double> x_copy = m_messung.getX_Data();
        std::vector<double> y_copy = m_results.GetFreqStepVector();

        sData::sParam* info = m_results.GetParameter();

        if (measurementNumber == 1)
            m_results.setNumberofPts_Array(x_copy.size());

        int xPos = 0, yPos = 0;
        m_results.getXYCord(xPos, yPos, measurementNumber);
        m_results.set3DDataReal(x_copy, xPos, yPos);

        std::vector<double> freqScale = m_results.GetFreqStepVector();

        if (m_messung.isImagValues())
            m_results.set3DDataImag(y_copy, xPos, yPos);

        // Update document state then notify observers
        // NOTE: observers must use CallAfter for any wxWidgets GUI calls
        m_x = freqScale;
        m_y = x_copy;   // matches original PlotWindow swap

        NotifyObservers("DataUpdated");

        // Save result to file
        wxString filePath = System::filePathRoot + System::fileSystemSlash
                          + "LogFiles" + System::fileSystemSlash
                          + "Messung " + info->Time;
        wxTextFile file(filePath);
        file.Create();
        if (!file.Open())
            csvFile.saveToCsvFile(filePath, m_results, measurementNumber);

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
