#include "CsvManagement.h"
#include "systemInfo.h"

#include <algorithm>


// Read Csv functions
//------sData3D Ende------

CsvFile::CsvFile(char separator)
{
    m_separator = separator;
}

CsvFile::~CsvFile() = default;

// save Csv Functions

bool CsvFile::saveCsvFile(wxString& filename, sData& data, int mesurementNumb)
{
    std::vector<double> real;
    std::vector<double> imag;

    real = data.getRealArray();
    imag = data.getImagArray();

    std::cout << "saveCsvFile()" << std::endl;

    // filename.Append(timestamp);
    if (!filename.Lower().EndsWith(".csv"))
    {
        filename.Append(".csv");
    }

    wxTextFile file(filename.ToStdString());
    // check if file Exists
    if (mesurementNumb <= 1)
    {
        if (!file.Exists())
        {
            file.Create();
        }
        else
        {
            file.Clear();
            file.Write();
        }
    }

    if (!file.Open())
    {
        std::cout << kErrPrefixStr.CsvSave <<"Failed to open file" << std::endl;
        return false;
    }

    const bool cont = (data.GetType() != "Line");

    if (mesurementNumb == 1 || mesurementNumb == 0) // weiteren check hinzufügen
    {
        // Import current fsu settings once before writing

        // create array
        //data.resize3DData(data.getNumberOfPts_X(),data.getNumberOfPts_Y(),data.getNumberOfPts_Array());

        // Write header
        std::cout << "save header" << std::endl;
        if(!saveCsvHeader(file, data))
        {
            std::cout << kErrPrefixStr.CsvSave <<"Failed to write header" << std::endl;
            return false;
        }
        std::cout << "write matrix" << std::endl;
        // Write indexes
        if (!writeMatrixIndexCsv(file, data, cont))
        {
            std::cout << kErrPrefixStr.CsvSave <<"Failed to write indexes" << std::endl;
            return false;
        }
        // create lookup table
        std::cout << "create lookup" << std::endl;
        if (!createCsvLookupTable(file))
        {
            std::cout << kErrPrefixStr.CsvSave <<"Failed to create lookup table" << std::endl;
            return false;
        }
    }


    // save whole File at once
    if (mesurementNumb == 0)
    {
        size_t count = data.getTotalNumberOfPts();

        for (size_t i = 1; i <= count; i++)
        {
            if (!saveCsvData(file, data, i, cont))
            {
                std::cout << kErrPrefixStr.CsvSave <<"Failed to save measurement " << i << std::endl;
                return false;
            }
        }

        file.Write();
        file.Close();
        return true;
    }

    // save one measurement
    if (!saveCsvData(file, data, mesurementNumb, cont))
    {
        std::cout << kErrPrefixStr.CsvSave <<"Failed to save measurement " << mesurementNumb << std::endl;
        return false;
    }


    file.Write();
    file.Close();

    std::cout << "File saved" << std::endl;

    return true;
}

bool CsvFile::saveCsvHeader(wxTextFile &file, sData& data)
{
    sData::sParam* dsParam = data.GetParameter();

    //file.AddLine("Header Information"); // Leerzeile
    file.AddLine(wxString::FromUTF8(HeaderInfo::fileName.data()) + m_separator + dsParam->File);
    file.AddLine(wxString::FromUTF8(HeaderInfo::date.data()) + m_separator + dsParam->Date);
    file.AddLine(wxString::FromUTF8(HeaderInfo::time.data()) + m_separator + dsParam->Time);
    file.AddLine(wxString::FromUTF8(HeaderInfo::type.data()) + m_separator + dsParam->Type);
    // Plotter info
    file.AddLine(wxString::FromUTF8(HeaderConfig::noPointsX.data()) + m_separator + wxString::Format("%d", dsParam->NoPoints_X));
    file.AddLine(wxString::FromUTF8(HeaderConfig::noPointsY.data()) + m_separator + wxString::Format("%d", dsParam->NoPoints_Y));
    if (dsParam->hasPlotterData && (dsParam->NoPoints_X * dsParam->NoPoints_Y) > 1 && !savePlotterData(file, data))
    {
        std::cout << kErrPrefixStr.CsvSave <<"Failed to write plotter data" << std::endl;
        return false;
    }
    // Measurement config
    //file.AddLine(wxString::Format("%s%c%d",   HeaderConfig::noPointsArray.data(), m_separator, dsParam->NoPoints_Array));

    file.AddLine(""); // Leerzeile

    // Messeinstellungen je nach Modus (from cached settings)
    MeasurementMode mode = data.getFsuSettings().mode;

    switch (mode)
    {
        case MeasurementMode::SWEEP:
            file.AddLine(wxString::FromUTF8(HeaderConfig::mesSettings.data()) + m_separator + "Sweep");
            saveCsvSettingsSweep(file, data);
            break;
        case MeasurementMode::IQ:
            file.AddLine(wxString::FromUTF8(HeaderConfig::mesSettings.data()) + m_separator + "IQ");
            saveCsvSettingsQI(file, data);
            break;
        case MeasurementMode::MARKER_PEAK:
            file.AddLine(wxString::FromUTF8(HeaderConfig::mesSettings.data()) + m_separator + "Marker Peak");
            saveCsvSettingsMarker(file, data);
            break;
        case MeasurementMode::COSTUM:
            file.AddLine(wxString::FromUTF8(HeaderConfig::mesSettings.data()) + m_separator + "Costum");
            file.AddLine(wxString::FromUTF8(HeaderConfig::customFile.data()) + m_separator + data.getFsuSettings().costumFile);
            break;
    }
    file.AddLine(""); // Leerzeile

    wxString lineLabel;
    std::vector<double> lineDataVector;
    std::vector<double> timeAxis;
    int IndexAmount;
    double samplerate;
    int recordLength;
    double t_sample;
    std::string timeUnit;

    switch (mode)
    {
        case MeasurementMode::SWEEP:
            // Frequenz-Zeile
            lineLabel = wxString::Format("f in %s", dsParam->ampUnit);

            IndexAmount = dsParam->NoPoints_Array;

            lineDataVector = data.GetFreqStepVector();

            for (int i = 0; i < dsParam->NoPoints_Array; i++)
            {
                lineLabel << m_separator << lineDataVector[i];
            }
            file.AddLine(lineLabel);

            break;
        case MeasurementMode::IQ:
            // time scale
            samplerate = dsParam->sampleRate;
            recordLength = dsParam->recordLength;
            IndexAmount = recordLength;
            t_sample = 1 / samplerate;

            if (t_sample < 0.000'000'1)
            {
                t_sample = t_sample * 1'000'000'000;
                timeUnit = "ns";
            }
            if (t_sample < 0.000'1)
            {
                t_sample = t_sample * 1'000'000;
                timeUnit = "us";
            }
            else if (t_sample < 0.1)
            {
                t_sample = t_sample * 1'000;
                timeUnit = "ms";
            }else
            if (t_sample < 1)
            {
                timeUnit = "s";
            }

            lineLabel = wxString::Format("t in %s", timeUnit);

            lineDataVector = data.GetTimeIQStepVector();

            for (int i = 0; i < recordLength; i++)
            {
                lineLabel << m_separator << lineDataVector[i];
            }
            file.AddLine(lineLabel);

            break;

        case MeasurementMode::MARKER_PEAK:

            IndexAmount = dsParam->NoPoints_Array;
            break;
    }


    // ID-Zeile zusammenbauen
    wxString lineID = "ID";
    for (int i = 1; i < IndexAmount + 1; i++)
    {
        lineID << m_separator << (int)i;
    }
    file.AddLine(lineID);

    return true;
}

bool CsvFile::savePlotterData(wxTextFile& file, sData& data)
{
    sData::sParam* dsParam = data.GetParameter();

    file.AddLine(wxString::FromUTF8(HeaderConfig::stepX.data()) + m_separator + wxString::FromDouble(dsParam->xSpacingmm));
    file.AddLine(wxString::FromUTF8(HeaderConfig::stepY.data()) + m_separator + wxString::FromDouble(dsParam->ySpacingmm));
    file.AddLine(wxString::FromUTF8(HeaderConfig::startX.data()) + m_separator + wxString::FromDouble(dsParam->xStartingPointmm));
    file.AddLine(wxString::FromUTF8(HeaderConfig::startY.data()) + m_separator + wxString::FromDouble(dsParam->yStartingPointmm));
    file.AddLine(wxString::FromUTF8(HeaderConfig::isVertical.data()) + m_separator + (dsParam->isVertical ? "1" : "0"));

    return true;
}

bool CsvFile::saveCsvSettingsSweep(wxTextFile& file, sData& data)
{
    const auto& s = data.getFsuSettings().sweep;

    file.AddLine(wxString::FromUTF8(HeaderConfig::startFreq.data()) + m_separator + wxString::FromDouble( s.startFreq) + " " + s.unit);
    file.AddLine(wxString::FromUTF8(HeaderConfig::endFreq.data()) + m_separator + wxString::FromDouble(s.stopFreq) + " " + s.unit);
    file.AddLine(wxString::FromUTF8(HeaderConfig::refPegel.data()) + m_separator + wxString::FromDouble(s.refLevel));
    file.AddLine(wxString::FromUTF8(HeaderConfig::HFDaempfung.data()) + m_separator + wxString::Format("%i", s.att));
    file.AddLine(wxString::FromUTF8(HeaderConfig::ampUnit.data()) + m_separator + s.unit);
    file.AddLine(wxString::FromUTF8(HeaderConfig::RBW.data()) + m_separator + wxString::Format("%i", s.rbw));
    file.AddLine(wxString::FromUTF8(HeaderConfig::VBW.data()) + m_separator + wxString::Format("%i", s.vbw));
    file.AddLine(wxString::FromUTF8(HeaderConfig::noPointsArray.data()) + m_separator + wxString::Format("%i", s.points));
    file.AddLine(wxString::FromUTF8(HeaderConfig::sweepTime.data()) + m_separator + s.sweepTime);
    file.AddLine(wxString::FromUTF8(HeaderConfig::detektor.data()) + m_separator + s.detector);

    return true;
}

bool CsvFile::saveCsvSettingsQI(wxTextFile& file, sData& data)
{
    const auto& s = data.getFsuSettings().iq;

    file.AddLine(wxString::FromUTF8(HeaderConfig::centerFreq.data()) + m_separator + wxString::FromDouble(s.centerFreq) + " " + s.unit);
    file.AddLine(wxString::FromUTF8(HeaderConfig::refPegel.data()) + m_separator + wxString::FromDouble(s.refLevel));
    file.AddLine(wxString::FromUTF8(HeaderConfig::HFDaempfung.data()) + m_separator + wxString::Format("%i", s.att));
    file.AddLine(wxString::FromUTF8(HeaderConfig::ampUnit.data()) + m_separator + s.unit);
    file.AddLine(wxString::FromUTF8(HeaderConfig::sampleRate.data()) + m_separator + wxString::FromDouble(s.sampleRate));
    file.AddLine(wxString::FromUTF8(HeaderConfig::recordLength.data()) + m_separator + wxString::Format("%i", s.recordLength));
    file.AddLine(wxString::FromUTF8(HeaderConfig::ifBandwidth.data()) + m_separator + wxString::FromDouble(s.ifBandwidth));
    file.AddLine(wxString::FromUTF8(HeaderConfig::triggerSource.data()) + m_separator + s.triggerSource);
    file.AddLine(wxString::FromUTF8(HeaderConfig::triggerLevel.data()) + m_separator + wxString::FromDouble(s.triggerLevel));
    file.AddLine(wxString::FromUTF8(HeaderConfig::triggerDelay.data()) + m_separator + wxString::FromDouble(s.triggerDelay));

    return true;
}

bool CsvFile::saveCsvSettingsMarker(wxTextFile& file, sData& data)
{
    const auto& s = data.getFsuSettings().marker;

    file.AddLine(wxString::FromUTF8(HeaderConfig::startFreq.data()) + m_separator + wxString::FromDouble(s.startFreq) + " " + s.unit);
    file.AddLine(wxString::FromUTF8(HeaderConfig::endFreq.data()) + m_separator + wxString::FromDouble(s.stopFreq) + " " + s.unit);
    file.AddLine(wxString::FromUTF8(HeaderConfig::refPegel.data()) + m_separator + wxString::FromDouble(s.refLevel));
    file.AddLine(wxString::FromUTF8(HeaderConfig::HFDaempfung.data()) + m_separator + wxString::Format("%i", s.att));
    file.AddLine(wxString::FromUTF8(HeaderConfig::ampUnit.data()) + m_separator + s.unit);
    file.AddLine(wxString::FromUTF8(HeaderConfig::RBW.data()) + m_separator + wxString::Format("%i", s.rbw));
    file.AddLine(wxString::FromUTF8(HeaderConfig::VBW.data()) + m_separator + wxString::Format("%i", s.vbw));
    file.AddLine(wxString::FromUTF8(HeaderConfig::detektor.data()) + m_separator + s.detector);

    return true;
}

bool CsvFile::saveCsvData(wxTextFile& file, sData data, int mesurementNumb, bool cont)
{
    if (!file.IsOpened())
    {
        std::cerr << kErrPrefixStr.CsvSave <<"File not opened" << std::endl;
        return false;
    }


    int xPoints = data.getNumberOfPts_X();
    int yPoints = data.getNumberOfPts_Y();

    // find line with current
    bool isMarker = (data.getFsuSettings().mode == MeasurementMode::MARKER_PEAK);
    const char* label1 = isMarker ? "Freq" : "Real";

    wxString indexText = getIndexNumbers(xPoints, yPoints, mesurementNumb, cont) + " " + label1;

    int lineNumber = findLineCsv(file, indexText);
    if (lineNumber < 0 || static_cast<size_t>(lineNumber + 1) >= file.GetLineCount())
    {
        std::cerr << kErrPrefixStr.CsvSave <<"Invalid data line for measurement " << mesurementNumb << std::endl;
        return false;
    }
    // std::cout << "line Found: " << lineNumber << std::endl;

    int xPosition;
    int yPosition;

    data.getXYCord(xPosition, yPosition, mesurementNumb);

    std::vector<double> real = data.get3DDataReal(xPosition, yPosition);
    std::vector<double> imag = data.get3DDataImag(xPosition, yPosition);

    int count = data.getNumberOfPts_Array();
    // Guard against stale NoPoints_Array values (e.g. mode switches) and avoid out-of-range access.
    count = std::min(count, static_cast<int>(std::min(real.size(), imag.size())));

    std::cout << "Trying to save data: X: " << real.size() << "Y: " << imag.size() << std::endl;

    int j = 0;
    for (int i = 0; i < count; i++)
    {
        file.GetLine(lineNumber) << m_separator << real[i];

        file.GetLine(lineNumber + 1) << m_separator << imag[i];
        j++;
    }

    std::cout << "saved " << j << " data points!";

    return true;
}

bool CsvFile::readCsvFile(wxString filename, sData& data)
{
    std::cerr << "read CSV" << std::endl;

    wxTextFile file(filename.ToStdString());

    if (!file.Open())
    {
        std::cout << kErrPrefixStr.CsvRead << "Failed to open file" << std::endl;
        file.Close();
        return false;
    }

    // Keep separator consistent for all following parsing/tokenizing stages.
    m_separator = detectSeparator(file);


    if (!createCsvLookupTable(file))
    {
        std::cout << kErrPrefixStr.CsvRead << "Failed to create lookup table" << std::endl;
        file.Close();
        return false;
    }

    if (!readCsvHeader(file, data))
    {
        std::cout << kErrPrefixStr.CsvRead <<"Failed to read header" << std::endl;
        file.Close();
        return false;
    }

    if (!readCsvData(file, data))
    {
        std::cout << kErrPrefixStr.CsvRead <<"Failed to read data" << std::endl;
        file.Close();
        return false;
    }

    std::cout << kErrPrefixStr.CsvRead <<"Finised file read" << std::endl;


    file.Close();
    return true;
}

bool CsvFile::readCsvHeader(wxTextFile&file, sData& data)
{
    if (!file.IsOpened())
    {
        std::cout << kErrPrefixStr.CsvRead <<"File not opened" << std::endl;
        return false;
    }

    sData::sParam* dsParam = data.GetParameter();

    size_t lineCount = file.GetLineCount();
    // Minimale Zeilenanzahl prüfen
    if (lineCount < 9) {
        std::cerr << kErrPrefixStr.CsvRead <<"File too short, insufficient header" << std::endl;
        return false;
    }

    // Use the cached separator from readCsvFile for deterministic parsing.
    char separator = m_separator;

    auto readByLabel = [&](std::string_view label, wxString& outVal) -> bool {
        int line = findLineCsv(file, wxString::FromUTF8(label.data()));
        if (line < 0)
        {
            return false;
        }
        outVal = file.GetLine(line).AfterFirst(separator).Trim(false).Trim();
        return true;
    };

    wxString value;
    if (!readByLabel(HeaderInfo::fileName, value)) return false;
    dsParam->File = value;
    if (!readByLabel(HeaderInfo::date, value)) return false;
    dsParam->Date = value;
    if (!readByLabel(HeaderInfo::time, value)) return false;
    dsParam->Time = value;
    if (!readByLabel(HeaderInfo::type, value)) return false;
    dsParam->Type = value;

    long lVal;
    if (!readByLabel(HeaderConfig::noPointsX, value) || !value.ToLong(&lVal)) return false;
    dsParam->NoPoints_X = lVal;
    if (!readByLabel(HeaderConfig::noPointsY, value) || !value.ToLong(&lVal)) return false;
    dsParam->NoPoints_Y = lVal;

    if (!readPlotterData(file, data))
    {
        std::cout << kErrPrefixStr.CsvRead <<"Failed to read plotter data" << std::endl;
    }

    // Messeinstellungen je nach Modus einlesen
    int mesSettingsLine = findLineCsv(file, wxString(HeaderConfig::mesSettings.data()));
    if (mesSettingsLine >= 0)
    {
        wxString modeName = file.GetLine(mesSettingsLine).AfterFirst(separator).Trim(false).Trim();
        dsParam->MeasurementType = modeName;
        std::cout << "Mode: " << modeName << std::endl;


        if (modeName == "Sweep")
            readCsvSettingsSweep(file, data);
        else if (modeName == "IQ")
            readCsvSettingsQI(file, data);
        else if (modeName == "Marker Peak")
            readCsvSettingsMarker(file, data);
        else if (modeName == "Costum")
            readCsvSettingsCostum(file, data);

    }

    std::cout << kErrPrefixStr.CsvRead <<"Read Header" << std::endl;

    // Resize Datastorage array for data
    data.resize3DData(dsParam->NoPoints_X, dsParam->NoPoints_Y, dsParam->NoPoints_Array);

    std::cout << dsParam->NoPoints_X << " x " << dsParam->NoPoints_Y << " x " << dsParam->NoPoints_Array << std::endl;

    return true;
}

bool CsvFile::readPlotterData(wxTextFile& file, sData& data)
{
    char separator = m_separator;

    auto readLine = [&](std::string_view label, wxString& out) -> bool {
        int line = findLineCsv(file, wxString::FromUTF8(label.data()));
        if (line < 0)
        {
            return false;
        }
        out = file.GetLine(line).AfterFirst(separator).Trim(false).Trim();
        return true;
    };

    wxString stepXText;
    wxString stepYText;
    wxString startXText;
    wxString startYText;
    wxString verticalText;

    // Optional block: if first line does not exist, simply skip.
    if (!readLine(HeaderConfig::stepX, stepXText)) return true;
    if (!readLine(HeaderConfig::stepY, stepYText)) return true;
    if (!readLine(HeaderConfig::startX, startXText)) return true;
    if (!readLine(HeaderConfig::startY, startYText)) return true;
    if (!readLine(HeaderConfig::isVertical, verticalText)) return true;

    double stepX = 0.0;
    double stepY = 0.0;
    double startX = 0.0;
    double startY = 0.0;
    long isVerticalLong = 0;

    if (!stepXText.ToDouble(&stepX)) return true;
    if (!stepYText.ToDouble(&stepY)) return true;
    if (!startXText.ToDouble(&startX)) return true;
    if (!startYText.ToDouble(&startY)) return true;

    bool isVertical = false;
    if (verticalText.CmpNoCase("true") == 0)
    {
        isVertical = true;
    }
    else if (verticalText.CmpNoCase("false") == 0)
    {
        isVertical = false;
    }
    else if (verticalText.ToLong(&isVerticalLong))
    {
        isVertical = (isVerticalLong != 0);
    }
    else
    {
        return true;
    }

    data.setPlotterPositions(stepX, stepY, startX, startY, isVertical);

    return true;
}

bool CsvFile::readCsvSettingsSweep(wxTextFile& file, sData& data)
{
    sData::sParam* dsParam = data.GetParameter();
    auto& settings = data.getMutableFsuSettings();
    dsParam->MeasurementType = "Sweep";
    settings.mode = MeasurementMode::SWEEP;
    char separator = m_separator;

    auto readLine = [&](std::string_view label) -> wxString {
        int line = findLineCsv(file, wxString(label.data()));
        if (line < 0) return "";
        return file.GetLine(line).AfterFirst(separator).Trim(false).Trim();
    };

    // Start/Stop Frequenz (Wert + Einheit)
    wxString startVal = readLine(HeaderConfig::startFreq).BeforeFirst(' ');
    wxString endVal   = readLine(HeaderConfig::endFreq).BeforeFirst(' ');

    long freqVal;
    if (startVal.ToLong(&freqVal))  dsParam->startFreq = freqVal;
    if (endVal.ToLong(&freqVal))    dsParam->endFreq   = freqVal;
    settings.sweep.startFreq = dsParam->startFreq;
    settings.sweep.stopFreq = dsParam->endFreq;

    long lVal;
    if (readLine(HeaderConfig::refPegel).ToLong(&lVal))
        dsParam->refPegel = static_cast<int>(lVal);
    settings.sweep.refLevel = dsParam->refPegel;

    if (readLine(HeaderConfig::HFDaempfung).ToLong(&lVal))
        dsParam->HFDaempfung = lVal;
    settings.sweep.att = dsParam->HFDaempfung;

    dsParam->ampUnit = readLine(HeaderConfig::ampUnit);
    settings.sweep.unit = dsParam->ampUnit;

    if (readLine(HeaderConfig::RBW).ToLong(&lVal))
        dsParam->RBW = lVal;
    if (readLine(HeaderConfig::VBW).ToLong(&lVal))
        dsParam->VBW = lVal;
    settings.sweep.rbw = dsParam->RBW;
    settings.sweep.vbw = dsParam->VBW;

    int iVal;
    if (readLine(HeaderConfig::noPointsArray).ToInt(&iVal))
        dsParam->NoPoints_Array = iVal;
    settings.sweep.points = dsParam->NoPoints_Array;

    dsParam->sweepTime = readLine(HeaderConfig::sweepTime).ToStdString();
    dsParam->detektor  = readLine(HeaderConfig::detektor).ToStdString();
    settings.sweep.sweepTime = dsParam->sweepTime;
    settings.sweep.detector = dsParam->detektor;

    return true;
}

bool CsvFile::readCsvSettingsQI(wxTextFile& file, sData& data)
{
    sData::sParam* dsParam = data.GetParameter();
    auto& settings = data.getMutableFsuSettings();
    dsParam->MeasurementType = "IQ";
    settings.mode = MeasurementMode::IQ;
    char separator = m_separator;

    auto readLine = [&](std::string_view label) -> wxString {
        int line = findLineCsv(file, wxString(label.data()));
        if (line < 0) return "";
        return file.GetLine(line).AfterFirst(separator).Trim(false).Trim();
    };

    wxString centerVal = readLine(HeaderConfig::centerFreq).BeforeFirst(' ');
    double dVal;
    if (centerVal.ToDouble(&dVal))
        dsParam->centerFreq = dVal;
    settings.iq.centerFreq = dsParam->centerFreq;

    long lVal;
    if (readLine(HeaderConfig::refPegel).ToLong(&lVal))
        dsParam->refPegel = static_cast<int>(lVal);
    settings.iq.refLevel = dsParam->refPegel;

    if (readLine(HeaderConfig::HFDaempfung).ToLong(&lVal))
        dsParam->HFDaempfung = lVal;
    settings.iq.att = dsParam->HFDaempfung;

    dsParam->ampUnit = readLine(HeaderConfig::ampUnit);
    settings.iq.unit = dsParam->ampUnit;

    if (readLine(HeaderConfig::sampleRate).ToDouble(&dVal))
        dsParam->sampleRate = dVal;
    settings.iq.sampleRate = dsParam->sampleRate;

    int iVal;
    if (readLine(HeaderConfig::recordLength).ToInt(&iVal))
        dsParam->recordLength = iVal;
    settings.iq.recordLength = dsParam->recordLength;

    if (readLine(HeaderConfig::ifBandwidth).ToDouble(&dVal))
        dsParam->ifBandwidth = dVal;
    settings.iq.ifBandwidth = dsParam->ifBandwidth;

    dsParam->triggerSource = readLine(HeaderConfig::triggerSource).ToStdString();
    settings.iq.triggerSource = dsParam->triggerSource;

    if (readLine(HeaderConfig::triggerLevel).ToDouble(&dVal))
        dsParam->triggerLevel = dVal;
    settings.iq.triggerLevel = dsParam->triggerLevel;

    if (readLine(HeaderConfig::triggerDelay).ToDouble(&dVal))
        dsParam->triggerDelay = dVal;
    settings.iq.triggerDelay = dsParam->triggerDelay;

    // IQ arrays are sampled in time domain; use record length for per-measurement point count.
    if (dsParam->recordLength > 0)
        dsParam->NoPoints_Array = dsParam->recordLength;

    return true;
}

bool CsvFile::readCsvSettingsMarker(wxTextFile& file, sData& data)
{
    sData::sParam* dsParam = data.GetParameter();
    auto& settings = data.getMutableFsuSettings();
    dsParam->MeasurementType = "Marker Peak";
    settings.mode = MeasurementMode::MARKER_PEAK;
    char separator = m_separator;

    auto readLine = [&](std::string_view label) -> wxString {
        int line = findLineCsv(file, wxString(label.data()));
        if (line < 0) return "";
        return file.GetLine(line).AfterFirst(separator).Trim(false).Trim();
    };

    wxString startVal = readLine(HeaderConfig::startFreq).BeforeFirst(' ');
    wxString endVal   = readLine(HeaderConfig::endFreq).BeforeFirst(' ');

    long freqVal;
    if (startVal.ToLong(&freqVal))  dsParam->startFreq = freqVal;
    if (endVal.ToLong(&freqVal))    dsParam->endFreq   = freqVal;
    settings.marker.startFreq = dsParam->startFreq;
    settings.marker.stopFreq = dsParam->endFreq;

    long lVal;
    if (readLine(HeaderConfig::refPegel).ToLong(&lVal))
        dsParam->refPegel = static_cast<int>(lVal);
    settings.marker.refLevel = dsParam->refPegel;

    if (readLine(HeaderConfig::HFDaempfung).ToLong(&lVal))
        dsParam->HFDaempfung = lVal;
    settings.marker.att = dsParam->HFDaempfung;

    dsParam->ampUnit = readLine(HeaderConfig::ampUnit);
    settings.marker.unit = dsParam->ampUnit;

    double dVal;
    if (readLine(HeaderConfig::RBW).ToDouble(&dVal))
        dsParam->RBW = (unsigned int)dVal;
    if (readLine(HeaderConfig::VBW).ToDouble(&dVal))
        dsParam->VBW = (unsigned int)dVal;
    settings.marker.rbw = dsParam->RBW;
    settings.marker.vbw = dsParam->VBW;

    dsParam->detektor = readLine(HeaderConfig::detektor).ToStdString();
    settings.marker.detector = dsParam->detektor;
    dsParam->NoPoints_Array = 1;

    return true;
}

bool CsvFile::readCsvSettingsCostum(wxTextFile& file, sData& data)
{
        sData::sParam* dsParam = data.GetParameter();
    auto& settings = data.getMutableFsuSettings();
    dsParam->MeasurementType = "Costum";
    settings.mode = MeasurementMode::COSTUM;
    char separator = m_separator;

    auto readLine = [&](std::string_view label) -> wxString {
        int line = findLineCsv(file, wxString(label.data()));
        if (line < 0) return "";
        return file.GetLine(line).AfterFirst(separator).Trim(false).Trim();
    };

    dsParam->costumFile = readLine(HeaderConfig::customFile);
    settings.costumFile = dsParam->costumFile;

    return true;
}

bool CsvFile::readCsvData(wxTextFile& file, sData& data)
{
    int xPoints = data.getNumberOfPts_X();
    int yPoints = data.getNumberOfPts_Y();
    int totalMeasurements = xPoints * yPoints;

    std::cout << "number of Points: " << xPoints << " x " << yPoints << std::endl;


    bool continuous = (data.GetType() != "Line");

    if (!file.IsOpened())
    {
        std::cout << "Failed to open file" << std::endl;
        return false;
    }

    const char separator = m_separator;
    const int expectedPoints = data.getNumberOfPts_Array();

    const std::string firstIdx = getIndexNumbers(xPoints, yPoints, 1, continuous);
    const bool isMarker = (findLineCsv(file, wxString::Format("%s Freq", firstIdx)) >= 0);
    const char* label1 = isMarker ? "Freq" : "Real";
    const char* label2 = isMarker ? "Amp"  : "Imag";

    bool hadErrors = false;

    // Read all available measurements
    for (int mesurementNumb = 1; mesurementNumb <= totalMeasurements; mesurementNumb++)
    {
        std::vector<double> real;
        std::vector<double> imag;

        // ID Für Real und Imag Nummern suchen (Marker: Freq/Amp)
        std::string index = getIndexNumbers(xPoints, yPoints, mesurementNumb, continuous);

        wxString realLabel = wxString(index) + " " + label1;
        wxString imagLabel = wxString(index) + " " + label2;

        // Find Real/Freq data line
        int realLineNum = findLineCsv(file, realLabel);
        if (realLineNum == -1)
        {
            std::cerr << kErrPrefixStr.CsvRead <<"Could not find " << label1 << " data line for measurement " << mesurementNumb << std::endl;
            hadErrors = true;
            continue;
        }

        // Find Imag/Amp data line
        int imagLineNum = findLineCsv(file, imagLabel);
        if (imagLineNum == -1)
        {
            std::cerr << kErrPrefixStr.CsvRead <<"Could not find " << label2 << " data line for measurement " << mesurementNumb << std::endl;
            hadErrors = true;
            continue;
        }

        // Parse Real data
        wxString realLine = file.GetLine(realLineNum);
        wxStringTokenizer realTokenizer(realLine, separator);
        if (realTokenizer.HasMoreTokens()) {
            realTokenizer.GetNextToken(); // skip label
        }
        while (realTokenizer.HasMoreTokens())
        {
            double val;
            if (realTokenizer.GetNextToken().ToDouble(&val))
            {
                real.push_back(val);
            }
        }

        // Parse Imag data
        wxString imagLine = file.GetLine(imagLineNum);
        wxStringTokenizer imagTokenizer(imagLine, separator);
        if (imagTokenizer.HasMoreTokens()) {
            imagTokenizer.GetNextToken(); // skip label
        }
        while (imagTokenizer.HasMoreTokens())
        {
            double val;
            if (imagTokenizer.GetNextToken().ToDouble(&val))
            {
                imag.push_back(val);
            }
        }

        // Validate parsed data
        if (real.size() != imag.size())
        {
            std::cerr << kErrPrefixStr.CsvRead <<"Warning: Real/Imag data arrays have different sizes for measurement " << mesurementNumb << "!" << std::endl;
            hadErrors = true;
            continue;
        }

        if (expectedPoints > 0 && static_cast<int>(real.size()) != expectedPoints)
        {
            std::cerr << kErrPrefixStr.CsvRead <<"Unexpected number of points for measurement " << mesurementNumb
                      << " (expected " << expectedPoints << ", got " << real.size() << ")" << std::endl;
            hadErrors = true;
            continue;
        }

        // Calculate x, y coordinates from measurement number using shared mapping logic.
        int xPos, yPos;
        if (!measurementToCoordinates(xPoints, yPoints, mesurementNumb, continuous, xPos, yPos))
        {
            hadErrors = true;
            continue;
        }

        // Store data into 3D arrays
        data.set3DDataReal(real, xPos, yPos);
        data.set3DDataImag(imag, xPos, yPos);

        std::cerr << kErrPrefixStr.CsvRead <<"Stored measurement " << mesurementNumb << " at position [" << xPos << "," << yPos << "]" << std::endl;
    }

    return !hadErrors;
}

// Helper functions Index

bool CsvFile::writeMatrixIndexCsv(wxTextFile& file, sData data, bool continuous)
{
    // ID Für Real Nummern einfügen
    int xPoints = data.getNumberOfPts_X();
    int yPoints = data.getNumberOfPts_Y();

    bool isMarker = (data.getFsuSettings().mode == MeasurementMode::MARKER_PEAK);
    const char* label1 = isMarker ? "Freq" : "Real";
    const char* label2 = isMarker ? "Amp"  : "Imag";

    size_t count = xPoints * yPoints;

    for (size_t i = 0; i < count; i++)
    {
        std::string index = getIndexNumbers(xPoints, yPoints, i + 1, continuous);

        file.AddLine(wxString::Format("%s %s", index, label1));

        file.AddLine(wxString::Format("%s %s", index, label2));
    }

    return true;
}

std::string CsvFile::getIndexNumbers(int xPoints, int yPoints, int mesurementNumb, bool continuous)
{
    int xPosition = 0;
    int yPosition = 0;
    std::string matrixSeparator;

    // Set second separator to save Matrix index
    if (m_separator == ',')
    {
        matrixSeparator = ";";
    }
    else
    {
        matrixSeparator = ",";
    }


    if (!measurementToCoordinates(xPoints, yPoints, mesurementNumb, continuous, xPosition, yPosition))
    {
        std::string text = "[0" + matrixSeparator + "0]";
        return text;
    }
    //std::cout << "x pts "<< xPoints << " y pts " << yPoints << std::endl;

    std::string text = "[" + std::to_string(xPosition + 1) + matrixSeparator + std::to_string(yPosition + 1) + "]";
    //std::cout << text << " Nr. " << mesurementNumb <<std::endl;

    return  text;
}

int CsvFile::findLineCsv(wxTextFile& file, wxString findText)
{
    size_t count = file.GetLineCount();

    std::cerr << findText.Upper() << " " << count << std::endl;

    auto it = m_CsvLookupTable.find(findText.Upper().ToStdString());

    if (it != m_CsvLookupTable.end())
    {
        int entryLine = it->second;
        //std::cerr << "Found on Line: " << entryLine << std::endl;
        return entryLine;
    }
    else
    {
        std::cerr << kErrPrefixStr.CsvHelper <<"Not found" << std::endl;
        return -1;
    }
}

bool CsvFile::createCsvLookupTable(wxTextFile& file)
{
    if (!file.IsOpened())
    {
        std::cerr << kErrPrefixStr.CsvHelper <<"file not open" << std::endl;
        return false;
    }

    int lineCount = file.GetLineCount();
    m_CsvLookupTable.clear();
    m_CsvLookupTable.reserve(lineCount);

    char separator = m_separator;

    for (size_t i = 0; i < (size_t)lineCount; i++)
    {
        std::string firstColumnValue = file.GetLine(i).Upper().BeforeFirst(separator).Trim(false).Trim().ToStdString();

        if (!firstColumnValue.empty())
        {
            m_CsvLookupTable[firstColumnValue] = i;
        }
    }

    return true;
}

bool CsvFile::measurementToCoordinates(int xPoints, int yPoints, int mesurementNumb, bool continuous, int& xPosition, int& yPosition) const
{
    if (xPoints <= 0 || yPoints <= 0 || mesurementNumb < 1 || mesurementNumb > xPoints * yPoints)
    {
        return false;
    }

    const int idx = mesurementNumb - 1;
    xPosition = idx / yPoints;

    if (!continuous)
    {
        yPosition = idx % yPoints;
    }
    else
    {
        const int xOneBased = xPosition + 1;
        if (xOneBased % 2 == 0)
        {
            yPosition = yPoints - (idx % yPoints) - 1;
        }
        else
        {
            yPosition = idx % yPoints;
        }
    }

    return true;
}

char CsvFile::detectSeparator(wxTextFile& file)
{
    if (!file.IsOpened())
    {
        std::cerr << kErrPrefixStr.CsvHelper <<"Failed to open file" << std::endl;
        return ',';
    }

    const char candidates[] = {',', ';', '\t', '|'};
    int maxCount = 0;
    char best = ',';

    // Check the first non-empty line after header for separator frequency
    for (size_t i = 0; i < file.GetLineCount() && i < 5; i++)
    {
        wxString line = file.GetLine(i);
        if (line.IsEmpty()) continue;

        for (char sep : candidates)
        {
            int count = std::count(line.begin(), line.end(), sep);

            if (count > maxCount)
            {
                maxCount = count;
                best = sep;
            }
        }
    }

    return best;
}
