
#include "FsuMeasurement.h"
#include "cmdGpib.h"
#include "mainHelper.h"

#include <cctype>
#include <algorithm>



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
    int TimePassed = 0;
    auto& adapter = PrologixUsbGpibAdapter::get_instance();
    int ProzessingTimeMs = 20;
    int WaitTimeMs = 50;
    int checkTimeoutMs = 1000;

    // Setup adapter settings for measurement
    // clears old msgs
    
    adapter.write("++clr"); // Clear buffer to prevent old messages from interfering with new measurement data
    sleepMs(ProzessingTimeMs);
    adapter.write("++mode 1");
    adapter.write("++auto 0");
    adapter.write("++eos 2");

    std::string commaSeparatedValues;

    switch (m_lastMeasurementMode)
    {
    case MeasurementMode::SWEEP:
        adapter.write("INIT:CONT OFF"); // turn of continous measurement
        adapter.write("FORM ASC");
        adapter.write("INIT:IMM");      // trigger measurement
        adapter.write("*WAI");          // wait for measurement to finish
        adapter.write("TRAC? TRACE1");
        std::cout << "Sweep Measurement Triggered!" << std::endl;
        sleepMs(ProzessingTimeMs);

        checkTimeoutMs = estimateMeasurementTime(); // calc estimated max time
        if (!adapter.checkIfMsgAvailable(checkTimeoutMs))
        {
            setErrorMessage("Sweep Measurement Timeout waiting for MAV");
            return false;
        }

        adapter.write("++read eoi");

        while (adapter.quaryBuffer() < (0.8*m_lastSwpSettings.points * 18)) // 18 bytes per data point Wait for 80% of estimated data  
        { // 18 bytes per data point
            sleepMs(WaitTimeMs);
            TimePassed += WaitTimeMs;
            if (TimeOutMs <= TimePassed)
            {
                setErrorMessage("Sweep: timeout while receiving data");
                break;
            }
        }
        sleepMs(WaitTimeMs);
        commaSeparatedValues = adapter.read();
        //commaSeparatedValues = adapter.send("++read eoi", 3000);


        break;

    case MeasurementMode::IQ:
        adapter.write("INST:SEL SAN");
        adapter.write("TRAC:IQ:STAT ON");
        adapter.write("INIT:CONT OFF");
        adapter.write("INIT:IMM");
        adapter.write("*WAI");          // wait for measurement to finish
        adapter.write("TRAC:IQ:DATA?");
        std::cout << "IQ Measurement Triggered!" << std::endl;
        sleepMs(ProzessingTimeMs);

        checkTimeoutMs = estimateMeasurementTimeIQ(); // calc estimated max time
        if (!adapter.checkIfMsgAvailable(checkTimeoutMs))
        {
            setErrorMessage("IQ Measurement Timeout waiting for MAV");
            return false;
        }

        adapter.write("++read eoi");
        while (adapter.quaryBuffer() < (0.8 * m_lastIqSettings.recordLength * 18)) // 18 bytes per data point Ascii       
        {
            sleepMs(WaitTimeMs);
            TimePassed += WaitTimeMs;
            if (TimeOutMs <= TimePassed)
            {
                setErrorMessage("IQ: timeout while receiving data");
                break;
            }
  
        }
        sleepMs(WaitTimeMs);
        commaSeparatedValues = adapter.read();
        break;

    case MeasurementMode::MARKER_PEAK:
        //adapter.write("++auto 1");
        adapter.write("INIT:CONT OFF;INIT;*WAI");          // wait for measurement to finishCALC:MARK1:X?;Y?
        adapter.write("CALC:MARK1:ON");
        adapter.write("CALC:MARK1:MAX");        // TODO make type of marker selectable MIN / MAX
        std::cout << "Marker Measurement Triggered!" << std::endl;
        adapter.write("CALC:MARK1:X?;Y?");
        
        sleepMs(ProzessingTimeMs);
        checkTimeoutMs = estimateMeasurementTime(); // calc estimated max time
        if (!adapter.checkIfMsgAvailable(checkTimeoutMs))
        {
            setErrorMessage("Marker Measurement Timeout waiting for MAV");
            return false;
        }

        adapter.write("++read eoi");

        while (adapter.quaryBuffer() < 1) // 18 bytes per data point        
        {
            sleepMs(WaitTimeMs);
            TimePassed += WaitTimeMs;
            if (TimeOutMs <= TimePassed)
            {
                setErrorMessage("Marker: timeout while receiving data");
                break;
            }
        }
        commaSeparatedValues = adapter.read(); // Save x and y values

        break;
    case MeasurementMode::COSTUM:
        adapter.readScriptFile(getFilePath(),getFileName()); // TODO FIX Implementation
        std::cout << "Costum Measurement Triggered!" << std::endl;

        return true;
        break;

    default:
        return false;
        break;
    }

    adapter.write("INIT:CONT ON"); // turn on continous measurement
    adapter.write("++auto 1");

    seperateDataBlock(commaSeparatedValues, m_x_Data, m_y_Data); // Separates the values and passes them to the internal data storage

    std::cout << "finished execute Measurement!" << std::endl;

    return true;
}

void fsuMeasurement::seperateDataBlock(const wxString& receivedString,
                                        std::vector<double>& Real, std::vector<double>& Imag)
{
    // removes \n at the end of the msg
    wxString str = receivedString; //.AfterFirst('\n');

    str.Trim(true).Trim(false);

    // Responses can be comma-separated (sweep/IQ) or semicolon-separated (marker X;Y).
    wxArrayString seperatedStrings = wxStringTokenize(str, ",;");

    //std::cout << "seperated Data: " << seperatedStrings << std::endl;

    double value;
    Real.clear(); // empty vectors
    Imag.clear();

    for (size_t i = 0; i < seperatedStrings.Count(); i++)
    {
        wxString data = seperatedStrings[i];

        if (data.ToCDouble(&value))
        {
            if ((m_lastMeasurementMode == MeasurementMode::IQ) 
                || (m_lastMeasurementMode == MeasurementMode::MARKER_PEAK))
            {
                m_ImagValues = true;
                // I/Q every other value alternates between the two
                if (i % 2 == 0) {
                    Real.push_back(value); // Realanteil (I)
                } else {
                    Imag.push_back(value); // Imaginäranteil (Q)
                }
            }
            else
            {
                m_ImagValues = false;
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

    std::cout << "Total points: " << totalPoints << std::endl;
    double range = m_FreqEnd-m_FreqStart;

    double step = 0;
    if (totalPoints != 1)
    {
        step = range/(totalPoints-1);
    }
    
    std::cout << "Range: " << range << "   Step: " << step << std::endl;
    double newYPoint = m_FreqStart;

    for(int i = 0; i < totalPoints; i++)
    {
        freqRange.push_back(newYPoint);
        newYPoint = newYPoint + step;
        std::cout << "Y Berechnet: " << freqRange[i] << std::endl;
    }

    return freqRange;
}
void fsuMeasurement::setFreqStartEnd(unsigned int FreqS, unsigned int FreqE)
{
    m_FreqStart = FreqS;
    m_FreqEnd = FreqE;
}

bool fsuMeasurement::checkIfSettingsValidSweep(ScpiCommand command, const SettingValue& value)
{
try {
    switch (command)
    {
        case ScpiCommand::CENTER_FREQUENCY:
        case ScpiCommand::SPAN_FREQUENCY:
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
            return (src == "IMM" || src == "EXT" || src == "IFP" || src == "FREE");
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

bool fsuMeasurement::writeSettingsToGpib()
{


    switch (m_lastMeasurementMode)
    {
        case MeasurementMode::SWEEP:
            return writeSweepSettings(m_lastSwpSettings);

        case MeasurementMode::IQ:
            return writeIqSettings(m_lastIqSettings);

        case MeasurementMode::MARKER_PEAK:
            return writeMarkerPeakSettings(m_lastMarkerPeakSettings);

        case MeasurementMode::COSTUM:
            return true;

        default:
            return false;

    }
}

bool fsuMeasurement::readSettingsFromGpib()
{
    switch (m_lastMeasurementMode) {
        case MeasurementMode::SWEEP: {
            return readSweepSettings();
        }
        case MeasurementMode::IQ: {
            return readIqSettings();
        }
        case MeasurementMode::MARKER_PEAK: {
            return readMarkerPeakSettings();
        }

        case MeasurementMode::COSTUM:{
            return true;
        }
        default:
            return false;
    }
}


bool fsuMeasurement::writeSweepSettings(lastSweepSettings settings)
{
std::string blockCmd = scpiSetCommands.at(ScpiCommand::START_FREQUENCY   )   + std::format("{}",settings.startFreq)+ ";:" +
                        scpiSetCommands.at(ScpiCommand::END_FREQUENCY   )   + std::format("{}",settings.stopFreq) + ";:" +
                        scpiSetCommands.at(ScpiCommand::REF_LEVEL       )   + std::format("{}",settings.refLevel) + ";:" +
                        scpiSetCommands.at(ScpiCommand::RF_ATTENUATION  )   + std::to_string(settings.att)      + ";:" +
                        scpiSetCommands.at(ScpiCommand::AMPLITUDE_UNIT  )   + settings.unit                     + ";:" +
                        scpiSetCommands.at(ScpiCommand::RBW             )   + std::to_string(settings.rbw)      + ";:" +
                        scpiSetCommands.at(ScpiCommand::VBW             )   + std::to_string(settings.vbw)      + ";:" +
                        scpiSetCommands.at(ScpiCommand::SWEEP_TIME      )   + settings.sweepTime                + ";:" +
                        scpiSetCommands.at(ScpiCommand::SWEEP_POINTS    )   + std::to_string(settings.points)   + ";:" +
                        scpiSetCommands.at(ScpiCommand::DETECTOR        )   + settings.detector                 + ";";

    std::string status = PrologixUsbGpibAdapter::get_instance().write(blockCmd);

    std::cout << "written Sweep data: " << status << std::endl;

    if (status.substr(0,3) == "Msg")
    {
        std::cout << "write successfull!" << std::endl;
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

    std::cout << "read Sweep settings: " << queryCmd << std::endl;

    adapter.resetGpibBusBuffer();
    std::string response = adapter.send(queryCmd);
    std::cout << "response Sweep settings: " << response << std::endl;


    if (response.substr(0,6)== "Failed") return false;

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
        m_lastSwpSettings.rbw       = std::stoi(tokens[5]);
        m_lastSwpSettings.vbw       = std::stoi(tokens[6]);
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
    std::string blockCmd = scpiSetCommands.at(ScpiCommand::CENTER_FREQUENCY)  + std::format("{}",settings.centerFreq)   + ";:" +
                           scpiSetCommands.at(ScpiCommand::REF_LEVEL)         + std::format("{}",settings.refLevel)                     + ";:" +
                           scpiSetCommands.at(ScpiCommand::RF_ATTENUATION)    + std::to_string(settings.att)          + ";:" +
                           scpiSetCommands.at(ScpiCommand::AMPLITUDE_UNIT)    + settings.unit                         + ";:" +
                           "TRAC:IQ:SET " + settings.filterType + "," +
                           std::format("{}",settings.ifBandwidth) + "," +
                           std::format("{}",settings.sampleRate) + "," +
                           settings.triggerSource + "," +
                           settings.triggerSlope + "," +
                           std::to_string(settings.pretriggerSamples) + "," +
                           std::to_string(settings.recordLength) + ";:" +
                           scpiSetCommands.at(ScpiCommand::TRIGGER_LEVEL)     + std::format("{}",settings.triggerLevel) + ";:" +
                           scpiSetCommands.at(ScpiCommand::TRIGGER_DELAY)     + std::format("{}",settings.triggerDelay) + ";";

    std::string status = PrologixUsbGpibAdapter::get_instance().write(blockCmd);
    return (status.substr(0, 3) == "Msg");
}

bool fsuMeasurement::readIqSettings()
{
    std::string queryCmd = scpiQueryCommands.at(ScpiCommand::CENTER_FREQUENCY) + ";:" +
                           scpiQueryCommands.at(ScpiCommand::REF_LEVEL)        + ";:" +
                           scpiQueryCommands.at(ScpiCommand::RF_ATTENUATION)   + ";:" +
                           scpiQueryCommands.at(ScpiCommand::AMPLITUDE_UNIT);

    auto& adapter = PrologixUsbGpibAdapter::get_instance();

    adapter.resetGpibBusBuffer();
    std::string response = adapter.send(queryCmd);

    std::cout << "read IQ base settings: " << queryCmd << std::endl;
    std::cout << "response IQ base settings: " << response << std::endl;
    if (response.substr(0,6)== "Failed") return false;

    std::vector<std::string> tokens;
    std::stringstream ss(response);
    std::string token;
    while (std::getline(ss, token, ';')) {
        token.erase(token.find_last_not_of(" \n\r\t") + 1);
        tokens.push_back(token);
    }

    if (tokens.size() < 4) return false;

    adapter.resetGpibBusBuffer();
    std::string iqSetResponse = adapter.send("TRAC:IQ:SET?");
    std::cout << "response IQ set settings: " << iqSetResponse << std::endl;
    if (iqSetResponse.substr(0,6)== "Failed") return false;

    std::vector<std::string> iqSetTokens;
    std::stringstream ssIq(iqSetResponse);
    while (std::getline(ssIq, token, ',')) {
        token.erase(token.find_last_not_of(" \n\r\t") + 1);
        token.erase(0, token.find_first_not_of(" \n\r\t"));
        iqSetTokens.push_back(token);
    }

    if (iqSetTokens.size() < 7) return false;

    try {
        m_lastIqSettings.centerFreq    = std::stod(tokens[0]);
        m_lastIqSettings.refLevel      = std::stod(tokens[1]);
        m_lastIqSettings.att           = std::stoi(tokens[2]);
        m_lastIqSettings.unit          = tokens[3];

        // TRAC:IQ:SET? -> <FilterType>,<RBW>,<SampleRate>,<TriggerSource>,<TriggerSlope>,<PretriggerSamples>,<NumberofSamples>
        m_lastIqSettings.filterType       = iqSetTokens[0];
        m_lastIqSettings.ifBandwidth      = std::stod(iqSetTokens[1]);
        m_lastIqSettings.sampleRate       = std::stod(iqSetTokens[2]);
        m_lastIqSettings.triggerSource    = iqSetTokens[3];
        m_lastIqSettings.triggerSlope     = iqSetTokens[4];
        m_lastIqSettings.pretriggerSamples= std::stoi(iqSetTokens[5]);
        m_lastIqSettings.recordLength     = std::stoi(iqSetTokens[6]);
        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "Exception read IQ Settings: " << e.what() << std::endl;
        return false;
    }
}

bool fsuMeasurement::writeMarkerPeakSettings(MarkerPeakSettings settings)
{
    std::string blockCmd = scpiSetCommands.at(ScpiCommand::START_FREQUENCY)  + std::format("{}",settings.startFreq) + ";:" +
                           scpiSetCommands.at(ScpiCommand::END_FREQUENCY)    + std::format("{}",settings.stopFreq)  + ";:" +
                           scpiSetCommands.at(ScpiCommand::REF_LEVEL)        + std::format("{}",settings.refLevel)  + ";:" +
                           scpiSetCommands.at(ScpiCommand::RF_ATTENUATION)   + std::format("{}",settings.att)       + ";:" +
                           scpiSetCommands.at(ScpiCommand::AMPLITUDE_UNIT)   + settings.unit                      + ";:" +
                           scpiSetCommands.at(ScpiCommand::RBW)              + std::format("{}",settings.rbw)       + ";:" +
                           scpiSetCommands.at(ScpiCommand::VBW)              + std::format("{}",settings.vbw)       + ";:" +
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

    adapter.resetGpibBusBuffer();
    std::string response = adapter.send(queryCmd);

    std::cout << "read Marker settings: " << queryCmd << std::endl;

    std::cout << "responce Marker settings: " << response << std::endl;

    if (response.substr(0,6)== "Failed") return false;

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

void fsuMeasurement::setErrorMessage(std::string error)
{
    m_lastError = error;
    std::cerr << terminalTimestampOutput << m_lastError << std::endl;
}

// estimate Time sweep will take for different settings
int fsuMeasurement::estimateMeasurementTime()
{
    double  startFreqHz =   m_lastSwpSettings.startFreq;
    double  stopFreqHz  = m_lastSwpSettings.stopFreq;
    double  rbwHz       = m_lastSwpSettings.rbw;
    double  vbwHz       = m_lastSwpSettings.vbw;
    int     points      =  m_lastSwpSettings.points;

    const SweepTimeoutRef& ref = m_sweepTimeoutRef;

    double safetyFactor = 1.4;
    int offsetMs        = 300;               // add extra saft time
    int minMs           = 500;                  // lower limit
    int maxMs           = 120000;                // upper limit

    // prevent too small values (std::max returns the bigger of two values)
    const double spanHz = std::max(1.0, std::abs(stopFreqHz - startFreqHz));
    const double rbw = std::max(1.0, rbwHz);
    const double vbw = std::max(1.0, vbwHz);
    const int nPts = std::max(1, points);

    // Ref values
    const double spanRef = std::max(1.0, ref.spanHzRef);
    const double rbwRef  = std::max(1.0, ref.rbwHzRef);
    const double vbwRef  = std::max(1.0, ref.vbwHzRef);
    const int ptsRef     = std::max(1, ref.pointsRef);
    const int tRefMs     = std::max(1, ref.checkTimeMsRef);

    // Factors
    const double fSpan   = spanHz / spanRef;
    const double fRbw    = (rbwRef / rbw) * (rbwRef / rbw);      // ~1/RBW^2
    const double fVbw    = std::max(1.0, vbwRef / vbw);          // only when vbw is smaller
    const double fPoints = static_cast<double>(nPts) / static_cast<double>(ptsRef);

    const double tMs =
        offsetMs +
        safetyFactor * static_cast<double>(tRefMs) * fSpan * fRbw * fVbw * fPoints;

    const int timeoutMs = static_cast<int>(std::ceil(tMs));
    return std::clamp(timeoutMs, minMs, maxMs);
}

int fsuMeasurement::estimateMeasurementTimeIQ()
{
    int recordLength        = m_lastIqSettings.recordLength;
    double sampleRateHz     = m_lastIqSettings.sampleRate;
    double ifBandwidthHz    = m_lastIqSettings.ifBandwidth;

    const IqTimeoutRef& ref = m_iqTimeoutRef;

    double safetyFactor = 1.5;
    int offsetMs        = 300;
    int minMs           = 500;
    int maxMs           = 120'000; 



    const int n     = std::max(1, recordLength);
    const double fs = std::max(1.0, sampleRateHz);
    const double bw = std::max(1.0, ifBandwidthHz);

    const int nRef      = std::max(1, ref.recordLengthRef);
    const double fsRef  = std::max(1.0, ref.sampleRateHzRef);
    const double bwRef  = std::max(1.0, ref.ifBandwidthHzRef);
    const int tRef      = std::max(1, ref.checkTimeMsRef);

    const double fCapture = (static_cast<double>(n) / fs) / (static_cast<double>(nRef) / fsRef);
    const double fIfBw = std::max(1.0, bwRef / bw);

    const double tMs = offsetMs + safetyFactor * static_cast<double>(tRef) * fCapture * fIfBw;

    const int timeoutMs = static_cast<int>(std::ceil(tMs));

    return std::clamp(timeoutMs, minMs, maxMs);
}
//------fsuMesurement End-----

// Helper functions



