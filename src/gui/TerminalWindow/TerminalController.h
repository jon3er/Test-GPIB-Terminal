#pragma once

#include <string>
#include <map>
#include <functional>

/**
 * TerminalController - Business Logic Layer
 * Handles all GPIB terminal commands completely independent from GUI (wxWidgets).
 * The GUI (TerminalWindow) uses this controller to execute commands.
 */
class TerminalController
{
public:
    // Callback type for output notifications
    using OutputCallback = std::function<void(const std::string&)>;

    TerminalController();
    ~TerminalController();

    /**
     * Set the callback function that receives output from command execution.
     * This allows the controller to communicate with GUI without wxWidgets dependency.
     */
    void setOutputCallback(OutputCallback callback);

    /**
     * Process a user command (e.g., "scan", "connect 0", "send *IDN?")
     * Parses the command, executes it, and sends output via callback.
     */
    void processCommand(const std::string& input);

    // Command Methods (public for testing, but mainly called via processCommand)
    std::string scanDevices(const std::string& args = "");
    std::string statusDevice(const std::string& args = "");
    std::string connectDevice(const std::string& args = "");
    std::string disconnectDevice(const std::string& args = "");
    std::string sendToDevice(const std::string& args);
    std::string readFromDevice(const std::string& args = "");
    std::string writeToDevice(const std::string& args);
    std::string configDevice(const std::string& args = "");
    std::string testDevice(const std::string& args = "");

private:

    OutputCallback m_outputCallback;

    // Command map setup
    using CommandMap = std::map<std::string, std::function<std::string(const std::string&)>>;
    CommandMap m_cmds;
    void setupCommands();

    // Helper for output
    void output(const std::string& text);
};
