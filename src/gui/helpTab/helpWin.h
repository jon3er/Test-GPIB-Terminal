#pragma once

#include <wx/wx.h>
#include "helpDocument.h"

class HelpWin : public wxDialog, public IHelpObserver
{
public:
	HelpWin(wxWindow* parent);
	virtual ~HelpWin();

	/** Attach / detach the document. Pass nullptr to detach before destruction. */
	void SetDocument(HelpDocument* document);

	// IHelpObserver
	void OnHelpDocumentChanged(const std::string& changeType) override;

private:
	// Event handlers
	void OnResetAdapter    (wxCommandEvent& event);
	void OnResetDevice     (wxCommandEvent& event);
	void OnGetAdapterStatus(wxCommandEvent& event);
	void OnGetDeviceStatus (wxCommandEvent& event);
	void OnUnloadAdapterDriver(wxCommandEvent& event);
	void OnDisplayLastError(wxCommandEvent& event);
	void OnUpdateDisplayLastErrorButton(wxUpdateUIEvent& event);

	/** Rebuild the view from current document state. */
	void UpdateView();

	// GUI Components
	wxTextCtrl* m_textHelpOutput = nullptr;
	wxButton* m_displayLastErrorButton = nullptr;
	wxButton* m_unloadAdapterDriverButton = nullptr;

	// Document (non-owning pointer)
	HelpDocument* m_document = nullptr;
};
