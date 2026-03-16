#pragma once

#include <unordered_map>

#include <wx/wx.h>
#include <wx/textfile.h>
#include <wx/tokenzr.h>

#include "dataManagement.h"


/**
 * @brief Save/Read measurement data to csv file
 */
class CsvFile
{
    public:
    // file operations
        /**
         * @brief Save measurement data to csv file
         * @param separator Set separator for US or DE usage
         */
        CsvFile(char separator = ';');
        ~CsvFile();

        // write
        /**
         * @brief Save file to set path
         * @param Filename full file path
         * @param data sData object containing measurement data
         * @param mesurementNumb number of the current measurement
         * @return true on success 
         */
        bool saveCsvFile(wxString& FilePath, sData& data, int mesurementNumb);
        // read

        /**
         * @brief reads data from a file to a sData object
         * @param Filename full file path
         * @param data sData object containing measurement data
         * @return true on success 
         */
        bool readCsvFile(wxString filePath, sData& data);
        
    protected:

        /**
         * @brief save header info from sData to file
         */
        bool saveCsvHeader(wxTextFile& file, sData& data);
        // save header Helper Functions
        bool savePlotterData(wxTextFile& file, sData& data);
        bool saveCsvSettingsSweep(wxTextFile& file, sData& data);
        bool saveCsvSettingsQI(wxTextFile& file, sData& data);
        bool saveCsvSettingsMarker(wxTextFile& file, sData& data);
        

        /**
         * @brief save measurement data to file
         */
        bool saveCsvData(wxTextFile& file, sData data, int mesurementNumb, bool cont = false);

        

        /**
         *  @brief reads header information from file into sData 
         */
        bool readCsvHeader(wxTextFile& file, sData& data);
        // read Header Helper Function
        bool readPlotterData(wxTextFile& file, sData& data);
        bool readCsvSettingsSweep(wxTextFile& file, sData& data);
        bool readCsvSettingsQI(wxTextFile& file, sData& data);
        bool readCsvSettingsMarker(wxTextFile& file, sData& data);
        bool readCsvSettingsCostum(wxTextFile& file, sData& data);
        
        /**
         * @brief reads measurement data from file into sData
         */
        bool readCsvData(wxTextFile& file, sData& data);

        // helper functions
        /**
         * @brief calculates matrix index for current position
         * @returns matrix index in format: [x;y], separator gets set depending on file separator
         */
        std::string getIndexNumbers(int xPoints, int yPoints, int mesurementNumb, bool continuous = false);

        /**
         * @brief writes all matrix indexes for one measurement to file
         */
        bool writeMatrixIndexCsv(wxTextFile& file, sData data, bool continuous = false);

        /**
         * @brief converts a measurement number to zero-based matrix coordinates
         */
        bool measurementToCoordinates(int xPoints, int yPoints, int mesurementNumb, bool continuous, int& xPosition, int& yPosition) const;

        /**
         * @brief finds the line number of a given entry
         * @returns line number
         */
        int findLineCsv(wxTextFile& file, wxString findText);

        /**
         * @brief creates a lookup table for all first column entries in a file
         */
        bool createCsvLookupTable(wxTextFile& file);

        /**
         * @brief detects what separator a given file is using
         * @returns separator as char
         */
        char detectSeparator(wxTextFile& file);

    private:

        // Lookup table
        std::unordered_map<std::string, int> m_CsvLookupTable;
        // set Separator (, ;)
        char m_separator;
};

struct HeaderInfo
{
    static constexpr std::string_view fileName      = "File Name";
    static constexpr std::string_view date          = "Date";
    static constexpr std::string_view time          = "Time";
    static constexpr std::string_view type          = "Type";

};

struct HeaderConfig
{
    // Plotter Parameter
    static constexpr std::string_view noPointsX     = "Number Points X";
    static constexpr std::string_view noPointsY     = "Number Points Y";
    static constexpr std::string_view stepX         = "Step X [mm]";
    static constexpr std::string_view stepY         = "Step Y [mm]";
    static constexpr std::string_view startX        = "Start X [mm]";
    static constexpr std::string_view startY        = "Start Y [mm]";
    static constexpr std::string_view isVertical    = "Scan Vertical";
    // Frequenz Parameter
    static constexpr std::string_view startFreq     = "Start Frequency";        // ScpiCmdLookup.at(ScpiCmd::FREQ_STAR)
    static constexpr std::string_view endFreq       = "End Frequency";          // ScpiCmdLookup.at(ScpiCmd::FREQ_STOP)
    // Amplitude und Pegel-Parameter
    static constexpr std::string_view refPegel      = "Referenz Pegel";         // ScpiCmdLookup.at(ScpiCmd::DISP_TRAC_Y_RLEV)
    static constexpr std::string_view HFDaempfung   = "HF-Daempfung";            // ScpiCmdLookup.at(ScpiCmd::DISP_TRAC_Y_OFFS)
    static constexpr std::string_view ampUnit       = "Amplituden Einheit";
    // Bandbreiten und Zeit
    static constexpr std::string_view RBW           = "Aufloesebandbreite (RBW)"; // ScpiCmdLookup.at(ScpiCmd::BAND_RES)
    static constexpr std::string_view VBW           = "Videobandbreite (VBW)";   // ScpiCmdLookup.at(ScpiCmd::BAND_VID_AUTO)
    static constexpr std::string_view sweepTime     = "Sweep dauer";             // ScpiCmdLookup.at(ScpiCmd::SWE_TIME)
    // Erfassung
    static constexpr std::string_view noPointsArray = "Number Points per mesurement"; // ScpiCmdLookup.at(ScpiCmd::SWE_POIN)
    static constexpr std::string_view detektor      = "Detektor";
    // IQ-spezifische Parameter
    static constexpr std::string_view centerFreq    = "Center Frequency";
    static constexpr std::string_view sampleRate    = "Sample Rate";
    static constexpr std::string_view recordLength  = "Record Length";
    static constexpr std::string_view ifBandwidth   = "IF Bandwidth";
    static constexpr std::string_view triggerSource  = "Trigger Source";
    static constexpr std::string_view triggerLevel   = "Trigger Level";
    static constexpr std::string_view triggerDelay   = "Trigger Delay";
    // Custom 
    static constexpr std::string_view customFile    = "Custom file used";
    // Abschnitts-Label
    static constexpr std::string_view mesSettings   = "Messeinstellungen";
};

