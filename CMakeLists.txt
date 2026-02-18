cmake_minimum_required(VERSION 3.15)
project(TEST-GPIB-Terminal VERSION 0.2)

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


# 3. Sources (Tipp: Vermeide GLOB, aber für Prototypen okay)
file(GLOB_RECURSE SOURCE_FILES "src/**/*.cpp" "third-party/wxMathplots/*.cpp")

add_executable(TEST-GPIB-Terminal ${SOURCE_FILES})
set_target_properties(TEST-GPIB-Terminal PROPERTIES WIN32_EXECUTABLE YES)

# 4. Linking & Includes
# WICHTIG: WXUSINGDLL für vcpkg/DLL builds
target_compile_definitions(TEST-GPIB-Terminal PRIVATE WXUSINGDLL wxUSE_GUI=1)

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