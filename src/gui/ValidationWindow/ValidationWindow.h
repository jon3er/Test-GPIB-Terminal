#pragma once

#include <wx/wx.h>
#include <wx/richtext/richtextctrl.h>
#include "ValidationDocument.h"

class ValidationWindow : public wxDialog, public IValidationObserver
{
public:
    explicit ValidationWindow(wxWindow* parent);
    ~ValidationWindow() override;

    void SetDocument(ValidationDocument* document);

    void OnValidationDocumentChanged(const std::string& changeType) override;

private:
    void OnRunAll(wxCommandEvent& event);
    void OnAdapterPresence(wxCommandEvent& event);
    void OnAdapterStatus(wxCommandEvent& event);
    void OnGpibDevicePresence(wxCommandEvent& event);
    void OnDeviceStatus(wxCommandEvent& event);
    void OnScpiIdn(wxCommandEvent& event);
    void OnOpcQuery(wxCommandEvent& event);
    void OnSweepRoundtrip(wxCommandEvent& event);
    void OnCsvRoundtrip(wxCommandEvent& event);
    void OnExportOutput(wxCommandEvent& event);
    void OnClearOutput(wxCommandEvent& event);

    void UpdateView();
    void AppendStyledLine(const wxString& line);

    wxRichTextCtrl* m_output = nullptr;
    ValidationDocument* m_document = nullptr;
};
