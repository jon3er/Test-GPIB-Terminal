#include "dataManagement.h"

//------sData Beginn------
sData::sData(const char* type)
{
    //init Struct
    m_dsParam = new sParam;
    m_dsParam->File = "Empty";
    m_dsParam->Date = "Empty";
    m_dsParam->Time = "Empty";
    m_dsParam->Type = type;
    m_dsParam->NoPoints_X = 1;
    m_dsParam->NoPoints_Y = 1;
    m_dsParam->ampUnit = "DB";
    m_dsParam->startFreq = 0;
    m_dsParam->endFreq = 100000;
    //get time
    setTimeAndDate();
    //test values
    m_dsR = {0,1,2,3,4};
    m_dsI = {0,1,2,3,4};

}
sData::~sData()
{
    //delete dsParam;
}

bool sData::SetData(sParam *par, std::vector<double> re, std::vector<double> im)
{
    try
    {
        m_dsParam = par;
        m_dsR = re;
        m_dsI = im;
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        return false;
    }
    return true;

}
bool sData::GetData(sParam *par,std::vector<double>& re, std::vector<double>& im)
{
    try
    {
        par = m_dsParam;
        re = m_dsR;
        im = m_dsI;
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        return false;
    }
    return true;

}
std::vector<double> sData::GetFreqStepVector()
{
    std::vector<double> freqSteps;

    double endFreq = double(m_dsParam->endFreq);
    double startFreq = double(m_dsParam->startFreq);
    double ArrayPts = double(m_dsParam->NoPoints_Array);
    double step;
    
    try
    {
        step = (endFreq - startFreq) / ArrayPts;
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    double freq = startFreq;

    std::cout << step << std::endl;


    for (size_t i = 0; i < ArrayPts; i++)
    {
        freqSteps.push_back(freq);
        freq += step;
    }

    return freqSteps;
}
void sData::getXYCord(int& x, int& y, int mesurementNumber)
{
    // für den das Array ist die addressierung von 0 bis n-1
    int yPoints = m_dsParam->NoPoints_Y;

    x = ((mesurementNumber - 1) / yPoints);
    y = ((mesurementNumber - 1) % yPoints);
}


bool sData::setFileName(wxString Name)
{
    try
    {
        m_dsParam->File = Name;
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        return false;
    }

    return true;
}
bool sData::setFileType(wxString Type)
{
    try
    {
        m_dsParam->Type = Type;
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        return false;
    }

    return true;
}
bool sData::setNumberOfPts_X(unsigned int NumbPtsX)
{
        try
    {
        m_dsParam->NoPoints_X = NumbPtsX;
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        return false;
    }

    return true;
} 
bool sData::setNumberOfPts_Y(unsigned int NumbPtsY)
{
        try
    {
        m_dsParam->NoPoints_Y = NumbPtsY;
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        return false;
    }

    return true;
}
bool sData::setAmpUnit(wxString Unit)
{
    try
    {
        m_dsParam->ampUnit = Unit;
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        return false;
    }

    return true;
}
bool sData::setStartFreq(unsigned int StartFreq)
{
    try
    {
        m_dsParam->startFreq = StartFreq;
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        return false;
    }

    return true;
}
bool sData::setEndFreq(unsigned int EndFreq)
{
    try
    {
        m_dsParam->endFreq = EndFreq;
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        return false;
    }

    return true;
}

void sData::setNumberofPts_Array(int numb) 
{ 
        if (numb == 0) 
        {
            m_dsParam->NoPoints_Array = m_dsR.size();
        } 
        else 
        {
            m_dsParam->NoPoints_Array = numb; 
        }

        try
        {
            resize3DData(m_dsParam->NoPoints_X, m_dsParam->NoPoints_Y, m_dsParam->NoPoints_Array);
        }
        catch(const std::exception& e)
        {
            std::cerr << "Set Number of Points Array Error" << e.what() << '\n';
        }
        
        
}

bool sData::setTimeAndDate()
{
    try
    {
        wxDateTime zeitJetzt = wxDateTime::Now();
        wxString timestamp = zeitJetzt.Format("%H:%M:%S");
        m_dsParam->Time = timestamp;

        wxString dateStamp = zeitJetzt.Format("%Y:%m:%d");
        m_dsParam->Date = dateStamp;
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        return false;
    }

    return true;
}
bool sData::set3DDataReal(std::vector<double> Array , int x, int y)
{
    try
    {
        std::memcpy(m_Real3D.getDataPtr(x,y), Array.data(), Array.size()* sizeof(double));
        m_dsR = Array;
        return true;
    }
    catch(const std::exception& e)
    {
        std::cerr << "Copy failed " <<e.what() << '\n';
    }
    return false;
}
std::vector<double> sData::get3DDataReal(int x, int y)
{
    return m_Real3D.getSingleArray(x,y);
}

bool sData::set3DDataImag(std::vector<double> Array , int x, int y)
{
    try
    {
        std::memcpy(m_Imag3D.getDataPtr(x,y), Array.data(), Array.size()* sizeof(double));
        m_dsI = Array;
        return true;
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    return false;
}
std::vector<double> sData::get3DDataImag(int x, int y)
{
    return m_Imag3D.getSingleArray(x,y);
}
//------sData Ende------

//------sData3D------

sData3D::sData3D(int x, int y, int Anzahl) : m_X_Messpunkte(x), m_Y_Messpunkte(y), m_Messpunkte(Anzahl)
{
    resize(m_X_Messpunkte, m_Y_Messpunkte, m_Messpunkte);
}

void sData3D::resize(int x, int y, int Anzahl) 
{
    m_X_Messpunkte = x;
    m_Y_Messpunkte = y;
    m_Messpunkte = Anzahl;
    m_dataArray.resize(x * y * Anzahl, 0.0);
}

double& sData3D::at(int x, int y, int dataIndex)
{
    int index = (y*m_X_Messpunkte + x) * m_Messpunkte + dataIndex;
    return m_dataArray[index];
}

double* sData3D::getDataPtr(int x, int y)
{
    int index = (y*m_X_Messpunkte + x) * m_Messpunkte;
    return &m_dataArray[index];
}

std::vector<double> sData3D::getSingleArray(int x, int y)
{
    int index = (y*m_X_Messpunkte + x) * m_Messpunkte;
    std::vector<double> subVector(m_dataArray.begin() + index, m_dataArray.begin() + index + m_Messpunkte);

    return subVector;
}

//------sData3D Ende------

// save Csv Functions

bool CsvFile::saveToCsvFile(wxString& filename, sData& data, int mesurementNumb)
{
    std::vector<double> real;
    std::vector<double> imag;

    real = data.getRealArray();
    imag = data.getImagArray();

    //filename.Append(timestamp);
    if (!filename.Lower().EndsWith(".csv"))
    {
        filename.Append(".csv");
    }

    wxTextFile file(filename.ToStdString());
    //check if file Exists
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
        std::cout << "Failed to open file" << std::endl;
        return false;
    }

    // save All avalible data
    if (mesurementNumb == 0)
    {
        // Write header
        saveHeaderCsv(file, data);
        // Write indexes
        writeMatrixIndexCsv(file, data);

        int count = data.getTotalNumberOfPts();

        for (size_t i = 1; i <= count; i++)
        {
            saveDataCsv(file, data, i);
            // stop programm freezing
            wxYield();
        }
        
        file.Write();
        return true;
    }
    else if (mesurementNumb == 1) // weiteren check hinzufügen
    {
        // Write header
        saveHeaderCsv(file, data);
        // Write indexes
        writeMatrixIndexCsv(file, data);

        file.Write();
    }

    bool cont;

    if (data.GetType() == "Line")
    {

        cont = false;
    }
    else
    {
        cont = true;
    }

    saveDataCsv(file, data, mesurementNumb, cont);


    file.Write();
    file.Close();

    return true;
}

bool CsvFile::saveHeaderCsv(wxTextFile &file, sData& data)
{
    sData::sParam* dsParam = data.GetParameter();
    //data.setNumberofPts_Array();

    file.AddLine(wxString::Format("File Name,%s", dsParam->File));
    file.AddLine(wxString::Format("Date,%s", dsParam->Date));
    file.AddLine(wxString::Format("Time,%s", dsParam->Time));
    file.AddLine(wxString::Format("Type,%s", dsParam->Type));
    file.AddLine(wxString::Format("Number Points X,%d", dsParam->NoPoints_X));
    file.AddLine(wxString::Format("Number Points Y,%d", dsParam->NoPoints_Y));
    file.AddLine(wxString::Format("Number Points per mesurement,%d", dsParam->NoPoints_Array));
    file.AddLine(wxString::Format("Start Frequency, %d %s", dsParam->startFreq, dsParam->ampUnit.ToAscii()));
    file.AddLine(wxString::Format("End Frequency, %d %s", dsParam->endFreq, dsParam->ampUnit.ToAscii()));

    file.AddLine(""); // Leerzeile

    // Frequenz-Zeile zusammenbauen
    wxString lineFreq = wxString::Format("f in %s", dsParam->ampUnit.ToAscii());

    std::vector<double> freq = data.GetFreqStepVector();

    for (size_t i = 0; i < dsParam->NoPoints_Array; i++)
    {
        lineFreq << "," << freq[i];
    }
    file.AddLine(lineFreq);

    // ID-Zeile zusammenbauen
    wxString lineID = "ID";
    for (size_t i = 0; i < dsParam->NoPoints_Array; i++)
    {
        lineID << "," << (int)i;
    }
    file.AddLine(lineID);

    // Wichtig: Änderungen speichern
    file.Write();

    return true;
}

bool CsvFile::saveDataCsv(wxTextFile& file, sData data, int mesurementNumb, bool cont)
{
    if (!file.IsOpened())
    {
        std::cerr << "File not opened" << std::endl;
        return false;
    }
    
    int xPoints = data.getNumberOfPts_X();
    int yPoints = data.getNumberOfPts_Y();
    
    // find line with current
    wxString indexText = getIndexNumbers(xPoints, yPoints, mesurementNumb, cont) + " Real";

    int lineNumber = findLineCsv(file, indexText);
    std::cout << "line Found: " << lineNumber << std::endl;

    int xPosition;
    int yPosition;

    data.getXYCord(xPosition, yPosition, mesurementNumb);

    std::vector<double> real = data.get3DDataReal(xPosition, yPosition);
    std::vector<double> imag = data.get3DDataImag(xPosition, yPosition);

    int count = data.getNumberOfPts_Array();

    for (int i = 0; i < count; i++)
    {
        file.GetLine(lineNumber) << ","<< real[i];

        file.GetLine(lineNumber + 1) << "," << imag[i];
    }

    return true;
}

// Read Csv functions


bool CsvFile::readCsvFile(wxString filename, sData& data)
{
    std::cerr << "read CSV" << std::endl;

    wxTextFile file(filename.ToStdString());

    if (!readCsvHeader(file, data))
    {
        std::cout << "Failed to read header" << std::endl;
        file.Close();
        return false;
    }
    readDataCsv(file, data);

    file.Close();
    return true;
}

bool CsvFile::readCsvHeader(wxTextFile&file, sData& data)
{
    if (!file.Open())
    {
        std::cout << "Failed to open file" << std::endl;
        return false;
    }   

    sData::sParam* dsParam = data.GetParameter();

    size_t lineCount = file.GetLineCount();
    // Minimale Zeilenanzahl prüfen
    if (lineCount < 9) {
        file.Close();
        std::cerr << "File too short, insufficient header" << std::endl;
        return false;
    }

    // Parse header metadata (lines 0-8)
    dsParam->File = file.GetLine(0).AfterFirst(',').Trim(false).Trim();
    dsParam->Date = file.GetLine(1).AfterFirst(',').Trim(false).Trim();
    dsParam->Time = file.GetLine(2).AfterFirst(',').Trim(false).Trim();
    dsParam->Type = file.GetLine(3).AfterFirst(',').Trim(false).Trim();

    long lVal;
    if (file.GetLine(4).AfterFirst(',').ToLong(&lVal)) 
        dsParam->NoPoints_X = lVal;
    if (file.GetLine(5).AfterFirst(',').ToLong(&lVal)) 
        dsParam->NoPoints_Y = lVal;
    if (file.GetLine(6).AfterFirst(',').ToLong(&lVal)) 
        dsParam->NoPoints_Array = lVal;

    // Parse frequency range
    wxString startFreqStr = file.GetLine(7).AfterFirst(',').Trim(false).Trim().BeforeFirst(' ');
    wxString endFreqStr = file.GetLine(8).AfterFirst(',').Trim(false).Trim().BeforeFirst(' ');
    
    long freqVal;
    if (startFreqStr.ToLong(&freqVal)) dsParam->startFreq = freqVal;
    if (endFreqStr.ToLong(&freqVal)) dsParam->endFreq = freqVal;

    std::cout << "Read Header" << std::endl;

    // Resize Datastorage array for data
    data.resize3DData(dsParam->NoPoints_X, dsParam->NoPoints_Y, dsParam->NoPoints_Array);


    file.Close();
    return true;
}

bool CsvFile::readDataCsv(wxTextFile& file, sData& data)
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
            std::cerr << "Could not find Real data line for measurement " << mesurementNumb << std::endl;
            continue;
        }

        // Find Imag data line
        int imagLineNum = findLineCsv(file, imagLabel);
        if (imagLineNum == -1)
        {
            std::cerr << "Could not find Imag data line for measurement " << mesurementNumb << std::endl;
            continue;
        }

        // Parse Real data
        wxString realLine = file.GetLine(realLineNum);
        wxStringTokenizer realTokenizer(realLine, ",");
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
        wxStringTokenizer imagTokenizer(imagLine, ",");
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
            std::cerr << "Warning: Real/Imag data arrays have different sizes for measurement " << mesurementNumb << "!" << std::endl;
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

        std::cerr << "Stored measurement " << mesurementNumb << " at position [" << xPos << "," << yPos << "]" << std::endl;
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

    // Wichtig: Änderungen speichern
    file.Write();

    return true;
}

std::string CsvFile::getIndexNumbers(int xPoints, int yPoints, int mesurementNumb, bool continuous)
{
    int xPosition;
    int yPosition;

    if (mesurementNumb < 1 || mesurementNumb > xPoints * yPoints)
    {
        return "[0;0]";
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
    std::cout << "x pts "<< xPoints << " y pts " << yPoints << std::endl;

    std::string text = "[" + std::to_string(xPosition) + ";" + std::to_string(yPosition) + "]";
    std::cout << text << " Nr. " << mesurementNumb <<std::endl;

    return  text;
}

int CsvFile::findLineCsv(wxTextFile& file, wxString findText)
{
    size_t count = file.GetLineCount();

    std::cerr << findText.Upper() << " " << count << std::endl;

    for (size_t i = 0; i < count; i++)
    {
        std::cerr << file.GetLine(i).Upper() << std::endl;
        if(file.GetLine(i).Upper().Contains(findText.Upper()))
        {
            return i;
        }
    }
    std::cerr << "Couldnt find text" << std::endl;
    return -1;
}

