# Configurator - Configuration library for LPZRobots

# Set minimum CMake version if not already set by parent
if(NOT CMAKE_VERSION)
    cmake_minimum_required(VERSION 3.16)
endif()

# If this is a standalone build, set up project
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    project(Configurator VERSION 1.0.0 LANGUAGES CXX)
    
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
    
    # Find selforg
    find_package(PkgConfig QUIET)
    if(PkgConfig_FOUND)
        pkg_check_modules(SELFORG selforg)
    endif()
    
    include(GNUInstallDirs)
endif()

# Check if Qt is available
if(NOT Qt${QT_VERSION_MAJOR}_FOUND)
    message(WARNING "Qt${QT_VERSION_MAJOR} not found. Skipping configurator build.")
    return()
endif()

# Set Qt properties
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Collect source files from .pro file structure
set(CONFIGURATOR_SOURCES
    src/QExtAction.cpp
    src/QConfigurator.cpp
    src/QLogViewWidget.cpp
    src/ConfiguratorProxy.cpp
    # qconfigurable sources
    src/qconfigurable/QConfigurableWidget.cpp
    src/qconfigurable/QAbstractConfigurableTileWidget.cpp
    src/qconfigurable/QBoolConfigurableTileWidget.cpp
    src/qconfigurable/QValConfigurableTileWidget.cpp
    src/qconfigurable/QIntConfigurableTileWidget.cpp
    src/qconfigurable/QDummyConfigurableTileWidget.cpp
    src/qconfigurable/QConfigurableTileShowHideDialog.cpp
    src/qconfigurable/QConfigurableSetBoundsDialog.cpp
    src/qconfigurable/QConfigurableLoadSaveDialog.cpp
    src/qconfigurable/QChangeNumberTileColumnsDialog.cpp
)

# Collect header files
set(CONFIGURATOR_HEADERS
    src/QExtAction.h
    src/QConfigurator.h
    src/QLogViewWidget.h
    include/configurator/ConfiguratorProxy.h
    # qconfigurable headers
    src/qconfigurable/QConfigurableWidget.h
    src/qconfigurable/QAbstractConfigurableTileWidget.h
    src/qconfigurable/QBoolConfigurableTileWidget.h
    src/qconfigurable/QValConfigurableTileWidget.h
    src/qconfigurable/QIntConfigurableTileWidget.h
    src/qconfigurable/QDummyConfigurableTileWidget.h
    src/qconfigurable/QConfigurableTileShowHideDialog.h
    src/qconfigurable/QConfigurableSetBoundsDialog.h
    src/qconfigurable/QConfigurableLoadSaveDialog.h
    src/qconfigurable/QChangeNumberTileColumnsDialog.h
    src/qconfigurable/QGridPos.h
)

# Create the library
add_library(configurator ${CONFIGURATOR_SOURCES} ${CONFIGURATOR_HEADERS})
add_library(lpzrobots::configurator ALIAS configurator)

# Set target properties
set_target_properties(configurator PROPERTIES
    OUTPUT_NAME configurator
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    POSITION_INDEPENDENT_CODE ON
)

# Include directories (mimic .pro file INCLUDEPATH)
target_include_directories(configurator
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        src/qconfigurable
        src
)

# Depend on selforg
if(TARGET lpzrobots::selforg)
    target_link_libraries(configurator PUBLIC lpzrobots::selforg)
    target_include_directories(configurator PRIVATE 
        $<TARGET_PROPERTY:lpzrobots::selforg,INTERFACE_INCLUDE_DIRECTORIES>
    )
else()
    # Try to find selforg as external package
    find_package(PkgConfig QUIET)
    if(PkgConfig_FOUND)
        pkg_check_modules(SELFORG selforg REQUIRED)
        target_include_directories(configurator PRIVATE ${SELFORG_INCLUDE_DIRS})
        target_link_libraries(configurator PUBLIC ${SELFORG_LIBRARIES})
    else()
        message(FATAL_ERROR "Could not find selforg library")
    endif()
endif()

# Link Qt libraries
target_link_libraries(configurator
    PUBLIC
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::Xml
)

# Platform-specific settings
if(APPLE)
    # Suppress warnings from system frameworks and Qt headers
    target_compile_options(configurator PRIVATE 
        -Wno-deprecated
        -Wno-unused-parameter
    )
    
    # OpenGL framework for macOS
    target_link_libraries(configurator PRIVATE "-framework OpenGL" "-framework Cocoa")
else()
    # Linux-specific compile options
    target_compile_options(configurator PRIVATE 
        -Wno-deprecated
        -Wno-unused-parameter
    )
endif()

# Installation
install(TARGETS configurator
    EXPORT LPZRobotsTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/configurator
)

# Install headers
install(DIRECTORY include/configurator
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    FILES_MATCHING PATTERN "*.h"
)

# Create configurator-config script for compatibility
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    # Only create config script if building standalone
    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/configurator-config.m4"
        "${CMAKE_CURRENT_BINARY_DIR}/configurator-config"
        @ONLY
    )
    
    install(PROGRAMS "${CMAKE_CURRENT_BINARY_DIR}/configurator-config"
        DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
endif()