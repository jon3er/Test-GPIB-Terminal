#include "MainDocument.h"

#include <algorithm>
#include <iostream>

// -----------------------------------------------------------------------
MainDocument::MainDocument()
    : m_fileOpen(false)
{
}

// -----------------------------------------------------------------------
// Observer management
// -----------------------------------------------------------------------
void MainDocument::AddObserver(IMainObserver* observer)
{
    if (observer &&
        std::find(m_observers.begin(), m_observers.end(), observer) == m_observers.end())
    {
        m_observers.push_back(observer);
    }
}

void MainDocument::RemoveObserver(IMainObserver* observer)
{
    m_observers.erase(
        std::remove(m_observers.begin(), m_observers.end(), observer),
        m_observers.end());
}

// -----------------------------------------------------------------------
// File operations
// -----------------------------------------------------------------------
bool MainDocument::OpenFile(const wxString& filePath)
{
    sData newData;
    if (!m_csvFile.readCsvFile(filePath, newData))
    {
        std::cerr << "[MainDocument] Failed to open: " << filePath << std::endl;
        m_fileOpen = false;
        NotifyFileChanged();
        return false;
    }

    m_data     = newData;
    m_filePath = filePath;
    m_fileOpen = true;

    std::cerr << "[MainDocument] Opened: " << filePath << std::endl;

    NotifyPathChanged();
    NotifyFileChanged();
    return true;
}

bool MainDocument::SaveFile()
{
    if (!m_fileOpen || m_filePath.IsEmpty())
        return false;

    const bool ok = m_csvFile.saveToCsvFile(m_filePath, m_data, 0);
    if (!ok)
        std::cerr << "[MainDocument] Save failed: " << m_filePath << std::endl;
    return ok;
}

bool MainDocument::SaveFileAs(const wxString& filePath)
{
    wxString path = filePath;
    if (!m_csvFile.saveToCsvFile(path, m_data, 0))
    {
        std::cerr << "[MainDocument] SaveAs failed: " << filePath << std::endl;
        return false;
    }

    m_filePath = filePath;
    m_fileOpen = true;

    std::cerr << "[MainDocument] Saved as: " << filePath << std::endl;

    NotifyPathChanged();
    NotifyFileChanged();
    return true;
}

void MainDocument::CloseFile()
{
    m_fileOpen = false;
    m_data     = sData();       // reset to empty default
    m_filePath.Clear();

    NotifyPathChanged();
    NotifyFileChanged();
}

// -----------------------------------------------------------------------
// Notifications
// -----------------------------------------------------------------------
void MainDocument::NotifyFileChanged()
{
    // Iterate over a copy so observers may safely deregister themselves
    // inside their callback without invalidating the iterator.
    auto observers = m_observers;
    for (IMainObserver* obs : observers)
        obs->OnFileChanged(m_data, m_fileOpen);
}

void MainDocument::NotifyPathChanged()
{
    auto observers = m_observers;
    for (IMainObserver* obs : observers)
        obs->OnFilePathChanged(m_filePath);
}
