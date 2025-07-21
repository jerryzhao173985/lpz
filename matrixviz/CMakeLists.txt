# Matrixviz - Matrix visualization tool

# Require Qt - Try Qt6 first, fall back to Qt5
find_package(Qt6 COMPONENTS Core Widgets OpenGL OpenGLWidgets Xml QUIET)
if(Qt6_FOUND)
    # qt6_standard_project_setup() requires Qt 6.3+
    # For older Qt6 versions, set the required properties manually
    if(Qt6Core_VERSION VERSION_LESS 6.3.0)
        set(CMAKE_AUTOMOC ON)
        set(CMAKE_AUTOUIC ON)
        set(CMAKE_AUTORCC ON)
    else()
        qt6_standard_project_setup()
    endif()
    set(QT_VERSION_MAJOR 6)
else()
    find_package(Qt5 COMPONENTS Core Widgets OpenGL Xml)
    if(NOT Qt5_FOUND)
        message(WARNING "Qt5 not found. Skipping matrixviz.")
        return()
    endif()
    set(CMAKE_AUTOMOC ON)
    set(CMAKE_AUTOUIC ON)
    set(CMAKE_AUTORCC ON)
    set(QT_VERSION_MAJOR 5)
endif()


find_package(OpenGL)
if(NOT OpenGL_FOUND)
    message(WARNING "OpenGL not found. Skipping matrixviz.")
    return()
endif()

# Source files
set(MATRIXVIZ_SOURCES
    src/main.cpp
    src/MatrixVisualizer.cpp
    src/visualisations/AbstractVisualisation.cpp
    src/AbstractRobotGUI.cpp
    src/visualisations/BarVisualisation.cpp
    src/InputReader/BufferedPipeReader.cpp
    src/ColorPalette.cpp
    src/configFile.cpp
    src/visualisations/LandscapeVisualisation.cpp
    src/ListEntity.cpp
    src/InputFilter/MatrixPipeFilter.cpp
    src/ScaleFunction.cpp
    src/InputReader/SimplePipeReader.cpp
    src/visualisations/TextureVisualisation.cpp
    src/visualisations/VectorPlotVisualisation.cpp
    src/VisualiserSubWidget.cpp
)

# Channel implementations
file(GLOB CHANNEL_SOURCES src/Channel/*PlotChannel.cpp)
list(APPEND MATRIXVIZ_SOURCES ${CHANNEL_SOURCES})

# Tools sources
list(APPEND MATRIXVIZ_SOURCES src/tools/stl_adds.cpp)

# Header files that need MOC
set(MATRIXVIZ_HEADERS
    src/MatrixVisualizer.h
    src/visualisations/AbstractVisualisation.h
    src/AbstractRobotGUI.h
    src/AbstractRobotSubWidget.h
    src/InputFilter/AbstractPipeFilter.h
    src/visualisations/BarVisualisation.h
    src/InputReader/BufferedPipeReader.h
    src/InputReader/AbstractPipeReader.h
    src/ColorPalette.h
    src/configFile.h
    src/visualisations/LandscapeVisualisation.h
    src/ListEntity.h
    src/InputFilter/MatrixPipeFilter.h
    src/ScaleFunction.h
    src/InputReader/SimplePipeReader.h
    src/InputFilter/SRGUIPipeFilter.h
    src/visualisations/TextureVisualisation.h
    src/tools/timer.h
    src/visualisations/VectorPlotVisualisation.h
    src/VisualiserSubWidget.h
)


# Channel headers
file(GLOB CHANNEL_HEADERS src/Channel/*.h)

# Create executable with headers for MOC processing
if(QT_VERSION_MAJOR EQUAL 6)
    qt6_add_executable(matrixviz ${MATRIXVIZ_SOURCES} ${MATRIXVIZ_HEADERS})
else()
    # For Qt5, we need to explicitly process MOC
    qt5_wrap_cpp(MATRIXVIZ_MOC_SOURCES ${MATRIXVIZ_HEADERS})
    add_executable(matrixviz ${MATRIXVIZ_SOURCES} ${MATRIXVIZ_MOC_SOURCES})
endif()

# Include directories
target_include_directories(matrixviz PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Channel
    ${CMAKE_CURRENT_SOURCE_DIR}/src/visualisations
    ${CMAKE_CURRENT_SOURCE_DIR}/src/InputFilter
    ${CMAKE_CURRENT_SOURCE_DIR}/src/InputReader
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tools
)

# Link libraries
if(QT_VERSION_MAJOR EQUAL 6)
    target_link_libraries(matrixviz PRIVATE
        Qt6::Core
        Qt6::Widgets
        Qt6::OpenGL
        Qt6::OpenGLWidgets
        Qt6::Xml
        OpenGL::GL
        OpenGL::GLU
    )
else()
    target_link_libraries(matrixviz PRIVATE
        Qt5::Core
        Qt5::Widgets
        Qt5::OpenGL
        Qt5::Xml
        OpenGL::GL
        OpenGL::GLU
    )
endif()

# Platform-specific settings
if(APPLE)
    set_target_properties(matrixviz PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/Info.plist.in
    )

    # Avoid deprecated OpenGL warnings on macOS
    target_compile_definitions(matrixviz PRIVATE GL_SILENCE_DEPRECATION)
elseif(WIN32)
    set_target_properties(matrixviz PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
endif()

# Set C++ standard
target_compile_features(matrixviz PRIVATE cxx_std_17)

# Apply comprehensive macOS Qt configuration
if(APPLE AND COMMAND lpzrobots_configure_qt_macos)
    lpzrobots_configure_qt_macos(matrixviz)
endif()

# Compiler options
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
    target_compile_options(matrixviz PRIVATE
        -Wno-deprecated-declarations
        -Wno-unused-parameter
        -Wno-sign-compare
    )
endif()

# Resources - commented out for now as files are missing
# if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/resources)
#     if(QT_VERSION_MAJOR EQUAL 6)
#         qt6_add_resources(matrixviz "matrixviz_resources"
#             PREFIX "/"
#             FILES
#                 resources/glass.ppm
#                 resources/wood.ppm
#         )
#     else()
#         qt5_add_resources(RESOURCE_FILES matrixviz.qrc)
#         target_sources(matrixviz PRIVATE ${RESOURCE_FILES})
#     endif()
# endif()

# Installation
install(TARGETS matrixviz
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    BUNDLE DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# ---------------------------------------------------------------------------
# Install configuration file (matrixVizConf.xml)
#
# Resolution order:
#   1) File located next to this CMakeLists.txt           (recommended)
#   2) Fallback to project-root level copy
# If neither copy is found we emit a warning so packagers/users
# immediately know why the file is missing after installation.
# ---------------------------------------------------------------------------
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/matrixVizConf.xml")
    message(STATUS "matrixviz: installing local matrixVizConf.xml")
    install(FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/matrixVizConf.xml"
        DESTINATION ${LPZROBOTS_DATA_DIR}/config
        RENAME matrixviz.conf
    )
elseif(EXISTS "${CMAKE_SOURCE_DIR}/matrixVizConf.xml")
    message(STATUS "matrixviz: installing root-level matrixVizConf.xml")
    install(FILES
        "${CMAKE_SOURCE_DIR}/matrixVizConf.xml"
        DESTINATION ${LPZROBOTS_DATA_DIR}/config
        RENAME matrixviz.conf
    )
else()
    message(WARNING "matrixviz: matrixVizConf.xml not found - default configuration will be used and no config file will be installed")
endif()

# Create wrapper script for non-bundle installations
if(NOT APPLE)
    configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/matrixviz-wrapper.sh
        ${CMAKE_CURRENT_BINARY_DIR}/matrixviz-wrapper
        @ONLY
    )
    install(PROGRAMS
        ${CMAKE_CURRENT_BINARY_DIR}/matrixviz-wrapper
        DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
endif()

# Generate desktop file for Linux
if(UNIX AND NOT APPLE)
    configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/matrixviz.desktop.in
        ${CMAKE_CURRENT_BINARY_DIR}/matrixviz.desktop
        @ONLY
    )
    install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/matrixviz.desktop
        DESTINATION ${CMAKE_INSTALL_DATADIR}/applications
    )
endif()
