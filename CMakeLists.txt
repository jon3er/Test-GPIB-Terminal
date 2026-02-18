cmake_minimum_required(VERSION 3.15)
project(TEST-GPIB-Terminal VERSION 0.2)

cmake_policy(SET CMP0003 NEW) # Linker Pfade
cmake_policy(SET CMP0167 NEW) # FindBoost Deprecation

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# 1. wxWidgets & Boost
find_package(wxWidgets REQUIRED COMPONENTS core base adv net)
include(${wxWidgets_USE_FILE})

find_package(Boost REQUIRED COMPONENTS asio system)

# 2. FTDI & wxMathplots
set(FTDI_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third-party/FTDI")
set(FTDI_LIBRARY     "${CMAKE_CURRENT_SOURCE_DIR}/third-party/FTDI/ftd2xx.lib")
set(wxMathplots_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third-party/wxMathplots")
set(VCPKG_APPLOCAL_DEPS ON CACHE BOOL "")
# supress wxMathplotswarnigs

# 3. Sources (Tipp: Vermeide GLOB, aber für Prototypen okay)
file(GLOB_RECURSE SOURCE_FILES "src/**/*.cpp" "third-party/wxMathplots/*.cpp")

add_executable(TEST-GPIB-Terminal ${SOURCE_FILES})
set_target_properties(TEST-GPIB-Terminal PROPERTIES WIN32_EXECUTABLE YES)

# --- 6. Präprozessor-Fixes (Löst deine Warnungen) ---
target_compile_definitions(TEST-GPIB-Terminal PRIVATE 
    WXUSINGDLL                      # Wichtig für wxWidgets DLLs
    wxUSE_GUI=1
    _WIN32_WINNT=0x0A00             # Fix für Boost: Ziel Windows 10
    BOOST_BIND_GLOBAL_PLACEHOLDERS  # Fix für Boost: Bind Warnung
)

# Unterdrücke wxMathplot C4996 Warnungen (SetStyle)
if(MSVC)
    target_compile_options(TEST-GPIB-Terminal PRIVATE /wd4996)
endif()

# 4. Linking & Includes
# WICHTIG: WXUSINGDLL für vcpkg/DLL builds
target_compile_definitions(TEST-GPIB-Terminal PRIVATE WXUSINGDLL wxUSE_GUI=1 _CRT_SECURE_NO_WARNINGS)

target_link_libraries(TEST-GPIB-Terminal PRIVATE 
    ${wxWidgets_LIBRARIES} 
    Boost::asio 
    Boost::system 
    ${FTDI_LIBRARY}
)

target_include_directories(TEST-GPIB-Terminal PRIVATE 
    "include"
    "include/hardware"
    "include/Plotter"
    "src"
    "src/data"
    "src/gui"
    "src/gui/FunctionWindow"
    "src/gui/MesurementWindow"
    "src/gui/SettingsWindow"
    "src/gui/TerminalWindow"
    "src/hardware"
    "src/Plotter"
    ${FTDI_INCLUDE_DIR}
    ${wxMathplots_INCLUDE_DIR}
    ${Boost_INCLUDE_DIRS}
    ${wxWidgets_INCLUDE_DIRS}
)

# 5. Automatischer DLL-Export (vcpkg Feature)
set(VCPKG_APPLOCAL_DEPS ON)