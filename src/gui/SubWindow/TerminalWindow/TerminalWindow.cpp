
#include "TerminalWindow.h"
#include "systemInfo.h"

//----- Terminal Window Constructor -----
TerminalWindow::TerminalWindow(wxWindow *parent)
    : wxDialog(parent, wxID_ANY, "GPIB Terminal Window", wxDefaultPosition, wxSize(1000, 600))
{
    wxPanel* panelTerm = new wxPanel(this);

    // Text Output (read-only log display)
    TerminalDisplay = new wxTextCtrl(panelTerm, wxID_ANY, "", wxDefaultPosition, wxSize(1000, 200), wxTE_MULTILINE | wxTE_READONLY);

    // Text Input (for user commands)
    wxTextCtrl* TerminalInput = new wxTextCtrl(panelTerm, wxID_ANY, "", wxDefaultPosition, wxSize(1000, 50), wxTE_MULTILINE | wxTE_PROCESS_ENTER);
    TerminalInput->SetFocus();

    wxStaticText* StaticTE = new wxStaticText(panelTerm, wxID_ANY, "GPIB Terminal log");
    wxStaticText* StaticTEInput = new wxStaticText(panelTerm, wxID_ANY, "Input GPIB Commands:");

    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    sizer->Add(StaticTE, 0, wxALL, 20);
    sizer->Add(TerminalDisplay, 0, wxEXPAND | wxALL, 20);
    sizer->Add(StaticTEInput, 0, wxALL, 20);
    sizer->Add(TerminalInput, 0, wxALL, 20);
    panelTerm->SetSizerAndFit(sizer);

    std::cerr << "Terminal Window Opened" << std::endl;

    // Bind event handler
    TerminalInput->Bind(wxEVT_TEXT_ENTER, &TerminalWindow::OnEnterTerminal, this);

    // Set controller's output callback to update display
    m_controller.setOutputCallback([this](const std::string& output) {
        TerminalDisplay->AppendText(formatOutput(output));
    });
}

//----- Terminal Window Destructor -----
TerminalWindow::~TerminalWindow()
{
    if (Global::AdapterInstance.getConnected() == true)
    {
        Global::AdapterInstance.disconnect();
    }
    std::cerr << "Terminal Window Closed" << std::endl;
}

//----- Helper Methods -----
wxString TerminalWindow::formatOutput(const std::string& text)
{
    return terminalTimestampOutput(text);
}

//----- Event Handlers -----
void TerminalWindow::OnEnterTerminal(wxCommandEvent& event)
{
    wxTextCtrl* Terminal = static_cast<wxTextCtrl*>(event.GetEventObject());
    wxString TText = Terminal->GetValue();
    Terminal->SetValue("");

    std::cerr << "User entered: " << TText.c_str() << std::endl;

    // Display user input in terminal
    TerminalDisplay->AppendText(formatOutput(std::string(TText.ToUTF8()) + "\n"));

    // Process command through the controller
    std::string input(TText.ToUTF8());
    m_controller.processCommand(input);
}
