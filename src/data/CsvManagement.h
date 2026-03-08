#pragma once

#include <unordered_map>

#include <wx/wx.h>
#include <wx/textfile.h>
#include <wx/tokenzr.h>

#include "dataManagement.h"



/**
 * @brief Save mesurement data to csv file
 */
class CsvFile
{
    public:
    // file operations
        CsvFile(char separator = ',');
        ~CsvFile();

        // write
        bool saveCsvFile(wxString& Filename, sData& data, int mesurementNumb);
        // read
        bool readCsvFile(wxString filename, sData& data);

        // Separator
        void setSeparator(char separator)   { m_separator = separator; };
        char getSeparator()                 { return m_separator; };

    protected:
        // save Helper Functions
        bool saveCsvHeader(wxTextFile& file, sData& data);
        bool saveCsvData(wxTextFile& file, sData data, int mesurementNumb, bool cont = false);

        // read Helper Functions
        bool readCsvHeader(wxTextFile& file, sData& data);
        bool readCsvData(wxTextFile& file, sData& data);

        // helper functions
        std::string getIndexNumbers(int xPoints, int yPoints, int mesurementNumb, bool continuous = false);
        bool writeMatrixIndexCsv(wxTextFile& file, sData data);
        int findLineCsv(wxTextFile& file, wxString findText);
        bool createCsvLookupTable(wxTextFile& file);
        char detectSeparator(wxTextFile& file);

    private:

        
        wxTextFile m_file;
        sData m_data;

        std::unordered_map<std::string, int> m_CsvLookupTable;

        char m_separator;
};