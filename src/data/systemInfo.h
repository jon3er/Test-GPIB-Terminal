#pragma once

#include <wx/wx.h>
#include <wx/filename.h>
#include <unordered_map>
#include "mathplot.h"
#include "dataManagement.h"
#include "FsuMeasurement.h"

// defines path for linux and windows systems
namespace System
{
    /**
     * Get the project root directory based on the current working directory or executable location.
     * This ensures cross-platform compatibility.
     */
    inline wxString GetProjectRoot()
    {
        wxString cwd = wxGetCwd();
        
        // If running from build directory, navigate back to project root
        if (cwd.EndsWith("Debug") || cwd.EndsWith("Release"))
        {
            // Navigate up two levels: build/Debug -> build -> project root
            wxFileName path(cwd + wxFileName::GetPathSeparator());
            path.RemoveLastDir();  // Remove Debug/Release
            path.RemoveLastDir();  // Remove build
            return path.GetFullPath() + wxFileName::GetPathSeparator();
        }
        
        // Otherwise assume cwd is already the project root
        return cwd + wxFileName::GetPathSeparator();
    }

    inline wxString filePathRoot = GetProjectRoot();
    inline wxString filePathSystem = GetProjectRoot() + "GpibScripts" + wxFileName::GetPathSeparator();
    inline wxString fileSystemSlash = wxFileName::GetPathSeparator();
}

// ID for main Window menu bar
enum MainMenuBar
{
    ID_Hello            = 01,
    ID_OpenTerminal     = 02,

    ID_Main_File        = 10,
    ID_Main_File_Open   = 11,
    ID_Main_File_Close  = 12,
    ID_Main_File_Save   = 13,
    ID_Main_File_SaveAs = 14,
    ID_Main_File_Exit   = 15,

    ID_Main_Simulation  = 20,

    ID_Main_Mesurement              = 30,
    ID_Main_Mesurement_New          = 31,
    ID_Main_Mesurement_Open         = 32,
    ID_Main_Mesurement_Load         = 33,
    ID_Main_Mesurement_Sweep        = 34,
    ID_Main_Mesurement_IQ           = 35,
    ID_Main_Mesurement_MarkerPeak   = 36,
    ID_Main_Mesurement_2D_Mess      = 37,
    ID_Main_Mesurement_SetMarker    = 38,
    ID_Main_Mesurement_Settings     = 39,
    ID_Main_Mesurement_Custom       = 40,


    ID_Main_Processing      = 45,

    ID_Main_Test            = 50,
    ID_Main_Test_Terminal   = 51,
    ID_Main_Test_Func       = 52,
    ID_Main_Test_Ploter     = 53,
    ID_Main_Test_Validation = 54,


    ID_Main_Help            = 60,
    ID_Main_Help_About      = 61,
    ID_Main_Help_ResetDevices = 62,

    // PlotWindow-local menu IDs (must not collide with main menu IDs)
    ID_Plot_File_Open       = 70,
    ID_Plot_File_Close      = 71,
    ID_Plot_File_Save       = 72,
    ID_Plot_File_SaveAs     = 73,
    ID_Plot_File_Exit       = 74
};

// Error Prefixes
struct ErrorPrefixStr
{
    wxString CsvRead        = "[CSV/READ] ";
    wxString CsvSave        = "[CSV/SAVE] ";
    wxString CsvHelper      = "[CSV/HELPER] ";
    wxString Prologix       = "[PROLOGIX] ";
    wxString Measurement    = "[MEASUREMENT] ";
    wxString GuiMain        = "[GUI/MAIN] ";
    wxString GuiFunc        = "[GUI/FUNC] ";
    wxString GuiTerminal    = "[GUI/TERMINAL] ";
    wxString GuiMeasurement = "[GUI/MEASUREMENT] ";
    wxString GuiProcessing  = "[GUI/PROCESSING] ";
    wxString GuiTest        = "[GUI/TEST] ";
    wxString GuiHelp        = "[GUI/HELP] ";
    wxString GuiGeneric     = "[GUI/GENERIC] ";
};

static const ErrorPrefixStr kErrPrefixStr;
