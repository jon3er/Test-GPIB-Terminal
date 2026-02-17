#pragma once

#include <wx/wx.h>



class FunctionWindow : public wxDialog
{
public:
    FunctionWindow(wxWindow *parent);
    virtual ~FunctionWindow();

private:
    //Button Functions
    void OnWriteGpib(wxCommandEvent& event);
    void OnReadGpib(wxCommandEvent& event);
    void OnReadWriteGpib(wxCommandEvent& event);
    void OnUsbScan(wxCommandEvent& event);
    void OnUsbConfig(wxCommandEvent& event);
    void OnConDisconGpib(wxCommandEvent& event);
    void OnTestSaveFile(wxCommandEvent& event);
    void OnTestMultiMess(wxCommandEvent& event);
    void OnTest(wxCommandEvent& event);

    //Text Boxes
    wxTextCtrl* m_textFuncOutput;
    wxTextCtrl* m_writeFuncInput;

    //device Class
    //GpibDevice Adapter;
};
