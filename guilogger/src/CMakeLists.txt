# Guilogger source

# Collect source files
set(GUILOGGER_SOURCES
    main.cpp
    guilogger.cpp
    gnuplot.cpp
    filelogger.cpp
    qserialreader.cpp
    qpipereader.cpp
    inifile.cpp
    stl_adds.cpp
    plotchannelstablemodel.cpp
    plotinfo.cpp
    channeldata.cpp
)

# Collect header files
set(GUILOGGER_HEADERS
    guilogger.h
    gnuplot.h
    filelogger.h
    qdatasource.h
    qserialreader.h
    qpipereader.h
    inifile.h
    commlineparser.h
    stl_adds.h
    plotchannelstablemodel.h
    plotinfo.h
    channeldata.h
    quickmp.h
)

# Create the executable
add_executable(guilogger ${GUILOGGER_SOURCES} ${GUILOGGER_HEADERS})

# Set target properties
set_target_properties(guilogger PROPERTIES
    OUTPUT_NAME guilogger
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

# Link Qt libraries
target_link_libraries(guilogger
    PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::Xml
)

# Platform-specific settings
if(APPLE)
    # Suppress warnings from system frameworks and Qt headers
    target_compile_options(guilogger PRIVATE 
        -Wno-deprecated-declarations
        -Wno-float-equal 
        -Wno-old-style-cast
    )
    
    # Add system include flags for external libraries
    target_compile_options(guilogger PRIVATE
        -isystem /opt/homebrew/include
        -isystem /opt/homebrew/lib
    )
    
    # Do not bundle as .app on macOS for command-line usage
    set_target_properties(guilogger PROPERTIES
        MACOSX_BUNDLE FALSE
    )
elseif(UNIX)
    # Linux-specific compile options
    target_compile_options(guilogger PRIVATE 
        -Wno-float-equal 
        -Wno-old-style-cast
    )
endif()

# Include current directory for headers
target_include_directories(guilogger PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# Installation
install(TARGETS guilogger
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)