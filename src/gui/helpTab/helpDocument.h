#pragma once

#include <string>
#include <vector>

// Forward declaration
class PrologixUsbGpibAdapter;

/**
 * @brief Observer interface for HelpDocument changes.
 */
class IHelpObserver
{
public:
    virtual ~IHelpObserver() = default;
    virtual void OnHelpDocumentChanged(const std::string& changeType) = 0;
};

/**
 * @brief Document in the Document/View pattern for the Reset Devices help window.
 */
class HelpDocument
{
public:
    explicit HelpDocument(PrologixUsbGpibAdapter& adapter);
    ~HelpDocument();

    // ---- Observer management ----
    void AddObserver   (IHelpObserver* observer);
    void RemoveObserver(IHelpObserver* observer);

    // ---- Read-only state accessors ----
    const std::string& GetOutputLog() const { return m_outputLog; }

    // ---- Command interface (called by the view's event handlers) ----
    void ResetAdapter();
    void ResetDevice();
    void GetAdapterStatus();
    void GetDeviceStatus();

private:
    void NotifyObservers(const std::string& changeType);
    void AppendToLog(const std::string& text);

    PrologixUsbGpibAdapter& m_adapter;

    std::string m_outputLog;
    std::vector<IHelpObserver*> m_observers;
};
