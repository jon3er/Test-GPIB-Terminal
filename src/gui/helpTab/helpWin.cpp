#include "helpWin.h"

//----- Help Window Constructor -----
HelpWin::HelpWin(wxWindow* parent)
	: wxDialog(parent, wxID_ANY, "Reset Devices", wxDefaultPosition, wxSize(420, 420))
{
	wxPanel* panelHelp = new wxPanel(this);

	wxStaticText* discHelpInput = new wxStaticText(panelHelp, wxID_ANY, "Reset / Status functions:");

	// Buttons
	wxButton* resetAdapterButton    = new wxButton(panelHelp, wxID_ANY, "Reset Adapter",     wxPoint(10, 0));
	wxButton* resetDeviceButton     = new wxButton(panelHelp, wxID_ANY, "Reset Device",      wxPoint(10, 0));
	wxButton* getAdapterStatusButton= new wxButton(panelHelp, wxID_ANY, "Get Adapter Status", wxPoint(10, 0));
	wxButton* getDeviceStatusButton = new wxButton(panelHelp, wxID_ANY, "Get Device Status",  wxPoint(10, 0));

	// Output area
	wxStaticText* discHelpOutput = new wxStaticText(panelHelp, wxID_ANY, "Function output:");
	m_textHelpOutput = new wxTextCtrl(panelHelp, wxID_ANY, "", wxDefaultPosition, wxSize(300, 180), wxTE_MULTILINE);

	// Bindings
	resetAdapterButton->    Bind(wxEVT_BUTTON, &HelpWin::OnResetAdapter,     this);
	resetDeviceButton->     Bind(wxEVT_BUTTON, &HelpWin::OnResetDevice,      this);
	getAdapterStatusButton->Bind(wxEVT_BUTTON, &HelpWin::OnGetAdapterStatus, this);
	getDeviceStatusButton-> Bind(wxEVT_BUTTON, &HelpWin::OnGetDeviceStatus,  this);

	// Layout
	wxBoxSizer* sizerHelp = new wxBoxSizer(wxVERTICAL);
	sizerHelp->Add(discHelpInput,          0, wxEXPAND | wxALL, 10);
	sizerHelp->Add(resetAdapterButton,     0, wxEXPAND | wxALL, 10);
	sizerHelp->Add(resetDeviceButton,      0, wxEXPAND | wxALL, 10);
	sizerHelp->Add(getAdapterStatusButton, 0, wxEXPAND | wxALL, 10);
	sizerHelp->Add(getDeviceStatusButton,  0, wxEXPAND | wxALL, 10);
	sizerHelp->Add(discHelpOutput,         0, wxEXPAND | wxALL, 10);
	sizerHelp->Add(m_textHelpOutput,       0, wxEXPAND | wxALL, 10);
	panelHelp->SetSizerAndFit(sizerHelp);
}

//----- Help Window Destructor -----
HelpWin::~HelpWin()
{
}

//----- Document binding -----
void HelpWin::SetDocument(HelpDocument* document)
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

//----- IHelpObserver -----
void HelpWin::OnHelpDocumentChanged(const std::string& changeType)
{
	if (changeType == "OutputAppended")
	{
		if (m_textHelpOutput && m_document)
			m_textHelpOutput->SetValue(wxString::FromUTF8(m_document->GetOutputLog()));
	}
}

void HelpWin::UpdateView()
{
	if (!m_document) return;
	if (m_textHelpOutput)
		m_textHelpOutput->SetValue(wxString::FromUTF8(m_document->GetOutputLog()));
}

//----- Event Handlers -----
void HelpWin::OnResetAdapter(wxCommandEvent& event)
{
	if (m_document) m_document->ResetAdapter();
}

void HelpWin::OnResetDevice(wxCommandEvent& event)
{
	if (m_document) m_document->ResetDevice();
}

void HelpWin::OnGetAdapterStatus(wxCommandEvent& event)
{
	if (m_document) m_document->GetAdapterStatus();
}

void HelpWin::OnGetDeviceStatus(wxCommandEvent& event)
{
	if (m_document) m_document->GetDeviceStatus();
}
