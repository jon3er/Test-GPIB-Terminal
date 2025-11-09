# --- Basic Project Setup ---
cmake_minimum_required(VERSION 3.15)
project(TEST-GPIB-Terminal VERSION 1.0)

# Set the C++ standard (e.g., C++17)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# --- 1. Find wxWidgets (from vcpkg) ---
# This will work automatically AFTER we configure the vcpkg toolchain (Step 3)
find_package(wxWidgets REQUIRED COMPONENTS core base)

# --- 2. Define the ftd2xx Library (Manual) ---
# This assumes your ftdi files are in a "vendor/ftdi" folder, as we discussed
set(FTDI_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third-party/FTDI)
set(FTDI_LIBRARY     ${CMAKE_CURRENT_SOURCE_DIR}/third-party/FTDI/ftd2xx.lib)

# --- 3. Create Your Executable ---
# Add your source files here.
add_executable(TEST-GPIB-Terminal
    src/main.cpp
    src/fkt_GPIB.cpp
    src/fkt_d2XX.cpp
)
set_target_properties(TEST-GPIB-Terminal PROPERTIES WIN32_EXECUTABLE YES)

# --- 4. Link All Libraries ---

# Link wxWidgets
target_link_libraries(TEST-GPIB-Terminal PRIVATE ${wxWidgets_LIBRARIES})
target_include_directories(TEST-GPIB-Terminal PRIVATE ${wxWidgets_INCLUDE_DIRS})

# Link the manual ftd2xx library
target_link_libraries(TEST-GPIB-Terminal PRIVATE ${FTDI_LIBRARY})
target_include_directories(TEST-GPIB-Terminal PRIVATE ${FTDI_INCLUDE_DIR})

# --- Add project include folder (for fkt_d2xx.h etc.) ---
target_include_directories(TEST-GPIB-Terminal PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# For wxWidgets on Windows, we need to add this
add_definitions(-DwxUSE_GUI=1)

# --- 5. (Optional) Copy DLL to build folder ---
# This copies the ftd2xx.dll to where your .exe is, so you can run it
#add_custom_command(TARGET TEST-GPIB-Terminal POST_BUILD
#    COMMAND ${CMAKE_COMMAND} -E copy
#    ${CMAKE_CURRENT_SOURCE_DIR}/third-party/ftdi/ftd2xx.h
#    $<TARGET_FILE_DIR:TEST-GPIB-Terminal>
#)