#include "helpWin.h"
#include "FsuMeasurement.h"
#include "GpibUsbAdapter.h"

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
	m_displayLastErrorButton = new wxButton(panelHelp, wxID_ANY, "Display Last Error", wxPoint(10, 0));

#ifdef __linux__
	m_unloadAdapterDriverButton = new wxButton(panelHelp, wxID_ANY, "Unload Adapter Driver", wxPoint(10, 0));
#endif

	// Output area
	wxStaticText* discHelpOutput = new wxStaticText(panelHelp, wxID_ANY, "Function output:");
	m_textHelpOutput = new wxTextCtrl(panelHelp, wxID_ANY, "", wxDefaultPosition, wxSize(300, 180), wxTE_MULTILINE);

	// Bindings
	resetAdapterButton->    Bind(wxEVT_BUTTON, &HelpWin::OnResetAdapter,     this);
	resetDeviceButton->     Bind(wxEVT_BUTTON, &HelpWin::OnResetDevice,      this);
	getAdapterStatusButton->Bind(wxEVT_BUTTON, &HelpWin::OnGetAdapterStatus, this);
	getDeviceStatusButton-> Bind(wxEVT_BUTTON, &HelpWin::OnGetDeviceStatus,  this);
	m_displayLastErrorButton->Bind(wxEVT_BUTTON, &HelpWin::OnDisplayLastError, this);
	m_displayLastErrorButton->Bind(wxEVT_UPDATE_UI, &HelpWin::OnUpdateDisplayLastErrorButton, this);

#ifdef __linux__
	m_unloadAdapterDriverButton->Bind(wxEVT_BUTTON, &HelpWin::OnUnloadAdapterDriver, this);
#endif

	// Layout
	wxBoxSizer* sizerHelp = new wxBoxSizer(wxVERTICAL);
	sizerHelp->Add(discHelpInput,          0, wxEXPAND | wxALL, 10);
	sizerHelp->Add(resetAdapterButton,     0, wxEXPAND | wxALL, 10);
	sizerHelp->Add(resetDeviceButton,      0, wxEXPAND | wxALL, 10);
	sizerHelp->Add(getAdapterStatusButton, 0, wxEXPAND | wxALL, 10);
	sizerHelp->Add(getDeviceStatusButton,  0, wxEXPAND | wxALL, 10);
	sizerHelp->Add(m_displayLastErrorButton, 0, wxEXPAND | wxALL, 10);
#ifdef __linux__
	sizerHelp->Add(m_unloadAdapterDriverButton, 0, wxEXPAND | wxALL, 10);
#endif
	sizerHelp->Add(discHelpOutput,         0, wxEXPAND | wxALL, 10);
	sizerHelp->Add(m_textHelpOutput,       0, wxEXPAND | wxALL, 10);
	panelHelp->SetSizerAndFit(sizerHelp);
}

//----- Help Window Destructor -----
HelpWin::~HelpWin()
{
	if (m_document)
		m_document->RemoveObserver(this);
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

//----- Update View -----
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
// ---- Button Functions
//----- Reset Devices Button Pressed ------
void HelpWin::OnResetDevice(wxCommandEvent& event)
{
	if (m_document) m_document->ResetDevice();
}
//----- Get Adapter Status Button Pressed ------
void HelpWin::OnGetAdapterStatus(wxCommandEvent& event)
{
	if (m_document) m_document->GetAdapterStatus();
}
//----- Get Adapter Status Button Pressed ------
void HelpWin::OnGetDeviceStatus(wxCommandEvent& event)
{
	if (m_document) m_document->GetDeviceStatus();
}

/**
 * @brief Function to unload the drivers that block the FTDI Drivers For the Prologix Adapter
 */
void HelpWin::OnUnloadAdapterDriver(wxCommandEvent& event)
{
#ifdef __linux__
	int response = wxMessageBox(
		"To unload Adapter Drivers make sure programm is running as SU.\n
		Please disconnect all other USB Devices",
		"Unload Adapter Driver",
		wxYES_NO | wxICON_WARNING,
		this);

	if (response == wxYES)
	{
		PrologixUsbGpibAdapter::get_instance().prepareFTDIDevice();
	}
#else
	(void)event;
#endif
}
// --------- Get the Last Measurement Error
void HelpWin::OnDisplayLastError(wxCommandEvent& event)
{
	(void)event;
	auto& fsu = fsuMeasurement::get_instance();
	std::string error = fsu.getLastError();

	if (error.empty())
	{
		wxMessageBox("No error is currently stored.", "Last Error", wxOK | wxICON_INFORMATION, this);
		return;
	}

	wxMessageBox(wxString::FromUTF8(error), "Last Error", wxOK | wxICON_INFORMATION, this);
}

void HelpWin::OnUpdateDisplayLastErrorButton(wxUpdateUIEvent& event)
{
	event.Enable(!fsuMeasurement::get_instance().getLastError().empty());
}
