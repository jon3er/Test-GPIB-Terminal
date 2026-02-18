#pragma once

#include <wx/wx.h>
#include "FunctionDocument.h"

class FunctionWindow : public wxDialog, public IFunctionObserver
{
public:
    FunctionWindow(wxWindow* parent);
    virtual ~FunctionWindow();

    /** Attach / detach the document. Pass nullptr to detach before destruction. */
    void SetDocument(FunctionDocument* document);

    // IFunctionObserver
    void OnFunctionDocumentChanged(const std::string& changeType) override;

private:
    // Event handlers
    void OnWriteGpib       (wxCommandEvent& event);
    void OnReadGpib        (wxCommandEvent& event);
    void OnReadWriteGpib   (wxCommandEvent& event);
    void OnUsbScan         (wxCommandEvent& event);
    void OnUsbConfig       (wxCommandEvent& event);
    void OnConDisconGpib   (wxCommandEvent& event);
    void OnTestSaveFile    (wxCommandEvent& event);
    void OnTestMultiMess   (wxCommandEvent& event);
    void OnTest            (wxCommandEvent& event);

    /** Rebuild the view from current document state. */
    void UpdateView();

    // GUI Components
    wxTextCtrl* m_textFuncOutput = nullptr;
    wxTextCtrl* m_writeFuncInput = nullptr;

    // Document (non-owning pointer)
    FunctionDocument* m_document = nullptr;
};
