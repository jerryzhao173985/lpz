# ECBRobots - Embedded Control Board Robots Support
# This component provides Qt-based tools for embedded robot control

if(NOT BUILD_GUI_TOOLS)
    message(STATUS "Skipping ecbrobots (BUILD_GUI_TOOLS is OFF)")
    return()
endif()

# Find Qt (try Qt6 first, fall back to Qt5)
lpzrobots_find_qt()
if(NOT Qt${QT_VERSION}_FOUND)
    message(WARNING "Qt not found, skipping ecbrobots")
    return()
endif()

# Find dependencies
find_package(Threads REQUIRED)

# Set automoc for Qt
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# Collect sources
set(ECBROBOTS_SOURCES
    console.cpp
    ecb.cpp
    ecbagent.cpp
    ECBCommunicationData.cpp
    ECBCommunicationEvent.cpp
    ecbrobot.cpp
    QExtAction.cpp
    QECBRobotsWindow.cpp
    QLogViewWidget.cpp
    QECBCommunicator.cpp
    QECBManager.cpp
    QGlobalData.cpp
)

set(ECBROBOTS_HEADERS
    commanddefs.h
    ecb.h
    ecbagent.h
    ecbrobot.h
    ECBCommunicationData.h
    ECBCommunicationEvent.h
    QExtAction.h
    QECBRobotsWindow.h
    QLogViewWidget.h
    QECBCommunicator.h
    QECBManager.h
    QGlobalData.h
)

# Message dispatcher sources
set(MESSAGEDISPATCHER_SOURCES
    messagedispatcher/QECBMessageDispatchServer.cpp
    messagedispatcher/QMessageDispatchWindow.cpp
    messagedispatcher/QCommunicationChannel.cpp
    messagedispatcher/QFT232DeviceManager.cpp
    messagedispatcher/QCCHelper.cpp
    messagedispatcher/QExtByteArray.cpp
    messagedispatcher/QExtTimer.cpp
    messagedispatcher/QLog.cpp
    messagedispatcher/QMDSHelper.cpp
    messagedispatcher/QDNSDeviceWidget.cpp
)

# QConfigurable sources (shared with configurator)
set(QCONFIGURABLE_SOURCES
    qconfigurable/QConfigurableWidget.cpp
    qconfigurable/QAbstractConfigurableTileWidget.cpp
    qconfigurable/QBoolConfigurableTileWidget.cpp
    qconfigurable/QValConfigurableTileWidget.cpp
    qconfigurable/QIntConfigurableTileWidget.cpp
    qconfigurable/QDummyConfigurableTileWidget.cpp
    qconfigurable/QConfigurableTileShowHideDialog.cpp
    qconfigurable/QConfigurableSetBoundsDialog.cpp
    qconfigurable/QConfigurableLoadSaveDialog.cpp
    qconfigurable/QChangeNumberTileColumnsDialog.cpp
)

# Create library
lpzrobots_add_component_library(ecbrobots
    SOURCES
        ${ECBROBOTS_SOURCES}
        ${MESSAGEDISPATCHER_SOURCES}
        ${QCONFIGURABLE_SOURCES}
    PUBLIC_HEADERS
        ${ECBROBOTS_HEADERS}
    SUBDIRS
        messagedispatcher
        qconfigurable
)

# Add Qt resources
qt_add_resources(ECBROBOTS_RESOURCES ecbrobots.qrc)
target_sources(ecbrobots PRIVATE ${ECBROBOTS_RESOURCES})

# Link dependencies
target_link_libraries(ecbrobots
    PUBLIC
        lpzrobots::selforg
        lpzrobots::ode_robots
        Qt${QT_VERSION}::Core
        Qt${QT_VERSION}::Gui
        Qt${QT_VERSION}::Widgets
        Qt${QT_VERSION}::Xml
        Threads::Threads
)

# Include directories
target_include_directories(ecbrobots
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/messagedispatcher>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/qconfigurable>
        $<INSTALL_INTERFACE:include/ecbrobots>
)

# Installation
if(LPZROBOTS_INSTALL_COMPONENT_LIBS)
    install(TARGETS ecbrobots
        EXPORT lpzrobots-targets
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )
    
    install(FILES ${ECBROBOTS_HEADERS}
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ecbrobots
    )
    
    install(DIRECTORY messagedispatcher qconfigurable
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ecbrobots
        FILES_MATCHING PATTERN "*.h"
    )
endif()

# Add subdirectories
if(BUILD_EXAMPLES)
    add_subdirectory(simulations)
endif()

# Install utility scripts
install(PROGRAMS simulations/createNewECBManager.sh
    DESTINATION ${CMAKE_INSTALL_BINDIR}
    RENAME lpzrobots-create-ecb-manager
)