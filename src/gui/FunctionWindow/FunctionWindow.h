#pragma once

#include <wx/wx.h>
#include "FunctionController.h"

class FunctionWindow : public wxDialog
{
public:
    FunctionWindow(wxWindow *parent);
    virtual ~FunctionWindow();

private:
    // Event handlers
    void OnWriteGpib(wxCommandEvent& event);
    void OnReadGpib(wxCommandEvent& event);
    void OnReadWriteGpib(wxCommandEvent& event);
    void OnUsbScan(wxCommandEvent& event);
    void OnUsbConfig(wxCommandEvent& event);
    void OnConDisconGpib(wxCommandEvent& event);
    void OnTestSaveFile(wxCommandEvent& event);
    void OnTestMultiMess(wxCommandEvent& event);
    void OnTest(wxCommandEvent& event);

    // Helper to format output with timestamp
    wxString formatOutput(const std::string& text);

    // GUI Components
    wxTextCtrl* m_textFuncOutput;
    wxTextCtrl* m_writeFuncInput;

    // Function Logic
    FunctionController m_FunctionLogic;
};
