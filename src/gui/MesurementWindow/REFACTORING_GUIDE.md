# Measurement Window Refactoring - Callback Pattern

## Overview

The Measurement-related GUI classes have been refactored to separate business logic from GUI components using a **callback pattern**, similar to the existing `TerminalWindow`/`TerminalController` and `FunctionWindow`/`FunctionController` architecture.

## Architecture

### New Controller Classes

Three new controller classes have been created to handle business logic independently of wxWidgets:

#### 1. MeasurementController
**File:** `MeasurementController.h` / `MeasurementController.cpp`

Handles plot window operations:
- Loading scripts from directories
- Executing measurements 
- Managing plot data updates
- Progress tracking

**Key Callbacks:**
```cpp
using OutputCallback = std::function<void(const std::string&)>;
using ProgressCallback = std::function<void(int current, int total)>;
using DataUpdateCallback = std::function<void(const std::vector<double>&, const std::vector<double>&)>;
```

#### 2. Measurement2DController
**File:** `Measurement2DController.h` / `Measurement2DController.cpp`

Handles 2D measurement operations:
- Parameter management (X/Y points, scale, orientation)
- Measurement start/stop/reset
- Progress tracking
- State management

**Key Callbacks:**
```cpp
using OutputCallback = std::function<void(const std::string&)>;
using ProgressCallback = std::function<void(int current, int total)>;
using UpdateCallback = std::function<void()>;
```

#### 3. MultiMessWindowController
**File:** `MultiMessWindowController.h` / `MultiMessWindowController.cpp`

Handles multi-point measurement operations:
- Coordinate and frequency configuration
- Sweep point management
- Progress tracking
- Batch measurement execution

**Key Callbacks:**
```cpp
using OutputCallback = std::function<void(const std::string&)>;
using ProgressCallback = std::function<void(int current, int total)>;
using UpdateCallback = std::function<void()>;
```

## Updated GUI Classes

### PlotWindow
- Now contains instance of `MeasurementController`
- Sets up callbacks in constructor to update plot display
- Callbacks handle data updates and progress

### Mesurement2D
- Now contains instance of `Measurement2DController`
- Sets up callbacks to synchronize GUI with controller state
- Progress updates trigger UI refresh

### MultiMessWindow
- Now contains instance of `MultiMessWindowController`
- Sets up callbacks for progress and UI updates
- Controller manages all state changes

## Callback Setup Pattern

Each GUI window constructor sets up callbacks like this:

```cpp
// Progress updates
m_Controller.setProgressCallback([this](int current, int total) {
    m_currentPoint = current;
    m_totalPoints = total;
    updateProgressBar();
});

// Output/logging
m_Controller.setOutputCallback([this](const std::string& output) {
    std::cerr << output;
});

// UI updates
m_Controller.setUpdateCallback([this]() {
    UpdateDisplay();
    RefreshLayout();
});
```

## Benefits

1. **Separation of Concerns**: Business logic is completely isolated from GUI framework
2. **Testability**: Controllers can be unit tested without wxWidgets dependency
3. **Reusability**: Controllers can be used in CLI tools or other GUI frameworks
4. **Thread Safety**: Callbacks allow safe communication between worker threads and GUI
5. **Consistency**: Follows established pattern used in TerminalWindow/FunctionWindow

## Usage Example

**Setting Parameters:**
```cpp
// GUI calls controller methods
m_MultiMessLogic.setXMeasurementPoints("5");
m_MultiMessLogic.setYMeasurementPoints("3");
m_MultiMessLogic.setStartFrequency("100");
m_MultiMessLogic.start();
```

**Receiving Updates:**
```cpp
// Controller calls callbacks to notify GUI
m_progressCallback(2, 15);  // Update progress
m_outputCallback("Measurement point 2/15...\n");  // Log output
m_updateCallback();  // Trigger GUI refresh
```

## Migration Path

When refactoring existing measurement logic:

1. **Extract Logic**: Move business logic from GUI methods to controller
2. **Identify State**: Controller maintains all non-GUI state
3. **Add Callbacks**: Inject callbacks where GUI updates needed
4. **Test Independently**: Verify controller works without GUI
5. **Connect GUI**: GUI window sets up callbacks and calls controller methods

## Future Enhancements

- Add more granular callbacks for specific events
- Implement async/await patterns for long-running operations
- Add listener pattern for multiple GUI consumers
- Create measurement profiles/scripts system
- Add measurement result validation in controller

## Reference Files

- [MeasurementController.h](MeasurementController.h)
- [MeasurementController.cpp](MeasurementController.cpp)
- [Measurement2DController.h](Measurement2DController.h)
- [Measurement2DController.cpp](Measurement2DController.cpp)
- [MultiMessWindowController.h](MultiMessWindowController.h)
- [MultiMessWindowController.cpp](MultiMessWindowController.cpp)
- [Mesurement.h](Mesurement.h) - Updated GUI classes
- [Mesurement.cpp](Mesurement.cpp) - Updated GUI implementation

## See Also

- [TerminalWindow](../TerminalWindow/) - Reference implementation
- [FunctionWindow](../FunctionWindow/) - Reference implementation
