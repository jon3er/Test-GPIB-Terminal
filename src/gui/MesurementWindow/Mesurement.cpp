#include "Mesurement.h"
#include "main.h"
#include "SettingsWindow.h"
#include "MSetDialog.h"
#include "cmdGpib.h"
#include "mainHelper.h"

#include <algorithm>


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
    
    subMenuNew->Append(MainMenuBar::ID_Main_Mesurement_Load,       wxT("Load config"));
    subMenuNew->AppendSeparator();
    subMenuNew->Append(MainMenuBar::ID_Main_Mesurement_Sweep,       wxT("Sweep"));
    subMenuNew->Append(MainMenuBar::ID_Main_Mesurement_IQ,          wxT("IQ"));
    subMenuNew->Append(MainMenuBar::ID_Main_Mesurement_MarkerPeak,  wxT("Marker"));
    subMenuNew->AppendSeparator();
    subMenuNew->Append(MainMenuBar::ID_Main_Mesurement_Custom,  wxT("custom script File"));

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
    Bind(wxEVT_MENU, &PlotWindow::OnMenuMesurementOpen,      this, MainMenuBar::ID_Main_Mesurement_Open);
    Bind(wxEVT_MENU, &PlotWindow::OnMenuMesurementLoad,      this, MainMenuBar::ID_Main_Mesurement_Load);
    Bind(wxEVT_MENU, &PlotWindow::OnMenuMesurementSweep,     this, MainMenuBar::ID_Main_Mesurement_Sweep);
    Bind(wxEVT_MENU, &PlotWindow::OnMenuMesurementIQ,        this, MainMenuBar::ID_Main_Mesurement_IQ);
    Bind(wxEVT_MENU, &PlotWindow::OnMenuMesurementMarker,   this, MainMenuBar::ID_Main_Mesurement_MarkerPeak);
    Bind(wxEVT_MENU, &PlotWindow::OnMenuMesurement2DMess,    this, MainMenuBar::ID_Main_Mesurement_2D_Mess);
    Bind(wxEVT_MENU, &PlotWindow::OnMenuMesurementSetMarker, this, MainMenuBar::ID_Main_Mesurement_SetMarker);
    Bind(wxEVT_MENU, &PlotWindow::OnMenuMesurementSettings,  this, MainMenuBar::ID_Main_Mesurement_Settings);
    //------------------ End Menubar --------------------

    wxButton* executeMesurment = new wxButton(this, wxID_ANY, "Execute Mesurement");
    executeMesurment->Bind(wxEVT_BUTTON, &PlotWindow::executeScriptEvent, this);
    wxButton* openLoadedSettingsBtn = new wxButton(this, wxID_ANY, "Open Measurement Settings");
    openLoadedSettingsBtn->Bind(wxEVT_BUTTON, &PlotWindow::OnOpenLoadedMeasurementSettings, this);
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
    m_vectorLayer = new mpFXYVector("Real/Amplitude");
    m_vectorLayer->SetData(freq, realAmp);
    m_vectorLayer->SetContinuity(true); // True = Linie zeichnen
    m_vectorLayer->SetPen(wxPen(wxColour(30, 100, 200), 2, wxPENSTYLE_SOLID)); // kräftiges Blau
    m_vectorLayer->ShowName(true);      // Wichtig für die Legende

    m_vectorLayerImag = new mpFXYVector("Imag");
    m_vectorLayerImag->SetData(std::vector<double>{}, std::vector<double>{});
    m_vectorLayerImag->SetContinuity(true);
    m_vectorLayerImag->SetPen(wxPen(wxColour(200, 90, 40), 2, wxPENSTYLE_SOLID));
    m_vectorLayerImag->ShowName(true);
    m_vectorLayerImag->SetVisible(false);

    m_plot->AddLayer(m_vectorLayer);
    m_plot->AddLayer(m_vectorLayerImag);

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
    leftSizer->Add(openLoadedSettingsBtn, 0, wxEXPAND | wxALL, 3);
    leftSizer->Add(m_selectMesurement, 0, wxEXPAND | wxALL, 3);

    // [x ; y] matrix measurement selector row
    wxBoxSizer* matrixSizer = new wxBoxSizer(wxHORIZONTAL);
    wxStaticText* matrixLabel = new wxStaticText(this, wxID_ANY, "Select [x ; y]:");
    m_choiceXSelector = new wxChoice(this, wxID_ANY);
    wxStaticText* matrixSep = new wxStaticText(this, wxID_ANY, ";");
    m_choiceYSelector = new wxChoice(this, wxID_ANY);
    wxButton* selectBtn = new wxButton(this, wxID_ANY, "Go");
    selectBtn->Bind(wxEVT_BUTTON, &PlotWindow::OnSelectMeasurement, this);
    matrixSizer->Add(matrixLabel,     0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
    matrixSizer->Add(m_choiceXSelector, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 3);
    matrixSizer->Add(matrixSep,          0, wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT, 3);
    matrixSizer->Add(m_choiceYSelector,  0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 3);
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
    m_selectedMeasurementText = new wxStaticText(m_infoPanel, wxID_ANY,
        "Selected [x ; y]: -", wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
    infoSizer->Add(m_selectedMeasurementText, 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, 8);
    m_infoText = new wxStaticText(m_infoPanel, wxID_ANY, wxEmptyString,
        wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
    infoSizer->Add(m_infoText, 1, wxEXPAND | wxALL, 8);
    m_infoPanel->SetSizer(infoSizer);

    // Populate info panel from pre-loaded document (or show placeholder)
    if (m_mainDoc != nullptr && m_mainDoc->IsFileOpen())
        UpdateInfoPanel(m_mainDoc->GetData().GetParameter());
    else
        UpdateInfoPanel(nullptr);

    // Current device settings panel
    m_settingsPanel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_SIMPLE);
    wxBoxSizer* settingsSizer = new wxBoxSizer(wxVERTICAL);
    wxStaticText* settingsTitle = new wxStaticText(m_settingsPanel, wxID_ANY, "Current Settings",
        wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER_HORIZONTAL);
    settingsTitle->SetFont(settingsTitle->GetFont().Bold());
    settingsSizer->Add(settingsTitle, 0, wxEXPAND | wxALL, 8);
    settingsSizer->Add(new wxStaticLine(m_settingsPanel), 0, wxEXPAND | wxLEFT | wxRIGHT, 5);
    m_settingsText = new wxStaticText(m_settingsPanel, wxID_ANY, wxEmptyString,
        wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
    settingsSizer->Add(m_settingsText, 1, wxEXPAND | wxALL, 8);
    m_settingsPanel->SetSizer(settingsSizer);
    UpdateSettingsPanel();

    // Bottom sizer: measurement info | device settings | controls
    wxBoxSizer* bottomSizer = new wxBoxSizer(wxHORIZONTAL);
    bottomSizer->Add(m_infoPanel,      1, wxEXPAND | wxALL, 5);
    bottomSizer->Add(m_settingsPanel,  1, wxEXPAND | wxALL, 5);
    bottomSizer->Add(leftSizer,        1, wxEXPAND | wxALL, 5);

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
            if (!m_document)
                return;

            sData& results = m_document->GetResultsMutable();
            sData::sParam* param = results.GetParameter();

            if (param)
            {
                PopulateSelectors(static_cast<unsigned int>(std::max(0, param->NoPoints_X)),
                                  static_cast<unsigned int>(std::max(0, param->NoPoints_Y)));

                int xi = m_choiceXSelector->GetSelection();
                int yi = m_choiceYSelector->GetSelection();
                if (ApplySelectionToPlot(xi, yi, false))
                {
                    std::cout << "[PlotWindow] Plot refreshed from selected matrix point" << std::endl;
                    return;
                }
            }

            // Fallback: show latest live vector data if matrix selection cannot be resolved yet.
            auto x = m_document->GetXData();
            auto y = m_document->GetYData();
            m_vectorLayer->SetData(x, y);
            m_vectorLayerImag->SetVisible(false);
            m_plot->Fit();
            m_plot->Refresh();
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
        m_vectorLayerImag->SetVisible(false);
    }
    m_plot->Fit();
    m_plot->Refresh();
}

bool PlotWindow::IsIqMode(const sData::sParam* param, const sData& data) const
{
    if (param)
    {
        wxString measurementType = param->MeasurementType;
        measurementType.MakeLower();
        if (!measurementType.IsEmpty())
            return measurementType.Find("iq") != wxNOT_FOUND;
    }

    if (data.getFsuSettings().mode == MeasurementMode::IQ)
        return true;

    if (!param)
        return false;

    // Fallback for legacy files that have no explicit measurement mode field.
    return (param->recordLength > 0 && param->sampleRate > 0.0);
}

bool PlotWindow::ApplySelectionToPlot(int xIndex, int yIndex, bool logSelection)
{
    if (!m_document)
    {
        wxLogWarning("PlotWindow: no document attached");
        return false;
    }

    sData& data = m_document->GetResultsMutable();
    sData::sParam* param = data.GetParameter();
    if (!param)
    {
        wxLogWarning("PlotWindow: no measurement header available");
        return false;
    }

    if (xIndex < 0 || yIndex < 0 || xIndex >= param->NoPoints_X || yIndex >= param->NoPoints_Y)
    {
        wxLogWarning("PlotWindow: selected index [%d ; %d] out of bounds", xIndex + 1, yIndex + 1);
        return false;
    }

    std::vector<double> yReal;
    try
    {
        yReal = data.get3DDataReal(xIndex, yIndex);
    }
    catch (const std::exception& e)
    {
        wxLogWarning("PlotWindow: failed reading real data [%d ; %d]: %s",
                     xIndex + 1, yIndex + 1, e.what());
        return false;
    }

    if (yReal.empty())
    {
        wxLogWarning("PlotWindow: no data for selection [%d ; %d]", xIndex + 1, yIndex + 1);
        return false;
    }

    const bool iqMode = IsIqMode(param, data);
    std::vector<double> xAxis = iqMode ? data.GetTimeIQStepVector() : data.GetFreqStepVector();
    if (xAxis.empty())
    {
        wxLogWarning("PlotWindow: failed to build x-axis for selection [%d ; %d]", xIndex + 1, yIndex + 1);
        return false;
    }

    size_t n = std::min(xAxis.size(), yReal.size());
    if (n == 0)
    {
        wxLogWarning("PlotWindow: invalid data length for selection [%d ; %d]", xIndex + 1, yIndex + 1);
        return false;
    }

    xAxis.resize(n);
    yReal.resize(n);
    m_vectorLayer->SetData(xAxis, yReal);

    if (iqMode)
    {
        std::vector<double> yImag;
        try
        {
            yImag = data.get3DDataImag(xIndex, yIndex);
        }
        catch (const std::exception& e)
        {
            wxLogWarning("PlotWindow: failed reading imag data [%d ; %d]: %s",
                         xIndex + 1, yIndex + 1, e.what());
            yImag.clear();
        }

        size_t ni = std::min(n, yImag.size());
        if (ni > 0)
        {
            std::vector<double> xImag(xAxis.begin(), xAxis.begin() + static_cast<long>(ni));
            yImag.resize(ni);
            m_vectorLayerImag->SetData(xImag, yImag);
            m_vectorLayerImag->SetVisible(true);
        }
        else
        {
            m_vectorLayerImag->SetVisible(false);
            wxLogWarning("PlotWindow: IQ imag data missing for selection [%d ; %d]", xIndex + 1, yIndex + 1);
        }
    }
    else
    {
        m_vectorLayerImag->SetVisible(false);
    }

    m_document->SetXData(xAxis);
    m_document->SetYData(yReal);

    const auto [xMinIt, xMaxIt] = std::minmax_element(xAxis.begin(), xAxis.end());
    double xMin = *xMinIt;
    double xMax = *xMaxIt;

    double yMin = 0.0;
    double yMax = 0.0;
    bool yRangeInitialized = false;

    auto extendYRange = [&](const std::vector<double>& values)
    {
        if (values.empty())
            return;

        const auto [minIt, maxIt] = std::minmax_element(values.begin(), values.end());
        if (!yRangeInitialized)
        {
            yMin = *minIt;
            yMax = *maxIt;
            yRangeInitialized = true;
            return;
        }

        yMin = std::min(yMin, *minIt);
        yMax = std::max(yMax, *maxIt);
    };

    extendYRange(yReal);
    if (iqMode && m_vectorLayerImag->IsVisible())
    {
        std::vector<double> yImagForRange = data.get3DDataImag(xIndex, yIndex);
        if (!yImagForRange.empty())
            yImagForRange.resize(std::min(xAxis.size(), yImagForRange.size()));
        extendYRange(yImagForRange);
    }

    // Avoid zero-size view box if all values are equal.
    if (xMin == xMax)
    {
        const double margin = (xMin == 0.0) ? 1.0 : std::abs(xMin) * 0.05;
        xMin -= margin;
        xMax += margin;
    }
    if (!yRangeInitialized)
    {
        yMin = -1.0;
        yMax = 1.0;
    }
    else if (yMin == yMax)
    {
        const double margin = (yMin == 0.0) ? 1.0 : std::abs(yMin) * 0.05;
        yMin -= margin;
        yMax += margin;
    }

    m_plot->Fit(xMin, xMax, yMin, yMax);
    m_plot->Refresh();

    if (logSelection)
    {
        wxLogMessage("PlotWindow: selected measurement [%d ; %d]", xIndex + 1, yIndex + 1);
    }

    if (m_selectedMeasurementText)
        m_selectedMeasurementText->SetLabel(wxString::Format("Selected [x ; y]: [%d ; %d]", xIndex + 1, yIndex + 1));

    return true;
}

void PlotWindow::OnOpenLoadedMeasurementSettings(wxCommandEvent& /*event*/)
{
    if (!m_document)
    {
        wxLogWarning("PlotWindow: no document attached");
        return;
    }

    sData& data = m_document->GetResultsMutable();
    sData::sParam* param = data.GetParameter();
    if (!param)
    {
        wxLogWarning("PlotWindow: no measurement loaded");
        return;
    }

    MeasurementMode mode = MeasurementMode::SWEEP;
    wxString measurementType = param->MeasurementType;
    measurementType.MakeLower();

    if (measurementType.Find("iq") != wxNOT_FOUND ||
        (param->recordLength > 0 && param->sampleRate > 0.0 && param->ifBandwidth > 0.0))
    {
        mode = MeasurementMode::IQ;
    }
    else if (measurementType.Find("marker") != wxNOT_FOUND || measurementType.Find("peak") != wxNOT_FOUND)
    {
        mode = MeasurementMode::MARKER_PEAK;
    }

    SettingsDialog dlg(this, mode, param);
    dlg.ShowModal();
}

void PlotWindow::OnSelectMeasurement(wxCommandEvent& /*event*/)
{
    int xi = m_choiceXSelector->GetSelection();
    int yi = m_choiceYSelector->GetSelection();

    if (xi == wxNOT_FOUND || yi == wxNOT_FOUND)
        return;

    if (!ApplySelectionToPlot(xi, yi, true))
    {
        wxLogWarning("PlotWindow: failed to show selected measurement");
    }
}

void PlotWindow::PopulateSelectors(unsigned int nX, unsigned int nY)
{
    int prevX = m_choiceXSelector->GetSelection();
    int prevY = m_choiceYSelector->GetSelection();

    m_choiceXSelector->Clear();
    for (unsigned int i = 1; i <= nX; ++i)
        m_choiceXSelector->Append(wxString::Format("%u", i));
    if (nX > 0)
        m_choiceXSelector->SetSelection((prevX >= 0 && static_cast<unsigned int>(prevX) < nX) ? prevX : 0);

    m_choiceYSelector->Clear();
    for (unsigned int i = 1; i <= nY; ++i)
        m_choiceYSelector->Append(wxString::Format("%u", i));
    if (nY > 0)
        m_choiceYSelector->SetSelection((prevY >= 0 && static_cast<unsigned int>(prevY) < nY) ? prevY : 0);
}

// -----------------------------------------------------------------------
// UpdateSettingsPanel — shows current fsuMeasurement device settings
// -----------------------------------------------------------------------
void PlotWindow::UpdateSettingsPanel()
{
    const fsuMeasurement::FsuSettings* settings = nullptr;
    const sData::sParam* param = nullptr;

    if (m_document)
    {
        sData& results = m_document->GetResultsMutable();
        param = results.GetParameter();
        if (param && !param->MeasurementType.IsEmpty())
        {
            settings = &results.getFsuSettings();
        }
    }

    fsuMeasurement* fsu = &fsuMeasurement::get_instance();
    wxString info;

    auto mode = settings ? settings->mode : fsu->getMeasurementMode();

    switch (mode) {
        case MeasurementMode::SWEEP: {
            auto s = settings ? settings->sweep : fsu->returnSweepSettings();
            info += "Mode:\t\tSweep\n";
            info += wxString::Format("Start Freq:\t%g Hz\n", s.startFreq);
            info += wxString::Format("Stop Freq:\t%g Hz\n", s.stopFreq);
            info += wxString::Format("Ref Level:\t%g dBm\n", s.refLevel);
            info += wxString::Format("Attenuation:\t%d dB\n", s.att);
            info += wxString::Format("Unit:\t\t%s\n",       s.unit.c_str());
            info += wxString::Format("RBW:\t\t%d Hz\n",     s.rbw);
            info += wxString::Format("VBW:\t\t%d Hz\n",   s.vbw);
            info += wxString::Format("Points:\t\t%d\n",     s.points);
            info += wxString::Format("Detector:\t%s",     s.detector.c_str());
            break;
        }
        case MeasurementMode::IQ: {
            auto s = settings ? settings->iq : fsu->returnIqSettings();
            info += "Mode:\t\tIQ\n";
            info += wxString::Format("Center Freq:\t%g Hz\n", s.centerFreq);
            info += wxString::Format("Ref Level:\t%g dBm\n", s.refLevel);
            info += wxString::Format("Attenuation:\t%d dB\n", s.att);
            info += wxString::Format("Unit:\t\t%s\n", s.unit.c_str());
            info += wxString::Format("Sample Rate:\t%g Hz\n", s.sampleRate);
            info += wxString::Format("Record Len:\t%d\n", s.recordLength);
            info += wxString::Format("IF Bandwidth:\t%g Hz\n", s.ifBandwidth);
            info += wxString::Format("Trigger Src:\t%s\n", s.triggerSource.c_str());
            info += wxString::Format("Trigger Lvl:\t%g dBm\n", s.triggerLevel);
            info += wxString::Format("Trigger Dly:\t%g s", s.triggerDelay);
            break;
        }
        case MeasurementMode::MARKER_PEAK: {
            auto s = settings ? settings->marker : fsu->returnMarkerPeakSettings();
            info += "Mode:\t\tMarker Peak\n";
            info += wxString::Format("Start Freq:\t%g Hz\n", s.startFreq);
            info += wxString::Format("Stop Freq:\t%g Hz\n", s.stopFreq);
            info += wxString::Format("Ref Level:\t%g dBm\n", s.refLevel);
            info += wxString::Format("Attenuation:\t%d dB\n", s.att);
            info += wxString::Format("Unit:\t\t%s\n", s.unit.c_str());
            info += wxString::Format("RBW:\t\t%d Hz\n", s.rbw);
            info += wxString::Format("VBW:\t\t%d Hz\n", s.vbw);
            info += wxString::Format("Detector:\t%s", s.detector.c_str());
            break;
        }
        case MeasurementMode::COSTUM: {
            wxString fileName = settings ? settings->costumFile : wxString::FromUTF8(fsu->getFileName());
            info += "Mode:\t\tCostum\n";
            info += wxString::Format("Script:\t\t%s", fileName);
            break;
        }
        default:
            info = "--- No mode set ---";
            break;
    }

    m_settingsText->SetLabel(info);
    m_settingsPanel->Layout();
}

// -----------------------------------------------------------------------
// File menu handlers (local to this PlotWindow)
// -----------------------------------------------------------------------
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
    info += wxString::Format("File Name:\t\t%s\n",      param->File);
    wxString measurementType = param->MeasurementType;
    if (measurementType.IsEmpty())
        measurementType = "---";
    info += wxString::Format("Measurement Type:\t%s\n", measurementType);
    info += wxString::Format("Date:\t\t\t%s\n",         param->Date);
    info += wxString::Format("Time:\t\t\t%s\n",         param->Time);
    info += wxString::Format("X Points:\t\t\t%u\n",       param->NoPoints_X);
    info += wxString::Format("Y Points:\t\t\t%u\n",       param->NoPoints_Y);

    m_infoText->SetLabel(info);
    m_infoPanel->Layout();
}

void PlotWindow::OnMenuFileOpen(wxCommandEvent& event)
{
    try
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

        if (!LoadImportedData(importedData, filePath))
        {
            wxMessageBox("Imported file does not contain plottable measurement data.",
                        "Import Error", wxOK | wxICON_ERROR, this);
        }
    }
    catch (const std::exception& e)
    {
        wxLogError("PlotWindow import failed: %s", e.what());
        wxMessageBox(wxString::Format("Import failed:\n%s", e.what()),
                    "Import Error", wxOK | wxICON_ERROR, this);
    }
    catch (...)
    {
        wxLogError("PlotWindow import failed: unknown exception");
        wxMessageBox("Import failed with an unknown error.",
                    "Import Error", wxOK | wxICON_ERROR, this);
    }
}

bool PlotWindow::LoadImportedData(const sData& importedData, const wxString& sourcePath)
{
    if (!m_document)
    {
        MeasurementDocument* measDoc = new MeasurementDocument(
            PrologixUsbGpibAdapter::get_instance(), fsuMeasurement::get_instance());
        SetDocument(measDoc);
        SetOwnsDocument(true);
        wxLogMessage("PlotWindow: created local document for imported CSV");
    }

    if (!m_document)
        return false;

    m_document->GetResultsMutable() = importedData;

    sData::sParam* docParam = m_document->GetResultsMutable().GetParameter();
    if (!docParam)
    {
        wxLogWarning("PlotWindow: imported data has no header");
        return false;
    }

    UpdateInfoPanel(docParam);
    PopulateSelectors(static_cast<unsigned int>(std::max(0, docParam->NoPoints_X)),
                      static_cast<unsigned int>(std::max(0, docParam->NoPoints_Y)));

    int xi = m_choiceXSelector->GetSelection();
    int yi = m_choiceYSelector->GetSelection();
    if (!ApplySelectionToPlot(xi, yi, false))
    {
        wxLogWarning("PlotWindow: failed to render imported matrix selection");
        return false;
    }

    UpdateSettingsPanel();

    if (!sourcePath.IsEmpty())
    {
        SetTitle(wxString::Format("Measurement Window %d - %s",
                 m_windowId, sourcePath.AfterLast('\\').AfterLast('/')));

        std::cout << "[PlotWindow " << m_windowId << "] Imported CSV: "
                  << sourcePath << std::endl;
    }

    return true;
}

bool PlotWindow::ShowMatrixPoint(int xIndex, int yIndex, bool logSelection)
{
    if (m_choiceXSelector)
        m_choiceXSelector->SetSelection(xIndex);
    if (m_choiceYSelector)
        m_choiceYSelector->SetSelection(yIndex);

    return ApplySelectionToPlot(xIndex, yIndex, logSelection);
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


// Measurement menu handlers (forward to parent MainProgrammWin)

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

void PlotWindow::OnMenuMesurementSweep(wxCommandEvent& event)
{
    MainProgrammWin* parent = dynamic_cast<MainProgrammWin*>(GetParent());
    if (parent)
        parent->MenuMesurementSweep(event);
}

void PlotWindow::OnMenuMesurementIQ(wxCommandEvent& event)
{
    MainProgrammWin* parent = dynamic_cast<MainProgrammWin*>(GetParent());
    if (parent)
        parent->MenuMesurementIQ(event);
}

void PlotWindow::OnMenuMesurementMarker(wxCommandEvent& event)
{
    MainProgrammWin* parent = dynamic_cast<MainProgrammWin*>(GetParent());
    if (parent)
        parent->MenuMesurementMarkerPeak(event);
}

void PlotWindow::OnMenuMesurementCustom(wxCommandEvent& event)
{
    MainProgrammWin* parent = dynamic_cast<MainProgrammWin*>(GetParent());
    if (parent)
        parent->MenuMesurementCustom(event);
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

