#include "dataManagement.h"
#include "FsuMeasurement.h"

//------sData Beginn------
sData::sData(const char* type)
{
    //init Struct
    m_dsParam = new sParam;
    m_dsParam->File = "Empty";
    m_dsParam->Date = "Empty";
    m_dsParam->Time = "Empty";
    m_dsParam->Type = type;
    m_dsParam->NoPoints_X = 1;
    m_dsParam->NoPoints_Y = 1;
    m_dsParam->ampUnit = "DB";
    m_dsParam->startFreq = 0;
    m_dsParam->endFreq = 100000;
    //get time
    setTimeAndDate();
    //test values
    m_dsR = {0,1,2,3,4};
    m_dsI = {0,1,2,3,4};

}
sData::~sData()
{
    //delete dsParam;
}

bool sData::SetData(sParam *par, std::vector<double> re, std::vector<double> im)
{
    try
    {
        m_dsParam = par;
        m_dsR = re;
        m_dsI = im;
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        return false;
    }
    return true;

}
bool sData::GetData(sParam *par,std::vector<double>& re, std::vector<double>& im, std::vector<double>& freq)
{
    try
    {
        par = m_dsParam;
        re = m_dsR;
        im = m_dsI;
        freq = GetFreqStepVector();
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        return false;
    }
    return true;

}
std::vector<double> sData::GetFreqStepVector()
{
    std::vector<double> freqSteps;

    double endFreq = double(m_dsParam->endFreq);
    double startFreq = double(m_dsParam->startFreq);
    double ArrayPts = double(m_dsParam->NoPoints_Array);
    double step;
    
    try
    {
        step = (endFreq - startFreq) / ArrayPts;
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    double freq = startFreq;

    std::cout << step << std::endl;


    for (size_t i = 0; i < ArrayPts; i++)
    {
        freqSteps.push_back(freq);
        freq += step;
    }

    return freqSteps;
}
void sData::getXYCord(int& x, int& y, int mesurementNumber)
{
    // für den das Array ist die addressierung von 0 bis n-1
    int yPoints = m_dsParam->NoPoints_Y;

    x = ((mesurementNumber - 1) / yPoints);
    y = ((mesurementNumber - 1) % yPoints);
}

void sData::importFsuSettings()
{
    fsuMeasurement& fsu = fsuMeasurement::get_instance();
    m_fsuSettings.mode   = fsu.getMeasurementMode();
    m_fsuSettings.sweep  = fsu.returnSweepSettings();
    m_fsuSettings.iq     = fsu.returnIqSettings();
    m_fsuSettings.marker = fsu.returnMarkerPeakSettings();
    m_fsuSettings.costumFile = fsu.getFileName();
}

void sData::applyFsuSettingsToParam()
{
    switch (m_fsuSettings.mode)
    {
        case MeasurementMode::SWEEP:
        {
            const auto& s = m_fsuSettings.sweep;
            m_dsParam->startFreq      = s.startFreq;
            m_dsParam->endFreq        = s.stopFreq;
            m_dsParam->refPegel       = s.refLevel;
            m_dsParam->HFDaempfung    = s.att;
            m_dsParam->ampUnit        = s.unit;
            m_dsParam->RBW            = s.rbw;
            m_dsParam->VBW            = s.vbw;
            m_dsParam->sweepTime      = s.sweepTime;
            m_dsParam->detektor       = s.detector;
            m_dsParam->NoPoints_Array = s.points;
            break;
        }
        case MeasurementMode::IQ:
        {
            const auto& s = m_fsuSettings.iq;
            m_dsParam->centerFreq    = s.centerFreq;
            m_dsParam->refPegel      = std::stoi(s.refLevel);
            m_dsParam->HFDaempfung   = s.att;
            m_dsParam->ampUnit       = s.unit;
            m_dsParam->sampleRate    = s.sampleRate;
            m_dsParam->recordLength  = s.recordLength;
            m_dsParam->ifBandwidth   = s.ifBandwidth;
            m_dsParam->triggerSource = s.triggerSource;
            m_dsParam->triggerLevel  = s.triggerLevel;
            m_dsParam->triggerDelay  = s.triggerDelay;
            break;
        }
        case MeasurementMode::MARKER_PEAK:
        {
            const auto& s = m_fsuSettings.marker;
            m_dsParam->startFreq   = s.startFreq;
            m_dsParam->endFreq     = s.stopFreq;
            m_dsParam->refPegel    = std::stoi(s.refLevel);
            m_dsParam->HFDaempfung = s.att;
            m_dsParam->ampUnit     = s.unit;
            m_dsParam->RBW         = s.rbw;
            m_dsParam->VBW         = s.vbw;
            m_dsParam->detektor    = s.detector;
            break;
        }
        case MeasurementMode::COSTUM:
            m_dsParam->costumFile = m_fsuSettings.costumFile.ToStdString();
            break;
    }
}


bool sData::setFileName(wxString Name)
{
    try
    {
        m_dsParam->File = Name;
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
        m_dsParam->Type = Type;
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
        m_dsParam->NoPoints_X = NumbPtsX;
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
        m_dsParam->NoPoints_Y = NumbPtsY;
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
        m_dsParam->ampUnit = Unit;
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
        m_dsParam->startFreq = StartFreq;
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
        m_dsParam->endFreq = EndFreq;
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        return false;
    }

    return true;
}

void sData::setNumberofPts_Array(int numb) 
{ 
        if (numb == 0) 
        {
            m_dsParam->NoPoints_Array = m_dsR.size();
        } 
        else 
        {
            m_dsParam->NoPoints_Array = numb; 
        }

        try
        {
            resize3DData(m_dsParam->NoPoints_X, m_dsParam->NoPoints_Y, m_dsParam->NoPoints_Array);
        }
        catch(const std::exception& e)
        {
            std::cerr << "Set Number of Points Array Error" << e.what() << '\n';
        }
        
        
}

bool sData::setTimeAndDate()
{
    try
    {
        wxDateTime zeitJetzt = wxDateTime::Now();
        wxString timestamp = zeitJetzt.Format("%H:%M:%S");
        m_dsParam->Time = timestamp;

        wxString dateStamp = zeitJetzt.Format("%Y:%m:%d");
        m_dsParam->Date = dateStamp;
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        return false;
    }

    return true;
}
bool sData::set3DDataReal(std::vector<double> Array , int x, int y)
{
    try
    {
        std::memcpy(m_Real3D.getDataPtr(x,y), Array.data(), Array.size()* sizeof(double));
        m_dsR = Array;
        return true;
    }
    catch(const std::exception& e)
    {
        std::cerr << "Copy failed " <<e.what() << '\n';
    }
    return false;
}
std::vector<double> sData::get3DDataReal(int x, int y)
{
    return m_Real3D.getSingleArray(x,y);
}

bool sData::set3DDataImag(std::vector<double> Array , int x, int y)
{
    try
    {
        std::memcpy(m_Imag3D.getDataPtr(x,y), Array.data(), Array.size()* sizeof(double));
        m_dsI = Array;
        return true;
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    return false;
}
std::vector<double> sData::get3DDataImag(int x, int y)
{
    return m_Imag3D.getSingleArray(x,y);
}
//------sData Ende------

//------sData3D------

sData3D::sData3D(int x, int y, int Anzahl) : m_X_Messpunkte(x), m_Y_Messpunkte(y), m_Messpunkte(Anzahl)
{
    resize(m_X_Messpunkte, m_Y_Messpunkte, m_Messpunkte);
}

void sData3D::resize(int x, int y, int Anzahl) 
{
    m_X_Messpunkte = x;
    m_Y_Messpunkte = y;
    m_Messpunkte = Anzahl;
    m_dataArray.resize(x * y * Anzahl, 0.0);
}

double& sData3D::at(int x, int y, int dataIndex)
{
    int index = (y*m_X_Messpunkte + x) * m_Messpunkte + dataIndex;
    return m_dataArray[index];
}

double* sData3D::getDataPtr(int x, int y)
{
    int index = (y*m_X_Messpunkte + x) * m_Messpunkte;
    return &m_dataArray[index];
}

std::vector<double> sData3D::getSingleArray(int x, int y)
{
    int index = (y*m_X_Messpunkte + x) * m_Messpunkte;
    std::vector<double> subVector(m_dataArray.begin() + index, m_dataArray.begin() + index + m_Messpunkte);

    return subVector;
}




