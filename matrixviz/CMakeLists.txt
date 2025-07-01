# Matrixviz - Matrix visualization tool for LPZRobots

# Set minimum CMake version if not already set by parent
if(NOT CMAKE_VERSION)
    cmake_minimum_required(VERSION 3.16)
endif()

# If this is a standalone build, set up project
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    project(Matrixviz VERSION 1.0.0 LANGUAGES CXX)
    
    # Set C++ standard
    set(CMAKE_CXX_STANDARD 17)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_CXX_EXTENSIONS OFF)
    
    # Find Qt
    find_package(Qt6 QUIET COMPONENTS Core Widgets Xml OpenGL OpenGLWidgets)
    if(NOT Qt6_FOUND)
        find_package(Qt5 REQUIRED COMPONENTS Core Widgets Xml OpenGL)
        set(QT_VERSION_MAJOR 5)
    else()
        set(QT_VERSION_MAJOR 6)
    endif()
    
    include(GNUInstallDirs)
endif()

# Check if Qt is available
if(NOT Qt${QT_VERSION_MAJOR}_FOUND)
    message(WARNING "Qt${QT_VERSION_MAJOR} not found. Skipping matrixviz build.")
    return()
endif()

# Set Qt properties
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Collect source files from .pro file structure
set(MATRIXVIZ_SOURCES
    src/main.cpp
    src/AbstractRobotGUI.cpp
    src/ColorPalette.cpp
    src/ListEntity.cpp
    src/MatrixVisualizer.cpp
    src/VisualiserSubWidget.cpp
    src/configFile.cpp
    src/ScaleFunction.cpp
    src/Channel/AxesPlotChannel.cpp
    src/Channel/DefaultPlotChannel.cpp
    src/Channel/GroupPlotChannel.cpp
    src/Channel/IRPlotChannel.cpp
    src/Channel/MatrixElementPlotChannel.cpp
    src/Channel/MatrixPlotChannel.cpp
    src/Channel/VectorElementPlotChannel.cpp
    src/Channel/VectorPlotChannel.cpp
    src/Channel/MotorCurrentPlotChannel.cpp
    src/Channel/MotorSpeedPlotChannel.cpp
    src/Channel/TiltPlotChannel.cpp
    src/Channel/TimeStampPlotChannel.cpp
    src/InputFilter/MatrixPipeFilter.cpp
    src/InputReader/BufferedPipeReader.cpp
    src/InputReader/SimplePipeReader.cpp
    src/visualisations/AbstractVisualisation.cpp
    src/visualisations/LandscapeVisualisation.cpp
    src/visualisations/TextureVisualisation.cpp
    src/visualisations/VectorPlotVisualisation.cpp
    src/visualisations/BarVisualisation.cpp
)

# Collect header files
set(MATRIXVIZ_HEADERS
    src/AbstractRobotGUI.h
    src/AbstractRobotSubWidget.h
    src/ColorPalette.h
    src/ListEntity.h
    src/MatrixVisualizer.h
    src/VisualiserSubWidget.h
    src/configFile.h
    src/ScaleFunction.h
    src/Channel/AbstractPlotChannel.h
    src/Channel/AxesPlotChannel.h
    src/Channel/DefaultPlotChannel.h
    src/Channel/GroupPlotChannel.h
    src/Channel/IRPlotChannel.h
    src/Channel/MatrixElementPlotChannel.h
    src/Channel/MatrixPlotChannel.h
    src/Channel/VectorElementPlotChannel.h
    src/Channel/VectorPlotChannel.h
    src/Channel/MotorCurrentPlotChannel.h
    src/Channel/MotorSpeedPlotChannel.h
    src/Channel/TiltPlotChannel.h
    src/Channel/TimeStampPlotChannel.h
    src/InputFilter/AbstractPipeFilter.h
    src/InputFilter/DefaultPipeFilter.h
    src/InputFilter/MatrixPipeFilter.h
    src/InputFilter/SRGUIPipeFilter.h
    src/InputReader/AbstractPipeReader.h
    src/InputReader/BufferedPipeReader.h
    src/InputReader/PipeIODevice.h
    src/InputReader/SimplePipeReader.h
    src/tools/sensortypes.h
    src/tools/stl_adds.h
    src/tools/timer.h
    src/visualisations/AbstractVisualisation.h
    src/visualisations/LandscapeVisualisation.h
    src/visualisations/TextureVisualisation.h
    src/visualisations/VectorPlotVisualisation.h
    src/visualisations/BarVisualisation.h
)

# Create the executable
add_executable(matrixviz ${MATRIXVIZ_SOURCES} ${MATRIXVIZ_HEADERS})

# Set target properties
set_target_properties(matrixviz PROPERTIES
    OUTPUT_NAME matrixviz
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin
)

# Include directories (mimic .pro file INCLUDEPATH)
target_include_directories(matrixviz PRIVATE
    src
    src/Channel
    src/InputFilter
    src/InputReader
    src/tools
    src/visualisations
)

# Link Qt libraries
set(QT_COMPONENTS Core Widgets Xml)

# Add OpenGL components based on Qt version
if(QT_VERSION_MAJOR EQUAL 6)
    list(APPEND QT_COMPONENTS OpenGL OpenGLWidgets)
else()
    list(APPEND QT_COMPONENTS OpenGL)
endif()

foreach(component ${QT_COMPONENTS})
    target_link_libraries(matrixviz PRIVATE Qt${QT_VERSION_MAJOR}::${component})
endforeach()

# Platform-specific settings
if(APPLE)
    # Suppress warnings from system frameworks and Qt headers
    target_compile_options(matrixviz PRIVATE 
        -Wno-float-equal 
        -Wno-old-style-cast
        -Wno-deprecated-declarations
    )
    
    # Add system include flags for external libraries
    target_compile_options(matrixviz PRIVATE
        -isystem /opt/homebrew/include
        -isystem /opt/homebrew/lib
    )
    
    # OpenGL framework for macOS
    target_link_libraries(matrixviz PRIVATE "-framework OpenGL" "-framework Cocoa")
    
    # Do not bundle as .app on macOS for command-line usage
    set_target_properties(matrixviz PROPERTIES
        MACOSX_BUNDLE FALSE
    )
else()
    # Linux - GLU library
    find_package(OpenGL REQUIRED)
    target_link_libraries(matrixviz PRIVATE OpenGL::GLU)
    
    # Suppress warnings
    target_compile_options(matrixviz PRIVATE 
        -Wno-float-equal 
        -Wno-old-style-cast
    )
endif()

# Installation
install(TARGETS matrixviz
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)