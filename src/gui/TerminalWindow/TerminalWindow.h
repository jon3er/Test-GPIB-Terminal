#pragma once

#include <wx/wx.h>
#include "TerminalController.h"

class TerminalWindow : public wxDialog
{
public:
    TerminalWindow(wxWindow *parent);
    virtual ~TerminalWindow();

private:
    // Event handler
    void OnEnterTerminal(wxCommandEvent& event);

    // Helper to format output with timestamp
    wxString formatOutput(const std::string& text);

    // GUI Components
    wxTextCtrl* m_TerminalDisplay;

    // Business Logic (Model)
    TerminalController m_controller;
};