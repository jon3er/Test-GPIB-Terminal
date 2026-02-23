#include "Mesurement.h"
#include "main.h"
#include "SettingsWindow.h"
#include "cmdGpib.h"
#include "mainHelper.h"
#include "GenericInputDialog.h"

int PlotWindow::s_windowCounter = 0;

//-----Plot Window BEGIN--------
PlotWindow::PlotWindow(wxWindow *parent, MainDocument* mainDoc)
    : wxFrame(parent, wxID_ANY,
              wxString::Format("Measurement Window %d", ++s_windowCounter),
              wxDefaultPosition, wxSize(1000,750),
              wxDEFAULT_FRAME_STYLE)
    , m_mainDoc(mainDoc)
    , m_windowId(s_windowCounter)
{
    getFileNames(m_filePath, m_fileNames);

    //------------------ Menubar --------------------
    m_menuBar = new wxMenuBar(0);

    //------------------ File menu --------------------
    wxMenu* menuFile = new wxMenu();
    menuFile->Append(MainMenuBar::ID_Plot_File_Open,    wxT("Open") + wxString(wxT("\t")) + wxT("Ctrl+O"));
    menuFile->AppendSeparator();
    menuFile->Append(MainMenuBar::ID_Plot_File_Close,   wxT("Close"));
    menuFile->AppendSeparator();
    menuFile->Append(MainMenuBar::ID_Plot_File_Save,    wxT("Save") + wxString(wxT("\t")) + wxT("Ctrl+S"));
    menuFile->Append(MainMenuBar::ID_Plot_File_SaveAs,  wxT("Save as ..."));
    menuFile->AppendSeparator();
    menuFile->Append(MainMenuBar::ID_Plot_File_Exit,    wxT("Exit"));

    //------------------ Measurement menu --------------------
    wxMenu* menuMesurement = new wxMenu();

    // "New Mesurement" submenu with Load config and Presets
    wxMenu* subMenuNew = new wxMenu();
    subMenuNew->Append(MainMenuBar::ID_Main_Mesurement_New,        wxT("New Mesurement"));
    subMenuNew->AppendSeparator();
    subMenuNew->Append(MainMenuBar::ID_Main_Mesurement_Load,       wxT("Load config"));
    subMenuNew->AppendSeparator();
    subMenuNew->Append(MainMenuBar::ID_Main_Mesurement_Preset_1,   wxT("Preset 1"));
    subMenuNew->Append(MainMenuBar::ID_Main_Mesurement_Preset_2,   wxT("Preset 2"));
    subMenuNew->Append(MainMenuBar::ID_Main_Mesurement_Preset_3,   wxT("Preset 3"));

    menuMesurement->AppendSubMenu(subMenuNew, wxT("New Mesurement"));
    menuMesurement->AppendSeparator();
    menuMesurement->Append(MainMenuBar::ID_Main_Mesurement_Open,       wxT("Open Saved Mesurement"));
    menuMesurement->AppendSeparator();
    menuMesurement->Append(MainMenuBar::ID_Main_Mesurement_2D_Mess,    wxT("2D Plot Mesurment"));
    menuMesurement->AppendSeparator();
    menuMesurement->Append(MainMenuBar::ID_Main_Mesurement_SetMarker,  wxT("Set Marker"));
    menuMesurement->AppendSeparator();
    menuMesurement->Append(MainMenuBar::ID_Main_Mesurement_Settings,   wxT("Settings"));

    m_menuBar->Append(menuFile,        wxT("File"));
    m_menuBar->Append(menuMesurement,  wxT("Mesurement"));
    this->SetMenuBar(m_menuBar);

    // Bind File menu handlers (local to this window)
    Bind(wxEVT_MENU, &PlotWindow::OnMenuFileOpen,    this, MainMenuBar::ID_Plot_File_Open);
    Bind(wxEVT_MENU, &PlotWindow::OnMenuFileClose,   this, MainMenuBar::ID_Plot_File_Close);
    Bind(wxEVT_MENU, &PlotWindow::OnMenuFileSave,    this, MainMenuBar::ID_Plot_File_Save);
    Bind(wxEVT_MENU, &PlotWindow::OnMenuFileSaveAs,  this, MainMenuBar::ID_Plot_File_SaveAs);
    Bind(wxEVT_MENU, &PlotWindow::OnMenuFileExit,    this, MainMenuBar::ID_Plot_File_Exit);

    // Bind Measurement menu handlers (forward to parent MainProgrammWin)
    Bind(wxEVT_MENU, &PlotWindow::OnMenuMesurementNew,       this, MainMenuBar::ID_Main_Mesurement_New);
    Bind(wxEVT_MENU, &PlotWindow::OnMenuMesurementOpen,      this, MainMenuBar::ID_Main_Mesurement_Open);
    Bind(wxEVT_MENU, &PlotWindow::OnMenuMesurementLoad,      this, MainMenuBar::ID_Main_Mesurement_Load);
    Bind(wxEVT_MENU, &PlotWindow::OnMenuMesurementPreset1,   this, MainMenuBar::ID_Main_Mesurement_Preset_1);
    Bind(wxEVT_MENU, &PlotWindow::OnMenuMesurementPreset2,   this, MainMenuBar::ID_Main_Mesurement_Preset_2);
    Bind(wxEVT_MENU, &PlotWindow::OnMenuMesurementPreset3,   this, MainMenuBar::ID_Main_Mesurement_Preset_3);
    Bind(wxEVT_MENU, &PlotWindow::OnMenuMesurement2DMess,    this, MainMenuBar::ID_Main_Mesurement_2D_Mess);
    Bind(wxEVT_MENU, &PlotWindow::OnMenuMesurementSetMarker, this, MainMenuBar::ID_Main_Mesurement_SetMarker);
    Bind(wxEVT_MENU, &PlotWindow::OnMenuMesurementSettings,  this, MainMenuBar::ID_Main_Mesurement_Settings);
    //------------------ End Menubar --------------------

    wxButton* executeMesurment = new wxButton(this, wxID_ANY, "Execute Mesurement");
    executeMesurment->Bind(wxEVT_BUTTON, &PlotWindow::executeScriptEvent, this);
    m_selectMesurement = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_fileNames);
    m_selectMesurement->SetSelection(0);

    // Plot container panel — the mpWindow will be kept square and centred
    // inside this panel via the wxEVT_SIZE handler below.
    m_plotPanel = new wxPanel(this, wxID_ANY);

    // 1. mpWindow (Zeichenfläche) erstellen — parented to m_plotPanel
    m_plot = new mpWindow(m_plotPanel, wxID_ANY);

    // Helles Theme: weißer Hintergrund, dunkle Achsen, dezentes Gitter
    m_plot->SetColourTheme(
        wxColour(255, 255, 255),     // Hintergrund: weiß
        wxColour(40,  40,  40),      // Achsen/Text: dunkelgrau
        wxColour(210, 210, 220)      // Gitterlinien: hellgrau
    );
    m_plot->SetMargins(40, 30, 55, 65);
    m_plot->EnableDoubleBuffer(true); // kein Flimmern

    // 2. Achsen als Layer hinzufügen
    wxFont axisFont(9, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
    wxPen  axisPen(wxColour(40, 40, 40), 1);

    mpScaleX* xAxis = new mpScaleX("Frequenz", mpALIGN_BORDER_BOTTOM, true, mpX_NORMAL);
    xAxis->SetFont(axisFont);
    xAxis->SetPen(axisPen);

    mpScaleY* yAxis = new mpScaleY("Amplitude", mpALIGN_BORDER_LEFT, true);
    yAxis->SetFont(axisFont);
    yAxis->SetPen(axisPen);

    m_plot->AddLayer(xAxis);
    m_plot->AddLayer(yAxis);

    std::vector<double> realAmp = {0.0, 1.0, 2.0, 3.0, 4.0, 5.0 };
    std::vector<double> imagAmp = {0.0, 1.0, 4.0, 2.0, 5.0, 3.0 };
    std::vector<double> freq = {0.0, 1.0, 2.0, 3.0, 4.0, 5.0 };
    // 3. Daten vorbereiten (std::vector laut Header Definition von mpFXYVector)

    //Holt in dem Haupt menu geladene daten
    if (m_mainDoc != nullptr && m_mainDoc->IsFileOpen())
    {
        sData::sParam* tempStruct = m_mainDoc->GetData().GetParameter();
        m_mainDoc->GetData().GetData(tempStruct, realAmp, imagAmp, freq);
    }



    // 4. Vektor-Layer erstellen
    m_vectorLayer = new mpFXYVector("Messdaten");
    m_vectorLayer->SetData(freq, realAmp);
    m_vectorLayer->SetContinuity(true); // True = Linie zeichnen
    m_vectorLayer->SetPen(wxPen(wxColour(30, 100, 200), 2, wxPENSTYLE_SOLID)); // kräftiges Blau
    m_vectorLayer->ShowName(true);      // Wichtig für die Legende

    m_plot->AddLayer(m_vectorLayer);

    // 5. Legende hinzufügen (mpInfoLegend ist ein Layer)
    static wxBrush legendBrush(wxColour(248, 248, 252), wxBRUSHSTYLE_SOLID);
    mpInfoLegend* legend = new mpInfoLegend(wxRect(25, 25, 90, 35), &legendBrush);
    legend->SetItemMode(mpLEGEND_LINE); // Zeigt Linie statt Quadrat in der Legende
    m_plot->AddLayer(legend);

    // 6. Layout-Management
    // Close handler for modeless window self-cleanup
    Bind(wxEVT_CLOSE_WINDOW, &PlotWindow::OnClose, this);

    // Let m_plot fill the entire plot panel
    wxBoxSizer* plotPanelSizer = new wxBoxSizer(wxVERTICAL);
    plotPanelSizer->Add(m_plot, 1, wxEXPAND);
    m_plotPanel->SetSizer(plotPanelSizer);

    // ---- Bottom 1/3: left = controls, right = info ----

    // Left: control buttons and [x ; y] selector
    wxBoxSizer* leftSizer = new wxBoxSizer(wxVERTICAL);
    leftSizer->Add(executeMesurment,   0, wxEXPAND | wxALL, 3);
    leftSizer->Add(m_selectMesurement, 0, wxEXPAND | wxALL, 3);

    // [x ; y] matrix measurement selector row
    wxBoxSizer* matrixSizer = new wxBoxSizer(wxHORIZONTAL);
    wxStaticText* matrixLabel = new wxStaticText(this, wxID_ANY, "Select [x ; y]:");
    m_textXSelector = new wxTextCtrl(this, wxID_ANY, "0", wxDefaultPosition, wxSize(50, -1));
    wxStaticText* matrixSep = new wxStaticText(this, wxID_ANY, ";");
    m_textYSelector = new wxTextCtrl(this, wxID_ANY, "0", wxDefaultPosition, wxSize(50, -1));
    wxButton* selectBtn = new wxButton(this, wxID_ANY, "Go");
    selectBtn->Bind(wxEVT_BUTTON, &PlotWindow::OnSelectMeasurement, this);
    matrixSizer->Add(matrixLabel,     0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
    matrixSizer->Add(m_textXSelector, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 3);
    matrixSizer->Add(matrixSep,       0, wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT, 3);
    matrixSizer->Add(m_textYSelector, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 3);
    matrixSizer->Add(selectBtn,       0, wxALIGN_CENTER_VERTICAL);
    leftSizer->Add(matrixSizer, 0, wxEXPAND | wxALL, 3);
    leftSizer->AddStretchSpacer(1);

    // Right: info panel (placeholder)
    m_infoPanel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_SIMPLE);
    wxBoxSizer* infoSizer = new wxBoxSizer(wxVERTICAL);
    wxStaticText* infoTitle = new wxStaticText(m_infoPanel, wxID_ANY, "Measurement Info",
        wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER_HORIZONTAL);
    infoTitle->SetFont(infoTitle->GetFont().Bold());
    infoSizer->Add(infoTitle, 0, wxEXPAND | wxALL, 8);
    infoSizer->Add(new wxStaticLine(m_infoPanel), 0, wxEXPAND | wxLEFT | wxRIGHT, 5);
    m_infoText = new wxStaticText(m_infoPanel, wxID_ANY, wxEmptyString,
        wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
    infoSizer->Add(m_infoText, 1, wxEXPAND | wxALL, 8);
    m_infoPanel->SetSizer(infoSizer);

    // Populate info panel from pre-loaded document (or show placeholder)
    if (m_mainDoc != nullptr && m_mainDoc->IsFileOpen())
        UpdateInfoPanel(m_mainDoc->GetData().GetParameter());
    else
        UpdateInfoPanel(nullptr);

    // Bottom sizer: left info | right controls (equal halves)
    wxBoxSizer* bottomSizer = new wxBoxSizer(wxHORIZONTAL);
    bottomSizer->Add(m_infoPanel,  1, wxEXPAND | wxALL, 5);
    bottomSizer->Add(leftSizer,    1, wxEXPAND | wxALL, 5);

    // Main sizer: plot (proportion 2) on top, controls (proportion 1) at bottom
    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
    mainSizer->Add(m_plotPanel,   2, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, 5);
    mainSizer->Add(bottomSizer,   1, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 5);
    this->SetSizer(mainSizer);
    this->Layout();
    // 7. Zoom auf Daten anpassen
    m_plot->Fit();

    // (callbacks wired via SetDocument / OnDocumentChanged)
}

PlotWindow::~PlotWindow()
{
    if (m_document)
    {
        m_document->RemoveObserver(this);
        if (m_ownsDocument)
        {
            delete m_document;
            m_document = nullptr;
        }
    }

    std::cerr << "PlotWindow " << m_windowId << " closed" << std::endl;
}

void PlotWindow::SetDocument(MeasurementDocument* doc)
{
    if (m_document)
    {
        m_document->RemoveObserver(this);
        if (m_ownsDocument)
        {
            delete m_document;
        }
    }

    m_document = doc;
    m_ownsDocument = false;  // caller-owned by default

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

void PlotWindow::OnSelectMeasurement(wxCommandEvent& /*event*/)
{
    long x = 0, y = 0;

    if (!m_textXSelector->GetValue().ToLong(&x) ||
        !m_textYSelector->GetValue().ToLong(&y))
    {
        wxMessageBox("Please enter valid integer values for x and y.",
                     "Invalid Input", wxOK | wxICON_WARNING, this);
        return;
    }

    // TODO: load the measurement at matrix position [x; y] from the data set.
    wxLogMessage("PlotWindow: selected measurement [%ld ; %ld]", x, y);

    wxLogMessage("PlotWindow: selected measurement [%ld ; %ld]", x, y);
}

void PlotWindow::UpdateInfoPanel(sData::sParam* param)
{
    if (!param)
    {
        m_infoText->SetLabel("--- No measurement loaded ---");
        m_infoPanel->Layout();
        return;
    }

    auto freqStr = [](unsigned int hz) -> wxString {
        if (hz < 0)         return wxT("---");
        if (hz >= 1000000u) return wxString::Format("%g MHz", hz / 1e6);
        if (hz >= 1000u)    return wxString::Format("%g kHz", hz / 1e3);
        return wxString::Format("%u Hz", hz);
    };

    wxString info;
    info += wxString::Format("File Name:          %s\n", param->File);
    info += wxString::Format("Measurement Type:   %s\n", param->Type);
    info += wxString::Format("Date:               %s\n", param->Date);
    info += wxString::Format("Time:               %s\n", param->Time);
    info += wxString::Format("X Points:           %u\n", param->NoPoints_X);
    info += wxString::Format("Y Points:           %u\n", param->NoPoints_Y);
    info += wxString::Format("Measurement Points: %u\n", param->NoPoints_Array);
    info += wxString::Format("Start Frequency:    %s\n", freqStr(param->startFreq));
    info += wxString::Format("End Frequency:      %s",   freqStr(param->endFreq));

    m_infoText->SetLabel(info);
    m_infoPanel->Layout();
}

// -----------------------------------------------------------------------
// File menu handlers (local to this PlotWindow)
// -----------------------------------------------------------------------
void PlotWindow::OnMenuFileOpen(wxCommandEvent& event)
{
    wxFileDialog openFileDialog(this, _("Import CSV Data"),
        System::filePathRoot,
        "",
        "CSV Files (*.csv)|*.csv|All Files (*.*)|*.*",
        wxFD_OPEN | wxFD_FILE_MUST_EXIST);

    if (openFileDialog.ShowModal() == wxID_CANCEL)
        return;

    wxString filePath = openFileDialog.GetPath();
    sData importedData;
    CsvFile csvFile;

    if (!csvFile.readCsvFile(filePath, importedData))
    {
        wxMessageBox("Failed to read CSV file: " + filePath,
                     "Import Error", wxOK | wxICON_ERROR, this);
        return;
    }

    // Extract data from the imported sData and update the plot
    sData::sParam* param = importedData.GetParameter();
    std::vector<double> realAmp, imagAmp, freq;
    importedData.GetData(param, realAmp, imagAmp, freq);
    // add Loaded data to document data
        // Update the document with the loaded data
    if (m_document)
    {
        // Copy the full sData into the document's result store
        m_document->GetResultsMutable() = importedData;

        // Push the x/y vectors so observers (e.g. live plot refresh) are notified
        m_document->SetXData(freq);
        m_document->SetYData(realAmp); // amplitude
    }
    
    m_vectorLayer->SetData(freq, realAmp);
    m_plot->AddLayer(m_vectorLayer);
    m_plot->Fit();
    m_plot->Refresh();

    sData::sParam* docParam = m_document
        ? m_document->GetResultsMutable().GetParameter()
        : param;
    UpdateInfoPanel(docParam);

    SetTitle(wxString::Format("Measurement Window %d \u2014 %s",
             m_windowId, filePath.AfterLast('\\').AfterLast('/')));

    std::cout << "[PlotWindow " << m_windowId << "] Imported CSV: "
              << filePath << std::endl;
}

void PlotWindow::OnMenuFileClose(wxCommandEvent& event)
{
    Close();
}

void PlotWindow::OnMenuFileSave(wxCommandEvent& event)
{
    // Forward to parent MainProgrammWin
    MainProgrammWin* parent = dynamic_cast<MainProgrammWin*>(GetParent());
    if (parent)
        parent->MenuFileSave(event);
}

void PlotWindow::OnMenuFileSaveAs(wxCommandEvent& event)
{
    // Forward to parent MainProgrammWin
    MainProgrammWin* parent = dynamic_cast<MainProgrammWin*>(GetParent());
    if (parent)
        parent->MenuFileSaveAs(event);
}

void PlotWindow::OnMenuFileExit(wxCommandEvent& event)
{
    Close();
}

// -----------------------------------------------------------------------
// Measurement menu handlers (forward to parent MainProgrammWin)
// -----------------------------------------------------------------------
void PlotWindow::OnMenuMesurementNew(wxCommandEvent& event)
{
    MainProgrammWin* parent = dynamic_cast<MainProgrammWin*>(GetParent());
    if (parent)
        parent->MenuMesurementNew(event);
}

void PlotWindow::OnMenuMesurementOpen(wxCommandEvent& event)
{
    MainProgrammWin* parent = dynamic_cast<MainProgrammWin*>(GetParent());
    if (parent)
        parent->MenuMesurementLoad(event);
}

void PlotWindow::OnMenuMesurementLoad(wxCommandEvent& event)
{
    MainProgrammWin* parent = dynamic_cast<MainProgrammWin*>(GetParent());
    if (parent)
        parent->MenuMesurementLoad(event);
}

void PlotWindow::OnMenuMesurementPreset1(wxCommandEvent& /*event*/)
{
    std::vector<InputFieldDef> fields = {
        { "Start Frequency:",    "1000000",   "SENS:FREQ:STAR {}"     },
        { "Stop Frequency:",     "100000000", "SENS:FREQ:STOP {}"     },
        { "Sweep Points:",       "512",       "SENS:SWE:POIN {}"      },
        { "IF Bandwidth (Hz):",  "1000",      "SENS:BWID {}"          },
        { "Power Level (dBm):",  "-10",       "SOUR:POW {}"           },
        { "Averaging:",          "1",         "SENS:AVER:COUN {}"     },
        { "Port:",               "1",         "SENS:PORT {}"          },
        { "Cal Group:",          "",          "SENS:CORR:CKIT:SEL {}" },
    };

    auto* dlg = new GenericInputDialog(this, "Preset 1 \u2014 Frequency Sweep", fields,
        [this](const std::vector<wxString>& vals)
        {
            // TODO: apply Preset 1 values to document / hardware
            wxLogMessage("Preset 1 confirmed: Start=%s  Stop=%s  Points=%s",
                vals[0], vals[1], vals[2]);
        });
    dlg->ShowModal();
    dlg->Destroy();
}

void PlotWindow::OnMenuMesurementPreset2(wxCommandEvent& /*event*/)
{
    std::vector<InputFieldDef> fields = {
        { "X Measurement Points:",  "5",   ""  },
        { "Y Measurement Points:",  "5",   ""  },
        { "X Start Coordinate:",    "0",   ""  },
        { "Y Start Coordinate:",    "0",   ""  },
        { "X Spacing (mm):",        "10",  ""  },
        { "Y Spacing (mm):",        "10",  ""  },
        { "Speed (mm/s):",          "50",  ""  },
        { "Dwell Time (ms):",       "200", ""  },
    };

    auto* dlg = new GenericInputDialog(this, "Preset 2 \u2014 2D Grid Scan", fields,
        [this](const std::vector<wxString>& vals)
        {
            // TODO: apply Preset 2 values to document / hardware
            wxLogMessage("Preset 2 confirmed: Grid %sx%s, spacing %s x %s",
                vals[0], vals[1], vals[4], vals[5]);
        });
    dlg->ShowModal();
    dlg->Destroy();
}

void PlotWindow::OnMenuMesurementPreset3(wxCommandEvent& /*event*/)
{
    std::vector<InputFieldDef> fields = {
        { "Center Frequency (Hz):", "50000000", "SENS:FREQ:CENT {}"  },
        { "Span (Hz):",             "10000000", "SENS:FREQ:SPAN {}"  },
        { "Resolution BW (Hz):",    "100000",   "SENS:BWID:RES {}"   },
        { "Video BW (Hz):",         "10000",    "SENS:BWID:VID {}"   },
        { "Reference Level (dBm):", "0",        "DISP:WIND:TRAC:Y:RLEV {}" },
        { "Attenuation (dB):",      "10",       "INP:ATT {}"         },
        { "Sweep Time (ms):",       "0",        "SENS:SWE:TIME {}"   },
        { "Marker Count:",          "1",        "CALC:MARK:COUN {}"  },
    };

    auto* dlg = new GenericInputDialog(this, "Preset 3 \u2014 Spectrum Analyser", fields,
        [this](const std::vector<wxString>& vals)
        {
            // TODO: apply Preset 3 values to document / hardware
            wxLogMessage("Preset 3 confirmed: Centre=%s  Span=%s  RBW=%s",
                vals[0], vals[1], vals[2]);
        });
    dlg->ShowModal();
    dlg->Destroy();
}

void PlotWindow::OnMenuMesurement2DMess(wxCommandEvent& event)
{
    MainProgrammWin* parent = dynamic_cast<MainProgrammWin*>(GetParent());
    if (parent)
        parent->MenuMesurement2DMess(event);
}

void PlotWindow::OnMenuMesurementSetMarker(wxCommandEvent& event)
{
    MainProgrammWin* parent = dynamic_cast<MainProgrammWin*>(GetParent());
    if (parent)
        parent->MenuMesurementSetMarker(event);
}

void PlotWindow::OnMenuMesurementSettings(wxCommandEvent& event)
{
    MainProgrammWin* parent = dynamic_cast<MainProgrammWin*>(GetParent());
    if (parent)
        parent->MenuMesurementSettings(event);
}

void PlotWindow::OnClose(wxCloseEvent& event)
{
    // Notify parent (MainProgrammWin) to remove us from its tracking set
    MainProgrammWin* parent = dynamic_cast<MainProgrammWin*>(GetParent());
    if (parent)
        parent->UnregisterMeasurementWindow(this);

    Destroy();
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
