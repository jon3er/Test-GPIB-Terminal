#include <wx/wx.h>
#include <unistd.h>
#include <map>
#include "fkt_d2xx.h"

void scanDevices(const std::string& args);

void statusDevice(const std::string& args);

void connectDevice(const std::string& args);

void disconnectDevice(const std::string& args);

void sendToDevice(const std::string& args);

void readFromDevice(const std::string& args);

void writeToDevice(const std::string& args);
