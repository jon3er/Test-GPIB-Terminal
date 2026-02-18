#pragma once

#include <string>
#include <functional>

/**
 * FunctionController - Business Logic Layer
 * Handles all GPIB function operations completely independent from GUI (wxWidgets).
 * The GUI (FunctionWindow) uses this controller to execute operations.
 */
class FunctionController
{
public:
    // Callback type for output notifications
    using OutputCallback = std::function<void(const std::string&)>;

    FunctionController();
    ~FunctionController();

    /**
     * Set the callback function that receives output from operations.
     * This allows the controller to communicate with GUI without wxWidgets dependency.
     */
    void setOutputCallback(OutputCallback callback);

    // GPIB Operations
    std::string writeToGpib(const std::string& data);
    std::string readFromGpib();
    std::string readWriteGpib(const std::string& data);
    std::string scanUsbDevices();
    std::string configureDevice();
    std::string connectDisconnect();

    // Test Operations
    std::string testSaveFile();
    std::string testMultiMeasurement();
    std::string test();

private:
    OutputCallback m_outputCallback;

    // Helper for output
    void output(const std::string& text);
};
