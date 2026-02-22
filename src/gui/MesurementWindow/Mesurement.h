#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <wx/wx.h>
#include <wx/tokenzr.h>
#include <wx/dir.h>
#include <wx/textfile.h>
#include <wx/msgdlg.h>
#include <wx/statline.h>

#include "MainDocument.h"
#include "dataManagement.h"
#include "fkt_GPIB.h"
#include "fkt_d2xx.h"
#include "systemInfo.h"

#include "mathplot.h"
#include "MeasurementDocument.h"
#include "MultiMessDocument.h"


/**
 * @brief Pure View for measurement plots. Observes a MeasurementDocument.
 *
 * This is a modeless wxFrame so multiple instances can be open simultaneously,
 * each with its own MeasurementDocument and independently imported data.
 */
class PlotWindow : public wxFrame, public IMeasurementObserver
{
public:
    /**
     * @param parent     wx parent window
     * @param mainDoc    pointer to the application-level MainDocument
     *                   (may be nullptr; used to pre-populate the plot).
     */
    explicit PlotWindow(wxWindow* parent, MainDocument* mainDoc = nullptr);
    virtual ~PlotWindow();

    /** Attach / detach document. Registers/unregisters this as observer. */
    void SetDocument(MeasurementDocument* doc);

    /** Set document ownership: if true, this window deletes the document on close. */
    void SetOwnsDocument(bool owns) { m_ownsDocument = owns; }

    // IMeasurementObserver
    void OnDocumentChanged(const std::string& changeType) override;

    /** Static counter for unique window titles */
    static int s_windowCounter;

    /** Public accessors for plot components (used when pushing imported data) */
    mpWindow*    GetPlot()        const { return m_plot; }
    mpFXYVector* GetVectorLayer() const { return m_vectorLayer; }

protected:
    wxChoice*     m_selectMesurement;
    mpWindow*     m_plot;
    mpFXYVector*  m_vectorLayer;
    wxMenuBar*    m_menuBar;

    // Layout panels
    wxPanel*      m_plotPanel;   ///< Container that constrains the plot to a square
    wxPanel*      m_infoPanel;   ///< Right-side info area (placeholder)
    wxStaticText* m_infoText;    ///< Placeholder text inside the info panel

    // Matrix measurement selector [x ; y]
    wxTextCtrl*   m_textXSelector;
    wxTextCtrl*   m_textYSelector;

private:
    wxString      m_filePath = System::filePathSystem;
    wxArrayString m_fileNames;

    void getFileNames(const wxString& dirPath, wxArrayString& files);
    void executeScriptEvent(wxCommandEvent& event);
    void OnSelectMeasurement(wxCommandEvent& event);
    void updatePlotData();
    void OnClose(wxCloseEvent& event);

    // Menu handlers (File)
    void OnMenuFileOpen(wxCommandEvent& event);
    void OnMenuFileClose(wxCommandEvent& event);
    void OnMenuFileSave(wxCommandEvent& event);
    void OnMenuFileSaveAs(wxCommandEvent& event);
    void OnMenuFileExit(wxCommandEvent& event);

    // Menu handlers (Measurement) — forward to parent MainProgrammWin
    void OnMenuMesurementNew(wxCommandEvent& event);
    void OnMenuMesurementOpen(wxCommandEvent& event);
    void OnMenuMesurementLoad(wxCommandEvent& event);
    void OnMenuMesurementPreset1(wxCommandEvent& event);
    void OnMenuMesurementPreset2(wxCommandEvent& event);
    void OnMenuMesurementPreset3(wxCommandEvent& event);
    void OnMenuMesurement2DMess(wxCommandEvent& event);
    void OnMenuMesurementSetMarker(wxCommandEvent& event);
    void OnMenuMesurementSettings(wxCommandEvent& event);

    // Helper for formatting output with timestamp
    wxString formatOutput(const std::string& text);

    // Owning pointer to document — each window owns its own document
    MeasurementDocument* m_document = nullptr;
    bool m_ownsDocument = false;

    // Non-owning pointer to application-level document (for pre-load)
    MainDocument* m_mainDoc = nullptr;

    int m_mesurementNumber = 1;
    int m_windowId = 0;
};

/**
 * @brief Marker-set dialog. Uses a MeasurementDocument to write SCPI marker commands.
 */
class PlotWindowSetMarker : public wxDialog
{
private:
    bool m_Marker1FreqSet = false;
    bool m_Marker1MaxSet  = false;
    wxString m_Marker1Freq;
    wxString m_Marker1Unit;
    wxString m_FreqMarker1Raw;

    bool m_Marker2FreqSet = false;
    bool m_Marker2MaxSet  = false;
    wxString m_Marker2Freq;
    wxString m_Marker2Unit;
    wxString m_FreqMarker2Raw;

    // Non-owning pointer to document
    MeasurementDocument* m_document = nullptr;

protected:
    wxCheckBox* m_checkBox1;
    wxCheckBox* m_checkBox2;
    wxCheckBox* m_checkBox3;
    wxCheckBox* m_checkBox4;
    wxTextCtrl* m_textCtrl1;
    wxTextCtrl* m_textCtrl2;
    wxChoice*   m_choice1;
    wxChoice*   m_choice2;
    wxButton*   m_button1;
    wxButton*   m_button2;

public:
    PlotWindowSetMarker(wxWindow* parent,
                        wxWindowID id    = wxID_ANY,
                        const wxString& title = wxT("Set Marker"),
                        const wxPoint& pos    = wxDefaultPosition,
                        const wxSize&  size   = wxSize(417, 300),
                        long style = wxDEFAULT_FRAME_STYLE | wxTAB_TRAVERSAL);
    ~PlotWindowSetMarker();

    /** Attach / detach document. */
    void SetDocument(MeasurementDocument* doc);

    void toggleSelection1(wxCommandEvent& event);
    void toggleSelection2(wxCommandEvent& event);
    void SetSelection1(wxCommandEvent& event);
    void SetSelection2(wxCommandEvent& event);
    void toggleSelection1fkt();
    void toggleSelection2fkt();
    void GetSelectedValue1();
    void GetSelectedValue2();
    void GetValues();
};

/**
 * @brief Pure View for multi-point measurements. Observes a MultiMessDocument.
 */
class MultiMessWindow : public wxDialog, public IMultiMessObserver
{
public:
    explicit MultiMessWindow(wxWindow* parent,
                             wxWindowID id    = wxID_ANY,
                             const wxString& title = wxT("Multi Punkt Messung"),
                             const wxPoint& pos    = wxDefaultPosition,
                             const wxSize&  size   = wxSize(621, 563),
                             long style = wxDEFAULT_DIALOG_STYLE);
    ~MultiMessWindow();

    /** Attach / detach document. Registers/unregisters this as observer. */
    void SetDocument(MultiMessDocument* doc);

    // IMultiMessObserver
    void OnMultiMessDocumentChanged(const std::string& changeType) override;

private:
    void startButton(wxCommandEvent& event);
    void stopButton (wxCommandEvent& event);
    void resetButton(wxCommandEvent& event);
    void nextButton (wxCommandEvent& event);

    void PushValuesToDocument();
    void PullValuesFromDocument();
    void UpdateProgressBar();
    void SetValues();

    // Non-owning pointer to document
    MultiMessDocument* m_document = nullptr;

protected:
        // Label text
		wxStaticText*   m_staticTextXMess;
        wxStaticText*   m_staticTextYMess;
        wxStaticText*   m_staticTextXStartCord;
        wxStaticText*   m_staticTextYStartCord;
        wxStaticText*   m_staticTextXAbstand;
        wxStaticText*   m_staticTextYAbstand;
        wxStaticText*   m_staticTextStrtFreq;
        wxStaticText*   m_staticTextEndFreq;
        wxStaticText*   m_staticTextAnzahlSweep;
        wxStaticText*   m_staticTextProgress;
        // Text input
		wxTextCtrl*     m_textCtrlXMess;
        wxTextCtrl*     m_textCtrlYMess;
		wxTextCtrl*     m_textCtrlXStartCord;
		wxTextCtrl*     m_textCtrlYStartCord;
		wxTextCtrl*     m_textCtrlXAbstand;
		wxTextCtrl*     m_textCtrlYAbstand;
		wxTextCtrl*     m_textCtrlStrtFreq;
		wxTextCtrl*     m_textCtrlEndFreq;
        wxTextCtrl*     m_textCtrlAnzahlSweep;
        // Unit selection
		wxChoice*       m_choiceEinheitFreq1;
		wxChoice*       m_choiceEinheitFreq2;
		// buttons
		wxButton*       m_buttonStart;
		wxButton*       m_buttonStop;
		wxButton*       m_buttonReset;
		wxButton*       m_buttonNext;
        // Progressbar
		wxGauge*        m_gaugeProgress;
};
