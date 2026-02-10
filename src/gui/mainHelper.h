#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <atomic>
#include <wx/wx.h>
#include <wx/tokenzr.h>

#include "dataManagement.h"
#include "systemInfo.h"

// Helper functions for komplex main functions

bool PlotterMesurement(sData* data, int mesurementNumber);