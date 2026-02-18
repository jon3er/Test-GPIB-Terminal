
#include "TerminalWindow.h"
#include "mainHelper.h"

//----- Terminal Window Constructor -----
TerminalWindow::TerminalWindow(wxWindow* parent)
    : wxDialog(parent, wxID_ANY, "GPIB Terminal Window", wxDefaultPosition, wxSize(1000, 600))
{
    wxPanel* panelTerm = new wxPanel(this);

    // Text Output (read-only log display)
    m_TerminalDisplay = new wxTextCtrl(panelTerm, wxID_ANY, "", wxDefaultPosition, wxSize(1000, 200),
                                       wxTE_MULTILINE | wxTE_READONLY);

    // Text Input (for user commands)
    wxTextCtrl* TerminalInput = new wxTextCtrl(panelTerm, wxID_ANY, "", wxDefaultPosition, wxSize(1000, 50),
                                               wxTE_MULTILINE | wxTE_PROCESS_ENTER);
    TerminalInput->SetFocus();

    wxStaticText* StaticTE      = new wxStaticText(panelTerm, wxID_ANY, "GPIB Terminal log");
    wxStaticText* StaticTEInput = new wxStaticText(panelTerm, wxID_ANY, "Input GPIB Commands:");

    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    sizer->Add(StaticTE,      0, wxALL, 20);
    sizer->Add(m_TerminalDisplay, 0, wxEXPAND | wxALL, 20);
    sizer->Add(StaticTEInput, 0, wxALL, 20);
    sizer->Add(TerminalInput, 0, wxALL, 20);
    panelTerm->SetSizerAndFit(sizer);

    // Bind event handler (input forwarded to document)
    TerminalInput->Bind(wxEVT_TEXT_ENTER, &TerminalWindow::OnEnterTerminal, this);

    std::cerr << "Terminal Window Opened" << std::endl;
}

//----- Terminal Window Destructor -----
TerminalWindow::~TerminalWindow()
{
    // Unsubscribe from document — document destructor handles disconnect
    if (m_document)
        m_document->RemoveObserver(this);

    std::cerr << "Terminal Window Closed" << std::endl;
}

//----- Document attachment -----
void TerminalWindow::SetDocument(TerminalDocument* document)
{
    if (m_document)
        m_document->RemoveObserver(this);

    m_document = document;

    if (m_document)
        m_document->AddObserver(this);
}

//----- ITerminalObserver -----
void TerminalWindow::OnDocumentChanged(const std::string& changeType)
{
    if (!m_document)
        return;

    if (changeType == "OutputAppended")
    {
        m_TerminalDisplay->AppendText(FormatOutput(m_document->GetLastOutput()));
    }
    // "ConnectionChanged" and "Cleared" require no visual update beyond the output log
}

//----- Helper Methods -----
wxString TerminalWindow::FormatOutput(const std::string& text)
{
    return terminalTimestampOutput(text);
}

//----- Event Handlers -----
void TerminalWindow::OnEnterTerminal(wxCommandEvent& event)
{
    if (!m_document)
        return;

    wxTextCtrl* Terminal = static_cast<wxTextCtrl*>(event.GetEventObject());
    wxString TText = Terminal->GetValue();
    Terminal->SetValue("");

    std::cerr << "User entered: " << TText.c_str() << std::endl;

    // Echo user input to display, then let the document process it
    m_TerminalDisplay->AppendText(FormatOutput(std::string(TText.ToUTF8()) + "\n"));

    m_document->ProcessCommand(std::string(TText.ToUTF8()));
}
