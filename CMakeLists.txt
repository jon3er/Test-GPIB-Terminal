# --- Basic Project Setup ---
cmake_minimum_required(VERSION 3.15)
project(TEST-GPIB-Terminal VERSION 1.0)

# Set the C++ standard (e.g., C++17)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# --- 1. Find wxWidgets (from vcpkg) ---
set(CMAKE_PREFIX_PATH "C:/Users/Jonathan/vcpkg/installed/x64-windows/share")
find_package(wxWidgets CONFIG REQUIRED)



# --- 2. Define the ftd2xx Library (Manual) ---
set(FTDI_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third-party/FTDI)
set(FTDI_LIBRARY     ${CMAKE_CURRENT_SOURCE_DIR}/third-party/FTDI/ftd2xx.lib)


set(wxMathplots_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third-party/wxMathplots)

# --- 3. Create Your Executable ---
# Add your source files here.
add_executable(TEST-GPIB-Terminal
    src/data/dataManagement.cpp
    src/gui/main.cpp
    src/gui/Mesurement.cpp
    src/hardware/fkt_GPIB.cpp
    src/hardware/fkt_d2XX.cpp
    # wxMathplots
    third-party/wxMathplots/mathplot.cpp
)
set_target_properties(TEST-GPIB-Terminal PROPERTIES WIN32_EXECUTABLE YES)

# --- 4. Link All Libraries ---
target_link_libraries(TEST-GPIB-Terminal PRIVATE wx::core wx::base)

# Link the manual ftd2xx library
target_link_libraries(TEST-GPIB-Terminal PRIVATE ${FTDI_LIBRARY})
target_include_directories(TEST-GPIB-Terminal PRIVATE ${FTDI_INCLUDE_DIR})

# Link the manual ftd2xx library
target_include_directories(TEST-GPIB-Terminal PRIVATE ${wxMathplots_INCLUDE_DIR})

# --- Add project include folder ---
target_include_directories(TEST-GPIB-Terminal PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src/hardware
    ${CMAKE_CURRENT_SOURCE_DIR}/src/data
)

# For wxWidgets on Windows, we need to add this
add_definitions(-DwxUSE_GUI=1)
