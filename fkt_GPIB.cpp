#include <wx/wx.h>
#include "fkt_GPIB.h"

wxString GPIBInput(wxString input)
{
    return "test";

}


int checkCMD(wxString input)
{
    wxString first4letters = input.substr(0,4);
    if (first4letters == "cmd ")
    {
        return 1;
    }
    else
    {
        return 0;
    }
}


int checkCMDinput(wxString input,wxString Compare)
{
    // comp = Compare.c_str();
    wxString first4letters = input.substr(0,Compare.length());
    if (first4letters == input)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

wxString sendGPIBcmd(wxString input, int leng)
{
    wxString onlyCmd = input.substr(leng);
    //ToDo pass string to GPIB Converter


    wxLogDebug("Command Resived: %s", onlyCmd.c_str());

    return onlyCmd;

}

wxString terminalTimestampOutput(wxString Text)
{
    //Set Terminal Output Format
    wxDateTime zeitJetzt = wxDateTime::Now();
    wxString timestamp = zeitJetzt.Format("%H:%M:%S");
    wxString FormatText = "[" + timestamp + "] " + Text;

    //Output to terminal
    return FormatText;
}
