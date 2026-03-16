#include "ValidationWindow.h"

#include "systemInfo.h"

#include <wx/filedlg.h>
#include <wx/filename.h>
#include <wx/tokenzr.h>

ValidationWindow::ValidationWindow(wxWindow* parent)
    : wxDialog(parent, wxID_ANY, "Validierung", wxDefaultPosition, wxSize(700, 620))
{
    wxPanel* panel = new wxPanel(this);

    wxStaticText* title = new wxStaticText(panel, wxID_ANY, "Ausfuehrbare Projektvalidierungen:");

    wxButton* runAllButton = new wxButton(panel, wxID_ANY, "Alle Tests ausfuehren");
    wxButton* adapterPresenceButton = new wxButton(panel, wxID_ANY, "Adapter erkennen");
    wxButton* adapterStatusButton = new wxButton(panel, wxID_ANY, "Adapterstatus lesen");
    wxButton* gpibPresenceButton = new wxButton(panel, wxID_ANY, "GPIB-Geraet pruefen");
    wxButton* deviceStatusButton = new wxButton(panel, wxID_ANY, "Geraetestatus lesen");
    wxButton* idnButton = new wxButton(panel, wxID_ANY, "SCPI *IDN?");
    wxButton* opcButton = new wxButton(panel, wxID_ANY, "SCPI *OPC?");
    wxButton* sweepRoundtripButton = new wxButton(panel, wxID_ANY, "Sweep-Settings Roundtrip");
    wxButton* csvRoundtripButton = new wxButton(panel, wxID_ANY, "CSV-Roundtrip");
    wxButton* exportButton = new wxButton(panel, wxID_ANY, "Protokoll exportieren");
    wxButton* clearButton = new wxButton(panel, wxID_ANY, "Ausgabe leeren");

    wxStaticText* outputLabel = new wxStaticText(panel, wxID_ANY, "Ausgabe:");
    m_output = new wxRichTextCtrl(panel, wxID_ANY, "", wxDefaultPosition, wxDefaultSize,
        wxRE_MULTILINE | wxRE_READONLY | wxBORDER_THEME);

    runAllButton->Bind(wxEVT_BUTTON, &ValidationWindow::OnRunAll, this);
    adapterPresenceButton->Bind(wxEVT_BUTTON, &ValidationWindow::OnAdapterPresence, this);
    adapterStatusButton->Bind(wxEVT_BUTTON, &ValidationWindow::OnAdapterStatus, this);
    gpibPresenceButton->Bind(wxEVT_BUTTON, &ValidationWindow::OnGpibDevicePresence, this);
    deviceStatusButton->Bind(wxEVT_BUTTON, &ValidationWindow::OnDeviceStatus, this);
    idnButton->Bind(wxEVT_BUTTON, &ValidationWindow::OnScpiIdn, this);
    opcButton->Bind(wxEVT_BUTTON, &ValidationWindow::OnOpcQuery, this);
    sweepRoundtripButton->Bind(wxEVT_BUTTON, &ValidationWindow::OnSweepRoundtrip, this);
    csvRoundtripButton->Bind(wxEVT_BUTTON, &ValidationWindow::OnCsvRoundtrip, this);
    exportButton->Bind(wxEVT_BUTTON, &ValidationWindow::OnExportOutput, this);
    clearButton->Bind(wxEVT_BUTTON, &ValidationWindow::OnClearOutput, this);

    wxBoxSizer* buttonSizer = new wxBoxSizer(wxVERTICAL);
    buttonSizer->Add(runAllButton, 0, wxEXPAND | wxBOTTOM, 8);
    buttonSizer->Add(adapterPresenceButton, 0, wxEXPAND | wxBOTTOM, 8);
    buttonSizer->Add(adapterStatusButton, 0, wxEXPAND | wxBOTTOM, 8);
    buttonSizer->Add(gpibPresenceButton, 0, wxEXPAND | wxBOTTOM, 8);
    buttonSizer->Add(deviceStatusButton, 0, wxEXPAND | wxBOTTOM, 8);
    buttonSizer->Add(idnButton, 0, wxEXPAND | wxBOTTOM, 8);
    buttonSizer->Add(opcButton, 0, wxEXPAND | wxBOTTOM, 8);
    buttonSizer->Add(sweepRoundtripButton, 0, wxEXPAND | wxBOTTOM, 8);
    buttonSizer->Add(csvRoundtripButton, 0, wxEXPAND | wxBOTTOM, 8);
    buttonSizer->Add(exportButton, 0, wxEXPAND | wxBOTTOM, 8);
    buttonSizer->Add(clearButton, 0, wxEXPAND, 0);

    wxBoxSizer* root = new wxBoxSizer(wxVERTICAL);
    root->Add(title, 0, wxEXPAND | wxALL, 10);
    root->Add(buttonSizer, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 10);
    root->Add(outputLabel, 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, 10);
    root->Add(m_output, 1, wxEXPAND | wxALL, 10);

    panel->SetSizer(root);

    wxBoxSizer* dialogSizer = new wxBoxSizer(wxVERTICAL);
    dialogSizer->Add(panel, 1, wxEXPAND);
    SetSizer(dialogSizer);
    Layout();
    Centre(wxBOTH);
}

ValidationWindow::~ValidationWindow()
{
    if (m_document)
        m_document->RemoveObserver(this);
}

void ValidationWindow::SetDocument(ValidationDocument* document)
{
    if (m_document)
        m_document->RemoveObserver(this);

    m_document = document;

    if (m_document)
    {
        m_document->AddObserver(this);
        UpdateView();
    }
}

void ValidationWindow::OnValidationDocumentChanged(const std::string& changeType)
{
    if (changeType == "OutputAppended")
        UpdateView();
}

void ValidationWindow::UpdateView()
{
    if (!m_document || !m_output)
        return;

    m_output->Freeze();
    m_output->Clear();

    wxStringTokenizer tokenizer(wxString::FromUTF8(m_document->GetOutputLog()), "\n", wxTOKEN_RET_EMPTY_ALL);
    while (tokenizer.HasMoreTokens())
    {
        AppendStyledLine(tokenizer.GetNextToken());
    }

    m_output->ShowPosition(m_output->GetLastPosition());
    m_output->Thaw();
}

void ValidationWindow::AppendStyledLine(const wxString& line)
{
    wxColour color = *wxBLACK;

    if (line.Find("PASS:") != wxNOT_FOUND)
        color = wxColour(0, 128, 0);
    else if (line.Find("FAIL:") != wxNOT_FOUND)
        color = wxColour(192, 0, 0);
    else if (line.Find("===") != wxNOT_FOUND)
        color = wxColour(0, 64, 160);

    m_output->BeginTextColour(color);
    m_output->WriteText(line);
    m_output->EndTextColour();
    m_output->Newline();
}

void ValidationWindow::OnRunAll(wxCommandEvent& event)
{
    if (m_document)
        m_document->RunAllValidations();
}

void ValidationWindow::OnAdapterPresence(wxCommandEvent& event)
{
    if (m_document)
        m_document->ValidateAdapterPresence();
}

void ValidationWindow::OnAdapterStatus(wxCommandEvent& event)
{
    if (m_document)
        m_document->ValidateAdapterStatus();
}

void ValidationWindow::OnGpibDevicePresence(wxCommandEvent& event)
{
    if (m_document)
        m_document->ValidateGpibDevicePresence();
}

void ValidationWindow::OnDeviceStatus(wxCommandEvent& event)
{
    if (m_document)
        m_document->ValidateDeviceStatus();
}

void ValidationWindow::OnScpiIdn(wxCommandEvent& event)
{
    if (m_document)
        m_document->ValidateScpiIdn();
}

void ValidationWindow::OnOpcQuery(wxCommandEvent& event)
{
    if (m_document)
        m_document->ValidateOpcQuery();
}

void ValidationWindow::OnSweepRoundtrip(wxCommandEvent& event)
{
    if (m_document)
        m_document->ValidateSweepSettingsRoundtrip();
}

void ValidationWindow::OnCsvRoundtrip(wxCommandEvent& event)
{
    if (m_document)
        m_document->ValidateCsvRoundtrip();
}

void ValidationWindow::OnExportOutput(wxCommandEvent& event)
{
    if (!m_document)
        return;

    wxFileName defaultFile(System::filePathRoot + "LogFiles" + System::fileSystemSlash + "ValidationLog.txt");
    wxFileDialog saveDialog(
        this,
        "Validierungsprotokoll exportieren",
        defaultFile.GetPath(),
        defaultFile.GetFullName(),
        "Text Files (*.txt)|*.txt|All Files (*.*)|*.*",
        wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

    if (saveDialog.ShowModal() == wxID_CANCEL)
        return;

    if (m_document->ExportOutputLog(std::string(saveDialog.GetPath().ToUTF8())))
    {
        wxMessageBox("Validierungsprotokoll wurde gespeichert.", "Export", wxOK | wxICON_INFORMATION, this);
    }
    else
    {
        wxMessageBox("Validierungsprotokoll konnte nicht gespeichert werden.", "Export", wxOK | wxICON_ERROR, this);
    }
}

void ValidationWindow::OnClearOutput(wxCommandEvent& event)
{
    if (m_document)
        m_document->ClearOutput();
}
