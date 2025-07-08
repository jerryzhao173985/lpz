# Guilogger - Real-time data plotting tool

# Require Qt - Try Qt6 first, fall back to Qt5
find_package(Qt6 COMPONENTS Core Widgets Network QUIET)
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
    find_package(Qt5 REQUIRED COMPONENTS Core Widgets Network)
    set(CMAKE_AUTOMOC ON)
    set(CMAKE_AUTOUIC ON)
    set(CMAKE_AUTORCC ON)
    set(QT_VERSION_MAJOR 5)
endif()


# Source files
set(GUILOGGER_SOURCES
    src/main.cpp
    src/guilogger.cpp
    src/channeldata.cpp
    src/filelogger.cpp
    src/gnuplot.cpp
    src/inifile.cpp
    src/plotchannelstablemodel.cpp
    src/plotinfo.cpp
    src/qpipereader.cpp
    src/qserialreader.cpp
    src/stl_adds.cpp
)

# Header files that need MOC
set(GUILOGGER_HEADERS
    src/guilogger.h
    src/channeldata.h
    src/filelogger.h
    src/plotchannelstablemodel.h
    src/plotinfo.h
    src/qdatasource.h
    src/qpipereader.h
    src/qserialreader.h
)

# Other headers
set(GUILOGGER_OTHER_HEADERS
    src/commlineparser.h
    src/gnuplot.h
    src/gnuplot_unix.h
    src/inifile.h
    src/quickmp.h
    src/stl_adds.h
)

# Create executable
if(QT_VERSION_MAJOR EQUAL 6)
    qt6_add_executable(guilogger ${GUILOGGER_SOURCES})
else()
    add_executable(guilogger ${GUILOGGER_SOURCES})
endif()

# Include directories
target_include_directories(guilogger PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Link libraries
if(QT_VERSION_MAJOR EQUAL 6)
    target_link_libraries(guilogger PRIVATE
        Qt6::Core
        Qt6::Widgets
        Qt6::Network
    )
else()
    target_link_libraries(guilogger PRIVATE
        Qt5::Core
        Qt5::Widgets
        Qt5::Network
    )
endif()

# Platform-specific settings
if(APPLE)
    set_target_properties(guilogger PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/Info.plist.in
    )
    
    # Avoid deprecated AGL framework
    target_compile_definitions(guilogger PRIVATE GL_SILENCE_DEPRECATION)
elseif(WIN32)
    set_target_properties(guilogger PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
endif()

# Set C++ standard
target_compile_features(guilogger PRIVATE cxx_std_17)

# Apply comprehensive macOS Qt configuration
lpzrobots_configure_qt_macos(guilogger)

# Compiler options
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
    target_compile_options(guilogger PRIVATE
        -Wno-deprecated-declarations
        -Wno-unused-parameter
    )
endif()

# Installation
install(TARGETS guilogger
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    BUNDLE DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Install configuration files
install(FILES
    guilogger.cfg
    guilogger_macos.cfg
    DESTINATION ${LPZROBOTS_DATA_DIR}/config
)

# Create wrapper script for non-bundle installations
if(NOT APPLE)
    configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/guilogger-wrapper.sh
        ${CMAKE_CURRENT_BINARY_DIR}/guilogger-wrapper
        @ONLY
    )
    install(PROGRAMS
        ${CMAKE_CURRENT_BINARY_DIR}/guilogger-wrapper
        DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
endif()

# Generate desktop file for Linux
if(UNIX AND NOT APPLE)
    configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/guilogger.desktop.in
        ${CMAKE_CURRENT_BINARY_DIR}/guilogger.desktop
        @ONLY
    )
    install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/guilogger.desktop
        DESTINATION ${CMAKE_INSTALL_DATADIR}/applications
    )
endif()