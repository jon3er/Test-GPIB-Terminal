#include "Mesurement.h"
#include "SettingsWindow.h"
#include "cmdGpib.h"
#include "mainHelper.h"

//-----Plot Window BEGIN--------
PlotWindow::PlotWindow(wxWindow *parent, MainDocument* mainDoc)
    : wxDialog(parent, wxID_ANY, "Plot Window", wxDefaultPosition, wxSize(1000,750))
    , m_mainDoc(mainDoc)
{
    getFileNames(m_filePath, m_fileNames);

    wxButton* executeMesurment = new wxButton(this, wxID_ANY, "Execute Mesurement");
    executeMesurment->Bind(wxEVT_BUTTON, &PlotWindow::executeScriptEvent,this);
    m_selectMesurement = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_fileNames);
    m_selectMesurement->SetSelection(0);
    // 1. mpWindow (Zeichenfläche) erstellen
    m_plot = new mpWindow(this, wxID_ANY);

    // Farbeinstellungen (Optional)
    m_plot->SetMargins(30, 30, 50, 50);

    // 2. Achsen als Layer hinzufügen
    // mpScaleX(Name, Ausrichtung, Ticks anzeigen, Typ)
    mpScaleX* xAxis = new mpScaleX("X-Achse", mpALIGN_BORDER_BOTTOM, true, mpX_NORMAL);
    mpScaleY* yAxis = new mpScaleY("Y-Achse", mpALIGN_BORDER_LEFT, true);

    m_plot->AddLayer(xAxis);
    m_plot->AddLayer(yAxis);

    std::vector<double> m_x = {0.0, 1.0, 2.0, 3.0, 4.0, 5.0 };
    std::vector<double> m_y = {0.0, 1.0, 4.0, 2.0, 5.0, 3.0 };

    // 3. Daten vorbereiten (std::vector laut Header Definition von mpFXYVector)

    //Holt in dem Haupt menu geladene daten
    if (m_mainDoc != nullptr && m_mainDoc->IsFileOpen())
    {
        sData::sParam* tempStruct = m_mainDoc->GetData().GetParameter();
        m_mainDoc->GetData().GetData(tempStruct, m_x, m_y);
    }



    // 4. Vektor-Layer erstellen
    m_vectorLayer = new mpFXYVector("Messdaten");
    m_vectorLayer->SetData(m_x, m_y);
    m_vectorLayer->SetContinuity(true); // True = Linie zeichnen
    m_vectorLayer->SetPen(wxPen(*wxBLUE, 2, wxPENSTYLE_SOLID));
    m_vectorLayer->ShowName(true);      // Wichtig für die Legende

    m_plot->AddLayer(m_vectorLayer);

    // 5. Legende hinzufügen (mpInfoLegend ist ein Layer)
    // wxRect definiert Startposition und ungefähre Größe
    mpInfoLegend* legend = new mpInfoLegend(wxRect(20, 20, 10, 10), wxTRANSPARENT_BRUSH);
    legend->SetItemMode(mpLEGEND_LINE); // Zeigt Linie statt Quadrat in der Legende
    m_plot->AddLayer(legend);

    // 6. Layout-Management
    wxBoxSizer* sizerButtons = new wxBoxSizer(wxHORIZONTAL);
    sizerButtons->Add(executeMesurment, 0, wxEXPAND | wxALL);
    sizerButtons->Add(m_selectMesurement, 0, wxEXPAND | wxALL);

    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    sizer->Add(m_plot, 1, wxEXPAND | wxALL, 5);
    sizer->Add(sizerButtons, 0, wxEXPAND| wxALL , 5);
    this->SetSizer(sizer);
    this->Layout();
    // 7. Zoom auf Daten anpassen
    m_plot->Fit();

    // (callbacks wired via SetDocument / OnDocumentChanged)
}

PlotWindow::~PlotWindow()
{
    // Document handles StopMeasurement + disconnect in its destructor
    if (m_document)
        m_document->RemoveObserver(this);

    std::cerr << "PlotWindow closed" << std::endl;
}

void PlotWindow::SetDocument(MeasurementDocument* doc)
{
    if (m_document)
        m_document->RemoveObserver(this);

    m_document = doc;

    if (m_document)
        m_document->AddObserver(this);
}

void PlotWindow::OnDocumentChanged(const std::string& changeType)
{
    if (!m_document)
        return;

    if (changeType == "DataUpdated")
    {
        // DataUpdated may arrive from a worker thread — guard with CallAfter
        wxEvtHandler::CallAfter([this]()
        {
            // Pull latest vectors from the document (already copied by WorkerThread)
            auto x = m_document->GetXData();
            auto y = m_document->GetYData();
            m_vectorLayer->SetData(x, y);
            m_plot->Fit();
            std::cout << "[PlotWindow] Plot refreshed from document" << std::endl;
        });
    }
    // MeasurementStarted / MeasurementStopped — no visual action needed here
}

wxString PlotWindow::formatOutput(const std::string& text)
{
    return terminalTimestampOutput(wxString::FromUTF8(text.c_str()));
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
    if (!m_document)
    {
        wxLogWarning("PlotWindow: no document attached");
        return;
    }

    if (m_document->IsMeasuring())
    {
        wxLogWarning("Measurement thread already running");
        return;
    }

    wxString fileName = m_selectMesurement->GetStringSelection();
    std::cerr << "Starting measurement for: " << fileName << std::endl;

    // Set basic metadata on the first pass
    if (m_mesurementNumber == 1)
    {
        wxDateTime now = wxDateTime::Now();
        sData::sParam* info = m_document->GetResultsMutable().GetParameter();
        info->File = "Mesurement";
        info->Date = now.FormatISODate();
        info->Time = now.FormatISOTime();
    }

    m_document->StartMeasurement(m_filePath.ToStdString(), fileName.ToStdString(), m_mesurementNumber);
}
void PlotWindow::updatePlotData()
{
    if (m_document)
    {
        auto x = m_document->GetXData();
        auto y = m_document->GetYData();
        m_vectorLayer->SetData(x, y);
    }
    m_plot->Fit();
}

//-----Plot Window Set Marker-------
PlotWindowSetMarker::PlotWindowSetMarker(wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style) 
    : wxDialog(parent, id, title, pos, size, style)
{
    // Frequency units
    wxArrayString freqUnits;
    freqUnits.Add("Hz");
    freqUnits.Add("kHz");
    freqUnits.Add("MHz");
    freqUnits.Add("GHz");

    // Window setup
    this->SetSizeHints(wxDefaultSize, wxDefaultSize);

    // Main sizer
    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

    // ===== Marker 1 Section =====
    wxStaticBoxSizer* marker1Box = new wxStaticBoxSizer(
        new wxStaticBox(this, wxID_ANY, wxT("Marker 1")), wxVERTICAL);

    // Marker 1: Frequency input row
    wxBoxSizer* marker1FreqRow = new wxBoxSizer(wxHORIZONTAL);
    m_checkBox1 = new wxCheckBox(marker1Box->GetStaticBox(), wxID_ANY, wxT("Set to Frequency"), 
        wxDefaultPosition, wxDefaultSize, 0);
    m_checkBox1->Bind(wxEVT_CHECKBOX, &PlotWindowSetMarker::toggleSelection1, this);
    m_textCtrl1 = new wxTextCtrl(marker1Box->GetStaticBox(), wxID_ANY, wxEmptyString, 
        wxDefaultPosition, wxDefaultSize, 0);
    m_choice1 = new wxChoice(marker1Box->GetStaticBox(), wxID_ANY, wxDefaultPosition, 
        wxDefaultSize, freqUnits, 0);
    m_choice1->SetSelection(0);

    marker1FreqRow->Add(m_checkBox1, 1, wxALL, 5);
    marker1FreqRow->Add(m_textCtrl1, 1, wxALL, 5);
    marker1FreqRow->Add(m_choice1, 1, wxALL, 5);
    marker1Box->Add(marker1FreqRow, 1, wxEXPAND, 5);

    // Marker 1: Max frequency option
    wxBoxSizer* marker1MaxRow = new wxBoxSizer(wxVERTICAL);
    m_checkBox2 = new wxCheckBox(marker1Box->GetStaticBox(), wxID_ANY, wxT("Set to highest freq"), 
        wxDefaultPosition, wxDefaultSize, 0);
    m_checkBox2->Bind(wxEVT_CHECKBOX, &PlotWindowSetMarker::toggleSelection1, this);
    marker1MaxRow->Add(m_checkBox2, 0, wxALL, 5);
    marker1Box->Add(marker1MaxRow, 1, wxEXPAND, 5);

    // Marker 1: Set button
    m_button1 = new wxButton(marker1Box->GetStaticBox(), wxID_ANY, wxT("Set"), 
        wxDefaultPosition, wxDefaultSize, 0);
    m_button1->Bind(wxEVT_BUTTON, &PlotWindowSetMarker::SetSelection1, this);
    marker1Box->Add(m_button1, 1, wxALL, 5);

    mainSizer->Add(marker1Box, 1, wxEXPAND, 5);

    // ===== Marker 2 Section =====
    wxStaticBoxSizer* marker2Box = new wxStaticBoxSizer(
        new wxStaticBox(this, wxID_ANY, wxT("Marker 2")), wxVERTICAL);

    // Marker 2: Frequency input row
    wxBoxSizer* marker2FreqRow = new wxBoxSizer(wxHORIZONTAL);
    m_checkBox3 = new wxCheckBox(marker2Box->GetStaticBox(), wxID_ANY, wxT("Set to Frequency"), 
        wxDefaultPosition, wxDefaultSize, 0);
    m_checkBox3->Bind(wxEVT_CHECKBOX, &PlotWindowSetMarker::toggleSelection2, this);
    m_textCtrl2 = new wxTextCtrl(marker2Box->GetStaticBox(), wxID_ANY, wxEmptyString, 
        wxDefaultPosition, wxDefaultSize, 0);
    m_choice2 = new wxChoice(marker2Box->GetStaticBox(), wxID_ANY, wxDefaultPosition, 
        wxDefaultSize, freqUnits, 0);
    m_choice2->SetSelection(0);

    marker2FreqRow->Add(m_checkBox3, 1, wxALL, 5);
    marker2FreqRow->Add(m_textCtrl2, 1, wxALL, 5);
    marker2FreqRow->Add(m_choice2, 1, wxALL, 5);
    marker2Box->Add(marker2FreqRow, 1, wxEXPAND, 5);

    // Marker 2: Max frequency option
    wxBoxSizer* marker2MaxRow = new wxBoxSizer(wxVERTICAL);
    m_checkBox4 = new wxCheckBox(marker2Box->GetStaticBox(), wxID_ANY, wxT("Set to highest Freq"), 
        wxDefaultPosition, wxDefaultSize, 0);
    m_checkBox4->Bind(wxEVT_CHECKBOX, &PlotWindowSetMarker::toggleSelection2, this);
    marker2MaxRow->Add(m_checkBox4, 1, wxALL, 5);
    marker2Box->Add(marker2MaxRow, 1, wxEXPAND, 5);

    // Marker 2: Set button
    m_button2 = new wxButton(marker2Box->GetStaticBox(), wxID_ANY, wxT("Set"), 
        wxDefaultPosition, wxDefaultSize, 0);
    m_button2->Bind(wxEVT_BUTTON, &PlotWindowSetMarker::SetSelection2, this);
    marker2Box->Add(m_button2, 1, wxALL, 5);

    mainSizer->Add(marker2Box, 1, wxEXPAND, 5);

    // Apply layout
    this->SetSizer(mainSizer);
    this->Layout();
    this->Centre(wxBOTH);

    // Initialize
    GetValues();
    toggleSelection1fkt();
    toggleSelection2fkt();
}
PlotWindowSetMarker::~PlotWindowSetMarker()
{
}

void PlotWindowSetMarker::GetValues()
{
    m_Marker1FreqSet      = m_checkBox1->GetValue();
    m_Marker1MaxSet       = m_checkBox2->GetValue();

    m_Marker2FreqSet      = m_checkBox3->GetValue();
    m_Marker2MaxSet       = m_checkBox4->GetValue();

    m_Marker1Freq         = m_textCtrl1->GetValue();
    m_Marker1Unit         = m_choice1->GetStringSelection();
    m_Marker2Freq         = m_textCtrl2->GetValue();
    m_Marker2Unit         = m_choice2->GetStringSelection();

}

void PlotWindowSetMarker::toggleSelection1fkt()
{
    if (!m_Marker1FreqSet && !m_Marker1MaxSet)
    {
        m_checkBox1->SetValue(true);
        m_checkBox2->SetValue(false);
        m_textCtrl1->Enable(true);
        m_choice1->Enable(true);
    }
    else if (!m_Marker1FreqSet && m_Marker1MaxSet)
    {
        m_checkBox1->SetValue(true);
        m_checkBox2->SetValue(false);
        m_textCtrl1->Enable(true);
        m_choice1->Enable(true);
    }
    else if (m_Marker1FreqSet && !m_Marker1MaxSet)
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
    if (!m_Marker2FreqSet && !m_Marker2MaxSet)
    {
        m_checkBox3->SetValue(true);
        m_checkBox4->SetValue(false);
        m_textCtrl2->Enable(true);
        m_choice2->Enable(true);
    }
    else if (!m_Marker2FreqSet && m_Marker2MaxSet)
    {
        m_checkBox3->SetValue(true);
        m_checkBox4->SetValue(false);
        m_textCtrl2->Enable(true);
        m_choice2->Enable(true);
    }
    else if (m_Marker2FreqSet && !m_Marker2MaxSet)
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

    if (!m_Marker1Freq.ToDouble(&val))
    {
        std::cerr << "Failed to convert input" << std::endl;
        return;
    }

    if (selection != wxNOT_FOUND) {
        double factor = multipliers[selection];
        double frequencyHz =  val * factor;
        m_FreqMarker1Raw = wxString::FromCDouble(frequencyHz);
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

    if (!m_Marker1Freq.ToDouble(&val))
    {
        std::cerr << "Failed to convert input" << std::endl;
        return;
    }

    if (selection != wxNOT_FOUND) {
        double factor = multipliers[selection];
        double frequencyHz =  val * factor;
        m_FreqMarker2Raw = wxString::FromCDouble(frequencyHz);
    }
}
void PlotWindowSetMarker::SetDocument(MeasurementDocument* doc)
{
    m_document = doc;
}

void PlotWindowSetMarker::SetSelection1(wxCommandEvent& event)
{
    GetValues();

    if (!m_document)
    {
        wxLogWarning("PlotWindowSetMarker: no document attached");
        return;
    }

    if (m_Marker1MaxSet)
    {
        m_document->WriteMarker1(true, "");
    }
    else if (m_Marker1FreqSet && m_FreqMarker1Raw.IsNumber())
    {
        GetSelectedValue1();
        m_document->WriteMarker1(false, std::string(m_FreqMarker1Raw.ToUTF8()));
    }
    //TODO Get X Y From Device and display in the Menu
}
void PlotWindowSetMarker::SetSelection2(wxCommandEvent& event)
{
    GetValues();

    if (!m_document)
    {
        wxLogWarning("PlotWindowSetMarker: no document attached");
        return;
    }

    if (m_Marker2MaxSet)
    {
        m_document->WriteMarker2(true, "");
    }
    else if (m_Marker2FreqSet && m_FreqMarker2Raw.IsNumber())
    {
        GetSelectedValue2();
        m_document->WriteMarker2(false, std::string(m_FreqMarker2Raw.ToUTF8()));
    }
    //TODO Get X Y From Device and display in the Menu
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
    // Document is attached later via SetDocument() from the caller
}

MultiMessWindow::~MultiMessWindow()
{
    if (m_document)
        m_document->RemoveObserver(this);
}

void MultiMessWindow::SetDocument(MultiMessDocument* doc)
{
    if (m_document)
        m_document->RemoveObserver(this);

    m_document = doc;

    if (m_document)
    {
        m_document->AddObserver(this);
        PullValuesFromDocument();
        UpdateProgressBar();
    }
}

void MultiMessWindow::OnMultiMessDocumentChanged(const std::string& changeType)
{
    if (!m_document)
        return;

    if (changeType == "ProgressChanged" || changeType == "Started" || changeType == "Stopped")
    {
        UpdateProgressBar();
    }
    else if (changeType == "Reset")
    {
        PullValuesFromDocument();
        UpdateProgressBar();
    }
    // "ConfigChanged" — no visual update required beyond what the view already shows
}

void MultiMessWindow::startButton(wxCommandEvent& event)
{
    if (!m_document) return;
    PushValuesToDocument();
    m_document->Start();
}

void MultiMessWindow::stopButton(wxCommandEvent& event)
{
    if (!m_document) return;
    m_document->Stop();
}

void MultiMessWindow::resetButton(wxCommandEvent& event)
{
    if (!m_document) return;
    m_document->Reset();
    // PullValuesFromDocument is triggered via OnMultiMessDocumentChanged("Reset")
}

void MultiMessWindow::nextButton(wxCommandEvent& event)
{
    if (!m_document) return;
    m_document->NextPoint();
}

void MultiMessWindow::PushValuesToDocument()
{
    if (!m_document) return;
    m_document->SetXMeasurementPoints (std::string(m_textCtrlXMess      ->GetValue().ToUTF8()));
    m_document->SetYMeasurementPoints (std::string(m_textCtrlYMess      ->GetValue().ToUTF8()));
    m_document->SetXStartCoordinate   (std::string(m_textCtrlXStartCord ->GetValue().ToUTF8()));
    m_document->SetYStartCoordinate   (std::string(m_textCtrlYStartCord ->GetValue().ToUTF8()));
    m_document->SetXMeasurementSpacing(std::string(m_textCtrlXAbstand   ->GetValue().ToUTF8()));
    m_document->SetYMeasurementSpacing(std::string(m_textCtrlYAbstand   ->GetValue().ToUTF8()));
    m_document->SetStartFrequency     (std::string(m_textCtrlStrtFreq   ->GetValue().ToUTF8()));
    m_document->SetStopFrequency      (std::string(m_textCtrlEndFreq    ->GetValue().ToUTF8()));
    m_document->SetNumberOfSweepPoints(std::string(m_textCtrlAnzahlSweep->GetValue().ToUTF8()));
    m_document->SetStartFrequencyUnit (std::string(m_choiceEinheitFreq1 ->GetStringSelection().ToUTF8()));
    m_document->SetStopFrequencyUnit  (std::string(m_choiceEinheitFreq2 ->GetStringSelection().ToUTF8()));
}

void MultiMessWindow::PullValuesFromDocument()
{
    if (!m_document) return;
    m_textCtrlXMess       ->SetValue(wxString(m_document->GetXMeasurementPoints()));
    m_textCtrlYMess       ->SetValue(wxString(m_document->GetYMeasurementPoints()));
    m_textCtrlXStartCord  ->SetValue(wxString(m_document->GetXStartCoordinate()));
    m_textCtrlYStartCord  ->SetValue(wxString(m_document->GetYStartCoordinate()));
    m_textCtrlXAbstand    ->SetValue(wxString(m_document->GetXMeasurementSpacing()));
    m_textCtrlYAbstand    ->SetValue(wxString(m_document->GetYMeasurementSpacing()));
    m_textCtrlStrtFreq    ->SetValue(wxString(m_document->GetStartFrequency()));
    m_textCtrlEndFreq     ->SetValue(wxString(m_document->GetStopFrequency()));
    m_textCtrlAnzahlSweep ->SetValue(wxString(m_document->GetNumberOfSweepPoints()));
}

void MultiMessWindow::UpdateProgressBar()
{
    if (!m_document) return;
    unsigned int current = m_document->GetCurrentPoint();
    unsigned int total   = m_document->GetTotalPoints();
    if (total == 0) return;

    int progress = static_cast<int>(current * 100 / total);
    wxString text = wxString::Format("%u", current) + "/" + wxString::Format("%u", total);
    m_staticTextProgress->SetLabel(text);
    m_gaugeProgress->SetValue(progress);
}

void MultiMessWindow::SetValues()
{
    // Populate fields from default document values when no document is attached yet
    m_textCtrlXMess       ->SetValue("1"  );
    m_textCtrlYMess       ->SetValue("1"  );
    m_textCtrlXStartCord  ->SetValue("0"  );
    m_textCtrlYStartCord  ->SetValue("0"  );
    m_textCtrlXAbstand    ->SetValue("10" );
    m_textCtrlYAbstand    ->SetValue("10" );
    m_textCtrlStrtFreq    ->SetValue("50" );
    m_choiceEinheitFreq1  ->SetSelection(1);
    m_textCtrlEndFreq     ->SetValue("100");
    m_choiceEinheitFreq2  ->SetSelection(1);
    m_textCtrlAnzahlSweep ->SetValue("512");
}
