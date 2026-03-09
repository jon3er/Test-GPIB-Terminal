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
        // Write header
        if(!saveCsvHeader(file, data))
        {
            std::cout << kErrPrefixStr.CsvSave <<"Failed to write header" << std::endl;
            return false;
        }
        // Write indexes
        if (!writeMatrixIndexCsv(file, data))
        {
            std::cout << kErrPrefixStr.CsvSave <<"Failed to write indexes" << std::endl;
            return false;
        }
        // create lookup table
        if (!createCsvLookupTable(file))
        {
            std::cout << kErrPrefixStr.CsvSave <<"Failed to create lookup table" << std::endl;
            return false;
        }
    }


    // save whole File at once
    if (mesurementNumb == 0)
    {
        int count = data.getTotalNumberOfPts();

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
    
    file.AddLine("Header Information"); // Leerzeile
    file.AddLine(wxString::Format("%s%c%s",   HeaderInfo::fileName.data(),        m_separator, dsParam->File));
    file.AddLine(wxString::Format("%s%c%s",   HeaderInfo::date.data(),            m_separator, dsParam->Date));
    file.AddLine(wxString::Format("%s%c%s",   HeaderInfo::time.data(),            m_separator, dsParam->Time));
    file.AddLine(wxString::Format("%s%c%s",   HeaderInfo::type.data(),            m_separator, dsParam->Type));
    // Config
    file.AddLine(wxString::Format("%s%c%d",   HeaderConfig::noPointsX.data(),     m_separator, dsParam->NoPoints_X));
    file.AddLine(wxString::Format("%s%c%d",   HeaderConfig::noPointsY.data(),     m_separator, dsParam->NoPoints_Y));
    // Measurement config
    file.AddLine(wxString::Format("%s%c%d",   HeaderConfig::noPointsArray.data(), m_separator, dsParam->NoPoints_Array));
    
    //Frequency
    file.AddLine(wxString::Format("%s%c%d %s", HeaderConfig::startFreq.data(),   m_separator, dsParam->startFreq, dsParam->ampUnit.ToAscii()));
    file.AddLine(wxString::Format("%s%c%d %s", HeaderConfig::endFreq.data(),     m_separator, dsParam->endFreq, dsParam->ampUnit.ToAscii()));

    file.AddLine(""); // Leerzeile

    // Frequenz-Zeile zusammenbauen
    wxString lineFreq = wxString::Format("f in %s", dsParam->ampUnit.ToAscii());

    std::vector<double> freq = data.GetFreqStepVector();

    for (size_t i = 0; i < dsParam->NoPoints_Array; i++)
    {
        lineFreq << m_separator << freq[i];
    }
    file.AddLine(lineFreq);

    // ID-Zeile zusammenbauen
    wxString lineID = "ID";
    for (size_t i = 0; i < dsParam->NoPoints_Array; i++)
    {
        lineID << m_separator << (int)i;
    }
    file.AddLine(lineID);

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
    wxString indexText = getIndexNumbers(xPoints, yPoints, mesurementNumb, cont) + " Real";

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

    // Parse header metadata (lines 0-8)
    dsParam->File = file.GetLine(0).AfterFirst(separator).Trim(false).Trim();
    dsParam->Date = file.GetLine(1).AfterFirst(separator).Trim(false).Trim();
    dsParam->Time = file.GetLine(2).AfterFirst(separator).Trim(false).Trim();
    dsParam->Type = file.GetLine(3).AfterFirst(separator).Trim(false).Trim();

    long lVal;
    if (file.GetLine(4).AfterFirst(separator).ToLong(&lVal)) 
        dsParam->NoPoints_X = lVal;
    if (file.GetLine(5).AfterFirst(separator).ToLong(&lVal)) 
        dsParam->NoPoints_Y = lVal;
    if (file.GetLine(6).AfterFirst(separator).ToLong(&lVal)) 
        dsParam->NoPoints_Array = lVal;

    // Parse frequency range
    wxString startFreqStr = file.GetLine(7).AfterFirst(separator).Trim(false).Trim().BeforeFirst(' ');
    wxString endFreqStr = file.GetLine(8).AfterFirst(separator).Trim(false).Trim().BeforeFirst(' ');
    
    long freqVal;
    if (startFreqStr.ToLong(&freqVal)) dsParam->startFreq = freqVal;
    if (endFreqStr.ToLong(&freqVal)) dsParam->endFreq = freqVal;

    std::cout << kErrPrefixStr.CsvRead <<"Read Header" << std::endl;

    // Resize Datastorage array for data
    data.resize3DData(dsParam->NoPoints_X, dsParam->NoPoints_Y, dsParam->NoPoints_Array);

    return true;
}

bool CsvFile::readCsvData(wxTextFile& file, sData& data)
{
    int xPoints = data.getNumberOfPts_X();
    int yPoints = data.getNumberOfPts_Y();
    int totalMeasurements = xPoints * yPoints;

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

        // ID Für Real und Imag Nummern suchen
        std::string index = getIndexNumbers(xPoints, yPoints, mesurementNumb, continuous);

        wxString realLabel = wxString::Format("%s Real", index);
        wxString imagLabel = wxString::Format("%s Imag", index);

        // Find Real data line
        int realLineNum = findLineCsv(file, realLabel);
        if (realLineNum == -1)
        {
            std::cerr << kErrPrefixStr.CsvRead <<"Could not find Real data line for measurement " << mesurementNumb << std::endl;
            continue;
        }

        // Find Imag data line
        int imagLineNum = findLineCsv(file, imagLabel);
        if (imagLineNum == -1)
        {
            std::cerr << kErrPrefixStr.CsvRead <<"Could not find Imag data line for measurement " << mesurementNumb << std::endl;
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

    size_t count = xPoints * yPoints;
    
    for (size_t i = 0; i < count; i++)
    {
        std::string index = getIndexNumbers(xPoints, yPoints, i + 1);

        file.AddLine(wxString::Format("%s Real", index));

        // ID Für Imagh Zahlen einfügen
        file.AddLine(wxString::Format("%s Imag", index));
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