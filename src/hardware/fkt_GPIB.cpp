
#include "fkt_GPIB.h"

//TODO Create Device Class and Create new read read and write funtions
GpibDevice::GpibDevice()
{

}

GpibDevice::~GpibDevice()
{
    disconnect();
}

std::string GpibDevice::read(int forceReadBytes)
{
    wxString Text;

    if (Connected)
    {
        std::vector<char> BigBuffer;
        DWORD BufferSize;
        FT_STATUS ftStatus;

        wxLogDebug("Reading from Device...");

        ftStatus = readUsbDev(ftHandle, BigBuffer, BufferSize, forceReadBytes);

        if (ftStatus == FT_OK)
        {
            lastMsgReceived = std::string(BigBuffer.data(),BigBuffer.size());
            Text = "Msg received: " + lastMsgReceived + "\n";

            if (BigBuffer.size() == 0)
            {
                Text = "No Message to Read\n";
            }
        }
        else
        {
            Text = "Failed to Receive Data - TimeOut after 5s\n";
            Connected = false;
        }
    }
    else
    {
        wxLogDebug("No Device to send too");
        Text = "Failed to Connect to a Device\n";
        Connected = false;
    }

    return std::string(Text.ToUTF8());
}

std::string GpibDevice::write(std::string msg)
{
    std::string Text;

    wxLogDebug("Write Command Entered");

    if (Connected)
    {
        DWORD bytesWritten;
        wxString GPIBText = msg;
        std::string CheckText(GPIBText.ToUTF8());
        //Check String if Adapter or GPIB Command and check for ASCII 10, 13, 27, 43
        std::vector<char> charArrWriteGpib = checkAscii(CheckText);

        wxLogDebug("Trying to write to Device... %s", std::string(charArrWriteGpib.begin(),charArrWriteGpib.end()));

        FT_STATUS ftStatus =writeUsbDev(ftHandle, charArrWriteGpib, bytesWritten);

        if (ftStatus == FT_OK)
        {
            Text = GPIBText;
            Text = "Msg sent: " + Text + " ; " + std::to_string(bytesWritten) + " Bytes Written to GPIB Device\n";
        }
        else
        {
            Text = "Failed to send Data\n";
        }
    }
    else
    {
        wxLogDebug("No Connection");
        Text = "Failed to Connect\n";
    }
    return Text;
}

std::string GpibDevice::send(std::string msg, int DelayMs)
{
    write(msg);
    sleepMs(DelayMs);
    quaryBuffer();

    return read();
}

DWORD GpibDevice::quaryBuffer()
{
    //Get Number of bytes to read from receive queue
    ftStatus = FT_GetQueueStatus(ftHandle,&BytesToRead);
    wxLogDebug("Bytes in Queue: %i", BytesToRead);
    printErr(ftStatus,"Failed to Get Queue Status");

    return BytesToRead;
}

void GpibDevice::connect(std::string args)
{
    if (!Connected)
    {
        ftStatus = FT_Open(numDev,&ftHandle);
        printErr(ftStatus,"Failed to Connect");
        write("SYST:DISP:UDP ON"); //Turn on monitor

        if (ftStatus == FT_OK)
        {
            wxLogDebug("Connected to %i", numDev);
            Connected = true;
        }
    }
}
void GpibDevice::disconnect(std::string args)
{
    write("++auto 0");
    write("*CLS");
    write("++loc");
    write("++ifc");

    sleepMs(200);


    ftStatus = FT_Close(ftHandle);
    printErr(ftStatus,"Failed to Disconnect");
    if (ftStatus == FT_OK)
    {
        wxLogDebug("Connected to %i", numDev);
        Connected = false;
    }
}
void GpibDevice::config()
{
    //set Baudrate
    ftStatus = FT_SetBaudRate(ftHandle,BaudRate);
    printErr(ftStatus,"Failed to set Baudrate");

    ftStatus = FT_SetDataCharacteristics(ftHandle, FT_BITS_8, FT_STOP_BITS_1, FT_PARITY_NONE);
    printErr(ftStatus,"Failed to set Data Characteristics");

    ftStatus = FT_SetFlowControl(ftHandle, FT_FLOW_NONE, 0, 0);
    printErr(ftStatus,"Failed to set flow Characteristics");

    ftStatus =  FT_SetTimeouts(ftHandle, 500,500);
    printErr(ftStatus,"Failed to set TimeOut");

    wxLogDebug("FT-Config complete");

    if (ftStatus == FT_OK)
    {
        configFin = true;
    }
    else
    {
        configFin = false;
    }
}
void GpibDevice::readScriptFile(const wxString& dirPath, const wxString& file, wxArrayString& logAdapterReceived)
{
    wxTextFile textFile;

    if (textFile.Open(dirPath + file))
    {
        if (!getConnected())
        {
            connect();
            config();
        }

        for (size_t i = 0; i < textFile.GetLineCount(); i++)
        {
            wxString line = textFile.GetLine(i);
            if(line.IsEmpty())
            {
                wxLogDebug("line %i: Empty", (int)i, line);
            }
            else if (line.substr(0,1) == "#")
            {
                wxLogDebug("line %i: Kommentar: %s", (int)i, line.substr(1));
            }
            else if (line.substr(0,5) == "wait ")
            {
                int wait;
                wxString strWait = line.substr(5);
                if (strWait.ToInt(&wait))
                {
                    wxLogDebug("wait for %ims", wait);
                    sleepMs(wait);
                }
                else
                {
                    wxLogDebug("Invalid wait Time input: %s", strWait);
                }
            }
            else if (line.substr(0,5) == "send ")
            {
                wxLogDebug("line %i: manuell send: %s", (int)i, line);
                line = line.substr(5);
                logAdapterReceived.Add(send(std::string(line.ToUTF8())));
                wxLogDebug("responce: %s", logAdapterReceived.Last());
            }
            else if (line.substr(0,6) == "write ")
            {
                wxLogDebug("line %i: manuell write: %s", (int)i, line);
                line = line.substr(6);
                write(std::string(line.ToUTF8()));
            }
            else if (line.substr(0,4) == "read")
            {
                wxLogDebug("line %i: manuell read", (int)i);
                logAdapterReceived.Add(read());
                wxLogDebug("responce: %s", logAdapterReceived.Last());
            }
            else if(line.Contains("?") && line.substr(0,4) == "TRAC")
            {
                wxLogDebug("line %i: send: %s", (int)i, line);

                write(std::string(line.ToUTF8()));
                write("++read eoi");
                sleepMs(300);
                logAdapterReceived.Add(read());
                wxLogDebug("responce: %s", logAdapterReceived.Last());
                Messung.seperateDataBlock(logAdapterReceived.Last());
                Messung.setFreqStartEnd(75'000'000,125'000'000);
                Messung.calcYdata(); //start und end frequenz angeben

            }
            else if(line.Contains("?"))
            {
                wxLogDebug("line %i: send: %s", (int)i, line);
                logAdapterReceived.Add(send(std::string(line.ToUTF8())));
                wxLogDebug("responce: %s", logAdapterReceived.Last());
            }
            else
            {
                wxLogDebug("line %i: write: %s", (int)i, line);
                write(std::string(line.ToUTF8()));
            }
        }
    }
}

std::string GpibDevice::statusText()
{
    std::string Text;

    if (Connected)
    {
        Text = "Connected to a Device ";
    }
    else
    {
        Text = "Not connected to a device";
    }
    if (Connected && configFin)
    {
        Text = Text + " and ";
    }
    if (configFin)
    {
        Text = Text + "Device config set Baudrate to: " + std::to_string(BaudRate);
    }

    Text += "\nLast Status code:" + wxString(statusString(ftStatus)) + "\n";

    return Text;
}

FT_STATUS GpibDevice::getStatus()
{
    return ftStatus;
}

FT_HANDLE GpibDevice::getHandle()
{
    return ftHandle;
}

bool GpibDevice::getConnected()
{
    return Connected;
}

std::string GpibDevice::getLastMsgReseived()
{
    return lastMsgReceived;
}

int GpibDevice::getBaudrate()
{
    return BaudRate;
}

void GpibDevice::setBaudrate(int BaudrateNew)
{
    BaudRate = BaudrateNew;
}
//------fsuMesurement Beginn-----
fsuMesurement::fsuMesurement()
{
    x_Data = {0};
    y_Data = {0};

    FreqStart = 75'000'000;
    FreqEnd = 125'000'000;

    NoPoints_x = 0;
    NoPoints_y = 0;
}
void fsuMesurement::seperateDataBlock(const wxString& receivedString)
{
    wxArrayString seperatedStrings = wxStringTokenize(receivedString, ",");

    double value;
    wxString data;
    std::vector<double> x;

    for (long unsigned int i = 0; i < seperatedStrings.Count(); i++)
    {
        data = seperatedStrings[i];

        if(data.ToCDouble(&value))
        {
             x.push_back(value);
             wxLogDebug("seperated value: %3f", value);
        }
        else
        {
            wxLogDebug("Failed to convert");
        }
    }

    x_Data.resize(x.size());
    x_Data=x;
}
std::vector<double> fsuMesurement::calcYdata()
{
    int totalPoints = x_Data.size();
    NoPoints_x = totalPoints;
    y_Data.clear();

    wxLogDebug("Total points: %i", totalPoints);
    double range = FreqEnd-FreqStart;

    double step = range/totalPoints;
    wxLogDebug("Range: %3f   Step: %3f",range,step);
    double newYPoint = FreqStart;

    for(int i = 0; i < totalPoints; i++)
    {
        newYPoint = newYPoint + step;
        y_Data.push_back(newYPoint);
        wxLogDebug("Y Berechnet: %f", y_Data[i]);
    }

    NoPoints_y = y_Data.size();

    return y_Data;
}
void fsuMesurement::setFreqStartEnd(double FreqS, double FreqE)
{
    FreqStart = FreqS;
    FreqEnd = FreqE;
}
/*
sData::sParam fsuMesurement::getMesurmentData()
{
    sData::sParam *tempOld = tempData.GetParameter();
    return *tempOld;

}
*/

//------fsuMesurement Ende-----

//------sData Beginn------
sData::sData(const char* type, unsigned int NoPoints)
{
    //init Struct
    dsParam = new sParam;
    dsParam->File = "Empty";
    dsParam->Date = "Empty";
    dsParam->Time = "Empty";
    dsParam->Type = type;
    dsParam->NoPoints_X = NoPoints;
    dsParam->NoPoints_Y = 0;
    dsParam->ampUnit = "DB";
    dsParam->startFreq = 0;
    dsParam->endFreq = 100000;

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

bool sData::saveToCsvFile(wxString& filename)
{
    bool setImgToZero = false;
    //Mussen noch in header mit rein Für eine messung erstmal alles auf 1
    
    int X_Cord = 1;
    int Y_Cord = 1;
    int countMess = X_Cord * Y_Cord; // anzahl der messungen


    if (!dsParam) {

        return false;
    }

    if (dsR.size() != dsI.size()) 
    {
        setImgToZero = true;
    }

    wxDateTime zeitJetzt = wxDateTime::Now();
    wxString timestamp = zeitJetzt.Format("%H:%M");
        

    //filename.Append(timestamp);
    if (!(filename.substr(strlen(filename)-4) == ".csv"))
    {
        filename.Append(".csv");
    }
    
    std::ofstream file(filename.ToStdString());

    if (!file.is_open()) {
        return false; 
    }

    file << "File Name," << dsParam->File.ToStdString() << "\n";
    file << "Date,"      << dsParam->Date.ToStdString() << "\n";
    file << "Time,"      << dsParam->Time.ToStdString() << "\n";
    file << "Type,"      << dsParam->Type.ToStdString() << "\n";
    file << "Number Points X,"  << dsParam->NoPoints_X << "\n";
    file << "Number Points Y,"  << dsParam->NoPoints_Y << "\n";
    
    file << "\n";

    file << "ID";

    //Array länge bestimmen
    size_t count = dsR.size();

    // Messwert ID
    for (size_t i = 0; i < count; i++)
    {
        file << "," << i ;
    }
    

    file << std::fixed << std::setprecision(15);

    for (size_t i = 0; i < countMess; i++)
    {
        // ID Für Real Nummern einfügen
        file << "\n" << "[" << X_Cord << ";" << Y_Cord << "] Real";
        // Daten für Real anfügen
        for (size_t i = 0; i < count; i++)
        {
            file << "," << dsR[i]; // Mehr dimisonales array einfügen und X-Y_Cord einfügen
        }
        
        // ID Für Imagh Zahlen einfügen
        file << "\n" << "[" << X_Cord << ";" << Y_Cord << "] Imag";
        for (size_t i = 0; i < count; i++)
        {
            file << "," << dsI[i];
        }
    }

    file.close();

    return true;
}
bool sData::openCsvFile(wxString& filename)
{
    wxLogDebug("open CSV");
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

    file.Close();
    return true;
}

bool sData::openCsvFileMultiline(wxString& filename)
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




wxString terminalTimestampOutput(wxString Text)
{
    //Set Terminal Output Format
    wxDateTime zeitJetzt = wxDateTime::Now();
    wxString timestamp = zeitJetzt.Format("%H:%M:%S");
    wxString FormatText = "[" + timestamp + "] " + Text;

    return FormatText;
}


std::vector<char> checkAscii(std::string input)
{
    const char* charInput = input.c_str();
    int DataSize = strlen(input.c_str());

    wxLogDebug("Length in function:");
    wxLogDebug("%s",std::to_string(DataSize));

    char* charInputBuffer = new char[input.length()+1];
    strcpy(charInputBuffer, charInput);
    //allocate output for max possible length
    char* charOutputBuffer = new char[input.length()*2 + 2];

    if (input.substr(0,2) == "++")  //Adapter Command
    {
        strcpy(charOutputBuffer,charInputBuffer);
        size_t BufferSize = strlen(charInputBuffer);
        charOutputBuffer[BufferSize] = '\n';
        charOutputBuffer[BufferSize + 1] = '\0';
        std::vector<char> vCharOutputAdptr(charOutputBuffer,charOutputBuffer + strlen(charOutputBuffer));

        wxLogDebug("Adapter Command: %s",std::string(vCharOutputAdptr.begin(),vCharOutputAdptr.end()));

        delete[] charInputBuffer;
        delete[] charOutputBuffer;
        //return adapter command
        return vCharOutputAdptr;
    }

    //for debuging
    wxString OgString;
    wxString ModString;

    int j = 0;

    for (int i=0;i < DataSize;i++)
    {
        switch(charInputBuffer[i])
        {
            case 10:
            case 13:
            case 27:
            case 43:
                charOutputBuffer[j] = {27};
                ModString = ModString + std::to_string(charOutputBuffer[j]) + " ";
                j++;

                break;
            default:

                break;
        }
        charOutputBuffer[j]= charInputBuffer[i];

        OgString = OgString + std::to_string(charInputBuffer[i]) + " ";
        ModString = ModString + std::to_string(charOutputBuffer[j]) + " ";

        j++;
    }
    //Add LF to end "nicht notwendig wenn ++eos 2 und eigenglich müsste ascii 27 angehängt werden"
    charOutputBuffer[j] = '\n';
    charOutputBuffer[j+1] = '\0';
    ModString = ModString + std::to_string(charOutputBuffer[j]) + " ";
    ModString = ModString + std::to_string(charOutputBuffer[j + 1]);

    wxLogDebug("Input str in ascii: %s", OgString);
    wxLogDebug("Output str in ascii: %s", ModString);

    std::vector<char> vCharOutputGpib(charOutputBuffer,charOutputBuffer + strlen(charOutputBuffer));

    delete[] charInputBuffer;
    delete[] charOutputBuffer;

    return vCharOutputGpib;
}

void sleepMs(int timeMs)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(timeMs));
}
