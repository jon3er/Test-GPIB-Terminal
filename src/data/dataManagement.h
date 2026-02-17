#pragma once

#include <fstream>
#include <iomanip>
#include <chrono>
#include <format>
#include <cstring>

#include <wx/wx.h>
#include <wx/textfile.h>
#include <wx/tokenzr.h>


// Class to save big Data blocks
class sData3D
{
private:

    int m_X_Messpunkte;
    int m_Y_Messpunkte;
    int m_Messpunkte;

    //Ein Großes array mit allen Messdaten hintereinander geschrieben
    // index fängt mit null an in beiden fällen
    std::vector<double> m_dataArray;
public:
    sData3D(int x = 1, int y = 1, int Anzahl = 512);

    void resize(int x, int y, int Anzahl);

    double& at(int x, int y, int dataIndex);

    double* getDataPtr(int x, int y);

    std::vector<double> getSingleArray(int x, int y);
};

class sData
{
public:
    //Struktur für ein Datenobjekt
    struct sParam                                   //structure for 1D and 3D dataset parameters
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

    //Konstruktor

    sData(const char* type = "Line" );
    //Destruktor
    ~sData();

    bool SetData(sParam *par, std::vector<double> re, std::vector<double> im);
    bool GetData(sParam *par,std::vector<double>& re, std::vector<double>& im);

    //get data
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
    void getXYCord(int& x, int& y, int MesurementNumber);

    std::vector<double> getRealArray() { return m_dsR; };
    std::vector<double> getImagArray() { return m_dsI; };

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

    //virtual bool LoadFile(const wxString &name);
    bool set3DDataReal(std::vector<double> Array , int x, int y);
    bool set3DDataImag(std::vector<double> Array , int x, int y);
    std::vector<double> get3DDataReal(int x, int y);
    std::vector<double> get3DDataImag(int x, int y);

    void resize3DData(int x, int y, int Anzahl) { m_Real3D.resize(x, y, Anzahl); m_Imag3D.resize(x, y, Anzahl); };


private:
    sParam*                 m_dsParam;                            //parameters
    std::vector<double>     m_dsR;                                //data
    std::vector<double>     m_dsI;

    sData3D m_Real3D;
    sData3D m_Imag3D;

};


// file operations
// write
bool saveToCsvFile(wxString& Filename, sData& data, int mesurementNumb);
bool saveHeaderCsv(wxTextFile& file, sData& data);
bool saveDataCsv(wxTextFile& file, sData data, int mesurementNumb, bool cont = false);

// read functions
bool readCsvFile(wxString filename, sData& data);
bool readCsvHeader(wxTextFile& file, sData& data);
bool readDataCsv(wxTextFile& file, sData& data);

// helper functions
std::string getIndexNumbers(int xPoints, int yPoints, int mesurementNumb, bool continuous = false);
bool writeMatrixIndexCsv(wxTextFile& file, sData data);
int findLineCsv(wxTextFile& file, wxString findText);


