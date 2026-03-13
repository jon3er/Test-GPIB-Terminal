#include "CsvManagement.h"
#include "systemInfo.h"


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
        if (!writeMatrixIndexCsv(file, data))
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
            saveCsvData(file, data, i);
        }

        file.Write();
        file.Close();
        return true;
    }

    // save one measurement
    // check Mesurement type
    bool cont;

    if (data.GetType() == "Line")
    {

        cont = false;
    }
    else
    {
        cont = true;
    }

    saveCsvData(file, data, mesurementNumb, cont);


    file.Write();
    file.Close();

    std::cout << "File saved" << std::endl;

    return true;
}

bool CsvFile::saveCsvHeader(wxTextFile &file, sData& data)
{
    sData::sParam* dsParam = data.GetParameter();

    //file.AddLine("Header Information"); // Leerzeile
    file.AddLine(wxString::Format("%s%c%s",   HeaderInfo::fileName.data(),        m_separator, dsParam->File));
    file.AddLine(wxString::Format("%s%c%s",   HeaderInfo::date.data(),            m_separator, dsParam->Date));
    file.AddLine(wxString::Format("%s%c%s",   HeaderInfo::time.data(),            m_separator, dsParam->Time));
    file.AddLine(wxString::Format("%s%c%s",   HeaderInfo::type.data(),            m_separator, dsParam->Type));
    // Config
    file.AddLine(wxString::Format("%s%c%d",   HeaderConfig::noPointsX.data(),     m_separator, dsParam->NoPoints_X));
    file.AddLine(wxString::Format("%s%c%d",   HeaderConfig::noPointsY.data(),     m_separator, dsParam->NoPoints_Y));
    // Measurement config
    //file.AddLine(wxString::Format("%s%c%d",   HeaderConfig::noPointsArray.data(), m_separator, dsParam->NoPoints_Array));

    file.AddLine(""); // Leerzeile

    // Messeinstellungen je nach Modus (from cached settings)
    MeasurementMode mode = data.getFsuSettings().mode;

    switch (mode)
    {
        case MeasurementMode::SWEEP:
            file.AddLine(wxString::Format("%s%c%s", HeaderConfig::mesSettings.data(), m_separator, "Sweep"));
            saveCsvSettingsSweep(file, data);
            break;
        case MeasurementMode::IQ:
            file.AddLine(wxString::Format("%s%c%s", HeaderConfig::mesSettings.data(), m_separator, "IQ"));
            saveCsvSettingsQI(file, data);
            break;
        case MeasurementMode::MARKER_PEAK:
            file.AddLine(wxString::Format("%s%c%s", HeaderConfig::mesSettings.data(), m_separator, "Marker Peak"));
            saveCsvSettingsMarker(file, data);
            break;
        case MeasurementMode::COSTUM:
            file.AddLine(wxString::Format("%s%c%s", HeaderConfig::mesSettings.data(), m_separator, "Costum"));
            file.AddLine(wxString::Format("%s%c%s", HeaderConfig::customFile.data(), m_separator, data.getFsuSettings().costumFile.ToUTF8()));
            break;
    }

    file.AddLine(""); // Leerzeile

    // Frequenz-Zeile zusammenbauen
    wxString lineFreq = wxString::Format("f in %s", dsParam->ampUnit.ToAscii());

    std::vector<double> freq = data.GetFreqStepVector();

    switch (mode)
    {
        case MeasurementMode::SWEEP:
        case MeasurementMode::IQ:

            for (size_t i = 0; i < dsParam->NoPoints_Array; i++)
            {
                lineFreq << m_separator << freq[i];
            }
            file.AddLine(lineFreq);

            break;

        case MeasurementMode::MARKER_PEAK:

            break;
    }

    // ID-Zeile zusammenbauen
    wxString lineID = "ID";
    for (size_t i = 0; i < dsParam->NoPoints_Array; i++)
    {
        lineID << m_separator << (int)i;
    }
    file.AddLine(lineID);

    return true;
}

bool CsvFile::saveCsvSettingsSweep(wxTextFile& file, sData& data)
{
    const auto& s = data.getFsuSettings().sweep;

    file.AddLine(wxString::Format("%s%c%u %s",  wxString::FromUTF8(HeaderConfig::startFreq.data()     ), m_separator, s.startFreq,  s.unit));
    file.AddLine(wxString::Format("%s%c%u %s",  wxString::FromUTF8(HeaderConfig::endFreq.data()       ), m_separator, s.stopFreq,   s.unit));
    file.AddLine(wxString::Format("%s%c%d",     wxString::FromUTF8(HeaderConfig::refPegel.data()      ), m_separator, s.refLevel));
    file.AddLine(wxString::Format("%s%c%u",     wxString::FromUTF8(HeaderConfig::HFDaempfung.data()   ), m_separator, s.att));
    file.AddLine(wxString::Format("%s%c%s",     wxString::FromUTF8(HeaderConfig::ampUnit.data()       ), m_separator, s.unit));
    file.AddLine(wxString::Format("%s%c%u",     wxString::FromUTF8(HeaderConfig::RBW.data()           ), m_separator, s.rbw));
    file.AddLine(wxString::Format("%s%c%u",     wxString::FromUTF8(HeaderConfig::VBW.data()           ), m_separator, s.vbw));
    file.AddLine(wxString::Format("%s%c%u",     wxString::FromUTF8(HeaderConfig::noPointsArray.data() ), m_separator, s.points));
    file.AddLine(wxString::Format("%s%c%s",     wxString::FromUTF8(HeaderConfig::sweepTime.data()     ), m_separator, s.sweepTime.c_str()));
    file.AddLine(wxString::Format("%s%c%s",     wxString::FromUTF8(HeaderConfig::detektor.data()      ), m_separator, s.detector.c_str()));

    return true;
}

bool CsvFile::saveCsvSettingsQI(wxTextFile& file, sData& data)
{
    const auto& s = data.getFsuSettings().iq;

    file.AddLine(wxString::Format("%s%c%g %s",  wxString::FromUTF8(HeaderConfig::centerFreq.data()    ), m_separator, s.centerFreq, s.unit));
    file.AddLine(wxString::Format("%s%c%s",     wxString::FromUTF8(HeaderConfig::refPegel.data()      ), m_separator, s.refLevel));
    file.AddLine(wxString::Format("%s%c%u",     wxString::FromUTF8(HeaderConfig::HFDaempfung.data()   ), m_separator, s.att));
    file.AddLine(wxString::Format("%s%c%s",     wxString::FromUTF8(HeaderConfig::ampUnit.data()       ), m_separator, s.unit));
    file.AddLine(wxString::Format("%s%c%g",     wxString::FromUTF8(HeaderConfig::sampleRate.data()    ), m_separator, s.sampleRate));
    file.AddLine(wxString::Format("%s%c%d",     wxString::FromUTF8(HeaderConfig::recordLength.data()  ), m_separator, s.recordLength));
    file.AddLine(wxString::Format("%s%c%g",     wxString::FromUTF8(HeaderConfig::ifBandwidth.data()   ), m_separator, s.ifBandwidth));
    file.AddLine(wxString::Format("%s%c%s",     wxString::FromUTF8(HeaderConfig::triggerSource.data() ), m_separator, s.triggerSource));
    file.AddLine(wxString::Format("%s%c%g",     wxString::FromUTF8(HeaderConfig::triggerLevel.data()  ), m_separator, s.triggerLevel));
    file.AddLine(wxString::Format("%s%c%g",     wxString::FromUTF8(HeaderConfig::triggerDelay.data()  ), m_separator, s.triggerDelay));

    return true;
}

bool CsvFile::saveCsvSettingsMarker(wxTextFile& file, sData& data)
{
    const auto& s = data.getFsuSettings().marker;

    file.AddLine(wxString::Format("%s%c%u %s",  wxString::FromUTF8(HeaderConfig::startFreq.data()  ), m_separator, s.startFreq,  s.unit));
    file.AddLine(wxString::Format("%s%c%u %s",  wxString::FromUTF8(HeaderConfig::endFreq.data()    ), m_separator, s.stopFreq,   s.unit));
    file.AddLine(wxString::Format("%s%c%s",     wxString::FromUTF8(HeaderConfig::refPegel.data()   ), m_separator, s.refLevel));
    file.AddLine(wxString::Format("%s%c%u",     wxString::FromUTF8(HeaderConfig::HFDaempfung.data()), m_separator, s.att));
    file.AddLine(wxString::Format("%s%c%s",     wxString::FromUTF8(HeaderConfig::ampUnit.data()    ), m_separator, s.unit));
    file.AddLine(wxString::Format("%s%c%u",     wxString::FromUTF8(HeaderConfig::RBW.data()        ), m_separator, s.rbw));
    file.AddLine(wxString::Format("%s%c%u",     wxString::FromUTF8(HeaderConfig::VBW.data()        ), m_separator, s.vbw));
    file.AddLine(wxString::Format("%s%c%s",     wxString::FromUTF8(HeaderConfig::detektor.data()   ), m_separator, s.detector));

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
    // std::cout << "line Found: " << lineNumber << std::endl;

    int xPosition;
    int yPosition;

    data.getXYCord(xPosition, yPosition, mesurementNumb);

    std::vector<double> real = data.get3DDataReal(xPosition, yPosition);
    std::vector<double> imag = data.get3DDataImag(xPosition, yPosition);

    int count = data.getNumberOfPts_Array();

    for (int i = 0; i < count; i++)
    {
        file.GetLine(lineNumber) << m_separator << real[i];

        file.GetLine(lineNumber + 1) << m_separator << imag[i];
    }

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

    // Detect separator
    char separator = detectSeparator(file);

    // Parse header metadata (lines 0-6)
    dsParam->File = file.GetLine(0).AfterFirst(separator).Trim(false).Trim();
    dsParam->Date = file.GetLine(1).AfterFirst(separator).Trim(false).Trim();
    dsParam->Time = file.GetLine(2).AfterFirst(separator).Trim(false).Trim();
    dsParam->Type = file.GetLine(3).AfterFirst(separator).Trim(false).Trim();

    long lVal;
    if (file.GetLine(4).AfterFirst(separator).ToLong(&lVal))
        dsParam->NoPoints_X = lVal;
    if (file.GetLine(5).AfterFirst(separator).ToLong(&lVal))
        dsParam->NoPoints_Y = lVal;

    // Messeinstellungen je nach Modus einlesen
    int mesSettingsLine = findLineCsv(file, wxString(HeaderConfig::mesSettings.data()));
    if (mesSettingsLine >= 0)
    {
        wxString modeName = file.GetLine(mesSettingsLine).AfterFirst(separator).Trim(false).Trim();
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

bool CsvFile::readCsvSettingsSweep(wxTextFile& file, sData& data)
{
    sData::sParam* dsParam = data.GetParameter();
    char separator = detectSeparator(file);

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

    long lVal;
    if (readLine(HeaderConfig::refPegel).ToLong(&lVal))
        dsParam->refPegel = static_cast<int>(lVal);

    if (readLine(HeaderConfig::HFDaempfung).ToLong(&lVal))
        dsParam->HFDaempfung = lVal;

    dsParam->ampUnit = readLine(HeaderConfig::ampUnit);

    if (readLine(HeaderConfig::RBW).ToLong(&lVal))
        dsParam->RBW = lVal;
    if (readLine(HeaderConfig::VBW).ToLong(&lVal))
        dsParam->VBW = lVal;

    int iVal;
    if (readLine(HeaderConfig::noPointsArray).ToInt(&iVal))
        dsParam->NoPoints_Array = iVal;

    dsParam->sweepTime = readLine(HeaderConfig::sweepTime).ToStdString();
    dsParam->detektor  = readLine(HeaderConfig::detektor).ToStdString();

    return true;
}

bool CsvFile::readCsvSettingsQI(wxTextFile& file, sData& data)
{
    sData::sParam* dsParam = data.GetParameter();
    char separator = detectSeparator(file);

    auto readLine = [&](std::string_view label) -> wxString {
        int line = findLineCsv(file, wxString(label.data()));
        if (line < 0) return "";
        return file.GetLine(line).AfterFirst(separator).Trim(false).Trim();
    };

    long lVal;
    if (readLine(HeaderConfig::refPegel).ToLong(&lVal))
        dsParam->refPegel = static_cast<int>(lVal);

    if (readLine(HeaderConfig::HFDaempfung).ToLong(&lVal))
        dsParam->HFDaempfung = lVal;

    dsParam->ampUnit = readLine(HeaderConfig::ampUnit);

    return true;
}

bool CsvFile::readCsvSettingsMarker(wxTextFile& file, sData& data)
{
    sData::sParam* dsParam = data.GetParameter();
    char separator = detectSeparator(file);

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

    long lVal;
    if (readLine(HeaderConfig::refPegel).ToLong(&lVal))
        dsParam->refPegel = static_cast<int>(lVal);

    if (readLine(HeaderConfig::HFDaempfung).ToLong(&lVal))
        dsParam->HFDaempfung = lVal;

    dsParam->ampUnit = readLine(HeaderConfig::ampUnit);

    double dVal;
    if (readLine(HeaderConfig::RBW).ToDouble(&dVal))
        dsParam->RBW = (unsigned int)dVal;
    if (readLine(HeaderConfig::VBW).ToDouble(&dVal))
        dsParam->VBW = (unsigned int)dVal;

    dsParam->detektor = readLine(HeaderConfig::detektor).ToStdString();

    return true;
}

bool CsvFile::readCsvSettingsCostum(wxTextFile& file, sData& data)
{
        sData::sParam* dsParam = data.GetParameter();
    char separator = detectSeparator(file);

    auto readLine = [&](std::string_view label) -> wxString {
        int line = findLineCsv(file, wxString(label.data()));
        if (line < 0) return "";
        return file.GetLine(line).AfterFirst(separator).Trim(false).Trim();
    };

    dsParam->costumFile = readLine(HeaderConfig::customFile);

    return true;
}

bool CsvFile::readCsvData(wxTextFile& file, sData& data)
{
    int xPoints = data.getNumberOfPts_X();
    int yPoints = data.getNumberOfPts_Y();
    int totalMeasurements = xPoints * yPoints;

    std::cout << "number of Points: " << xPoints << " x " << yPoints << std::endl;


    bool continuous = (data.GetType() != "Line");

    if (!file.Open())
    {
        std::cout << "Failed to open file" << std::endl;
        return false;
    }

    // Read all available measurements
    for (int mesurementNumb = 1; mesurementNumb <= totalMeasurements; mesurementNumb++)
    {
        std::vector<double> real;
        std::vector<double> imag;

        // ID Für Real und Imag Nummern suchen (Marker: Freq/Amp)
        std::string index = getIndexNumbers(xPoints, yPoints, mesurementNumb, continuous);

        // Detect marker mode from CSV: check if Freq label exists for first measurement
        bool isMarker = false;
        if (mesurementNumb == 1)
        {
            std::string firstIdx = getIndexNumbers(xPoints, yPoints, 1, continuous);
            wxString testLabel = wxString::Format("%s Freq", firstIdx);
            isMarker = (findLineCsv(file, testLabel) >= 0);
        }
        static bool s_isMarker = false;
        if (mesurementNumb == 1) s_isMarker = isMarker;

        const char* label1 = s_isMarker ? "Freq" : "Real";
        const char* label2 = s_isMarker ? "Amp"  : "Imag";

        wxString realLabel = wxString::Format("%s %s", index, label1);
        wxString imagLabel = wxString::Format("%s %s", index, label2);

        // Find Real/Freq data line
        int realLineNum = findLineCsv(file, realLabel);
        if (realLineNum == -1)
        {
            std::cerr << kErrPrefixStr.CsvRead <<"Could not find " << label1 << " data line for measurement " << mesurementNumb << std::endl;
            continue;
        }

        // Find Imag/Amp data line
        int imagLineNum = findLineCsv(file, imagLabel);
        if (imagLineNum == -1)
        {
            std::cerr << kErrPrefixStr.CsvRead <<"Could not find " << label2 << " data line for measurement " << mesurementNumb << std::endl;
            continue;
        }

        // Parse Real data
        wxString realLine = file.GetLine(realLineNum);
        wxStringTokenizer realTokenizer(realLine, m_separator);
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
        wxStringTokenizer imagTokenizer(imagLine, m_separator);
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
            continue;
        }

        // Calculate x, y coordinates from measurement number
        int xPos, yPos;
        if (!continuous) // line by line
        {
            xPos = ((mesurementNumb - 1) / yPoints);
            yPos = ((mesurementNumb - 1) % yPoints);
        }
        else // snaking
        {
            xPos = ((mesurementNumb - 1) / yPoints);
            if (xPos % 2 == 0) // is even
            {
                yPos = yPoints - ((mesurementNumb - 1) % yPoints) - 1;
            }
            else
            {
                yPos = ((mesurementNumb - 1) % yPoints);
            }
        }

        // Store data into 3D arrays
        data.set3DDataReal(real, xPos, yPos);
        data.set3DDataImag(imag, xPos, yPos);

        std::cerr << kErrPrefixStr.CsvRead <<"Stored measurement " << mesurementNumb << " at position [" << xPos << "," << yPos << "]" << std::endl;
    }

    file.Close();

    return true;
}

// Helper functions Index

bool CsvFile::writeMatrixIndexCsv(wxTextFile& file, sData data)
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
        std::string index = getIndexNumbers(xPoints, yPoints, i + 1);

        file.AddLine(wxString::Format("%s %s", index, label1));

        file.AddLine(wxString::Format("%s %s", index, label2));
    }

    return true;
}

std::string CsvFile::getIndexNumbers(int xPoints, int yPoints, int mesurementNumb, bool continuous)
{
    int xPosition;
    int yPosition;
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


    if (mesurementNumb < 1 || mesurementNumb > xPoints * yPoints)
    {
        std::string text = "[0" + matrixSeparator + "0]";
        return text;
    }

    int idx = mesurementNumb - 1;

    xPosition = (idx / yPoints) + 1;
    // get position in matrix for normal test
    if (!continuous) // line by line
    {
        yPosition = (idx % yPoints) + 1;
    }
    else // snaking
    {

        if (xPosition % 2 == 0) // is even
        {
            yPosition = yPoints - (idx % yPoints) + 1;
        }
        else
        {
            yPosition = (idx % yPoints) + 1;
        }
    }
    //std::cout << "x pts "<< xPoints << " y pts " << yPoints << std::endl;

    std::string text = "[" + std::to_string(xPosition) + matrixSeparator + std::to_string(yPosition) + "]";
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
        int entryLine = m_CsvLookupTable[findText.Upper().ToStdString()];
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

    char separator = detectSeparator(file);

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
