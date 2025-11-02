#pragma once
#include <wx/wx.h>

wxString GPIBInput(wxString input);

int checkCMD(wxString input);

int checkCMDinput(wxString input, wxString Compare);

wxString sendGPIBcmd(wxString input, int leng);

