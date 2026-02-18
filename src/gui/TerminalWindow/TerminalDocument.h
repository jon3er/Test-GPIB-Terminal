#pragma once

#include <string>
#include <vector>
#include <map>
#include <functional>

// Forward declaration — no wxWidgets in the document
class PrologixUsbGpibAdapter;

/**
 * @brief Observer interface for TerminalDocument changes.
 *
 * Implement this interface in any view that wants to be notified
 * when the document state changes.
 *
 * Change types:
 *   "OutputAppended"      — new text was added to the output log
 *   "ConnectionChanged"   — adapter connected / disconnected
 *   "Cleared"             — document state was reset
 */
class ITerminalObserver
{
public:
    virtual ~ITerminalObserver() = default;
    virtual void OnDocumentChanged(const std::string& changeType) = 0;
};

/**
 * @brief Document in the Document/View pattern for the GPIB terminal.
 *
 * Responsibilities:
 *  - Own all terminal state (output log, connection state)
 *  - Execute all GPIB hardware commands via the injected adapter reference
 *  - Notify registered observers on every state change
 *
 * This class is pure C++ — it has NO wxWidgets dependency and is fully
 * unit-testable without a GUI.
 */
class TerminalDocument
{
public:
    /**
     * @param adapter  Reference to the hardware adapter singleton.
     *                 Ownership stays with the caller (Global::AdapterInstance).
     */
    explicit TerminalDocument(PrologixUsbGpibAdapter& adapter);
    ~TerminalDocument();

    // ---- Observer management ----
    void AddObserver(ITerminalObserver* observer);
    void RemoveObserver(ITerminalObserver* observer);

    // ---- Read-only state accessors ----
    const std::string&              GetLastOutput()      const { return m_lastOutput; }
    const std::vector<std::string>& GetCommandHistory()  const { return m_commandHistory; }
    bool                            IsConnected()        const;

    // ---- Command interface (called by the view's event handler) ----

    /**
     * Parse and execute a raw user input string.
     * Dispatches to the appropriate command method below.
     */
    void ProcessCommand(const std::string& input);

    // Individual commands — also callable directly (e.g. from tests)
    std::string ScanDevices     (const std::string& args = "");
    std::string StatusDevice    (const std::string& args = "");
    std::string ConnectDevice   (const std::string& args = "");
    std::string DisconnectDevice(const std::string& args = "");
    std::string SendToDevice    (const std::string& args);
    std::string ReadFromDevice  (const std::string& args = "");
    std::string WriteToDevice   (const std::string& args);
    std::string ConfigDevice    (const std::string& args = "");
    std::string TestDevice      (const std::string& args = "");

private:
    // ---- Internal helpers ----
    void NotifyObservers(const std::string& changeType);

    /**
     * Append text to m_lastOutput and notify observers with "OutputAppended".
     * Every command result is routed through here.
     */
    void AppendOutput(const std::string& text);

    void SetupCommands();

    // ---- State ----
    PrologixUsbGpibAdapter&    m_adapter;
    std::string                m_lastOutput;
    std::vector<std::string>   m_commandHistory;
    std::vector<ITerminalObserver*> m_observers;

    // ---- Command dispatch map ----
    using CommandMap = std::map<std::string, std::function<std::string(const std::string&)>>;
    CommandMap m_cmds;
};
