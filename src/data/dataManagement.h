#pragma once

#include <fstream>
#include <iomanip>
#include <chrono>
#include <format>
#include <cstring>

#include <wx/wx.h>
#include <wx/textfile.h>
#include <wx/tokenzr.h>

#include "FsuMeasurement.h"


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
    std::vector<double> getSingleArray(int x, int y) const;
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
        // info
        wxString        File;
        wxString        Date;
        wxString        Time;
        wxString        Type;
        wxString        MeasurementType;

        int             MeasurementNumb;
        // Plotter
        int             NoPoints_X;
        int             NoPoints_Y;
        double          xSpacingmm;
        double          ySpacingmm;
        double          xStartingPointmm;
        double          yStartingPointmm;
        bool            isVertical;
        bool            hasPlotterData;
        // Detection
        int             NoPoints_Array;
        // Frequenz
        double          startFreq;
        double          endFreq;
        double          centerFreq;
        double          spanFreq;
        // Amplitude und Pegel
        double          refPegel;
        int             HFDaempfung;
        std::string     ampUnit;
        int             RBW ;
        int             VBW;
        // Detectors
        std::string     sweepTime;
        std::string     detektor;
        // IQ Settings
        double          sampleRate;
        int             recordLength;
        double          ifBandwidth;
        std::string     triggerSource;
        double          triggerLevel;
        double          triggerDelay;
        // Costum file name:
        std::string     costumFile;
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
    wxString GetMeasurementType() { return m_dsParam->MeasurementType; };

    int GetMeasurementNumb() { return m_dsParam->MeasurementNumb; };

    int getNumberOfPts_X() const {return m_dsParam->NoPoints_X; };
    int getNumberOfPts_Y() const {return m_dsParam->NoPoints_Y; };
    int getTotalNumberOfPts() const {return m_dsParam->NoPoints_X * m_dsParam->NoPoints_Y; };
    int getNumberOfPts_Array() const {return m_dsParam->NoPoints_Array; };
    int getRecordLength() const {return m_dsParam->recordLength; };


    std::vector<double> GetFreqStepVector();
    std::vector<double> GetTimeIQStepVector();

    std::vector<double> getRealArray() { return m_dsR; };
    std::vector<double> getImagArray() { return m_dsI; };


    // set methodes
    // set File Var
    bool setFileName(wxString Name);
    bool setFileType(wxString Type);

    bool setMeasurementNumb(int Numb);
    // set Points
    bool setNumberOfPts_X(int NumbPtsX);
    bool setNumberOfPts_Y(int NumbPtsY);
    void setNumberofPts_Array(int numb = 0);
    void setPlotterPositions(double xSpacingmm,
        double ySpacingmm,
        double xStartingPointmm,
        double yStartingPointmm,
        bool   isHorizontal);
    // set Mesurement var
    bool setAmpUnit(std::string Unit);
    bool setStartFreq(double StartFreq);
    bool setEndFreq(double EndFreq);
    bool setTimeAndDate();
    // set data array
    bool set3DDataReal(std::vector<double> Array , int x, int y);
    bool set3DDataImag(std::vector<double> Array , int x, int y);

    // get methodes
    std::vector<double> get3DDataReal(int x, int y) const;
    std::vector<double> get3DDataImag(int x, int y) const;

    // helper methodes
    void resize3DData(int x, int y, int Anzahl) { m_Real3D.resize(x, y, Anzahl); m_Imag3D.resize(x, y, Anzahl); };

    /**
     * @brief gets x, y Coodinates for current mesurement number
    */
    void getXYCord(int& x, int& y, int MesurementNumber) const;

    /**
     * @brief Reads current settings from fsuMeasurement singleton into m_fsuSettings
     */
    void importFsuSettings();

    /**
     * @brief Loads current settings from m_fsuSettings into fsuMeasurement singleton
     */
    void exportFsuSettings();


    /**
     * @brief Returns const reference to the cached fsu settings
     */
    const fsuMeasurement::FsuSettings& getFsuSettings() const { return m_fsuSettings; };

    /**
     * @brief Returns mutable access for CSV import to populate cached settings.
     */
    fsuMeasurement::FsuSettings& getMutableFsuSettings() { return m_fsuSettings; };

    /**
     * @brief Copies the matching fields from m_fsuSettings into m_dsParam
     * (mode-dependent: Sweep, IQ, MarkerPeak, Costum)
     */
    void applyFsuSettingsToParam();

private:
    // header data
    sParam*                 m_dsParam;
    // last stored arrays
    std::vector<double>     m_dsR;
    std::vector<double>     m_dsI;
    // all stored arrays
    sData3D m_Real3D;
    sData3D m_Imag3D;
    // Cached fsu measurement settings
    fsuMeasurement::FsuSettings m_fsuSettings;

};




