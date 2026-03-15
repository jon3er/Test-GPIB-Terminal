#include "helpDocument.h"
#include "GpibUsbAdapter.h"

#include <algorithm>
#include <ctime>

namespace
{
    std::string helpTimestamp()
    {
        std::time_t t = std::time(nullptr);
        char buf[16] = {};
        std::strftime(buf, sizeof(buf), "[%H:%M:%S] ", std::localtime(&t));
        return std::string(buf);
    }
}

HelpDocument::HelpDocument(PrologixUsbGpibAdapter& adapter)
    : m_adapter(adapter)
{
}

HelpDocument::~HelpDocument()
{
}

void HelpDocument::AddObserver(IHelpObserver* observer)
{
    if (observer && std::find(m_observers.begin(), m_observers.end(), observer) == m_observers.end())
        m_observers.push_back(observer);
}

void HelpDocument::RemoveObserver(IHelpObserver* observer)
{
    m_observers.erase(
        std::remove(m_observers.begin(), m_observers.end(), observer),
        m_observers.end());
}

void HelpDocument::NotifyObservers(const std::string& changeType)
{
    for (IHelpObserver* obs : m_observers)
    {
        if (obs)
            obs->OnHelpDocumentChanged(changeType);
    }
}

void HelpDocument::AppendToLog(const std::string& text)
{
    m_outputLog += helpTimestamp() + text;
}

void HelpDocument::ResetAdapter()
{
    m_adapter.write("++rst");
    std::string response = m_adapter.send("++spoll", 5000);

    if (!response.empty() && response.find("0") != std::string::npos) // After reset, SPOLL should return 0
    {
        AppendToLog("Adapter reset successfully.\n");
    }
    else
    {
        AppendToLog("Failed to reset adapter. Check connection.\n");
    }
    NotifyObservers("OutputAppended");
}

void HelpDocument::ResetDevice()
{

    // 1. Prologix: Interface initialisieren
    m_adapter.write("++ifc"); 

    // 2. FSU: Gerät auf Standardwerte und Fehler löschen
    m_adapter.write("*RST;*CLS");

    // 3. Optional: Warten, bis Operation abgeschlossen
    std::string response = m_adapter.send("*OPC?", 5000);

    if (!response.empty() && response.find("1") != std::string::npos)
    {
        AppendToLog("Device reset successfully.\n");
    }
    else
    {
        AppendToLog("Failed to reset device. Check connection.\n");
    }
    NotifyObservers("OutputAppended");
}

void HelpDocument::GetAdapterStatus()
{
    std::string status = m_adapter.statusText();
    AppendToLog("Adapter Status: " + status + "\n");
    NotifyObservers("OutputAppended");
}

void HelpDocument::GetDeviceStatus()
{
    std::string response = m_adapter.send("SYST:ERR?", 1000);
    if (!response.empty())
    {
        AppendToLog("Device Status: " + response + "\n");
    }
    else
    {
        AppendToLog("Failed to get device status. Check connection.\n");
    }
    NotifyObservers("OutputAppended");
}
