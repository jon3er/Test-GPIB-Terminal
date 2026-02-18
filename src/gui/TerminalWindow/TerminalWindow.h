#pragma once

#include <wx/wx.h>
#include "TerminalDocument.h"

/**
 * @brief Pure View in the Document/View pattern for the GPIB terminal.
 *
 * TerminalWindow is responsible only for:
 *  - Building the UI widgets
 *  - Forwarding user input to the document via ProcessCommand()
 *  - Rendering document output via OnDocumentChanged()
 *
 * It holds a NON-OWNING pointer to TerminalDocument.
 * The document is owned and lifetime-managed by the caller (main.cpp).
 */
class TerminalWindow : public wxDialog, public ITerminalObserver
{
public:
    explicit TerminalWindow(wxWindow* parent);
    virtual ~TerminalWindow();

    /**
     * Attach this view to a document.
     * Automatically registers/unregisters the observer.
     */
    void SetDocument(TerminalDocument* document);

    // ITerminalObserver
    void OnDocumentChanged(const std::string& changeType) override;

private:
    // Event handler
    void OnEnterTerminal(wxCommandEvent& event);

    // Helper to format output with timestamp
    wxString FormatOutput(const std::string& text);

    // GUI Components
    wxTextCtrl* m_TerminalDisplay;

    // Non-owning pointer to the document (owned by main.cpp)
    TerminalDocument* m_document = nullptr;
};