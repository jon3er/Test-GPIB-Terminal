#pragma once

#include <string>
#include <vector>

// Forward declaration — no wxWidgets in the document
class PrologixUsbGpibAdapter;

/**
 * @brief Observer interface for FunctionDocument changes.
 *
 * Implement this interface in any view that wants to be notified
 * when the document state changes.
 *
 * Change types:
 *   "OutputAppended"    — new text was added to the output log
 *   "ConnectionChanged" — adapter connected / disconnected
 */
class IFunctionObserver
{
public:
    virtual ~IFunctionObserver() = default;
    virtual void OnFunctionDocumentChanged(const std::string& changeType) = 0;
};

/**
 * @brief Document in the Document/View pattern for the GPIB function test window.
 *
 * Responsibilities:
 *  - Own all function-test state (output log, last command, connection state)
 *  - Execute all GPIB hardware operations via the injected adapter reference
 *  - Notify registered observers on every state change
 *
 * This class is pure C++ — its public interface has NO wxWidgets dependency.
 */
class FunctionDocument
{
public:
    /**
     * @param adapter  Reference to the hardware adapter singleton.
     *                 Ownership stays with the caller (Global::AdapterInstance).
     */
    explicit FunctionDocument(PrologixUsbGpibAdapter& adapter);
    ~FunctionDocument();

    // ---- Observer management ----
    void AddObserver(IFunctionObserver* observer);
    void RemoveObserver(IFunctionObserver* observer);

    // ---- Read-only state accessors ----
    const std::string& GetOutputLog()   const { return m_outputLog; }
    const std::string& GetLastCommand() const { return m_lastCommand; }
    bool IsConnected() const;

    // ---- Command interface (called by the view's event handlers) ----
    void WriteToGpib    (const std::string& cmd);
    void ReadFromGpib   ();
    void ReadWriteGpib  (const std::string& cmd);
    void ScanUsbDevices ();
    void ConfigureDevice();
    void ConnectDisconnect();
    void TestSaveFile   ();
    void TestMultiMeasurement();
    void Test           ();

private:
    void NotifyObservers(const std::string& changeType);

    /** Append a timestamped line to the output log (does NOT notify). */
    void AppendToLog(const std::string& text);

    PrologixUsbGpibAdapter& m_adapter;

    std::string m_outputLog;
    std::string m_lastCommand;
    std::vector<IFunctionObserver*> m_observers;
};
