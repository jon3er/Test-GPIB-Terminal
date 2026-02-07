#include "dataManagement.h"

//------sData Beginn------
sData::sData(const char* type)
{
    //init Struct
    dsParam = new sParam;
    dsParam->File = "Empty";
    dsParam->Date = "Empty";
    dsParam->Time = "Empty";
    dsParam->Type = type;
    dsParam->NoPoints_X = 1;
    dsParam->NoPoints_Y = 1;
    dsParam->ampUnit = "DB";
    dsParam->startFreq = 0;
    dsParam->endFreq = 100000;
    //get time
    setTimeAndDate();
    //test values
    dsR = {0,1,2,3,4};
    dsI = {0,1,2,3,4};

}
sData::~sData()
{
    //delete dsParam;
}

bool sData::SetData(sParam *par, std::vector<double> re, std::vector<double> im)
{
    try
    {
        dsParam = par;
        dsR = re;
        dsI = im;
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
        par = dsParam;
        re = dsR;
        im = dsI;
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        return false;
    }
    return true;

}

bool sData::setFileName(wxString Name)
{
    try
    {
        dsParam->File = Name;
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
        dsParam->Type = Type;
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
        dsParam->NoPoints_X = NumbPtsX;
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
        dsParam->NoPoints_Y = NumbPtsY;
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
        dsParam->ampUnit = Unit;
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
        dsParam->startFreq = StartFreq;
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
        dsParam->endFreq = EndFreq;
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        return false;
    }

    return true;
}

bool sData::setTimeAndDate()
{
    try
    {
        wxDateTime zeitJetzt = wxDateTime::Now();
        wxString timestamp = zeitJetzt.Format("%H:%M:%S");
        dsParam->Time = timestamp;

        wxString dateStamp = zeitJetzt.Format("%Y:%m:%d");
        dsParam->Date = dateStamp;
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
        std::memcpy(Real3D.getDataPtr(x,y), Array.data(), Array.size()* sizeof(double));
        return true;
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    return false;
}
std::vector<double> sData::get3DDataReal(int x, int y)
{
    return Real3D.getSingleArray(x,y);
}

bool sData::set3DDataImag(std::vector<double> Array , int x, int y)
{
    try
    {
        std::memcpy(Imag3D.getDataPtr(x,y), Array.data(), Array.size()* sizeof(double));
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
    return Imag3D.getSingleArray(x,y);
}
//------sData Ende------

//------sData3D------

sData3D::sData3D(int x, int y, int Anzahl) : X_Messpunkte(x), Y_Messpunkte(y), Messpunkte(Anzahl)
{
    resize(X_Messpunkte, Y_Messpunkte, Messpunkte);
}

void sData3D::resize(int x, int y, int Anzahl)
{
    dataArray.resize(X_Messpunkte * Y_Messpunkte * Messpunkte, 0.0);
}

double& sData3D::at(int x, int y, int dataIndex)
{
    int index = (y*X_Messpunkte + x) * Messpunkte + dataIndex;
    return dataArray[index];
}

double* sData3D::getDataPtr(int x, int y)
{
    int index = (y*X_Messpunkte + x) * Messpunkte;
    return &dataArray[index];
}

std::vector<double> sData3D::getSingleArray(int x, int y)
{
    int index = (y*X_Messpunkte + x) * Messpunkte;
    std::vector<double> subVector(dataArray.begin() + index, dataArray.begin() + index + Messpunkte);

    return subVector;
}

bool saveToCsvFile(wxString& filename, sData& data, int mesurementNumb)
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
    file.Open();

    if (!file.IsOpened())
    {
        wxLogDebug("Failed to open file");
        return false;
    }

    if (mesurementNumb == 1) // weiteren check hinzufügen
    {
        // Write header
        saveHeaderCsv(file, data);
        // Write indexes
        writeMatrixIndexCsv(file, data);
    }

    bool cont;

    if (data.GetType() == "Line")
    {
        wxLogDebug("Linear mesurement");
        cont = false;
    }
    else
    {
        wxLogDebug("Continuous mesurement");
        cont = true;
    }

    // find line with current
    wxString indexText = getIndexNumbers(data.getNumberOfPts_X(),data.getNumberOfPts_Y(), mesurementNumb, cont) + " Real";

    int lineNumber = findLineCsv(file, indexText);
    wxLogDebug("found Line %d", lineNumber);

    int count = data.getNumberOfPts_Array();

    for (int i = 0; i < count; i++)
    {
        file.GetLine(lineNumber) << ","<< real[i];

        file.GetLine(lineNumber + 1) << "," << imag[i];
    }

    file.Write();
    file.Close();

    return true;
}

bool saveHeaderCsv(wxTextFile &file, sData& data)
{
    sData::sParam* dsParam = data.GetParameter();
    data.setNumberofPts_Array();

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
    double startFreq = dsParam->startFreq;
    double endFreq = dsParam->endFreq;
    double step = 1;
    try
    {
        double step = (endFreq - startFreq) / data.getNumberOfPts_Array();
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    double freq = startFreq;

    for (size_t i = 0; i < dsParam->NoPoints_Array; i++)
    {
        lineFreq << "," << freq;
        freq += step;
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

bool readCsvHeader(wxTextFile &file, sData::sParam& dsParam)
{
    size_t lineCount = file.GetLineCount();

    // Minimale Zeilenanzahl prüfen
    if (lineCount < 9) {
        file.Close();
        wxLogDebug("File too short, insufficient header");
        return false;
    }

    // Parse header metadata (lines 0-8)
    dsParam.File = file.GetLine(0).AfterFirst(',').Trim(false).Trim();
    dsParam.Date = file.GetLine(1).AfterFirst(',').Trim(false).Trim();
    dsParam.Time = file.GetLine(2).AfterFirst(',').Trim(false).Trim();
    dsParam.Type = file.GetLine(3).AfterFirst(',').Trim(false).Trim();

    long lVal;
    if (file.GetLine(4).AfterFirst(',').ToLong(&lVal)) 
        dsParam.NoPoints_X = lVal;
    if (file.GetLine(5).AfterFirst(',').ToLong(&lVal)) 
        dsParam.NoPoints_Y = lVal;
    if (file.GetLine(6).AfterFirst(',').ToLong(&lVal)) 
        dsParam.NoPoints_Array = lVal;

    // Parse frequency range
    wxString startFreqStr = file.GetLine(7).AfterFirst(',').Trim(false).Trim().BeforeFirst(' ');
    wxString endFreqStr = file.GetLine(8).AfterFirst(',').Trim(false).Trim().BeforeFirst(' ');
    
    long freqVal;
    if (startFreqStr.ToLong(&freqVal)) dsParam.startFreq = freqVal;
    if (endFreqStr.ToLong(&freqVal)) dsParam.endFreq = freqVal;

    file.Close();
    return true;
}

bool saveDataCsv(wxTextFile& file, sData data, int mesurementNumb)
{
    return true;
}

bool writeDataCsv(wxTextFile& file, sData data, int mesurementNumb)
{
    std::vector<double> real = data.getRealArray();
    std::vector<double> imag = data.getImagArray();
    size_t count = data.getNumberOfPts_Array();

    if (data.GetType() == "Line")
    {
        // ID Für Real Nummern einfügen
        std::string index = getIndexNumbers(data.getNumberOfPts_X(), data.getNumberOfPts_Y(), mesurementNumb);

        wxString buffer = wxString::Format("%s Real", index);
        // Daten für Real anfügen
        for (size_t i = 0; i < count; i++)
        {
            buffer << "," << real[i]; // Mehr dimisonales array einfügen und X-Y_Cord einfügen
        }
        file.AddLine(buffer);
        // ID Für Imagh Zahlen einfügen

        buffer = wxString::Format("%s Imag", index);
        for (size_t i = 0; i < count; i++)
        {
            buffer << "," << real[i];
        }
        file.AddLine(buffer);
    }

    return true;
}

bool readDataCsv(wxTextFile& file, sData& data)
{
    int xPoints = data.getNumberOfPts_X();
    int yPoints = data.getNumberOfPts_Y();
    int totalMeasurements = xPoints * yPoints;

    bool continuous = (data.GetType() != "Line");

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
            wxLogDebug("Could not find Real data line for measurement %d", mesurementNumb);
            continue;
        }

        // Find Imag data line
        int imagLineNum = findLineCsv(file, imagLabel);
        if (imagLineNum == -1)
        {
            wxLogDebug("Could not find Imag data line for measurement %d", mesurementNumb);
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
            wxLogDebug("Warning: Real/Imag data arrays have different sizes for measurement %d!", mesurementNumb);
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

        wxLogDebug("Stored measurement %d at position [%d,%d]", mesurementNumb, xPos, yPos);
    }

    return true;
}

bool writeMatrixIndexCsv(wxTextFile& file, sData data)
{
    // ID Für Real Nummern einfügen
    size_t count = data.getNumberOfPts_X() * data.getNumberOfPts_Y();

    for (size_t i = 0; i < count; i++)
    {
        std::string index = getIndexNumbers(data.getNumberOfPts_X(), data.getNumberOfPts_Y(), i + 1);

        file.AddLine(wxString::Format("%s Real", index));

        // ID Für Imagh Zahlen einfügen
        file.AddLine(wxString::Format("%s Imag", index));
    }

    // Wichtig: Änderungen speichern
    file.Write();

    return true;
}

std::string getIndexNumbers(int xPoints, int yPoints, int mesurementNumb, bool continuous)
{
    int xPosition;
    int yPosition;

    if (mesurementNumb < 1 || mesurementNumb > xPoints * yPoints)
    {
        return "[0;0]";
    }

    // get position in matrix for normal test
    if (!continuous) // line by line
    {
        xPosition = ((mesurementNumb  -1) / yPoints) + 1;
        yPosition = ((mesurementNumb - 1) % xPoints) + 1;
    }
    else // snaking
    {
        xPosition = ((mesurementNumb - 1) / yPoints) + 1;
        if (xPosition % 2 == 0) // is even
        {
            yPosition = yPoints - ((mesurementNumb - 1) % xPoints) + 1;
        }
        else
        {
            yPosition = ((mesurementNumb - 1) % xPoints) + 1;
        }
    }

    std::string text = "[" + std::to_string(xPosition) + ";" + std::to_string(yPosition) + "]";

    return  text;
}

int findLineCsv(wxTextFile& file, wxString findText)
{
    size_t count = file.GetLineCount();
    wxLogDebug("%s %i", findText.Upper(), count);

    for (size_t i = 0; i < count; i++)
    {
        wxLogDebug("%s", file.GetLine(i).Upper());
        if(file.GetLine(i).Upper().Contains(findText.Upper()))
        {
            return i;
        }
    }
    wxLogDebug("Couldnt find text");
    return -1;
}

bool openCsvFile(wxString& filename, sData& data)
{
    wxLogDebug("open CSV");
    sData::sParam* dsParam = new sData::sParam;;
    std::vector<double> dsR;
    std::vector<double> dsI;

    data.GetData(dsParam, dsR, dsI);
    // 1. Parameter-Objekt prüfen
    if (!dsParam) {
        return false;
    }

    // 2. Datei mit wxTextFile öffnen (liest Zeilen in den Speicher)
    wxTextFile file;
    if (!file.Open(filename)) {
        return false;
    }

    dsR.clear();
    dsI.clear();

    size_t lineCount = file.GetLineCount();

    // Minimale Zeilenanzahl prüfen (Header + min. 1 Datenzeile)
    if (lineCount < 9) {
        file.Close();
        return false;
    }

    int HeaderEnd = 6; //Minimum header Size

    //search for fist empty cell (Header end)
    for (size_t i = 0 ; i < lineCount; i++)
    {
        if (file.GetLine(i).IsEmpty() && file.GetLine( i+1 ).Contains("ID"))
        {
            HeaderEnd = i;
            wxLogDebug("Header Ende :%i", HeaderEnd);
            break;
        }
    }


    // 3. Header Metadaten parsen (Zeilen 0-5)
    // AfterFirst(',') extrahiert den Wert nach dem Komma
    dsParam->File = file.GetLine(0).AfterFirst(',').Trim(false).Trim();
    dsParam->Date = file.GetLine(1).AfterFirst(',').Trim(false).Trim();
    dsParam->Time = file.GetLine(2).AfterFirst(',').Trim(false).Trim();
    dsParam->Type = file.GetLine(3).AfterFirst(',').Trim(false).Trim();

    long lVal;
    if (file.GetLine(4).AfterFirst(',').ToLong(&lVal)) dsParam->NoPoints_X = lVal;
    if (file.GetLine(5).AfterFirst(',').ToLong(&lVal)) dsParam->NoPoints_Y = lVal;

    // Data
    for (size_t i = HeaderEnd + 2; i < lineCount; ++i)
    {
        wxString line = file.GetLine(i);

        if (line.IsEmpty()) continue;

        // Pointer auf den Zielvektor setzen
        std::vector<double>* currentVec = nullptr;

        // Prüfen, ob es eine Real- oder Imag-Zeile ist (Case Insensitive)
        if (line.Upper().Contains("REAL"))
        {
            currentVec = &dsR; // kopiert vector auf Real Addresse
        }
        else if (line.Upper().Contains("IMAG"))
        {
            currentVec = &dsI; // kopiert vector auf Imag Addresse
        }
        else
        {
            continue;
        }

        // Tokenizer nur einmal für die aktuelle Zeile und den gewählten Vektor nutzen
        wxStringTokenizer tokenizer(line, ",");

        // 1. Token (Label/ID) entfernen: "[1;1] Real"
        if (tokenizer.HasMoreTokens()) {
            tokenizer.GetNextToken();
        }

        // Restliche Werte parsen
        while (tokenizer.HasMoreTokens())
        {
            double val;
            if (tokenizer.GetNextToken().ToDouble(&val))
            {
                // Sicherstellen, dass currentVec gültig ist (sollte durch if oben gegeben sein)
                if(currentVec) currentVec->push_back(val);
            }
        }
    }

    // Optional: Validierung
    if (dsR.size() != dsI.size()) {
        wxLogDebug("Warnung: Real/Imag Vektoren ungleich lang!");
    }

    data.SetData(dsParam, dsR, dsI);

    file.Close();
    return true;
}

