# --- Basic Project Setup ---
cmake_minimum_required(VERSION 3.15)

# Suppress CMP0167 warning about FindBoost BEFORE any find_package calls
cmake_policy(SET CMP0167 NEW)

project(TEST-GPIB-Terminal VERSION 0.2)

# Set the C++ standard (e.g., C++17)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# --- 1. Find wxWidgets (from vcpkg) ---
set(CMAKE_PREFIX_PATH "C:/Users/Jonathan/vcpkg/installed/x64-windows/share")
find_package(wxWidgets CONFIG REQUIRED)
# Define Windows target version for Boost.Asio
add_compile_definitions(_WIN32_WINNT=0x0601)
# Suppress Boost.Bind deprecation warning
add_compile_definitions(BOOST_BIND_GLOBAL_PLACEHOLDERS)
# Find Boost
find_package(Boost REQUIRED COMPONENTS asio system)


# --- 2. Define the ftd2xx Library (Manual) ---
set(FTDI_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third-party/FTDI)
set(FTDI_LIBRARY     ${CMAKE_CURRENT_SOURCE_DIR}/third-party/FTDI/ftd2xx.lib)


set(wxMathplots_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third-party/wxMathplots)

# --- 3. Create Your Executable ---
# Automatically discover all source files
file(GLOB_RECURSE SOURCE_FILES 
    "src/**/*.cpp"
)

# Add third-party sources
file(GLOB_RECURSE THIRD_PARTY_SOURCES 
    "third-party/wxMathplots/*.cpp"
)

# Combine all sources
list(APPEND SOURCE_FILES ${THIRD_PARTY_SOURCES})

# Create executable with discovered files
add_executable(TEST-GPIB-Terminal ${SOURCE_FILES})
set_target_properties(TEST-GPIB-Terminal PROPERTIES WIN32_EXECUTABLE YES)

# --- 4. Link All Libraries ---
target_link_libraries(TEST-GPIB-Terminal PRIVATE wx::core wx::base)

# Link the manual ftd2xx library
target_link_libraries(TEST-GPIB-Terminal PRIVATE ${FTDI_LIBRARY})
target_include_directories(TEST-GPIB-Terminal PRIVATE ${FTDI_INCLUDE_DIR})

# Link wxMathplots library
target_include_directories(TEST-GPIB-Terminal PRIVATE ${wxMathplots_INCLUDE_DIR})

# --- Add project include folders ---
# Add all directories in include/ and src/
target_include_directories(TEST-GPIB-Terminal PRIVATE
    # Include folders
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include/hardware
    ${CMAKE_CURRENT_SOURCE_DIR}/include/Plotter
    # Source folders
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/application
    ${CMAKE_CURRENT_SOURCE_DIR}/src/application/documents
    ${CMAKE_CURRENT_SOURCE_DIR}/src/data
    ${CMAKE_CURRENT_SOURCE_DIR}/src/domain
    ${CMAKE_CURRENT_SOURCE_DIR}/src/domain/models
    ${CMAKE_CURRENT_SOURCE_DIR}/src/gui
    ${CMAKE_CURRENT_SOURCE_DIR}/src/gui/SubWindow
    ${CMAKE_CURRENT_SOURCE_DIR}/src/gui/SubWindow/TerminalWindow
    ${CMAKE_CURRENT_SOURCE_DIR}/src/hardware
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Plotter
    ${CMAKE_CURRENT_SOURCE_DIR}/src/presentation
    ${CMAKE_CURRENT_SOURCE_DIR}/src/presentation/views
)

# For wxWidgets on Windows, we need to add this
add_definitions(-DwxUSE_GUI=1)
