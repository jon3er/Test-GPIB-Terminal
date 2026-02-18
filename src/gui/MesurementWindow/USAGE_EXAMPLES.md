// Example Usage Patterns for Measurement Controllers
// This file demonstrates how to use the new controller classes

#include "MeasurementController.h"
#include "Measurement2DController.h"
#include "MultiMessWindowController.h"

// ============================================================================
// EXAMPLE 1: PlotWindow with MeasurementController
// ============================================================================

// In PlotWindow constructor:
void PlotWindow_Example() {
    // Create controller instance (done in header)
    MeasurementController m_MeasurementLogic;
    
    // Set data update callback
    m_MeasurementLogic.setDataUpdateCallback(
        [this](const std::vector<double>& x, const std::vector<double>& y) {
            m_x = x;
            m_y = y;
            updatePlotData();  // Refresh plot
        }
    );
    
    // Set progress callback
    m_MeasurementLogic.setProgressCallback(
        [this](int current, int total) {
            updateProgressUI(current, total);
        }
    );
    
    // Set output callback
    m_MeasurementLogic.setOutputCallback(
        [this](const std::string& output) {
            logOutput(output);
        }
    );
}

// In event handler:
void PlotWindow_ExampleExecute() {
    // Get selected script
    wxString scriptName = m_selectMesurement->GetStringSelection();
    
    // Call controller method - business logic is separated
    m_MeasurementLogic.startMeasurement(
        m_filePath.ToStdString(),
        scriptName.ToStdString(),
        m_mesurementNumber
    );
}

// ============================================================================
// EXAMPLE 2: Measurement2D with Measurement2DController
// ============================================================================

// In Measurement2D constructor:
void Measurement2D_Example() {
    // Create controller instance
    Measurement2DController m_MeasurementLogic;
    
    // Set progress callback
    m_MeasurementLogic.setProgressCallback(
        [this](int current, int total) {
            m_currentMeasurementPoint = current;
            m_totalMeasurementPoints = total;
            updateProgressBar();
        }
    );
    
    // Set output callback
    m_MeasurementLogic.setOutputCallback(
        [this](const std::string& output) {
            std::cerr << output;
        }
    );
    
    // Set UI update callback
    m_MeasurementLogic.setUpdateCallback(
        [this]() {
            SetSliderValues();
            updateProgressBar();
        }
    );
}

// In event handlers:
void Measurement2D_ExampleStart() {
    // Read values from GUI
    int yPoints = m_slider1->GetValue();
    int xPoints = m_slider2->GetValue();
    int scale = m_slider3->GetValue();
    
    // Update controller state
    m_MeasurementLogic.setXPoints(xPoints);
    m_MeasurementLogic.setYPoints(yPoints);
    m_MeasurementLogic.setScale(scale);
    
    // Start measurement
    m_MeasurementLogic.start();
}

void Measurement2D_ExampleReset() {
    // Reset logic is in controller
    m_MeasurementLogic.reset();
    // Controller triggers callback which updates GUI
}

void Measurement2D_ExampleIncrement() {
    // Increment measurement point
    m_MeasurementLogic.incrementMeasurementPoint();
    // Controller updates progress via callback
}

// ============================================================================
// EXAMPLE 3: MultiMessWindow with MultiMessWindowController
// ============================================================================

// In MultiMessWindow constructor:
void MultiMessWindow_Example() {
    // Create controller instance
    MultiMessWindowController m_MultiMessLogic;
    
    // Set progress callback
    m_MultiMessLogic.setProgressCallback(
        [this](int current, int total) {
            m_currentMeasurementPoint = current;
            m_totalMeasurementPoints = total;
            UpdateProgressBar();
        }
    );
    
    // Set output callback
    m_MultiMessLogic.setOutputCallback(
        [this](const std::string& output) {
            logMeasurementOutput(output);
        }
    );
    
    // Set UI update callback
    m_MultiMessLogic.setUpdateCallback(
        [this]() {
            UpdateProgressBar();
            RefreshDisplay();
        }
    );
}

// In event handlers:
void MultiMessWindow_ExampleStart() {
    // Get values from GUI
    int xMeas = std::stoi(m_textCtrlXMess->GetValue().ToStdString());
    int yMeas = std::stoi(m_textCtrlYMess->GetValue().ToStdString());
    
    // Update controller with configuration
    m_MultiMessLogic.setXMeasurementPoints(std::to_string(xMeas));
    m_MultiMessLogic.setYMeasurementPoints(std::to_string(yMeas));
    m_MultiMessLogic.setXMeasurementSpacing(m_textCtrlXAbstand->GetValue().ToStdString());
    m_MultiMessLogic.setYMeasurementSpacing(m_textCtrlYAbstand->GetValue().ToStdString());
    
    // Get frequency settings
    m_MultiMessLogic.setStartFrequency(m_textCtrlStrtFreq->GetValue().ToStdString());
    m_MultiMessLogic.setStartFrequencyUnit(m_choiceEinheitFreq1->GetStringSelection().ToStdString());
    m_MultiMessLogic.setStopFrequency(m_textCtrlEndFreq->GetValue().ToStdString());
    m_MultiMessLogic.setStopFrequencyUnit(m_choiceEinheitFreq2->GetStringSelection().ToStdString());
    
    // Start measurement
    m_MultiMessLogic.start();
}

void MultiMessWindow_ExampleStop() {
    // Stop measurement
    m_MultiMessLogic.stop();
    // Controller updates progress via callback
}

void MultiMessWindow_ExampleReset() {
    // Reset all parameters to defaults
    m_MultiMessLogic.reset();
    // Controller updates GUI via callback
}

void MultiMessWindow_ExampleNext() {
    // Move to next measurement point
    m_MultiMessLogic.nextMeasurementPoint();
    // Controller updates progress via callback
}

// ============================================================================
// BEST PRACTICES
// ============================================================================

/*
1. SEPARATION OF CONCERNS
   - Controllers handle logic, GUI handles display
   - Controllers have NO wxWidgets dependencies
   - GUI calls controller methods, not vice versa

2. CALLBACK PATTERN
   - Controller notifies GUI through callbacks
   - GUI updates are always through callbacks
   - Thread-safe communication mechanism

3. TESTING
   - Controllers can be tested independently
   - Mock callbacks for unit testing
   - No need for GUI framework in tests

4. EXAMPLE TEST:
*/

#include <iostream>

void TestMeasurement2DControllerExample() {
    Measurement2DController controller;
    
    int progressCurrent = 0;
    int progressTotal = 0;
    std::string lastOutput;
    int updateCount = 0;
    
    // Set up test callbacks
    controller.setProgressCallback([&](int current, int total) {
        progressCurrent = current;
        progressTotal = total;
    });
    
    controller.setOutputCallback([&](const std::string& output) {
        lastOutput = output;
    });
    
    controller.setUpdateCallback([&]() {
        updateCount++;
    });
    
    // Test operations
    controller.setXPoints(5);
    controller.setYPoints(3);
    controller.start();
    
    assert(controller.getTotalMeasurements() == 15);
    assert(progressTotal == 15);
    assert(lastOutput.find("started") != std::string::npos);
    
    // Increment several points
    for (int i = 0; i < 5; i++) {
        controller.incrementMeasurementPoint();
    }
    
    assert(progressCurrent == 5);
    assert(updateCount > 0);
    
    std::cout << "All tests passed!" << std::endl;
}

// ============================================================================
// COMMON PATTERNS
// ============================================================================

/*
PATTERN 1: Get current state from controller
    int current = m_controller.getCurrentMeasurement();
    int total = m_controller.getTotalMeasurements();

PATTERN 2: Batch update configuration
    m_controller.setXPoints(10);
    m_controller.setYPoints(5);
    m_controller.setScale(100);
    // setUpdateCallback triggered only once at end

PATTERN 3: Handle long operations with callbacks
    m_controller.setProgressCallback([this](int c, int t) {
        // Called frequently, keep lightweight
        updateProgressBar();
    });
    
    m_controller.setOutputCallback([this](const std::string& s) {
        // Append to log in thread-safe manner
        wxTheApp->CallAfter([this, s]() {
            logDisplay->AppendText(s);
        });
    });

PATTERN 4: Reset and reinitialize
    m_controller.reset();
    // All state reset to defaults
    // Callback triggered to update UI
    
    m_controller.setXPoints(newValue);
    // State modified again
*/
