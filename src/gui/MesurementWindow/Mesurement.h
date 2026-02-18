#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <atomic>
#include <wx/wx.h>
#include <wx/tokenzr.h>


#include "main.h"
#include "dataManagement.h"
#include "fkt_GPIB.h"
#include "fkt_d2xx.h"
#include "systemInfo.h"

#include "mathplot.h"
// Temporarily disabled for debugging
#include "MeasurementController.h"
#include "Measurement2DController.h"
#include "MultiMessWindowController.h"


class PlotWindow : public wxDialog
{
public:
    PlotWindow(wxWindow *parent);
    virtual ~PlotWindow();
protected:
    wxChoice* m_selectMesurement;
    //Plot var
    mpWindow* m_plot;
    mpFXYVector* m_vectorLayer;
private:

    wxString m_filePath = System::filePathSystem;
    wxArrayString m_fileNames;

    void getFileNames(const wxString& dirPath, wxArrayString& files);
    void executeScriptEvent(wxCommandEvent& event);
    void updatePlotData();
    void MeasurementWorkerThread(const wxString& dirPath, 
                                    const wxString& fileSkript, 
                                    sData* MessErgebnisse, // muss als pointer übergeben werden
                                    int mesurementNumber);
    std::vector<double> m_x;
    std::vector<double> m_y;

    // Threading members
    std::thread m_measurementThread;
    std::atomic<bool> m_stopMeasurement{false};

    // Mesurement var
    sData m_MessErgebnisse;
    int m_mesurementNumber = 1;

    int m_ptsX;
    int m_ptsY;

    // Business logic controller - disabled for debugging
    MeasurementController m_MeasurementLogic;

    // Helper for formatting output with timestamp
    wxString formatOutput(const std::string& text);

    //device Class
    //GpibDevice Adapter;
};

class PlotWindowSetMarker : public wxDialog
{
	private:

        bool m_Marker1FreqSet = false;
        bool m_Marker1MaxSet = false;


        wxString m_Marker1Freq;
        wxString m_Marker1Unit;
        wxString m_FreqMarker1Raw;

        bool m_Marker2FreqSet = false;
        bool m_Marker2MaxSet = false;

        wxString m_Marker2Freq;
        wxString m_Marker2Unit;
        wxString m_FreqMarker2Raw;

	protected:
		wxCheckBox* m_checkBox1;
        wxCheckBox* m_checkBox2;
        wxCheckBox* m_checkBox3;
        wxCheckBox* m_checkBox4;

		wxTextCtrl* m_textCtrl1;
        wxTextCtrl* m_textCtrl2;

		wxChoice* m_choice1;
		wxChoice* m_choice2;

		wxButton* m_button1;
		wxButton* m_button2;

	public:
		PlotWindowSetMarker( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("Set Marker"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 417,300 ), long style = wxDEFAULT_FRAME_STYLE|wxTAB_TRAVERSAL );
		~PlotWindowSetMarker();

        void toggleSelection1(  wxCommandEvent& event);
        void toggleSelection2(  wxCommandEvent& event);
        void SetSelection1(     wxCommandEvent& event);
        void SetSelection2(     wxCommandEvent& event);
        void toggleSelection1fkt();
        void toggleSelection2fkt();
        void GetSelectedValue1();
        void GetSelectedValue2();

        void GetValues();

};

class Mesurement2D : public wxDialog
{
	private:
        int m_sliderY;
        int m_sliderX;
        int m_sliderScale;
        int m_progressbar;
        int m_currentMesurmentPoint;
        int m_totalMesurmentPoints;

        wxStaticText* m_staticText1;
		wxStaticText* m_staticText2;
        wxStaticText* m_staticText3;
        wxStaticText* m_staticText4;
        wxStaticText* m_staticText5;
        wxStaticText* m_staticText6;
		wxStaticText* m_staticText7;


		wxSlider* m_slider1;
		wxSlider* m_slider2;
		wxSlider* m_slider3;

		wxChoice* m_choice1;

        wxButton* m_button1;
		wxButton* m_button2;
		wxButton* m_button3;
		wxButton* m_button4;
		wxButton* m_button5;

		wxGauge* m_gauge1;

        // Business logic controller - disabled for debugging
        Measurement2DController m_MeasuementLogic;

	public:

		Mesurement2D( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("2D Mesurement"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 421,345 ), long style = wxDEFAULT_DIALOG_STYLE );
		~Mesurement2D();
        //Gui Event Functions
        void OnSliderUpdate(wxCommandEvent& event);
        void OnReset(       wxCommandEvent& event);
        void OnSettings(    wxCommandEvent& event);
        void OnStart(       wxCommandEvent& event);
        void OnRestart(     wxCommandEvent& event);
        void OnStop(        wxCommandEvent& event);
        //Gui Helper Functions
        void GetValues();
        void SetValues();
        void GetTotalMesurements();
        void incrementCurrentMesurmentPoint();
        void updateProgressBar();
        void resetProgressBar();
        void SetSliderValues();
        void resetGuiValues();

        //Mesurement Functions
        void singleMesurement();

};


class MultiMessWindow : public wxDialog
{
	private:
        //button functions
        void startButton(   wxCommandEvent& event);
        void stopButton(    wxCommandEvent& event);
        void resetButton(   wxCommandEvent& event);
        void nextButton(    wxCommandEvent& event);

        //Helper functions
        void GetValues();
        void SetValues();
        void UpdateProgressBar();

        void testMessFunction();

        // Set Default values (now managed by controller)
        wxString m_X_Messpunkte   = "1";
        wxString m_Y_Messpunkte   = "1";
        wxString m_X_Cord         = "0";
        wxString m_Y_Cord         = "0";
        wxString m_X_MessAbstand  = "10";
        wxString m_Y_MessAbstand  = "10";

        wxString m_startFreq      = "50";
        wxString m_startFreqUnit;
        wxString m_stopFreq       = "100";
        wxString m_stopFreqUnit;
        wxString m_AnzSweepMessPkt= "512";

        int unsigned m_currentMesurmentPoint = 0;
        int unsigned m_totalMesurmentPoints;

        // Business logic controller - disabled for debugging
        // MultiMessWindowController m_MultiMessLogic;

        //sData Data1D;
        //sData3D Data3D;

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

	public:

		MultiMessWindow( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("Multi Punkt Messung"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 621,563 ), long style = wxDEFAULT_DIALOG_STYLE );
		~MultiMessWindow();

};
