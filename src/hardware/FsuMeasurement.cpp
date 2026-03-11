
#include "FsuMeasurement.h"
#include "cmdGpib.h"
#include "mainHelper.h"



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
    case MeasurementMode::COSTUM:
        adapter.readScriptFile(getFilePath(),getFileName());
        break;

    default:
        return false;
        break;
    }

    seperateDataBlock(commaSeparatedValues, m_x_Data, m_y_Data); // Separates the values and passes them to the internal data storage

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
                            scpiSetCommands.at(ScpiCommand::END_FREQUENCY   )   + std::to_string(settings.stopFreq) + ";:" +
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



