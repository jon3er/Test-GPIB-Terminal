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
    delete dsParam;
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
//------sData Ende------

//------sData3D------

sData3D::sData3D(int x, int y, int Anzahl) : X_Messpunkte(x), Y_Messpunkte(y), Messpunkte(Anzahl)
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

/*
bool saveToCsvFile(wxString& filename, sData data, int mesurementNumb)
{

    std::vector<double> real;
    std::vector<double> imag;

    sData::sParam *dsParam = data.GetParameter();
    data.GetData(dsParam, real, imag); 

    bool setImgToZero = false;
    //Mussen noch in header mit rein Für eine messung erstmal alles auf 1
    
    int X_Cord = 1;
    int Y_Cord = 1;
    int countMess = X_Cord * Y_Cord; // anzahl der messungen


    if (!dsParam) {

        return false;
    }

    //filename.Append(timestamp);
    if (!(filename.substr(strlen(filename)-4) == ".csv"))
    {
        filename.Append(".csv");
    }
    
    std::ofstream file(filename.ToStdString());

    if (!file.is_open()) {
        return false; 
    }

    if (mesurementNumb == 0) // weiteren check hinzufügen
    {
        saveHeaderCsv(file, data);
    }
    
    
    // TODO Einstellbar machen
    file << std::fixed << std::setprecision(15);

    writeDataCsv(file, data, mesurementNumb);

    file.close();

    return true;
}*/

bool saveToCsvFile(wxString& filename, sData& data, int mesurementNumb)
{
    std::vector<double> real;
    std::vector<double> imag;

    sData::sParam *dsParam = data.GetParameter();
    data.GetData(dsParam, real, imag); 

    if (!dsParam) {

        return false;
    }

    //filename.Append(timestamp);
    if (!(filename.substr(strlen(filename)-4) == ".csv"))
    {
        filename.Append(".csv");
    }
    
    wxTextFile file(filename.ToStdString());

    if (!file.IsOpened()) {
        return false; 
    }

    if (mesurementNumb == 0) // weiteren check hinzufügen
    {
        // Write header
        saveHeaderCsv(file, data);
        // Write indexes 
        writeMatrixIndexCsv(file, data);
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

    // find line with current 
    wxString indexText = getIndexNumbers(data.getNumberOfPts_X(),data.getNumberOfPts_Y(), mesurementNumb, cont) + " Real";

    int lineNumber = findLineCsv(file, indexText);

    int count = data.getNumberOfPts_Array();

    for (size_t i = 0; i < count; i++)
    {
        file.GetLine(lineNumber).Append(",%d",real[i]);

        file.GetLine(lineNumber + 1).Append(",%d",imag[i]);
    }

    return true;
}

bool saveHeaderCsv(wxTextFile &file, sData data)
{
    sData::sParam* dsParam = data.GetParameter(); 

    file.AddLine(wxString::Format("File Name,%s", dsParam->File));
    file.AddLine(wxString::Format("Date,%s", dsParam->Date));
    file.AddLine(wxString::Format("Time,%s", dsParam->Time));
    file.AddLine(wxString::Format("Type,%s", dsParam->Type));
    file.AddLine(wxString::Format("Number Points X,%d", dsParam->NoPoints_X));
    file.AddLine(wxString::Format("Number Points Y,%d", dsParam->NoPoints_Y));
    file.AddLine(wxString::Format("Number Points per mesurement,%zu", dsParam->NoPoints_Array));
    file.AddLine(wxString::Format("Start Frequency, %f%s", dsParam->startFreq, dsParam->ampUnit));
    file.AddLine(wxString::Format("End Frequency, %f%s", dsParam->endFreq, dsParam->ampUnit));

    file.AddLine(""); // Leerzeile
    data.setNumberofPts_Array();

    // Frequenz-Zeile zusammenbauen
    wxString lineFreq = wxString::Format("f in %s", dsParam->ampUnit);
    double startFreq = dsParam->startFreq;
    double endFreq = dsParam->endFreq;
    double step = (endFreq - startFreq) / data.getNumberOfPts_Array();
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

bool saveDataCsv(wxTextFile& file, sData data, int mesurementNumb)
{
    return true;
}

bool writeDataCsv(wxTextFile& file, sData data, int mesurementNumb)
{
    std::vector<double> real = data.getRealArray();
    std::vector<double> imag = data.getImagArray();
    int count = data.getNumberOfPts_Array();

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

bool writeMatrixIndexCsv(wxTextFile& file, sData data)
{
    // ID Für Real Nummern einfügen
    int count = data.getNumberOfPts_X() * data.getNumberOfPts_Y(); 

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
    int count = file.GetLineCount();

    for (size_t i = 0; i < count; i++)
    {
        if(file.GetLine(i).Upper().Contains(findText))
        {
            return i;
        }
    }

    return -1;
}

bool openCsvFile(wxString& filename, sData& data)
{
    wxLogDebug("open CSV");
    sData::sParam* dsParam;
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
    for (int i = 0 ; i < lineCount; i++)
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
/*
bool openCsvFileMultiline(wxString& filename)
{


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

    // 3. Header Metadaten parsen (Zeilen 0-5)
    // AfterFirst(',') extrahiert den Wert nach dem Komma
    dsParam->File = file.GetLine(0).AfterFirst(',').Trim(false).Trim();
    dsParam->Date = file.GetLine(1).AfterFirst(',').Trim(false).Trim();
    dsParam->Time = file.GetLine(2).AfterFirst(',').Trim(false).Trim();
    dsParam->Type = file.GetLine(3).AfterFirst(',').Trim(false).Trim();

    long lVal;
    if (file.GetLine(4).AfterFirst(',').ToLong(&lVal)) dsParam->NoPoints_X = lVal;
    if (file.GetLine(5).AfterFirst(',').ToLong(&lVal)) dsParam->NoPoints_Y = lVal;

    // Zeile 6 ist leer, Zeile 7 ist Header ("Index,Real,Imaginary") -> Überspringen

    // 4. Datenpunkte einlesen (ab Zeile 8)
    for (size_t i = 8; i < lineCount; ++i)
    {
        wxString line = file.GetLine(i);
        
        // Leere Zeilen ignorieren
        if (line.IsEmpty()) continue;

        // Zerlegen der Zeile am Komma
        wxStringTokenizer tokenizer(line, ",");

        // Erwartet: Index, Real, Imaginary
        if (tokenizer.CountTokens() >= 3)
        {
            tokenizer.GetNextToken(); // Index verwerfen
            
            wxString sReal = tokenizer.GetNextToken();
            wxString sImag = tokenizer.GetNextToken();

            double dReal = 0.0;
            double dImag = 0.0;

            // ToCDouble erwartet Punkt als Dezimaltrenner (Standard in CSV/C++)
            // Falls dein System Komma erwartet, nutze ToDouble()
            if (sReal.ToCDouble(&dReal) && sImag.ToCDouble(&dImag))
            {
                dsR.push_back(dReal);
                dsI.push_back(dImag);
            }
        }
    }

    file.Close();
    return true;
}
*/