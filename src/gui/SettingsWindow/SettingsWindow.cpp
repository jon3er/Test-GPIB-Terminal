#include "SettingsWindow.h"
#include "systemInfo.h"
#include "cmdGpib.h"



//-----Settings Window--------
SettingsWindow::SettingsWindow(wxWindow *parent)
    : wxDialog(parent, wxID_ANY, "Settings", wxDefaultPosition, wxSize(500,750))
{
    wxPanel* mainPanel = new wxPanel(this, wxID_ANY);

    //Main settingswindow elements
    wxStaticText* infoText = new wxStaticText(mainPanel, wxID_ANY, "Settings for FSU Display, Adapter and General programm enviroment");
    wxButton* resetButton = new wxButton(mainPanel,wxID_ANY,"reset all");
    //Subtab elements
    wxNotebook* notebook = new wxNotebook(mainPanel, wxID_ANY);

    m_displayTab = new SettingsTabDisplay(notebook, wxString::FromUTF8("Inhalt für 'display'"));
    SettingsTabAdapter* adapterTab = new SettingsTabAdapter(notebook, wxString::FromUTF8("Inhalt für 'adapter'"));
    SettingsTabGeneral* generalTab = new SettingsTabGeneral(notebook, wxString::FromUTF8("Inhalt für 'adapter'"));

    notebook->AddPage(m_displayTab, "Display");
    notebook->AddPage(adapterTab, "Adapter");
    notebook->AddPage(generalTab, "General");

    //  Layout-Management (Sizer) verwenden, damit das Notebook
    //    das Hauptfenster ausfüllt
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    sizer->Add(infoText, 1, wxEXPAND | wxALL, 5);
    sizer->Add(notebook, 18, wxEXPAND | wxALL, 5); // 1 = dehnbar, wxEXPAND = ausfüllen
    sizer->Add(resetButton,1 ,wxEXPAND | wxALL, 5);
    mainPanel->SetSizer(sizer);
}

void SettingsWindow::SetDocument(SettingsDocument* doc)
{
    if (m_displayTab)
        m_displayTab->SetDocument(doc);
}

//-----settings window subtabs------
SettingsTabDisplay::SettingsTabDisplay(wxNotebook *parent, const wxString &label)
    : wxPanel(parent, wxID_ANY)
{
    wxArrayString freqEinheiten;
    freqEinheiten.Add("Hz");
    freqEinheiten.Add("kHz");
    freqEinheiten.Add("MHz");
    freqEinheiten.Add("GHz");

    wxArrayString pegelEinheiten;
    pegelEinheiten.Add("DBM");
    pegelEinheiten.Add("DBMU");
    pegelEinheiten.Add("DBUV");
    pegelEinheiten.Add("DBUA");
    pegelEinheiten.Add("DBPW");
    pegelEinheiten.Add("VOLT");
    pegelEinheiten.Add("AMPERE");
    pegelEinheiten.Add("WATT");

    wxArrayString scalingY;
    scalingY.Add("Logarithmic");
    scalingY.Add("Linear");

    // Check if input is number
    wxTextValidator val(wxFILTER_NUMERIC);

    wxStaticText* labelText = new wxStaticText(this, wxID_ANY, label, wxPoint(10,10));

    // ----- Start-Ende Elemente-----Start
    m_startEndeCheck = new wxCheckBox(this, wxID_ANY, "Start - Ende Nutzen");
    // Frequenz Start
    wxStaticText* descriptionText_1 = new wxStaticText(this, wxID_ANY, "Start-Frequenz:", wxPoint(10,10));
    m_inputText_1          = new wxTextCtrl(this, wxID_ANY, "75");
    m_freqEinheitAuswahl_1 = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, freqEinheiten);

    m_inputText_1         ->SetValidator(val);
    m_freqEinheitAuswahl_1->SetSelection(2);

    // Frequenz Ende
    wxStaticText* descriptionText_2 = new wxStaticText(this, wxID_ANY, "End-Frequenz:", wxPoint(10,10));
    m_inputText_2          = new wxTextCtrl(this, wxID_ANY, "125");
    m_freqEinheitAuswahl_2 = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, freqEinheiten);

    m_inputText_2         ->SetValidator(val);
    m_freqEinheitAuswahl_2->SetSelection(2);
    // ----- Start-Ende Elemente-----Ende

    // ----- Center-Spanne Elemente-----Start
    m_centerSpanCheck = new wxCheckBox(this, wxID_ANY, "Center - Spanne Nutzen");
    // Frequenz Center
    wxStaticText* descriptionText_3 = new wxStaticText(this, wxID_ANY, "Center-Frequenz:", wxPoint(10,10));
    m_inputText_3          = new wxTextCtrl(this, wxID_ANY, "100");
    m_freqEinheitAuswahl_3 = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, freqEinheiten);

    m_inputText_3         ->SetValidator(val);
    m_freqEinheitAuswahl_3->SetSelection(2);

    // Frequenz Spanne
    wxStaticText* descriptionText_4 = new wxStaticText(this, wxID_ANY, "Span-Frequenz:", wxPoint(10,10));
    m_inputText_4          = new wxTextCtrl(this, wxID_ANY, "50");
    m_freqEinheitAuswahl_4 = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, freqEinheiten);

    m_inputText_4         ->SetValidator(val);
    m_freqEinheitAuswahl_4->SetSelection(2);
    // ----- Center-Spanne Elemente-----Ende

    //Y-Achsen Elemente-----Start
    wxStaticText* descriptionText_5 = new wxStaticText(this, wxID_ANY, "Y-Scaling:", wxPoint(10,10));

    m_inputText_5         = new wxTextCtrl(this, wxID_ANY, "100");
    m_pegelEinheitAuswahl = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, pegelEinheiten);

    m_inputText_5         ->SetValidator(val);
    m_pegelEinheitAuswahl ->SetSelection(0);

    wxStaticText* descriptionText_6 = new wxStaticText(this, wxID_ANY, "Y-Scale Spacing:", wxPoint(10,10));

    m_yScalingAuswahl = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, scalingY);
    m_yScalingAuswahl ->SetSelection(0);

    wxStaticText* descriptionText_7 = new wxStaticText(this, wxID_ANY, "Referenzpegel in dB:", wxPoint(10,10));

    m_inputText_7 = new wxTextCtrl(this, wxID_ANY, "-20");
    m_inputText_7 ->SetValidator(val);
    //Y-Achsen Elemente-----Ende

    //Knöpfe START
    wxButton* anwendenButton   = new wxButton(this, wxID_ANY, "Anwenden");
    wxButton* getCurrentButton = new wxButton(this, wxID_ANY, wxString::FromUTF8("Messgerät Einstellungen Laden"));

    m_startEndeCheck ->Bind(wxEVT_CHECKBOX, &SettingsTabDisplay::OnToggleSelection, this);
    m_centerSpanCheck->Bind(wxEVT_CHECKBOX, &SettingsTabDisplay::OnToggleSelection, this);
    anwendenButton   ->Bind(wxEVT_BUTTON,   &SettingsTabDisplay::OnAnwenden,        this);
    getCurrentButton ->Bind(wxEVT_BUTTON,   &SettingsTabDisplay::OnGetCurrent,      this);
    //Knöpfe ENDE

    //Start-Ende Sizer-----START
    wxBoxSizer* sizerHorizontal_1 = new wxBoxSizer(wxHORIZONTAL);
    sizerHorizontal_1->Add(descriptionText_1,    1, wxALL, 5);
    sizerHorizontal_1->Add(m_inputText_1,         1, wxALL, 5);
    sizerHorizontal_1->Add(m_freqEinheitAuswahl_1,1, wxALL, 5);

    wxBoxSizer* sizerHorizontal_2 = new wxBoxSizer(wxHORIZONTAL);
    sizerHorizontal_2->Add(descriptionText_2,    1, wxALL, 5);
    sizerHorizontal_2->Add(m_inputText_2,         1, wxALL, 5);
    sizerHorizontal_2->Add(m_freqEinheitAuswahl_2,1, wxALL, 5);

    wxStaticBoxSizer* staticSizer_1 = new wxStaticBoxSizer(wxVERTICAL, this, "Anzeigebereich Start-Ende Frequenz");
    staticSizer_1->Add(m_startEndeCheck,   0, wxALL | wxEXPAND, 5);
    staticSizer_1->Add(sizerHorizontal_1,  0, wxALL | wxEXPAND, 5);
    staticSizer_1->Add(sizerHorizontal_2,  0, wxALL | wxEXPAND, 5);
    //Start-Ende Sizer-----ENDE

    //Center-Spann Sizer-----START
    wxBoxSizer* sizerHorizontal_3 = new wxBoxSizer(wxHORIZONTAL);
    sizerHorizontal_3->Add(descriptionText_3,    1, wxALL, 5);
    sizerHorizontal_3->Add(m_inputText_3,         1, wxALL, 5);
    sizerHorizontal_3->Add(m_freqEinheitAuswahl_3,1, wxALL, 5);

    wxBoxSizer* sizerHorizontal_4 = new wxBoxSizer(wxHORIZONTAL);
    sizerHorizontal_4->Add(descriptionText_4,    1, wxALL, 5);
    sizerHorizontal_4->Add(m_inputText_4,         1, wxALL, 5);
    sizerHorizontal_4->Add(m_freqEinheitAuswahl_4,1, wxALL, 5);

    wxStaticBoxSizer* staticSizer_2 = new wxStaticBoxSizer(wxVERTICAL, this, "Anzeigebereich Center-Span Frequenz");
    staticSizer_2->Add(m_centerSpanCheck,  0, wxALL | wxEXPAND, 5);
    staticSizer_2->Add(sizerHorizontal_3,  0, wxALL | wxEXPAND, 5);
    staticSizer_2->Add(sizerHorizontal_4,  0, wxALL | wxEXPAND, 5);
    //Center-Spann Sizer-----ENDE

    //Y-Achsen Sizer-----START
    wxBoxSizer* sizerHorizontal_5 = new wxBoxSizer(wxHORIZONTAL);
    sizerHorizontal_5->Add(descriptionText_5,    1, wxALL | wxEXPAND, 5);
    sizerHorizontal_5->Add(m_inputText_5,         1, wxALL | wxEXPAND, 5);
    sizerHorizontal_5->Add(m_pegelEinheitAuswahl, 1, wxALL | wxEXPAND, 5);

    wxBoxSizer* sizerHorizontal_6 = new wxBoxSizer(wxHORIZONTAL);
    sizerHorizontal_6->Add(descriptionText_6,    2, wxALL | wxEXPAND, 5);
    sizerHorizontal_6->Add(m_yScalingAuswahl,     1, wxALL | wxEXPAND, 5);

    wxBoxSizer* sizerHorizontal_7 = new wxBoxSizer(wxHORIZONTAL);
    sizerHorizontal_7->Add(descriptionText_7,    2, wxALL | wxEXPAND, 5);
    sizerHorizontal_7->Add(m_inputText_7,         1, wxALL | wxEXPAND, 5);

    wxStaticBoxSizer* staticSizer_3 = new wxStaticBoxSizer(wxVERTICAL, this, "Anzeigebereich Pegel-Skalierung");
    staticSizer_3->Add(sizerHorizontal_6, 0, wxALL | wxEXPAND, 5);
    staticSizer_3->Add(sizerHorizontal_5, 0, wxALL | wxEXPAND, 5);
    staticSizer_3->Add(sizerHorizontal_7, 0, wxALL | wxEXPAND, 5);
    //Y-Achsen Sizer-----ENDE

    wxBoxSizer* sizerHorizontal_8 = new wxBoxSizer(wxHORIZONTAL);
    sizerHorizontal_8->Add(getCurrentButton, 1, wxALL | wxEXPAND, 5);
    sizerHorizontal_8->Add(anwendenButton,   1, wxALL | wxEXPAND, 5);

    wxBoxSizer* sizerVertical = new wxBoxSizer(wxVERTICAL);
    sizerVertical->Add(labelText,          0, wxALL | wxEXPAND, 5);
    sizerVertical->Add(staticSizer_1,      0, wxALL | wxEXPAND, 5);
    sizerVertical->Add(staticSizer_2,      0, wxALL | wxEXPAND, 5);
    sizerVertical->Add(staticSizer_3,      0, wxALL | wxEXPAND, 5);
    sizerVertical->Add(sizerHorizontal_8,  0, wxALL | wxEXPAND, 5);

    this->SetSizerAndFit(sizerVertical);

    // Set initial widget enable state (Start/End active by default)
    UpdateToggleWidgetState();
}

// ---------------------------------------------------------------------------
// Document/View wiring
// ---------------------------------------------------------------------------

void SettingsTabDisplay::SetDocument(SettingsDocument* doc)
{
    if (m_doc)
        m_doc->RemoveObserver(this);

    m_doc = doc;

    if (m_doc)
    {
        m_doc->AddObserver(this);
        RefreshFromDocument();
        UpdateToggleWidgetState();
    }
}

void SettingsTabDisplay::OnDocumentChanged(const std::string& changeType)
{
    if (changeType == "SettingsLoaded" || changeType == "SettingsApplied")
    {
        RefreshFromDocument();
    }
    else if (changeType == "FreqModeChanged")
    {
        UpdateToggleWidgetState();
    }
}

// ---------------------------------------------------------------------------
// View helpers
// ---------------------------------------------------------------------------

void SettingsTabDisplay::RefreshFromDocument()
{
    if (!m_doc)
        return;

    m_inputText_1         ->SetValue(m_doc->GetFreqStart());
    m_inputText_2         ->SetValue(m_doc->GetFreqEnd());
    m_inputText_3         ->SetValue(m_doc->GetFreqCenter());
    m_inputText_4         ->SetValue(m_doc->GetFreqSpan());
    m_inputText_5         ->SetValue(m_doc->GetPegelValue());
    m_inputText_7         ->SetValue(m_doc->GetRefLevel());

    m_freqEinheitAuswahl_1->SetStringSelection(m_doc->GetFreqStartUnit());
    m_freqEinheitAuswahl_2->SetStringSelection(m_doc->GetFreqEndUnit());
    m_freqEinheitAuswahl_3->SetStringSelection(m_doc->GetFreqCenterUnit());
    m_freqEinheitAuswahl_4->SetStringSelection(m_doc->GetFreqSpanUnit());
    m_pegelEinheitAuswahl ->SetStringSelection(m_doc->GetPegelUnit());
    m_yScalingAuswahl     ->SetStringSelection(m_doc->GetYSpacing());
}

void SettingsTabDisplay::UpdateToggleWidgetState()
{
    bool useStartEnd = m_doc ? m_doc->IsUsingStartEnd() : true;

    m_startEndeCheck ->SetValue(useStartEnd);
    m_centerSpanCheck->SetValue(!useStartEnd);

    m_inputText_1         ->Enable(useStartEnd);
    m_inputText_2         ->Enable(useStartEnd);
    m_freqEinheitAuswahl_1->Enable(useStartEnd);
    m_freqEinheitAuswahl_2->Enable(useStartEnd);

    m_inputText_3         ->Enable(!useStartEnd);
    m_inputText_4         ->Enable(!useStartEnd);
    m_freqEinheitAuswahl_3->Enable(!useStartEnd);
    m_freqEinheitAuswahl_4->Enable(!useStartEnd);
}

// ---------------------------------------------------------------------------
// Event handlers
// ---------------------------------------------------------------------------

void SettingsTabDisplay::OnAnwenden(wxCommandEvent& event)
{
    if (!m_doc)
        return;

    m_doc->ApplySettings(
        m_inputText_1->GetValue().ToStdString(),          m_freqEinheitAuswahl_1->GetStringSelection().ToStdString(),
        m_inputText_2->GetValue().ToStdString(),          m_freqEinheitAuswahl_2->GetStringSelection().ToStdString(),
        m_inputText_3->GetValue().ToStdString(),          m_freqEinheitAuswahl_3->GetStringSelection().ToStdString(),
        m_inputText_4->GetValue().ToStdString(),          m_freqEinheitAuswahl_4->GetStringSelection().ToStdString(),
        m_inputText_5->GetValue().ToStdString(),          m_pegelEinheitAuswahl->GetStringSelection().ToStdString(),
        m_yScalingAuswahl->GetStringSelection().ToStdString(),
        m_inputText_7->GetValue().ToStdString(),
        m_startEndeCheck->GetValue()
    );
}

void SettingsTabDisplay::OnGetCurrent(wxCommandEvent& event)
{
    if (m_doc)
        m_doc->QueryFromDevice();
}

void SettingsTabDisplay::OnToggleSelection(wxCommandEvent& event)
{
    if (!m_doc)
    {
        // No document yet — just toggle widget state locally
        bool useStartEnd = m_startEndeCheck->GetValue();
        m_startEndeCheck ->SetValue(useStartEnd);
        m_centerSpanCheck->SetValue(!useStartEnd);
        UpdateToggleWidgetState();
        return;
    }

    // The checkbox that fired the event tells us the desired mode
    bool useStartEnd = (event.GetEventObject() == m_startEndeCheck)
                       ? m_startEndeCheck->GetValue()
                       : !m_centerSpanCheck->GetValue();

    m_doc->SetFreqMode(useStartEnd);
}


SettingsTabAdapter::SettingsTabAdapter(wxNotebook *parent, const wxString &label)
    : wxPanel(parent, wxID_ANY)
{
    new wxStaticText(this, wxID_ANY, label, wxPoint(10,10));
}

SettingsTabGeneral::SettingsTabGeneral(wxNotebook *parent, const wxString &label)
    : wxPanel(parent, wxID_ANY)
{
    new wxStaticText(this, wxID_ANY, label, wxPoint(10,10));
}
//------Settings window subtab end-----