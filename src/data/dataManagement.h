#pragma once

#include <fstream>
#include <iomanip>
#include <chrono>
#include <format>

#include <wx/wx.h>
#include <wx/textfile.h>
#include <wx/tokenzr.h>



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
    sParam* GetParameter() { return(dsParam); };
    wxString GetFile() { return dsParam->File; };
    wxString GetDate() { return dsParam->Date; };
    wxString GetTime() { return dsParam->Time; };
    wxString GetType() { return dsParam->Type; };

    unsigned int getNumberOfPts_X() {return dsParam->NoPoints_X; };
    unsigned int getNumberOfPts_Y() {return dsParam->NoPoints_Y; };
    unsigned int getNumberOfPts_Array() {return dsParam->NoPoints_Array; };
    
    std::vector<double> getRealArray(int index = 0) { return dsR; };
    std::vector<double> getImagArray(int index = 0) { return dsI; };


    // set File Var
    bool setFileName(wxString Name);
    bool setFileType(wxString Type);
    // set Points
    bool setNumberOfPts_X(unsigned int NumbPtsX);
    bool setNumberOfPts_Y(unsigned int NumbPtsY);
    void setNumberofPts_Array() { dsParam->NoPoints_Array = dsR.size();};
    // set Mesurement var
    bool setAmpUnit(wxString Unit);
    bool setStartFreq(unsigned int StartFreq);
    bool setEndFreq(unsigned int EndFreq);
    bool setTimeAndDate();



    
    //virtual bool LoadFile(const wxString &name);

private:
    sParam*                 dsParam;                            //parameters
    std::vector<double>     dsR;                                //data
    std::vector<double>     dsI;

};

// Class to save big Data blocks
class sData3D
{
private:
    int X_Messpunkte;
    int Y_Messpunkte;
    int Messpunkte;

    //Ein Großes array mit allen Messdaten hintereinander geschrieben
    std::vector<double> dataArray;
public:
    sData3D(int x = 1, int y = 1, int Anzahl = 512);
    
    double& at(int x, int y, int dataIndex);
 
    double* getDataPtr(int x, int y);
};

// file operations
bool saveToCsvFile(wxString& Filename, sData& data, int mesurementNumb);

//bool testSaveCsv(wxString& Filename, sData& data, int mesurementNumb);

bool saveHeaderCsv(wxTextFile& file, sData& data);

bool saveDataCsv(wxTextFile& file, sData data, int xCord, int yCord);

std::string getIndexNumbers(int xPoints, int yPoints, int mesurementNumb, bool continuous = false);

bool writeDataCsv(wxTextFile& file, sData data, int mesurementNumb);

bool writeMatrixIndexCsv(wxTextFile& file, sData data);

int findLineCsv(wxTextFile& file, wxString findText);

bool openCsvFile(wxString& filename, sData& data);

//bool openCsvFileMultiline(wxString& filename);