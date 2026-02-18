#pragma once

#include <string>
#include <vector>
#include <thread>
#include <atomic>
#include "dataManagement.h"
#include "fkt_GPIB.h"

/**
 * @brief Observer interface for MeasurementDocument.
 *
 * Change types emitted:
 *   "MeasurementStarted"  — worker thread has been launched
 *   "DataUpdated"         — new x/y data is ready  ← may come from worker thread!
 *   "MeasurementStopped"  — thread finished or was stopped
 *
 * IMPORTANT: "DataUpdated" can arrive from a background thread.
 * Views *must* guard GUI updates with wxEvtHandler::CallAfter().
 */
class IMeasurementObserver
{
public:
    virtual ~IMeasurementObserver() = default;
    virtual void OnDocumentChanged(const std::string& changeType) = 0;
};

/**
 * @brief Document in the Document/View pattern for PlotWindow / PlotWindowSetMarker.
 *
 * Owns:
 *  - Injected reference to the hardware adapter (not owned by document)
 *  - Injected reference to the global fsuMesurement data sink (not owned)
 *  - Worker thread for GPIB script execution
 *  - Aggregated measurement result (sData)
 *  - Processed x/y plot vectors
 *
 * The document is wxWidgets-free and fully testable in isolation.
 */
class MeasurementDocument
{
public:
    /**
     * @param adapter   Hardware adapter singleton reference.
     * @param messung   Global measurement data object reference.
     */
    MeasurementDocument(PrologixUsbGpibAdapter& adapter, fsuMesurement& messung);
    ~MeasurementDocument();

    // ---- Observer management ----
    void AddObserver(IMeasurementObserver* observer);
    void RemoveObserver(IMeasurementObserver* observer);

    // ---- Read-only data accessors ----
    const std::vector<double>& GetXData()            const { return m_x; }
    const std::vector<double>& GetYData()            const { return m_y; }
    bool                       IsMeasuring()         const { return m_measuring.load(); }
    int                        GetMeasurementNumber()const { return m_measurementNumber; }
    const sData&               GetResults()          const { return m_results; }
    sData&                     GetResultsMutable()         { return m_results; }

    // ---- Commands ----

    /**
     * Launch the measurement worker thread for a given script.
     * No-op if a thread is already running.
     */
    void StartMeasurement(const std::string& dirPath,
                          const std::string& scriptName,
                          int                measurementNumber);

    /**
     * Signal the running thread to stop and block until it joins.
     */
    void StopMeasurement();

    /**
     * Send a SCPI marker-1 command via the adapter.
     * @param setToMax  true  → CALC:MARK:MAX
     *                  false → CALC:MARK:MAX <freqRaw>
     * @param freqRaw   Pre-computed frequency in Hz as string.
     */
    void WriteMarker1(bool setToMax, const std::string& freqRaw);

    /**
     * Send a SCPI marker-2 command via the adapter.
     */
    void WriteMarker2(bool setToMax, const std::string& freqRaw);

private:
    void NotifyObservers(const std::string& changeType);

    void WorkerThread(const std::string& dirPath,
                      const std::string& scriptName,
                      int             measurementNumber);

    // ---- Injected dependencies ----
    PrologixUsbGpibAdapter& m_adapter;
    fsuMesurement&          m_messung;

    // ---- State ----
    std::vector<double>     m_x;
    std::vector<double>     m_y;
    sData                   m_results;
    int                     m_measurementNumber = 1;

    std::thread             m_thread;
    std::atomic<bool>       m_stopFlag{false};
    std::atomic<bool>       m_measuring{false};

    std::vector<IMeasurementObserver*> m_observers;
};
