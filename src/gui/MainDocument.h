#pragma once

#include <vector>
#include <wx/string.h>

#include "dataManagement.h"

// -----------------------------------------------------------------------
// Observer interface — implemented by any view that watches MainDocument
// -----------------------------------------------------------------------
class IMainObserver
{
public:
    virtual ~IMainObserver() = default;

    /**
     * Called whenever the loaded data or open-state changes
     * (after open, close, save-as).
     */
    virtual void OnFileChanged(const sData& data, bool isOpen) = 0;

    /**
     * Called whenever the current file path changes.
     */
    virtual void OnFilePathChanged(const wxString& path) = 0;
};

// -----------------------------------------------------------------------
// MainDocument — owns the application-level CSV data and file path.
//
// Lifetime: created once in MainWin::OnInit, destroyed when the app exits.
// Thread-safety: all methods must be called from the main (UI) thread.
// -----------------------------------------------------------------------
class MainDocument
{
public:
    MainDocument();
    ~MainDocument() = default;

    // No copy
    MainDocument(const MainDocument&) = delete;
    MainDocument& operator=(const MainDocument&) = delete;

    // ------------------------------------------------------------------
    // Observer registration
    // ------------------------------------------------------------------
    void AddObserver(IMainObserver* observer);
    void RemoveObserver(IMainObserver* observer);

    // ------------------------------------------------------------------
    // File operations
    // ------------------------------------------------------------------

    /**
     * Open a CSV file from disk.
     * @return true on success, false if the file could not be read.
     */
    bool OpenFile(const wxString& filePath);

    /**
     * Save to the currently open file path.
     * If no file is open, returns false without doing anything.
     */
    bool SaveFile();

    /**
     * Save to an explicit path and update the stored path on success.
     * @return true on success.
     */
    bool SaveFileAs(const wxString& filePath);

    /**
     * Clear the currently loaded data and mark the document as closed.
     */
    void CloseFile();

    // ------------------------------------------------------------------
    // Accessors (read-only via const reference)
    // ------------------------------------------------------------------
    const sData&    GetData()     const { return m_data; }
    const wxString& GetFilePath() const { return m_filePath; }
    bool            IsFileOpen()  const { return m_fileOpen; }

    /**
     * Non-const data accessor — used by sub-windows that need to
     * read the raw measurement arrays (e.g. PlotWindow).
     */
    sData& GetData() { return m_data; }

private:
    void NotifyFileChanged();
    void NotifyPathChanged();

    sData     m_data;
    CsvFile   m_csvFile;
    wxString  m_filePath;
    bool      m_fileOpen = false;

    std::vector<IMainObserver*> m_observers;
};
