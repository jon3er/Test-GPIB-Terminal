#pragma once

#include <wx/wx.h>
#include "mathplot.h"

// keyword inline um in den header definieren zu können
namespace System 
{
#if defined(_WIN32)
    inline wxString filePathSystem = "D:\\CodeProjects\\VSCode\\projects\\Diplom\\Test-GPIB-Terminal\\GpibScripts\\";
    inline wxString filePathRoot = "D:\\CodeProjects\\VSCode\\projects\\Diplom\\Test-GPIB-Terminal\\";
    inline wxString fileSystemSlash = "\\";
#elif defined(__linux__)
    inline wxString filePathSytem = "/home/jon3r/Documents/Code/CodeBlocks/Test_GPIB_Terminal/GpibScripts/";
    inline wxString filePathRoot = "/home/jon3r/Documents/Code/CodeBlocks/Test_GPIB_Terminal/";
    inline wxString fileSystemSlash = "/";
#endif
}

namespace Global
{
    inline GpibDevice&     AdapterInstance = GpibDevice::get_instance();
    inline fsuMesurement   Messung;
}

enum MainMenuBar
{
    ID_Hello = 01,
    ID_OpenTerminal = 02,

    ID_Main_File = 10,
    ID_Main_File_Open = 11,
    ID_Main_File_Close = 12,
    ID_Main_File_Save = 13,
    ID_Main_File_SaveAs = 14,
    ID_Main_File_Exit = 15,

    ID_Main_Simulation = 20,

    ID_Main_Mesurement = 30,
    ID_Main_Mesurement_New = 31,
    ID_Main_Mesurement_Open = 32,
    ID_Main_Mesurement_Load = 33,
    ID_Main_Mesurement_Preset_1 = 34,
    ID_Main_Mesurement_Preset_2 = 35,
    ID_Main_Mesurement_Preset_3 = 36,
    ID_Main_Mesurement_2D_Mess = 37,
    ID_Main_Mesurement_SetMarker = 38,
    ID_Main_Mesurement_Settings = 39,

    ID_Main_Processing = 40,

    ID_Main_Test = 50,
    ID_Main_Test_Terminal = 51,
    ID_Main_Test_Func = 52,

    ID_Main_Help = 60,
    ID_Main_Help_About = 61
};