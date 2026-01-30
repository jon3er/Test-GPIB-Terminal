#include "main.h"


//-----Plot Window BEGIN--------
PlotWindow::PlotWindow(wxWindow *parent) : wxDialog(parent, wxID_ANY, "Plot Window", wxDefaultPosition, wxSize(1000,750))
{
    getFileNames(filePath, fileNames);

    wxButton* executeMesurment = new wxButton(this, wxID_ANY, "Execute Mesurement");
    executeMesurment->Bind(wxEVT_BUTTON, &PlotWindow::executeScriptEvent,this);
    selectMesurement = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, fileNames);
    selectMesurement->SetSelection(0);
    // 1. mpWindow (Zeichenfläche) erstellen
    plot = new mpWindow(this, wxID_ANY);

    // Farbeinstellungen (Optional)
    plot->SetMargins(30, 30, 50, 50);

    // 2. Achsen als Layer hinzufügen
    // mpScaleX(Name, Ausrichtung, Ticks anzeigen, Typ)
    mpScaleX* xAxis = new mpScaleX("X-Achse", mpALIGN_BORDER_BOTTOM, true, mpX_NORMAL);
    mpScaleY* yAxis = new mpScaleY("Y-Achse", mpALIGN_BORDER_LEFT, true);

    plot->AddLayer(xAxis);
    plot->AddLayer(yAxis);

    x = {0.0, 1.0, 2.0, 3.0, 4.0, 5.0 };
    y = {0.0, 1.0, 4.0, 2.0, 5.0, 3.0 };

    // 3. Daten vorbereiten (std::vector laut Header Definition von mpFXYVector)

    //Holt in dem Haupt menu geladene daten
    MainProgrammWin* MainFrame = dynamic_cast<MainProgrammWin*>(parent);
    if (MainFrame != nullptr)
    {
        if (MainFrame->isFileOpen())
        {
            sData temp = MainFrame->returnOpendData();
            sData::sParam *tempStruct = temp.GetParameter();
            temp.GetData(tempStruct, x,y);
        }
    }



    // 4. Vektor-Layer erstellen
    vectorLayer = new mpFXYVector("Messdaten");
    vectorLayer->SetData(x, y);
    vectorLayer->SetContinuity(true); // True = Linie zeichnen
    vectorLayer->SetPen(wxPen(*wxBLUE, 2, wxPENSTYLE_SOLID));
    vectorLayer->ShowName(true);      // Wichtig für die Legende

    plot->AddLayer(vectorLayer);

    // 5. Legende hinzufügen (mpInfoLegend ist ein Layer)
    // wxRect definiert Startposition und ungefähre Größe
    mpInfoLegend* legend = new mpInfoLegend(wxRect(20, 20, 10, 10), wxTRANSPARENT_BRUSH);
    legend->SetItemMode(mpLEGEND_LINE); // Zeigt Linie statt Quadrat in der Legende
    plot->AddLayer(legend);

    // 6. Layout-Management
    wxBoxSizer* sizerButtons = new wxBoxSizer(wxHORIZONTAL);
    sizerButtons->Add(executeMesurment, 0, wxEXPAND | wxALL);
    sizerButtons->Add(selectMesurement, 0, wxEXPAND | wxALL);

    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    sizer->Add(plot, 1, wxEXPAND | wxALL, 5);
    sizer->Add(sizerButtons, 0, wxEXPAND| wxALL , 5);
    this->SetSizer(sizer);
    this->Layout();
    // 7. Zoom auf Daten anpassen
    plot->Fit();

}
PlotWindow::~PlotWindow()
{
    Global::AdapterInstance.disconnect();
}

void PlotWindow::getFileNames(const wxString& dirPath, wxArrayString& files)
{
    wxDir dir(dirPath);

    if (!dir.IsOpened())
    {
        return;
    }

    wxString filename;
    bool cont = dir.GetFirst(&filename, "*.txt", wxDIR_FILES);

    while (cont) {

        files.Add(filename);
        cont = dir.GetNext(&filename);
    }
}
void PlotWindow::executeScriptEvent(wxCommandEvent& event)
{
    wxArrayString logAdapterReceived;
    wxString fileName = selectMesurement->GetStringSelection();

    wxLogDebug("Reading Scriptfile...");
    Global::AdapterInstance.readScriptFile(filePath, fileName, logAdapterReceived);

    //output received msg
    for (size_t i = 0; i < logAdapterReceived.GetCount(); i++)
    {
        wxLogDebug(logAdapterReceived[i]);
    }
    sData MessErgebnisse;
    sData::sParam *MessInfo = MessErgebnisse.GetParameter();
    wxDateTime now = wxDateTime::Now();
    MessInfo->File = fileName;
    MessInfo->Date = now.FormatISODate();
    MessInfo->Time = now.FormatISOTime();

    y = Global::Messung.getX_Data(); //zum test vertauscht
    x = Global::Messung.getY_Data();

    MessErgebnisse.SetData(MessInfo, y, x);
    wxString filePathSave = System::filePathRoot + System::fileSystemSlash + "LogFiles" + System::fileSystemSlash + fileName;
    MessErgebnisse.saveToCsvFile(filePathSave);
    //test
    updatePlotData();
}
void PlotWindow::updatePlotData()
{
    vectorLayer->SetData(x, y);
    plot->Fit();
}

//-----Plot Window ENDE--------

//-----Plot Window Set Marker-------
PlotWindowSetMarker::PlotWindowSetMarker( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
    wxArrayString freqEinheiten;
    freqEinheiten.Add("Hz");
    freqEinheiten.Add("kHz");
    freqEinheiten.Add("MHz");
    freqEinheiten.Add("GHz");

	this->SetSizeHints( wxDefaultSize, wxDefaultSize );

	wxBoxSizer* bSizer1;
	bSizer1 = new wxBoxSizer( wxVERTICAL );

	wxStaticBoxSizer* sbSizer1;
	sbSizer1 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Marker 1") ), wxVERTICAL );

	wxBoxSizer* bSizer2;
	bSizer2 = new wxBoxSizer( wxHORIZONTAL );

	m_checkBox1 = new wxCheckBox( sbSizer1->GetStaticBox(), wxID_ANY, wxT("Set to Frequency"), wxDefaultPosition, wxDefaultSize, 0 );
	m_checkBox1->Bind(wxEVT_CHECKBOX, &PlotWindowSetMarker::toggleSelection1, this);
    bSizer2->Add( m_checkBox1, 1, wxALL, 5 );

	m_textCtrl1 = new wxTextCtrl( sbSizer1->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	bSizer2->Add( m_textCtrl1, 1, wxALL, 5 );

	m_choice1 = new wxChoice( sbSizer1->GetStaticBox(), wxID_ANY, wxDefaultPosition, wxDefaultSize, freqEinheiten, 0 );
	m_choice1->SetSelection( 0 );
	bSizer2->Add( m_choice1, 1, wxALL, 5 );


	sbSizer1->Add( bSizer2, 1, wxEXPAND, 5 );

	wxBoxSizer* bSizer6;
	bSizer6 = new wxBoxSizer( wxVERTICAL );

	m_checkBox2 = new wxCheckBox( sbSizer1->GetStaticBox(), wxID_ANY, wxT("Set to highest freq"), wxDefaultPosition, wxDefaultSize, 0 );
	m_checkBox2->Bind(wxEVT_CHECKBOX, &PlotWindowSetMarker::toggleSelection1, this);
    bSizer6->Add( m_checkBox2, 0, wxALL, 5 );


	sbSizer1->Add( bSizer6, 1, wxEXPAND, 5 );

	m_button1 = new wxButton( sbSizer1->GetStaticBox(), wxID_ANY, wxT("Set"), wxDefaultPosition, wxDefaultSize, 0 );
	m_button1->Bind(wxEVT_BUTTON, &PlotWindowSetMarker::SetSelection1, this);
    sbSizer1->Add( m_button1, 1, wxALL, 5 );


	bSizer1->Add( sbSizer1, 1, wxEXPAND, 5 );

	wxStaticBoxSizer* sbSizer11;
	sbSizer11 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Marker 2") ), wxVERTICAL );

	wxBoxSizer* bSizer21;
	bSizer21 = new wxBoxSizer( wxHORIZONTAL );

	m_checkBox3 = new wxCheckBox( sbSizer11->GetStaticBox(), wxID_ANY, wxT("Set to Frequency"), wxDefaultPosition, wxDefaultSize, 0 );
	m_checkBox3->Bind(wxEVT_CHECKBOX, &PlotWindowSetMarker::toggleSelection2, this);
    bSizer21->Add( m_checkBox3, 1, wxALL, 5 );

	m_textCtrl2 = new wxTextCtrl( sbSizer11->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	bSizer21->Add( m_textCtrl2, 1, wxALL, 5 );


	m_choice2 = new wxChoice( sbSizer11->GetStaticBox(), wxID_ANY, wxDefaultPosition, wxDefaultSize, freqEinheiten, 0 );
	m_choice2->SetSelection( 0 );
	bSizer21->Add( m_choice2, 1, wxALL, 5 );


	sbSizer11->Add( bSizer21, 1, wxEXPAND, 5 );

	wxBoxSizer* bSizer5;
	bSizer5 = new wxBoxSizer( wxVERTICAL );

	m_checkBox4 = new wxCheckBox( sbSizer11->GetStaticBox(), wxID_ANY, wxT("Set to highest Freq"), wxDefaultPosition, wxDefaultSize, 0 );
	m_checkBox4->Bind(wxEVT_CHECKBOX, &PlotWindowSetMarker::toggleSelection2, this);
    bSizer5->Add( m_checkBox4, 1, wxALL, 5 );


	sbSizer11->Add( bSizer5, 1, wxEXPAND, 5 );

	m_button2 = new wxButton( sbSizer11->GetStaticBox(), wxID_ANY, wxT("Set"), wxDefaultPosition, wxDefaultSize, 0 );
	m_button2->Bind(wxEVT_BUTTON, &PlotWindowSetMarker::SetSelection2, this);
    sbSizer11->Add( m_button2, 1, wxALL, 5 );


	bSizer1->Add( sbSizer11, 1, wxEXPAND, 5 );


	this->SetSizer( bSizer1 );
	this->Layout();

	this->Centre( wxBOTH );

    GetValues();
    toggleSelection1fkt();
    toggleSelection2fkt();

}
PlotWindowSetMarker::~PlotWindowSetMarker()
{
}

void PlotWindowSetMarker::GetValues()
{
    Marker1FreqSet      = m_checkBox1->GetValue();
    Marker1MaxSet       = m_checkBox2->GetValue();

    Marker2FreqSet      = m_checkBox3->GetValue();
    Marker2MaxSet       = m_checkBox4->GetValue();

    Marker1Freq         = m_textCtrl1->GetValue();
    Marker1Unit         = m_choice1->GetStringSelection();
    Marker2Freq         = m_textCtrl2->GetValue();
    Marker2Unit         = m_choice2->GetStringSelection();

}

void PlotWindowSetMarker::toggleSelection1fkt()
{
    if (!Marker1FreqSet && !Marker1MaxSet)
    {
        m_checkBox1->SetValue(true);
        m_checkBox2->SetValue(false);
        m_textCtrl1->Enable(true);
        m_choice1->Enable(true);
    }
    else if (!Marker1FreqSet && Marker1MaxSet)
    {
        m_checkBox1->SetValue(true);
        m_checkBox2->SetValue(false);
        m_textCtrl1->Enable(true);
        m_choice1->Enable(true);
    }
    else if (Marker1FreqSet && !Marker1MaxSet)
    {
        m_checkBox1->SetValue(false);
        m_checkBox2->SetValue(true);
        m_textCtrl1->Enable(false);
        m_choice1->Enable(false);
    }

    GetValues();
}
void PlotWindowSetMarker::toggleSelection1(wxCommandEvent& event)
{
    toggleSelection1fkt();
}

void PlotWindowSetMarker::toggleSelection2fkt()
{
    if (!Marker2FreqSet && !Marker2MaxSet)
    {
        m_checkBox3->SetValue(true);
        m_checkBox4->SetValue(false);
        m_textCtrl2->Enable(true);
        m_choice2->Enable(true);
    }
    else if (!Marker2FreqSet && Marker2MaxSet)
    {
        m_checkBox3->SetValue(true);
        m_checkBox4->SetValue(false);
        m_textCtrl2->Enable(true);
        m_choice2->Enable(true);
    }
    else if (Marker2FreqSet && !Marker2MaxSet)
    {
        m_checkBox3->SetValue(false);
        m_checkBox4->SetValue(true);
        m_textCtrl2->Enable(false);
        m_choice2->Enable(false);
    }

    GetValues();
}
void PlotWindowSetMarker::toggleSelection2(wxCommandEvent& event)
{
    toggleSelection2fkt();
}
void PlotWindowSetMarker::GetSelectedValue1()
{
    const double multipliers[] = {
    1.0,        // Index 0: Hz
    1000.0,     // Index 1: kHz
    1.0e6,      // Index 2: MHz
    1.0e9       // Index 3: GHz
    };

    int selection = m_choice1->GetSelection();
    double val;

    if (!Marker1Freq.ToDouble(&val))
    {
        wxLogDebug("Failed to convert input");
        return;
    }

    if (selection != wxNOT_FOUND) {
        double factor = multipliers[selection];
        double frequencyHz =  val * factor;
        FreqMarker1Raw = wxString::FromCDouble(frequencyHz);
    }
}
void PlotWindowSetMarker::GetSelectedValue2()
{
    GetValues();

    const double multipliers[] = {
    1.0,        // Index 0: Hz
    1000.0,     // Index 1: kHz
    1.0e6,      // Index 2: MHz
    1.0e9       // Index 3: GHz
    };

    int selection = m_choice2->GetSelection();
    double val;

    if (!Marker1Freq.ToDouble(&val))
    {
        wxLogDebug("Failed to convert input");
        return;
    }

    if (selection != wxNOT_FOUND) {
        double factor = multipliers[selection];
        double frequencyHz =  val * factor;
        FreqMarker2Raw = wxString::FromCDouble(frequencyHz);
    }
}
void PlotWindowSetMarker::SetSelection1(wxCommandEvent& event)
{
    GetValues();

    if (Marker1MaxSet)
    {
        std::string Text = "CALC:MARK:MAX";
        Global::AdapterInstance.write(Text);
    }
    else if (Marker1FreqSet && (FreqMarker1Raw.IsNumber()))
    {
        GetSelectedValue1();

        std::string Text = "CALC:MARK:MAX " + std::string(FreqMarker1Raw.ToUTF8());;
        Global::AdapterInstance.write(Text);
    }
    //TODO Get X Y From Device and display in the Menu
}
void PlotWindowSetMarker::SetSelection2(wxCommandEvent& event)
{
    GetValues();

    if (Marker2MaxSet)
    {
        std::string Text = "CALC:MARK2:MAX";
        Global::AdapterInstance.write(Text);
    }
    else if (Marker2FreqSet && (FreqMarker2Raw.IsNumber()))
    {
        GetSelectedValue1();

        std::string Text = "CALC:MARK2:MAX " + std::string(FreqMarker2Raw.ToUTF8());;
        Global::AdapterInstance.write(Text);
    }
    //TODO Get X Y From Device and display in the Menu
}


Mesurement2D::Mesurement2D( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );

	wxBoxSizer* bSizer1;
	bSizer1 = new wxBoxSizer( wxVERTICAL );

	wxStaticBoxSizer* sbSizer1;
	sbSizer1 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Mesurment Settings") ), wxVERTICAL );

	m_staticText1 = new wxStaticText( sbSizer1->GetStaticBox(), wxID_ANY, wxT("Mesurement Points:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText1->Wrap( -1 );
	sbSizer1->Add( m_staticText1, 0, wxALL, 5 );

	wxBoxSizer* bSizer2;
	bSizer2 = new wxBoxSizer( wxHORIZONTAL );

	m_staticText2 = new wxStaticText( sbSizer1->GetStaticBox(), wxID_ANY, wxT("Y Points:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText2->Wrap( -1 );
	bSizer2->Add( m_staticText2, 1, wxALL, 5 );


	m_slider1 = new wxSlider( sbSizer1->GetStaticBox(), wxID_ANY, 10, 1, 100, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL );
	m_slider1->Bind(wxEVT_SLIDER, &Mesurement2D::OnSliderUpdate, this);
    bSizer2->Add( m_slider1, 1, wxALL, 5 );


	sbSizer1->Add( bSizer2, 1, wxEXPAND, 5 );

	wxBoxSizer* bSizer21;
	bSizer21 = new wxBoxSizer( wxHORIZONTAL );

	m_staticText3 = new wxStaticText( sbSizer1->GetStaticBox(), wxID_ANY, wxT("X Points:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText3->Wrap( -1 );
	bSizer21->Add( m_staticText3, 1, wxALL, 5 );

	m_slider2 = new wxSlider( sbSizer1->GetStaticBox(), wxID_ANY, 10, 1, 100, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL );
	m_slider2->Bind(wxEVT_SLIDER, &Mesurement2D::OnSliderUpdate, this);
    bSizer21->Add( m_slider2, 1, wxALL, 5 );


	sbSizer1->Add( bSizer21, 1, wxEXPAND, 5 );

	wxBoxSizer* bSizer211;
	bSizer211 = new wxBoxSizer( wxHORIZONTAL );

	m_staticText4 = new wxStaticText( sbSizer1->GetStaticBox(), wxID_ANY, wxT("Scale:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText4->Wrap( -1 );
	bSizer211->Add( m_staticText4, 1, wxALL, 5 );

	m_slider3 = new wxSlider( sbSizer1->GetStaticBox(), wxID_ANY, 100, 1, 100, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL );
	m_slider3->Bind(wxEVT_SLIDER, &Mesurement2D::OnSliderUpdate, this);
    bSizer211->Add( m_slider3, 1, wxALL, 5 );


	sbSizer1->Add( bSizer211, 1, wxEXPAND, 5 );

	wxBoxSizer* bSizer9;
	bSizer9 = new wxBoxSizer( wxHORIZONTAL );

	m_staticText5 = new wxStaticText( sbSizer1->GetStaticBox(), wxID_ANY, wxT("Orientation point:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText5->Wrap( -1 );
	bSizer9->Add( m_staticText5, 1, wxALL, 5 );

	wxArrayString StartPosition;
    StartPosition.Add("Center");
    StartPosition.Add("Top Left");
    StartPosition.Add("Top Right");
    StartPosition.Add("Bottom Right");
    StartPosition.Add("Bottom Left");

	m_choice1 = new wxChoice( sbSizer1->GetStaticBox(), wxID_ANY, wxDefaultPosition, wxDefaultSize, StartPosition, 0 );
	m_choice1->SetSelection( 0 );
	bSizer9->Add( m_choice1, 1, wxALL, 5 );


	sbSizer1->Add( bSizer9, 1, wxEXPAND, 5 );

	wxBoxSizer* bSizer10;
	bSizer10 = new wxBoxSizer( wxHORIZONTAL );

	m_button2 = new wxButton( sbSizer1->GetStaticBox(), wxID_ANY, wxT("Reset"), wxDefaultPosition, wxDefaultSize, 0 );
	m_button2->Bind(wxEVT_BUTTON, &Mesurement2D::OnReset, this);
    bSizer10->Add( m_button2, 1, wxALL, 5 );

	m_button1 = new wxButton( sbSizer1->GetStaticBox(), wxID_ANY, wxT("Open Device settings"), wxDefaultPosition, wxDefaultSize, 0 );
	m_button1->Bind(wxEVT_BUTTON, &Mesurement2D::OnSettings, this);
    bSizer10->Add( m_button1, 1, wxALL, 5 );


	sbSizer1->Add( bSizer10, 1, wxEXPAND, 5 );


	bSizer1->Add( sbSizer1, 1, wxEXPAND, 5 );

	wxStaticBoxSizer* sbSizer2;
	sbSizer2 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Mesurment") ), wxVERTICAL );

	wxBoxSizer* bSizer14;
	bSizer14 = new wxBoxSizer( wxHORIZONTAL );

	m_staticText6 = new wxStaticText( sbSizer2->GetStaticBox(), wxID_ANY, wxT("Progress:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText6->Wrap( -1 );
	bSizer14->Add( m_staticText6, 1, wxALL, 5 );

	m_staticText7 = new wxStaticText( sbSizer2->GetStaticBox(), wxID_ANY, wxT("0/100"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText7->Wrap( -1 );
	bSizer14->Add( m_staticText7, 1, wxALL, 5 );

	m_gauge1 = new wxGauge( sbSizer2->GetStaticBox(), wxID_ANY, 100, wxDefaultPosition, wxDefaultSize, wxGA_HORIZONTAL );
	m_gauge1->SetValue( 0 );
	bSizer14->Add( m_gauge1, 5, wxALL, 5 );


	sbSizer2->Add( bSizer14, 1, wxEXPAND, 5 );

	wxBoxSizer* bSizer15;
	bSizer15 = new wxBoxSizer( wxHORIZONTAL );

	m_button3 = new wxButton( sbSizer2->GetStaticBox(), wxID_ANY, wxT("Start"), wxDefaultPosition, wxDefaultSize, 0 );
	m_button3->Bind(wxEVT_BUTTON, &Mesurement2D::OnStart, this);
    bSizer15->Add( m_button3, 1, wxALL, 5 );

	m_button4 = new wxButton( sbSizer2->GetStaticBox(), wxID_ANY, wxT("Stop"), wxDefaultPosition, wxDefaultSize, 0 );
	m_button4->Bind(wxEVT_BUTTON, &Mesurement2D::OnStop, this);
    bSizer15->Add( m_button4, 1, wxALL, 5 );

	m_button5 = new wxButton( sbSizer2->GetStaticBox(), wxID_ANY, wxT("Restart"), wxDefaultPosition, wxDefaultSize, 0 );
	m_button5->Bind(wxEVT_BUTTON, &Mesurement2D::OnRestart, this);
    bSizer15->Add( m_button5, 1, wxALL, 5 );


	sbSizer2->Add( bSizer15, 1, wxEXPAND, 5 );


	bSizer1->Add( sbSizer2, 1, wxEXPAND, 5 );


	this->SetSizer( bSizer1 );
	this->Layout();

	this->Centre( wxBOTH );

    resetGuiValues();
    GetValues();
    GetTotalMesurements();
    updateProgressBar();
    SetSliderValues();

}
void Mesurement2D::OnReset(wxCommandEvent& event)
{
    resetGuiValues();
    SetSliderValues();
    updateProgressBar();

}
void Mesurement2D::OnSettings(wxCommandEvent& event)
{
    //Create new sub window
    SettingsWindow *SettingsWin = new SettingsWindow(this);
    //open Window Pauses Main Window
    SettingsWin->ShowModal();
    //Close Window
    SettingsWin->Destroy();
}
void Mesurement2D::OnStart(wxCommandEvent& event)
{
    GetTotalMesurements();
    updateProgressBar();
}
void Mesurement2D::OnStop(wxCommandEvent& event)
{
    incrementCurrentMesurmentPoint();
    updateProgressBar();
}
void Mesurement2D::OnRestart(wxCommandEvent& event)
{
    resetProgressBar();
    SetSliderValues();
    updateProgressBar();
}

void Mesurement2D::OnSliderUpdate(wxCommandEvent& event)
{
    SetSliderValues();
}

void Mesurement2D::GetValues()
{
    sliderY         = m_slider1->GetValue();
    sliderX         = m_slider2->GetValue();
    sliderScale     = m_slider3->GetValue();

    progressbar     = m_gauge1->GetValue();
}
void Mesurement2D::SetValues()
{
    m_slider1->SetValue(sliderY);
    m_slider2->SetValue(sliderX);
    m_slider3->SetValue(sliderScale);

    m_gauge1->SetValue(progressbar);
}
void Mesurement2D::GetTotalMesurements()
{
    GetValues();

    totalMesurmentPoints = sliderX * sliderY;
}
void Mesurement2D::incrementCurrentMesurmentPoint()
{
    currentMesurmentPoint++;
}
void Mesurement2D::updateProgressBar()
{
    if (currentMesurmentPoint < totalMesurmentPoints)
    {
        int Progress = currentMesurmentPoint * 100 / totalMesurmentPoints;

        wxString Text = wxString::Format("%d",currentMesurmentPoint) + "/" + wxString::Format("%d",totalMesurmentPoints);
        m_staticText7->SetLabel(Text);

        m_gauge1->SetValue(Progress);
    }
}
void Mesurement2D::resetProgressBar()
{
    currentMesurmentPoint = 0;
    updateProgressBar();
}
void Mesurement2D::SetSliderValues()
{
    GetValues();
    wxString Text = "Y Points:    " + wxString::Format("%d", sliderY);
    m_staticText2->SetLabel(Text);

    Text = "X Points:    " + wxString::Format("%d", sliderX);
    m_staticText3->SetLabel(Text);

    Text = "Scale:       " + wxString::Format("%d", sliderScale);
    m_staticText4->SetLabel(Text);
}
void Mesurement2D::resetGuiValues()
{
    sliderY                 = 10;
    sliderX                 = 10;
    sliderScale             = 100;
    progressbar             = 0;
    currentMesurmentPoint   = 0;

    SetValues();
    GetTotalMesurements();
    SetSliderValues();
    updateProgressBar();
}


Mesurement2D::~Mesurement2D()
{
}


//-----Plot Window Set Marker ENDE-------


MultiMessWindow::MultiMessWindow( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{

    // Unit Selection
    wxArrayString freqEinheiten;
    freqEinheiten.Add("Hz");
    freqEinheiten.Add("kHz");
    freqEinheiten.Add("MHz");
    freqEinheiten.Add("GHz");

	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	
	wxBoxSizer* bSizerMain;
	bSizerMain = new wxBoxSizer( wxVERTICAL );
	
    // ----- Messpunkte Sizerbox -----
	    wxStaticBoxSizer* sbSizer1 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Messpunkte") ), wxVERTICAL );

            m_staticTextXMess   = new wxStaticText( sbSizer1->GetStaticBox(),   wxID_ANY, wxT("X Messpunkte:"), wxDefaultPosition, wxDefaultSize, 0 );
            m_staticTextXMess->Wrap( -1 );
            m_textCtrlXMess     = new wxTextCtrl( sbSizer1->GetStaticBox(),     wxID_ANY, wxEmptyString,        wxDefaultPosition, wxDefaultSize, 0 );
            

            wxBoxSizer* bSizerSub11 = new wxBoxSizer( wxHORIZONTAL );

            bSizerSub11->Add( m_staticTextXMess,    1, wxALL, 5 );
            bSizerSub11->Add( m_textCtrlXMess,      1, wxALL, 5 );

            
            m_staticTextYMess = new wxStaticText(   sbSizer1->GetStaticBox(), wxID_ANY, wxT("Y Messpunkte:"),   wxDefaultPosition, wxDefaultSize, 0 );
            m_staticTextYMess->Wrap( -1 );
            m_textCtrlYMess = new wxTextCtrl(       sbSizer1->GetStaticBox(), wxID_ANY, wxEmptyString,          wxDefaultPosition, wxDefaultSize, 0 );
            
            wxBoxSizer* bSizerSub12 = new wxBoxSizer( wxHORIZONTAL );
            bSizerSub12->Add( m_staticTextYMess,    1, wxALL, 5 );
            bSizerSub12->Add( m_textCtrlYMess,      1, wxALL, 5 );

        sbSizer1->Add( bSizerSub11, 1, wxEXPAND, 5 );
        sbSizer1->Add( bSizerSub12, 1, wxEXPAND, 5 );
	
	bSizerMain->Add( sbSizer1, 1, wxEXPAND, 5 );
	// ----- Messpunkte Sizerbox ende-----

    // ----- Lage Parameter Sizerbox -----
            
        wxStaticBoxSizer* sbSizer2      = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Lage Parameter") ), wxVERTICAL );
            
            wxStaticBoxSizer* sbSizerSub211 = new wxStaticBoxSizer( new wxStaticBox( sbSizer2->GetStaticBox(), wxID_ANY, wxT("Messstartpunktkoordinaten [mm]") ), wxHORIZONTAL );
            
            m_staticTextXStartCord          = new wxStaticText( sbSizerSub211->GetStaticBox(), wxID_ANY, wxT("X:"),     wxDefaultPosition, wxDefaultSize, 0 );
            m_staticTextXStartCord->Wrap( -1 );
            m_textCtrlXStartCord            = new wxTextCtrl(   sbSizerSub211->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
            
            m_staticTextYStartCord          = new wxStaticText( sbSizerSub211->GetStaticBox(), wxID_ANY, wxT("Y:"),     wxDefaultPosition, wxDefaultSize, 0 );
            m_staticTextYStartCord->Wrap( -1 );
            m_textCtrlYStartCord            = new wxTextCtrl(   sbSizerSub211->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
            
            wxBoxSizer* bSizerSub21         = new wxBoxSizer( wxVERTICAL );
            sbSizerSub211->Add( m_staticTextXStartCord, 1, wxALL, 5 );
            sbSizerSub211->Add( m_textCtrlXStartCord,   3, wxALL, 5 );

            sbSizerSub211->Add( m_staticTextYStartCord, 1, wxALL, 5 );
            sbSizerSub211->Add( m_textCtrlYStartCord,   3, wxALL, 5 );
            
            bSizerSub21->Add( sbSizerSub211, 1, wxEXPAND, 5 );
            
            // ----- X Messpunkte Sizerbox -----
            m_staticTextXAbstand    = new wxStaticText( sbSizer2->GetStaticBox(), wxID_ANY, wxT("X-Messpunkt abstand  [mm]"), wxDefaultPosition, wxDefaultSize, 0 );
            m_staticTextXAbstand->Wrap( -1 );
            m_textCtrlXAbstand      = new wxTextCtrl(   sbSizer2->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
            
            wxBoxSizer* bSizerSub22 = new wxBoxSizer( wxHORIZONTAL );
            bSizerSub22->Add( m_textCtrlXAbstand,   1, wxALL, 5 );
            bSizerSub22->Add( m_staticTextXAbstand, 1, wxALL, 5 );

            // ----- Y Messpunkte Sizerbox -----
            m_staticTextYAbstand    = new wxStaticText( sbSizer2->GetStaticBox(), wxID_ANY, wxT("Y-Messpunkt abstand  [mm]"), wxDefaultPosition, wxDefaultSize, 0 );
            m_staticTextYAbstand->Wrap( -1 );
            m_textCtrlYAbstand      = new wxTextCtrl(   sbSizer2->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
            
            
            wxBoxSizer* bSizerSub23 = new wxBoxSizer( wxHORIZONTAL );
            bSizerSub23->Add( m_staticTextYAbstand, 1, wxALL, 5 );
            bSizerSub23->Add( m_textCtrlYAbstand,   1, wxALL, 5 );

        // Textbox Sizer 2
        sbSizer2->Add( bSizerSub21, 1, wxEXPAND, 5 );
        sbSizer2->Add( bSizerSub22, 1, wxEXPAND, 5 );
        sbSizer2->Add( bSizerSub23, 1, wxEXPAND, 5 );
	
	bSizerMain->Add( sbSizer2, 1, wxEXPAND, 5 );
    // ----- Lage Parameter Sizerbox ende-----

        // ----- Messeinstellungen Sizerbox -----
        wxStaticBoxSizer* sbSizer3 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Messeinstellungen") ), wxVERTICAL );
            
            // Start Frequenz
            m_staticTextStrtFreq = new wxStaticText( sbSizer3->GetStaticBox(), wxID_ANY, wxT("Start Frequenz:"), wxDefaultPosition, wxDefaultSize, 0 );
            m_staticTextStrtFreq->Wrap( -1 );
            
            m_textCtrlStrtFreq = new wxTextCtrl( sbSizer3->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
            
            m_choiceEinheitFreq1 = new wxChoice( sbSizer3->GetStaticBox(), wxID_ANY, wxDefaultPosition, wxDefaultSize, freqEinheiten, 0 );
            m_choiceEinheitFreq1->SetSelection( 1 );
            
            
            wxBoxSizer* bSizerSub31 = new wxBoxSizer( wxHORIZONTAL );
            bSizerSub31->Add( m_staticTextStrtFreq, 2, wxALL, 5 );
            bSizerSub31->Add( m_textCtrlStrtFreq,   1, wxALL, 5 );
            bSizerSub31->Add( m_choiceEinheitFreq1, 1, wxALL, 5 );
            
            // End Frequenz
            m_staticTextEndFreq = new wxStaticText( sbSizer3->GetStaticBox(), wxID_ANY, wxT("End Frequenz:"), wxDefaultPosition, wxDefaultSize, 0 );
            m_staticTextEndFreq->Wrap( -1 );
            
            m_textCtrlEndFreq = new wxTextCtrl( sbSizer3->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
            
            m_choiceEinheitFreq2 = new wxChoice( sbSizer3->GetStaticBox(), wxID_ANY, wxDefaultPosition, wxDefaultSize, freqEinheiten, 0 );
            m_choiceEinheitFreq2->SetSelection( 1 );

            
            wxBoxSizer* bSizerSub32 = new wxBoxSizer( wxHORIZONTAL );
            bSizerSub32->Add( m_staticTextEndFreq,  2, wxALL, 5 );
            bSizerSub32->Add( m_textCtrlEndFreq,    1, wxALL, 5 );
            bSizerSub32->Add( m_choiceEinheitFreq2, 1, wxALL, 5 );

            
            
            // Anz Messpunkte
            m_staticTextAnzahlSweep = new wxStaticText( sbSizer3->GetStaticBox(), wxID_ANY, wxT("Anzahl der Sweep Messpunkte:"), wxDefaultPosition, wxDefaultSize, 0 );
            m_staticTextAnzahlSweep->Wrap( -1 );

            m_textCtrlAnzahlSweep = new wxTextCtrl( sbSizer3->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
            
            wxBoxSizer* bSizerSub33 = new wxBoxSizer( wxHORIZONTAL );
            bSizerSub33->Add( m_staticTextAnzahlSweep,  1, wxALL, 5 );
            bSizerSub33->Add( m_textCtrlAnzahlSweep,    1, wxALL, 5 );

        // Textbox Sizer 3
        sbSizer3->Add( bSizerSub31, 1, wxEXPAND, 5 );
        sbSizer3->Add( bSizerSub32, 1, wxEXPAND, 5 );
        sbSizer3->Add( bSizerSub33, 1, wxEXPAND, 5 );
	
	bSizerMain->Add( sbSizer3, 1, wxEXPAND, 5 );
	
	
        // ----- Buttons -----
        m_buttonStart   = new wxButton( this, wxID_ANY, wxT("Start"),   wxDefaultPosition, wxDefaultSize, 0 );
        m_buttonStop    = new wxButton( this, wxID_ANY, wxT("Stop"),    wxDefaultPosition, wxDefaultSize, 0 );
        m_buttonReset   = new wxButton( this, wxID_ANY, wxT("Reset"),   wxDefaultPosition, wxDefaultSize, 0 );
        
        
        wxBoxSizer* bSizer4 = new wxBoxSizer( wxHORIZONTAL );
        bSizer4->Add( m_buttonStart,    1, wxALL, 5 );
        bSizer4->Add( m_buttonStop,     1, wxALL, 5 );
        bSizer4->Add( m_buttonReset,    1, wxALL, 5 );
	
	bSizerMain->Add( bSizer4, 1, wxEXPAND, 5 );
	
    // ----- Test -----
        wxStaticBoxSizer* sbSizer5 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Test") ), wxVERTICAL );
        
            m_staticTextProgress = new wxStaticText( sbSizer5->GetStaticBox(), wxID_ANY, wxT("0 / 100"), wxDefaultPosition, wxDefaultSize, 0 );
            m_staticTextProgress->Wrap( -1 );
            
            m_gaugeProgress = new wxGauge( sbSizer5->GetStaticBox(), wxID_ANY, 100, wxDefaultPosition, wxDefaultSize, wxGA_HORIZONTAL );
            m_gaugeProgress->SetValue( 0 ); 

            m_buttonNext    = new wxButton(sbSizer5->GetStaticBox(), wxID_ANY, wxT("Next"), wxDefaultPosition, wxDefaultSize, 0 );
            
            
            wxBoxSizer* bSizerSub51 = new wxBoxSizer( wxHORIZONTAL );
            bSizerSub51->Add( m_staticTextProgress, 0, wxALL, 5 );
            bSizerSub51->Add( m_gaugeProgress,      0, wxALL, 5 );
            bSizerSub51->Add( m_buttonNext,         0, wxALL, 5 );
        
        sbSizer5->Add( bSizerSub51, 1, wxEXPAND, 5 );
	
	bSizerMain->Add( sbSizer5, 1, wxEXPAND, 5 );
    // ----- Test Ende -----
	
	//Button function binds
    m_buttonStart   ->Bind(wxEVT_BUTTON, &MultiMessWindow::startButton, this);
    m_buttonStop    ->Bind(wxEVT_BUTTON, &MultiMessWindow::stopButton,  this);
    m_buttonReset   ->Bind(wxEVT_BUTTON, &MultiMessWindow::resetButton, this);
    m_buttonNext    ->Bind(wxEVT_BUTTON, &MultiMessWindow::nextButton,  this);

    SetValues();

	this->SetSizer( bSizerMain );
	this->Layout();
	
	this->Centre( wxBOTH );
}

MultiMessWindow::~MultiMessWindow()
{


}

void MultiMessWindow::startButton(wxCommandEvent& event)
{
    GetValues();
    //set total point in the test section
    totalMesurmentPoints = std::stoi(X_Messpunkte.ToStdString())*std::stoi(Y_Messpunkte.ToStdString());
    
    UpdateProgressBar();

}
void MultiMessWindow::stopButton(wxCommandEvent& event)
{
    GetValues();
    //set total point in the test section
    totalMesurmentPoints = std::stoi(X_Messpunkte.ToStdString())*std::stoi(Y_Messpunkte.ToStdString());
    
    currentMesurmentPoint = 0;
    
    UpdateProgressBar();
    
}
void MultiMessWindow::resetButton(wxCommandEvent& event)
{
    m_textCtrlXMess         ->SetValue("1"  );
    m_textCtrlYMess         ->SetValue("1"  );
    m_textCtrlXStartCord    ->SetValue("0"  );
    m_textCtrlYStartCord    ->SetValue("0"  );
    m_textCtrlXAbstand      ->SetValue("10" );
    m_textCtrlYAbstand      ->SetValue("10" );

    m_textCtrlStrtFreq      ->SetValue("50" );
    m_choiceEinheitFreq1    ->SetSelection(1);
    m_textCtrlEndFreq       ->SetValue("100");
    m_choiceEinheitFreq1    ->SetSelection(1);
    m_textCtrlAnzahlSweep   ->SetValue("512");

    totalMesurmentPoints = 1;
    currentMesurmentPoint = 0;
    
    GetValues();
    
    UpdateProgressBar();
}
void MultiMessWindow::nextButton(wxCommandEvent& event)
{
    GetValues();

    if (currentMesurmentPoint < totalMesurmentPoints)
    {
        currentMesurmentPoint++;
    }

    UpdateProgressBar();
}

void MultiMessWindow::GetValues()
{
    X_Messpunkte    = m_textCtrlXMess       ->GetValue();
    Y_Messpunkte    = m_textCtrlYMess       ->GetValue();
    X_Cord          = m_textCtrlXStartCord  ->GetValue();
    Y_Cord          = m_textCtrlYStartCord  ->GetValue();
    X_MessAbstand   = m_textCtrlXAbstand    ->GetValue();
    Y_MessAbstand   = m_textCtrlYAbstand    ->GetValue();  
    startFreq       = m_textCtrlStrtFreq    ->GetValue();
    stopFreq        = m_textCtrlEndFreq     ->GetValue();
    AnzSweepMessPkt = m_textCtrlAnzahlSweep ->GetValue();

    startFreqUnit   = m_choiceEinheitFreq1  ->GetStringSelection();
    stopFreqUnit    = m_choiceEinheitFreq2  ->GetStringSelection();
}
void MultiMessWindow::SetValues()
{
    m_textCtrlXMess         ->SetValue(X_Messpunkte    );
    m_textCtrlYMess         ->SetValue(Y_Messpunkte    );
    m_textCtrlXStartCord    ->SetValue(X_Cord          );
    m_textCtrlYStartCord    ->SetValue(Y_Cord          );
    m_textCtrlXAbstand      ->SetValue(X_MessAbstand   );
    m_textCtrlYAbstand      ->SetValue(Y_MessAbstand   );

    m_textCtrlStrtFreq      ->SetValue(startFreq       );
    m_textCtrlEndFreq       ->SetValue(stopFreq        );
    m_textCtrlAnzahlSweep   ->SetValue(AnzSweepMessPkt );
}
void MultiMessWindow::UpdateProgressBar()
{
    if (currentMesurmentPoint <= totalMesurmentPoints)
    {
        int Progress = currentMesurmentPoint * 100 / totalMesurmentPoints;

        wxString Text = wxString::Format("%d",currentMesurmentPoint) + "/" + wxString::Format("%d",totalMesurmentPoints);
        m_staticTextProgress->SetLabel(Text);

        m_gaugeProgress->SetValue(Progress);
    }
}

void MultiMessWindow::testMessFunction()
{
    if (currentMesurmentPoint = 0)
    {
        //Adapter.send()
    }

    if (totalMesurmentPoints = 1)
    {
        Global::AdapterInstance.getMesurement();
    }
    else
    {

    }

}