
#include "fkt_GPIB.h"
#include "cmdGpib.h"
#include "mainHelper.h"

//TODO Create Device Class and Create new read read and write funtions
PrologixUsbGpibAdapter::PrologixUsbGpibAdapter()
{

}

PrologixUsbGpibAdapter::~PrologixUsbGpibAdapter()
{
    if (m_deviceInfo.Connected)
    {
        disconnect();
    }
}

std::string PrologixUsbGpibAdapter::read(unsigned int forceReadBytes)
{
    std::string Text;

    if (m_deviceInfo.Connected)
    {
        std::vector<char> BigBuffer;
        DWORD BufferSize;
        FT_STATUS ftStatus;

        std::cerr << "Reading from Device..." << std::endl;

        ftStatus = readUsbDev(m_deviceInfo.ftHandle, BigBuffer, BufferSize, forceReadBytes);

        if (ftStatus == FT_OK)
        {
            m_deviceInfo.lastMsgReceived = std::string(BigBuffer.data(),BigBuffer.size());
            Text = m_deviceInfo.lastMsgReceived + "\n";

            if (BigBuffer.size() == 0)
            {
                Text = "No Message to Read\n";
            }
        }
        else
        {
            Text = "Failed to Receive Data - TimeOut after 5s\n";
            m_deviceInfo.Connected = false;
        }
    }
    else
    {
        std::cerr << "No Device to send too" << std::endl;
        Text = "Failed to Connect to a Device\n";
        m_deviceInfo.Connected = false;
    }

    return Text;
}

std::string PrologixUsbGpibAdapter::write(std::string msg)
{
    std::string Text;

    std::cerr << "Write Command Entered" << std::endl;

    if (m_deviceInfo.Connected)
    {
        DWORD bytesWritten;
        wxString GPIBText = msg;
        std::string CheckText(GPIBText.ToUTF8());
        //Check String if Adapter or GPIB Command and check for ASCII 10, 13, 27, 43
        std::vector<char> charArrWriteGpib = checkAscii(CheckText);

        std::cerr << "Trying to write to Device... " << std::string(charArrWriteGpib.begin(),charArrWriteGpib.end()) << std::endl;

        FT_STATUS ftStatus =writeUsbDev(m_deviceInfo.ftHandle, charArrWriteGpib, bytesWritten);

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
        std::cerr << "No Connection" << std::endl;
        Text = "Failed to Connect\n";
    }
    return Text;
}

std::string PrologixUsbGpibAdapter::send(std::string msg, int DelayMs)
{
    write(msg);
    checkIfMsgAvailable(DelayMs);
    quaryBuffer();
    return read();
}

bool PrologixUsbGpibAdapter::checkIfMsgAvailable(int TimeOutMs)
{
    int elapsedMs = 0;
    int pollIntervalMs = 10; // Abfrage-Intervall (verhindert 100% CPU-Last)

    while (elapsedMs < TimeOutMs) {
        // Status-Byte abfragen
        write("++spoll\n");
        quaryBuffer();
        std::string statusStr = read();

        try {
            int statusByte = std::stoi(statusStr);
            
            // check MAV-Bit (Message Available, Bit 4)
            if (statusByte & (1 << 4)) {
                break; // Msg available
            }
        } catch (...) {
            
        }

        sleepMs(pollIntervalMs);
        elapsedMs += pollIntervalMs;
    }

    return true;
}

DWORD PrologixUsbGpibAdapter::quaryBuffer()
{
    //Get Number of bytes to read from receive queue
    m_deviceInfo.ftStatus = FT_GetQueueStatus(m_deviceInfo.ftHandle,&m_deviceInfo.BytesToRead);
    std::cerr << "Bytes in Queue: " << m_deviceInfo.BytesToRead << std::endl;
    printErrD2XX(m_deviceInfo.ftStatus,"Failed to Get Queue Status");

    return m_deviceInfo.BytesToRead;
}

void PrologixUsbGpibAdapter::connect()
{
    if (!m_deviceInfo.Connected)
    {
        m_deviceInfo.ftStatus = FT_Open(m_deviceInfo.numDev,&m_deviceInfo.ftHandle);
        printErrD2XX(m_deviceInfo.ftStatus,"Failed to Connect");
        write("SYST:DISP:UDP ON"); //Turn on monitor

        if (m_deviceInfo.ftStatus == FT_OK)
        {
            std::cerr << "Connected to " << m_deviceInfo.numDev << std::endl;
            m_deviceInfo.Connected = true;
        }
    }
}
void PrologixUsbGpibAdapter::disconnect()
{
    write(ProLogixCmdLookup.at(ProLogixCmd::AUTO) + " 0");
    write(ScpiCmdLookup.at(ScpiCmd::CLR));
    write(ProLogixCmdLookup.at(ProLogixCmd::LOC));
    write(ProLogixCmdLookup.at(ProLogixCmd::IFC));

    sleepMs(200);


    m_deviceInfo.ftStatus = FT_Close(m_deviceInfo.ftHandle);
    printErrD2XX(m_deviceInfo.ftStatus,"Failed to Disconnect");
    if (m_deviceInfo.ftStatus == FT_OK)
    {
        std::cerr << "Connected to " << m_deviceInfo.numDev << std::endl;
        m_deviceInfo.Connected = false;
    }
}
void PrologixUsbGpibAdapter::config()
{
    //set Baudrate
    m_deviceInfo.ftStatus = FT_SetBaudRate(m_deviceInfo.ftHandle,m_deviceInfo.BaudRate);
    printErrD2XX(m_deviceInfo.ftStatus,"Failed to set Baudrate");

    m_deviceInfo.ftStatus = FT_SetDataCharacteristics(m_deviceInfo.ftHandle, FT_BITS_8, FT_STOP_BITS_1, FT_PARITY_NONE);
    printErrD2XX(m_deviceInfo.ftStatus,"Failed to set Data Characteristics");

    m_deviceInfo.ftStatus = FT_SetFlowControl(m_deviceInfo.ftHandle, FT_FLOW_NONE, 0, 0);
    printErrD2XX(m_deviceInfo.ftStatus,"Failed to set flow Characteristics");

    m_deviceInfo.ftStatus =  FT_SetTimeouts(m_deviceInfo.ftHandle, 500,500);
    printErrD2XX(m_deviceInfo.ftStatus,"Failed to set TimeOut");

    std::cerr << "FT-Config complete" << std::endl;

    if (m_deviceInfo.ftStatus == FT_OK)
    {
        m_deviceInfo.configFin = true;
    }
    else
    {
        m_deviceInfo.configFin = false;
    }
}
void PrologixUsbGpibAdapter::readScriptFile(const wxString& dirPath, const wxString& fileName, wxArrayString& logAdapterReceived, const std::atomic<bool>* stopFlag)
{
    wxTextFile textFile;

    if (textFile.Open(dirPath + fileName))
    {
        if (!getConnected())
        {
            connect();
            config();
        }

        for (size_t i = 0; i < textFile.GetLineCount(); i++)
        {
            // Check for stop signal from calling thread
            if (stopFlag && *stopFlag)
            {
                std::cout << "[GPIB] Measurement stopped by user" << std::endl;
                break;
            }

            wxString line = textFile.GetLine(i);
            if(line.IsEmpty())
            {
                std::cerr << "line " << i << ": Empty" << std::endl;
            }
            else if (line.substr(0,1) == "#")
            {
                std::cerr << "line " << i << ": Kommentar: " << line.substr(1) << std::endl;
            }
            else if (line.substr(0,5) == "wait ")
            {
                int wait;
                wxString strWait = line.substr(5);
                if (strWait.ToInt(&wait))
                {
                    std::cerr << "wait for " << wait << "ms" << std::endl;
                    sleepMs(wait);
                }
                else
                {
                    std::cerr << "Invalid wait Time input: " << strWait << std::endl;
                }
            }
            else if (line.substr(0,5) == "send ")
            {
                std::cerr << "line " << i << ": manuell send: " << line << std::endl;
                line = line.substr(5);
                logAdapterReceived.Add(send(std::string(line.ToUTF8())));
                std::cerr << "responce: " << logAdapterReceived.Last() << std::endl;
            }
            else if (line.substr(0,6) == "write ")
            {
                std::cerr << "line " << i << ": manuell write: " << line << std::endl;
                line = line.substr(6);
                write(std::string(line.ToUTF8()));
            }
            else if (line.substr(0,4) == "read")
            {
                std::cerr << "line " << i << ": manuell read" << std::endl;
                logAdapterReceived.Add(read());
                std::cerr << "responce: " << logAdapterReceived.Last() << std::endl;
            }
            else if(line.Contains("?") && line.substr(0,4) == "TRAC")
            {
                std::vector<double> bufferReal;
                std::vector<double> bufferImag;
                std::cerr << "line " << i << ": send: " << line << std::endl;

                write(std::string(line.ToUTF8()));
                write(ProLogixCmdLookup.at(ProLogixCmd::READ) + " eoi");
                sleepMs(300); // TODO change logic to be more efficent
                logAdapterReceived.Add(read());
                std::cerr << "responce: " << logAdapterReceived.Last() << std::endl;
                fsuMeasurement::get_instance().seperateDataBlock(logAdapterReceived.Last(), bufferReal, bufferImag);
                fsuMeasurement::get_instance().setX_Data(bufferReal);
                fsuMeasurement::get_instance().setY_Data(bufferImag);
                fsuMeasurement::get_instance().setFreqStartEnd(75'000'000,125'000'000);
                //Messung.calcYdata(); //start und end frequenz angeben

            }
            else if(line.Contains("?"))
            {
                std::cerr << "line " << i << ": send: " << line << std::endl;
                logAdapterReceived.Add(send(std::string(line.ToUTF8())));
                std::cerr << "responce: " << logAdapterReceived.Last() << std::endl;
            }
            else
            {
                std::cerr << "line " << i << ": write: " << line << std::endl;
                write(std::string(line.ToUTF8()));
            }
        }
    }
}

std::string PrologixUsbGpibAdapter::statusText()
{
    std::string Text;

    if (m_deviceInfo.Connected)
    {
        Text = "Connected to a Device ";
    }
    else
    {
        Text = "Not connected to a device";
    }
    if (m_deviceInfo.Connected && m_deviceInfo.configFin)
    {
        Text = Text + " and ";
    }
    if (m_deviceInfo.configFin)
    {
        Text = Text + "Device config set Baudrate to: " + std::to_string(m_deviceInfo.BaudRate);
    }

    Text += "\nLast Status code:" + wxString(statusString(m_deviceInfo.ftStatus)) + "\n";

    return Text;
}

FT_STATUS PrologixUsbGpibAdapter::getStatus()
{
    return m_deviceInfo.ftStatus;
}

FT_HANDLE PrologixUsbGpibAdapter::getHandle()
{
    return m_deviceInfo.ftHandle;
}

bool PrologixUsbGpibAdapter::getConnected()
{
    return m_deviceInfo.Connected;
}

std::string PrologixUsbGpibAdapter::getLastMsgReseived()
{
    return m_deviceInfo.lastMsgReceived;
}

int PrologixUsbGpibAdapter::getBaudrate()
{
    return m_deviceInfo.BaudRate;
}

void PrologixUsbGpibAdapter::setBaudrate(int BaudrateNew)
{
    m_deviceInfo.BaudRate = BaudrateNew;
}

std::vector<char> PrologixUsbGpibAdapter::checkAscii(std::string input)
{
    const char* charInput = input.c_str();
    int DataSize = strlen(input.c_str());

    std::cerr << "Length in function:" << std::endl;
    std::cerr << std::to_string(DataSize) << std::endl;

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

        std::cerr << "Adapter Command: " << std::string(vCharOutputAdptr.begin(),vCharOutputAdptr.end()) << std::endl;

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

    std::cerr << "Input str in ascii: " << OgString << std::endl;
    std::cerr << "Output str in ascii: " << ModString << std::endl;

    std::vector<char> vCharOutputGpib(charOutputBuffer,charOutputBuffer + strlen(charOutputBuffer));

    delete[] charInputBuffer;
    delete[] charOutputBuffer;

    return vCharOutputGpib;
}

void prepareFTDIDevice() {
    std::cout << "Prüfe auf aktive VCP-Treiber (ftdi_sio)..." << std::endl;

    // check if module is loaded (lsmod | grep)
    // 0 means found
    if (std::system("lsmod | grep ftdi_sio > /dev/null") == 0) {
        std::cout << "Treiber ftdi_sio gefunden. Versuche zu entladen..." << std::endl;

        // unload module
        int res1 = std::system("sudo rmmod ftdi_sio");
        int res2 = std::system("sudo rmmod usbserial");

        if (res1 == 0) {
            std::cout << "VCP-Treiber erfolgreich entladen." << std::endl;
        } else {
            std::cerr << "Fehler beim Entladen. Wurde das Passwort eingegeben?" << std::endl;
        }
    } else {
        std::cout << "Keine blockierenden Treiber aktiv. D2XX Zugriff bereit." << std::endl;
    }
}

void PrologixUsbGpibAdapter::checkForGpibBusError(wxWindow* parent)
{
    if (!getConnected()) return;
    // Check if SRQ-Line (Service Request) is activ
    
    std::string srqStatus = send("++srq");

    //  "1" signals Service Request
    if (srqStatus.find("1") != std::string::npos) {
        
        // Serial Poll  (R&S Analysator) 
        std::string statusStr = send("++spoll");
        try {
            int statusByte = std::stoi(statusStr);

            // Event Status Bit (Bit 5 / Dec 32)
            if (statusByte & (1 << 5)) {
                
                // check error msg of R&S device
                std::string errorMsg = send("SYST:ERR?");

                // 5. wxWidgets warning window popup
                wxMessageBox(
                    wxString::Format("Spektrumanalysator meldet einen Fehler:\n%s", errorMsg), 
                    "Gerätefehler", 
                    wxOK | wxICON_WARNING | wxCENTRE, 
                    parent
                );
            }
        } catch (const std::exception& e) {
            std::cerr << "Fehler beim Auslesen des Status-Bytes: " << e.what() << std::endl;
            wxMessageBox("Fehler beim Auslesen des Status-Bytes.", "Kommunikationsfehler", wxOK | wxICON_ERROR, parent);
        }
    }
}

//------fsuMesurement Beginn-----

fsuMeasurement::fsuMeasurement()
{
    /*
    m_x_Data = {0};
    m_y_Data = {0};

    m_FreqStart = 75'000'000;
    m_FreqEnd = 125'000'000;

    m_NoPoints_x = 0;
    m_NoPoints_y = 0;
    */
}

fsuMeasurement::~fsuMeasurement()
{
}

bool fsuMeasurement::executeMeasurement(int TimeOutMs)
{
    // TODO Auf enums anpassen
    auto& adapter = PrologixUsbGpibAdapter::get_instance();

    // Setup adapter settings for measurement
    adapter.write("++mode 1");
    adapter.write("++auto 0");
    adapter.write("++eos 2");
    
    std::string commaSeparatedValues;

    switch (m_lastMeasurementMode)
    {
    case MeasurementMode::SWEEP:
        adapter.write("INIT:CONT OFF"); // turn of continous measurement
        adapter.write("INIT:IMM");      // trigger measurement
        adapter.write("*WAI");          // wait for measurement to finish
        adapter.write("TRAC? TRACE1");
        if (adapter.checkIfMsgAvailable(TimeOutMs))
            commaSeparatedValues = adapter.send("++read eoi");

        break;
    
    case MeasurementMode::IQ:
        adapter.write("TRAC:IQ:STAT ON"); 
        adapter.write("TRAC:IQ:SET NORM, 10MHz, 1024, FREE, POS, 0, 0");      // TODO auf andere Trigger anpassbar machen
        adapter.write("*WAI");          // wait for measurement to finish
        adapter.write("TRAC:IQ:DATA?");
        if (adapter.checkIfMsgAvailable(TimeOutMs))
            commaSeparatedValues = adapter.send("++read eoi");

        break;
    
    case MeasurementMode::MARKER_PEAK:
        adapter.write("INIT:CONT OFF"); // turn of continous measurement
        adapter.write("INIT:IMM");      // trigger measurement
        adapter.write("*WAI");          // wait for measurement to finish
        adapter.write("CALC:MARK1:MAX");        // TODO make type of marker selectable
        commaSeparatedValues = adapter.send("CALC:MARK1:X?"); 
        commaSeparatedValues += ",";
        commaSeparatedValues += adapter.send("CALC:MARK1:Y?");  // Save x and y values

        break;

    default:
        return false;
        break;
    }

    return true;
}

void fsuMeasurement::seperateDataBlock(const wxString& receivedString, 
                                        std::vector<double>& Real, std::vector<double>& Imag)
{
    wxArrayString seperatedStrings = wxStringTokenize(receivedString, ",");

    double value;
    Real.clear(); // empty vectors
    Imag.clear();

    for (size_t i = 0; i < seperatedStrings.Count(); i++)
    {
        wxString data = seperatedStrings[i];

        if (data.ToCDouble(&value))
        {
            if (m_lastMeasurementMode == MeasurementMode::IQ)
            {
                // I/Q every other value alternates between the two
                if (i % 2 == 0) {
                    Real.push_back(value); // Realanteil (I)
                } else {
                    Imag.push_back(value); // Imaginäranteil (Q)
                }
            }
            else
            {
                // Normal Sweep
                Real.push_back(value);
            }
        }
        else
        {
            wxLogError("Konvertierungsfehler bei Wert %zu: %s", i, data);
        }
    }
    
    // Debug Output
    std::cerr << "Daten verarbeitet. X-Groesse: " << Real.size() 
              << " Y-Groesse: " << Imag.size() << std::endl;

}
std::vector<double> fsuMeasurement::calcFreqData()
{
    int totalPoints = m_x_Data.size();
    m_NoPoints_x = totalPoints;
    std::vector<double> freqRange;
    freqRange.clear();

    std::cerr << "Total points: " << totalPoints << std::endl;
    double range = m_FreqEnd-m_FreqStart;

    double step = range/totalPoints;
    std::cerr << "Range: " << range << "   Step: " << step << std::endl;
    double newYPoint = m_FreqStart;

    for(int i = 0; i < totalPoints; i++)
    {
        newYPoint = newYPoint + step;
        freqRange.push_back(newYPoint);
        std::cerr << "Y Berechnet: " << freqRange[i] << std::endl;
    }

    return freqRange;
}
void fsuMeasurement::setFreqStartEnd(double FreqS, double FreqE)
{
    m_FreqStart = FreqS;
    m_FreqEnd = FreqE;
}

bool fsuMeasurement::checkIfSettingsValidSweep(ScpiCommand command, const SettingValue& value)
{
try {
    switch (command) 
    {
        case ScpiCommand::START_FREQUENCY:
        case ScpiCommand::END_FREQUENCY: {
            double freq = std::get<double>(value);
            // Beispiel: R&S FSU26 geht bis 26.5 GHz
            return (freq >= 0.0 && freq <= 26.5e9);
        }

        case ScpiCommand::REF_LEVEL: {
            double level = std::get<double>(value);
            return (level >= -130.0 && level <= 30.0);
        }

        case ScpiCommand::RF_ATTENUATION: {
            int att = std::get<int>(value);
            // Dämpfung meist in 5dB Schritten bis 75dB
            return (att >= 0 && att <= 75 && (att % 5 == 0));
        }

        case ScpiCommand::RBW:
        case ScpiCommand::VBW: {
            double bw = std::get<double>(value);
            // Bandbreiten meist 1Hz bis 50MHz (FSU)
            return (bw >= 1.0 && bw <= 50.0e6);
        }

        case ScpiCommand::SWEEP_POINTS: {
                int points = std::get<int>(value);
                
                // Definierte zulässige Festwerte für R&S FSU
                static const std::set<int> allowedPoints = {
                    155, 313, 625, 1251, 1999, 2501, 5001, 10001, 20001, 30001
                };

                // Prüfen, ob der eingegebene Wert exakt in der Liste enthalten ist
                return allowedPoints.find(points) != allowedPoints.end();
            }

        case ScpiCommand::DETECTOR: {
            std::string det = std::get<std::string>(value);
            // Erlaubte SCPI Parameter für Detektoren
            return (det == "APE" || det == "POS" || det == "NEG" || 
                    det == "SAMP" || det == "RMS" || det == "AVER");
        }

        case ScpiCommand::AMPLITUDE_UNIT: {
            std::string unit = std::get<std::string>(value);
            return (unit == "DBM" || unit == "V" || unit == "W" || unit == "DBUV");
        }

        case ScpiCommand::CENTER_FREQUENCY:
        case ScpiCommand::SPAN_FREQUENCY: {
            double freq = std::get<double>(value);
            return (freq >= 0.0 && freq <= 26.5e9);
        }

        case ScpiCommand::IQ_SAMPLE_RATE: {
            double rate = std::get<double>(value);
            // 10 kHz bis 32 MHz (mit Option B70 bis 70.4 MHz)
            return (rate >= 10.0e3 && rate <= 70.4e6);
        }

        case ScpiCommand::IQ_RECORD_LENGTH: {
            int len = std::get<int>(value);
            // 1 bis 16M Samples (mit Option B70)
            return (len >= 1 && len <= 16'000'000);
        }

        case ScpiCommand::IQ_IF_BANDWIDTH: {
            double bw = std::get<double>(value);
            // 10 Hz bis 50 MHz (mit Erweiterung)
            return (bw >= 10.0 && bw <= 50.0e6);
        }

        case ScpiCommand::TRIGGER_SOURCE: {
            std::string src = std::get<std::string>(value);
            return (src == "IMM" || src == "EXT" || src == "IFP");
        }

        case ScpiCommand::TRIGGER_LEVEL: {
            double lvl = std::get<double>(value);
            return (lvl >= -130.0 && lvl <= 30.0);
        }

        case ScpiCommand::TRIGGER_DELAY: {
            double del = std::get<double>(value);
            return (del >= -1.0 && del <= 65.0);
        }

        default:
            return false;
        }
    } 
    catch (const std::bad_variant_access&) 
    {
        // Falscher Datentyp für diesen Befehl übergeben
        return false;
    }
}

bool fsuMeasurement::writeSweepSettings(lastSweepSettings settings)
{
   std::string blockCmd = scpiSetCommands.at(ScpiCommand::START_FREQUENCY   )   + std::to_string(settings.startFreq)+ ";:" +
                            scpiSetCommands.at(ScpiCommand::END_FREQUENCY )   + std::to_string(settings.stopFreq) + ";:" +
                            scpiSetCommands.at(ScpiCommand::REF_LEVEL       )   + std::to_string(settings.refLevel) + ";:" +      
                            scpiSetCommands.at(ScpiCommand::RF_ATTENUATION  )   + std::to_string(settings.att)      + ";:" +
                            scpiSetCommands.at(ScpiCommand::AMPLITUDE_UNIT  )   + settings.unit                     + ";:" +
                            scpiSetCommands.at(ScpiCommand::RBW             )   + std::to_string(settings.rbw)      + ";:" +
                            scpiSetCommands.at(ScpiCommand::VBW             )   + std::to_string(settings.vbw)      + ";:" +
                            scpiSetCommands.at(ScpiCommand::SWEEP_TIME      )   + settings.sweepTime                + ";:" +
                            scpiSetCommands.at(ScpiCommand::SWEEP_POINTS    )   + std::to_string(settings.points)   + ";:" +
                            scpiSetCommands.at(ScpiCommand::DETECTOR        )   + settings.detector                 + ";";
         
    std::string status = PrologixUsbGpibAdapter::get_instance().write(blockCmd);

    if (status.substr(0,2) == "Msg")
    {
        return true;
    }
    else
    {
        return false;
    }
}
bool fsuMeasurement::readSweepSettings()
{
    std::string queryCmd = scpiQueryCommands.at(ScpiCommand::START_FREQUENCY) + ";:" +
                           scpiQueryCommands.at(ScpiCommand::END_FREQUENCY)   + ";:" +
                           scpiQueryCommands.at(ScpiCommand::REF_LEVEL)       + ";:" +
                           scpiQueryCommands.at(ScpiCommand::RF_ATTENUATION)  + ";:" +
                           scpiQueryCommands.at(ScpiCommand::AMPLITUDE_UNIT)  + ";:" +
                           scpiQueryCommands.at(ScpiCommand::RBW)             + ";:" +
                           scpiQueryCommands.at(ScpiCommand::VBW)             + ";:" +
                           scpiQueryCommands.at(ScpiCommand::SWEEP_TIME)      + ";:" +
                           scpiQueryCommands.at(ScpiCommand::SWEEP_POINTS)    + ";:" +
                           scpiQueryCommands.at(ScpiCommand::DETECTOR)                  ;

    // send commands and read responce
    auto& adapter = PrologixUsbGpibAdapter::get_instance();
    // TODO Testen ob send mit verzögerung notwendig ist.
    adapter.write(queryCmd);
    std::string response = adapter.read(); 
    
    if (response.empty()) return false;

    // split strings
    std::vector<std::string> tokens;
    std::stringstream ss(response);
    std::string token;
    
    while (std::getline(ss, token, ';')) {
        // Steuerzeichen am Ende (wie \r oder \n) entfernen
        token.erase(token.find_last_not_of(" \n\r\t") + 1);
        tokens.push_back(token);
    }

    // check if 10 strings where received
    if (tokens.size() < 10) return false;

    // save to member struct
    try {
        m_lastSwpSettings.startFreq = std::stod(tokens[0]);
        m_lastSwpSettings.stopFreq  = std::stod(tokens[1]);
        m_lastSwpSettings.refLevel  = std::stod(tokens[2]);
        m_lastSwpSettings.att       = std::stoi(tokens[3]);
        m_lastSwpSettings.unit      = tokens[4];                // std::string
        m_lastSwpSettings.rbw       = std::stod(tokens[5]);
        m_lastSwpSettings.vbw       = std::stod(tokens[6]);
        m_lastSwpSettings.sweepTime = tokens[7];                // std::string
        m_lastSwpSettings.points    = std::stoi(tokens[8]);
        m_lastSwpSettings.detector  = tokens[9];                // std::string
        
        return true;
    } 
    catch (const std::exception& e) {
        std::cerr << "Exception read Sweep Settings: " << e.what() << std::endl;
        return false;
    }
}

bool fsuMeasurement::writeIqSettings(IqSettings settings)
{
    std::string blockCmd = scpiSetCommands.at(ScpiCommand::CENTER_FREQUENCY)  + std::to_string(settings.centerFreq)   + ";:" +
                           scpiSetCommands.at(ScpiCommand::REF_LEVEL)         + std::to_string(settings.refLevel)     + ";:" +
                           scpiSetCommands.at(ScpiCommand::RF_ATTENUATION)    + std::to_string(settings.att)          + ";:" +
                           scpiSetCommands.at(ScpiCommand::AMPLITUDE_UNIT)    + settings.unit                         + ";:" +
                           scpiSetCommands.at(ScpiCommand::IQ_SAMPLE_RATE)    + std::to_string(settings.sampleRate)   + ";:" +
                           scpiSetCommands.at(ScpiCommand::IQ_RECORD_LENGTH)  + std::to_string(settings.recordLength) + ";:" +
                           scpiSetCommands.at(ScpiCommand::IQ_IF_BANDWIDTH)   + std::to_string(settings.ifBandwidth)  + ";:" +
                           //scpiSetCommands.at(ScpiCommand::TRIGGER_SOURCE)    + settings.triggerSource                + ";:" +
                           scpiSetCommands.at(ScpiCommand::TRIGGER_LEVEL)     + std::to_string(settings.triggerLevel) + ";:" +
                           scpiSetCommands.at(ScpiCommand::TRIGGER_DELAY)     + std::to_string(settings.triggerDelay) + ";";

    std::string status = PrologixUsbGpibAdapter::get_instance().write(blockCmd);
    return (status.substr(0, 3) == "Msg");
}

bool fsuMeasurement::readIqSettings()
{
    std::string queryCmd = scpiQueryCommands.at(ScpiCommand::CENTER_FREQUENCY) + ";:" +
                           scpiQueryCommands.at(ScpiCommand::REF_LEVEL)        + ";:" +
                           scpiQueryCommands.at(ScpiCommand::RF_ATTENUATION)   + ";:" +
                           scpiQueryCommands.at(ScpiCommand::AMPLITUDE_UNIT)   + ";:" +
                           scpiQueryCommands.at(ScpiCommand::IQ_SAMPLE_RATE)   + ";:" +
                           scpiQueryCommands.at(ScpiCommand::IQ_RECORD_LENGTH) + ";:" +
                           scpiQueryCommands.at(ScpiCommand::IQ_IF_BANDWIDTH)  + ";:" +
                           //scpiQueryCommands.at(ScpiCommand::TRIGGER_SOURCE)   + ";:" +
                           scpiQueryCommands.at(ScpiCommand::TRIGGER_LEVEL)    + ";:" +
                           scpiQueryCommands.at(ScpiCommand::TRIGGER_DELAY);

    auto& adapter = PrologixUsbGpibAdapter::get_instance();
    adapter.write(queryCmd);
    std::string response = adapter.read();

    if (response.empty()) return false;

    std::vector<std::string> tokens;
    std::stringstream ss(response);
    std::string token;
    while (std::getline(ss, token, ';')) {
        token.erase(token.find_last_not_of(" \n\r\t") + 1);
        tokens.push_back(token);
    }

    if (tokens.size() < 10) return false;

    try {
        m_lastIqSettings.centerFreq    = std::stod(tokens[0]);
        m_lastIqSettings.refLevel      = std::stod(tokens[1]);
        m_lastIqSettings.att           = std::stoi(tokens[2]);
        m_lastIqSettings.unit          = tokens[3];
        m_lastIqSettings.sampleRate    = std::stod(tokens[4]);
        m_lastIqSettings.recordLength  = std::stoi(tokens[5]);
        m_lastIqSettings.ifBandwidth   = std::stod(tokens[6]);
        m_lastIqSettings.triggerSource = tokens[7];
        m_lastIqSettings.triggerLevel  = std::stod(tokens[8]);
        m_lastIqSettings.triggerDelay  = std::stod(tokens[9]);
        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "Exception read IQ Settings: " << e.what() << std::endl;
        return false;
    }
}

bool fsuMeasurement::writeMarkerPeakSettings(MarkerPeakSettings settings)
{
    std::string blockCmd = scpiSetCommands.at(ScpiCommand::START_FREQUENCY)  + std::to_string(settings.startFreq) + ";:" +
                           scpiSetCommands.at(ScpiCommand::END_FREQUENCY)    + std::to_string(settings.stopFreq)  + ";:" +
                           scpiSetCommands.at(ScpiCommand::REF_LEVEL)        + std::to_string(settings.refLevel)  + ";:" +
                           scpiSetCommands.at(ScpiCommand::RF_ATTENUATION)   + std::to_string(settings.att)       + ";:" +
                           scpiSetCommands.at(ScpiCommand::AMPLITUDE_UNIT)   + settings.unit                      + ";:" +
                           scpiSetCommands.at(ScpiCommand::RBW)              + std::to_string(settings.rbw)       + ";:" +
                           scpiSetCommands.at(ScpiCommand::VBW)              + std::to_string(settings.vbw)       + ";:" +
                           scpiSetCommands.at(ScpiCommand::DETECTOR)         + settings.detector                  + ";";

    std::string status = PrologixUsbGpibAdapter::get_instance().write(blockCmd);

    return (status.substr(0, 3) == "Msg");
}

bool fsuMeasurement::readMarkerPeakSettings()
{
    std::string queryCmd = scpiQueryCommands.at(ScpiCommand::START_FREQUENCY) + ";:" +
                           scpiQueryCommands.at(ScpiCommand::END_FREQUENCY)   + ";:" +
                           scpiQueryCommands.at(ScpiCommand::REF_LEVEL)       + ";:" +
                           scpiQueryCommands.at(ScpiCommand::RF_ATTENUATION)  + ";:" +
                           scpiQueryCommands.at(ScpiCommand::AMPLITUDE_UNIT)  + ";:" +
                           scpiQueryCommands.at(ScpiCommand::RBW)             + ";:" +
                           scpiQueryCommands.at(ScpiCommand::VBW)             + ";:" +
                           scpiQueryCommands.at(ScpiCommand::DETECTOR);

    auto& adapter = PrologixUsbGpibAdapter::get_instance();
    adapter.write(queryCmd);
    std::string response = adapter.read();

    if (response.empty()) return false;

    std::vector<std::string> tokens;
    std::stringstream ss(response);
    std::string token;
    while (std::getline(ss, token, ';')) {
        token.erase(token.find_last_not_of(" \n\r\t") + 1);
        tokens.push_back(token);
    }

    if (tokens.size() < 8) return false;

    try {
        m_lastMarkerPeakSettings.startFreq = std::stod(tokens[0]);
        m_lastMarkerPeakSettings.stopFreq  = std::stod(tokens[1]);
        m_lastMarkerPeakSettings.refLevel  = std::stod(tokens[2]);
        m_lastMarkerPeakSettings.att       = std::stoi(tokens[3]);
        m_lastMarkerPeakSettings.unit      = tokens[4];
        m_lastMarkerPeakSettings.rbw       = std::stod(tokens[5]);
        m_lastMarkerPeakSettings.vbw       = std::stod(tokens[6]);
        m_lastMarkerPeakSettings.detector  = tokens[7];
        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "Exception read MarkerPeak Settings: " << e.what() << std::endl;
        return false;
    }
}

//------fsuMesurement Ende-----

// Helper functions



