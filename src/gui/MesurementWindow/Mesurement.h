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


class PlotWindow : public wxDialog
{
public:
    PlotWindow(wxWindow *parent);
    virtual ~PlotWindow();
protected:
    wxChoice* selectMesurement;
    //Plot var
    mpWindow* plot;
    mpFXYVector* vectorLayer;
private:

    wxString filePath = System::filePathSystem;
    wxArrayString fileNames;

    void getFileNames(const wxString& dirPath, wxArrayString& files);
    void executeScriptEvent(wxCommandEvent& event);
    void updatePlotData();
    void MeasurementWorkerThread(const wxString& dirPath, 
                                    const wxString& fileSkript, 
                                    sData* MessErgebnisse, // muss als pointer übergeben werden
                                    int mesurementNumber);
    std::vector<double> x;
    std::vector<double> y;

    // Threading members
    std::thread m_measurementThread;
    std::atomic<bool> m_stopMeasurement{false};

    // Mesurement var
    sData MessErgebnisse;
    int mesurementNumber = 1;

    int ptsX;
    int ptsY;

    //device Class
    //GpibDevice Adapter;
};

class PlotWindowSetMarker : public wxDialog
{
	private:

        bool Marker1FreqSet = false;
        bool Marker1MaxSet = false;


        wxString Marker1Freq;
        wxString Marker1Unit;
        wxString FreqMarker1Raw;

        bool Marker2FreqSet = false;
        bool Marker2MaxSet = false;

        wxString Marker2Freq;
        wxString Marker2Unit;
        wxString FreqMarker2Raw;

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
        int sliderY;
        int sliderX;
        int sliderScale;
        int progressbar;
        int currentMesurmentPoint;
        int totalMesurmentPoints;

	protected:
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

        // Set Default values
        wxString X_Messpunkte   = "1";
        wxString Y_Messpunkte   = "1";
        wxString X_Cord         = "0";
        wxString Y_Cord         = "0";
        wxString X_MessAbstand  = "10";
        wxString Y_MessAbstand  = "10";

        wxString startFreq      = "50";
        wxString startFreqUnit;
        wxString stopFreq       = "100";
        wxString stopFreqUnit;
        wxString AnzSweepMessPkt= "512";

        int unsigned currentMesurmentPoint = 0;
        int unsigned totalMesurmentPoints;

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
