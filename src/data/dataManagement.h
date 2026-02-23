#pragma once

#include <fstream>
#include <iomanip>
#include <chrono>
#include <format>
#include <cstring>

#include <wx/wx.h>
#include <wx/textfile.h>
#include <wx/tokenzr.h>


/** 
 * @brief Class to save big 3-Dimentions
 * [x;y] matrix with n values in each element
*/
class sData3D
{
public:
    // Constructor
    sData3D(int x = 1, int y = 1, int Anzahl = 512);

    /**
     * @brief change size of the data array
     */
    void resize(int x, int y, int Anzahl);

    /**
     * @brief get pointer to specific data point
     * @param x x-coordinte (column)
     * @param y y-coordinte (row)
     * @param dataIndex index of the selected array
     */
    double& at(int x, int y, int dataIndex);

    /**
     * @brief get pointer to first element of selected array
     * @param x x-coordinte (column)
     * @param y y-coordinte (row)
     * @return pointer to first array element
     */
    double* getDataPtr(int x, int y);

    /**
     * @brief returns entire selected array [x;y]
     * @param x x-coordinte (column)
     * @param y y-coordinte (row)
     * @return selected Array as std::vector
     */
    std::vector<double> getSingleArray(int x, int y);
private:
    // Array scaling variables
    int m_X_Messpunkte;
    int m_Y_Messpunkte;
    int m_Messpunkte;

    //Ein Großes array mit allen Messdaten hintereinander geschrieben
    // index fängt mit null an in beiden fällen
    std::vector<double> m_dataArray;
};

class sData
{
public:
    // header data for mesurements
    struct sParam                                   
    {
        wxString        File;
        wxString        Date;
        wxString        Time;
        wxString        Type;
        unsigned int    NoPoints_X;
        unsigned int    NoPoints_Y;
        unsigned int    NoPoints_Array;
        wxString        ampUnit;
        unsigned int    startFreq;
        unsigned int    endFreq;
    };

    // constructor

    sData(const char* type = "Line" );
    // destruktor
    ~sData();

    bool SetData(sParam *par, std::vector<double> re, std::vector<double> im);
    bool GetData(sParam *par,std::vector<double>& re, std::vector<double>& im, std::vector<double>& freq);

    //get methodes
    //get parameter
    sParam* GetParameter() { return(m_dsParam); };
    wxString GetFile() { return m_dsParam->File; };
    wxString GetDate() { return m_dsParam->Date; };
    wxString GetTime() { return m_dsParam->Time; };
    wxString GetType() { return m_dsParam->Type; };

    unsigned int getNumberOfPts_X() {return m_dsParam->NoPoints_X; };
    unsigned int getNumberOfPts_Y() {return m_dsParam->NoPoints_Y; };
    unsigned int getTotalNumberOfPts() {return m_dsParam->NoPoints_X * m_dsParam->NoPoints_Y; };
    unsigned int getNumberOfPts_Array() {return m_dsParam->NoPoints_Array; };
    std::vector<double> GetFreqStepVector();

    std::vector<double> getRealArray() { return m_dsR; };
    std::vector<double> getImagArray() { return m_dsI; };


    // set methodes
    // set File Var
    bool setFileName(wxString Name);
    bool setFileType(wxString Type);
    // set Points
    bool setNumberOfPts_X(unsigned int NumbPtsX);
    bool setNumberOfPts_Y(unsigned int NumbPtsY);
    void setNumberofPts_Array(int numb = 0);
    // set Mesurement var
    bool setAmpUnit(wxString Unit);
    bool setStartFreq(unsigned int StartFreq);
    bool setEndFreq(unsigned int EndFreq);
    bool setTimeAndDate();
    // set data array
    bool set3DDataReal(std::vector<double> Array , int x, int y);
    bool set3DDataImag(std::vector<double> Array , int x, int y);
    
    // get methodes
    std::vector<double> get3DDataReal(int x, int y);
    std::vector<double> get3DDataImag(int x, int y);

    // helper methodes
    void resize3DData(int x, int y, int Anzahl) { m_Real3D.resize(x, y, Anzahl); m_Imag3D.resize(x, y, Anzahl); };

    /**
     * @brief gets x, y Coodinates for current mesurement number
    */
    void getXYCord(int& x, int& y, int MesurementNumber);

private:
    // header data
    sParam*                 m_dsParam;
    // last stored arrays
    std::vector<double>     m_dsR;                                
    std::vector<double>     m_dsI;
    // all stored arrays
    sData3D m_Real3D;
    sData3D m_Imag3D;

};



/**
 * @brief Save mesurement data to csv file
 */
class CsvFile
{
    public:
    // file operations
        // write
        bool saveToCsvFile(wxString& Filename, sData& data, int mesurementNumb);
        // read
        bool readCsvFile(wxString filename, sData& data);
    protected:
        // save Helper Functions
        bool saveHeaderCsv(wxTextFile& file, sData& data);
        bool saveDataCsv(wxTextFile& file, sData data, int mesurementNumb, bool cont = false);

        // read Helper Functions
        bool readCsvHeader(wxTextFile& file, sData& data);
        bool readDataCsv(wxTextFile& file, sData& data);

        // helper functions
        std::string getIndexNumbers(int xPoints, int yPoints, int mesurementNumb, bool continuous = false);
        bool writeMatrixIndexCsv(wxTextFile& file, sData data);
        int findLineCsv(wxTextFile& file, wxString findText);

    private:

        wxTextFile m_file;
        sData m_data;


};
