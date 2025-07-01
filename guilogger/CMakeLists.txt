# Guilogger - GUI logging tool for LPZRobots

# Set minimum CMake version if not already set by parent
if(NOT CMAKE_VERSION)
    cmake_minimum_required(VERSION 3.16)
endif()

# If this is a standalone build, set up project
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    project(Guilogger VERSION 1.0.0 LANGUAGES CXX)
    
    # Set C++ standard
    set(CMAKE_CXX_STANDARD 17)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_CXX_EXTENSIONS OFF)
    
    # Find Qt
    find_package(Qt6 QUIET COMPONENTS Core Widgets Xml)
    if(NOT Qt6_FOUND)
        find_package(Qt5 REQUIRED COMPONENTS Core Widgets Xml)
        set(QT_VERSION_MAJOR 5)
    else()
        set(QT_VERSION_MAJOR 6)
    endif()
    
    include(GNUInstallDirs)
endif()

# Check if Qt is available
if(NOT Qt${QT_VERSION_MAJOR}_FOUND)
    message(WARNING "Qt${QT_VERSION_MAJOR} not found. Skipping guilogger build.")
    return()
endif()

# Set Qt properties
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Add the src subdirectory which contains the actual build
add_subdirectory(src)