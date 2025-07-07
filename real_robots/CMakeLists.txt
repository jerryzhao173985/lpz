# Real Robots - Hardware Robot Interfaces
# This component provides interfaces to real hardware robots

# AMOSII Robot
set(AMOSII_SOURCES
    robots/amosii/amosIIserialv1.cpp
    robots/amosii/amosIIserialv2.cpp
    robots/amosii/console.cpp
    robots/amosii/cmdline.cpp
)

set(AMOSII_HEADERS
    robots/amosii/amosIIserialv1.h
    robots/amosii/amosIIserialv2.h
    robots/amosii/console.h
    robots/amosii/cmdline.h
    robots/amosii/globaldata.h
)

# Create AMOSII library
lpzrobots_add_component_library(amosii
    SOURCES ${AMOSII_SOURCES}
)

target_link_libraries(amosii
    PUBLIC
        lpzrobots::selforg
        lpzrobots::ode_robots
)

target_include_directories(amosii
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/robots/amosii>
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/ode_robots/include>
        $<INSTALL_INTERFACE:include/real_robots/amosii>
)

# EPuck Robot (requires Qt)
lpzrobots_find_qt()
if(Qt${QT_VERSION}_FOUND)
    set(CMAKE_AUTOMOC ON)
    
    set(EPUCK_SOURCES
        robots/epuck/epuckbluetooth.cpp
        robots/epuck/SerialComm.cpp
    )
    
    set(EPUCK_HEADERS
        robots/epuck/epuckbluetooth.h
        robots/epuck/SerialComm.h
    )
    
    # Create EPuck library
    lpzrobots_add_component_library(epuck
        SOURCES ${EPUCK_SOURCES}
    )
    
    target_link_libraries(epuck
        PUBLIC
            lpzrobots::selforg
            Qt${QT_VERSION}::Core
            Qt${QT_VERSION}::SerialPort
    )
    
    target_include_directories(epuck
        PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/robots/epuck>
            $<INSTALL_INTERFACE:include/real_robots/epuck>
    )
    
    # Installation for EPuck
    if(LPZROBOTS_INSTALL_COMPONENT_LIBS)
        install(TARGETS epuck
            EXPORT lpzrobots-targets
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
            INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        )
        
        install(FILES ${EPUCK_HEADERS}
            DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/real_robots/epuck
        )
        
        # Install embedded code reference
        install(DIRECTORY robots/epuck/epuck_embedded
            DESTINATION ${CMAKE_INSTALL_DATADIR}/lpzrobots/real_robots/epuck
        )
    endif()
else()
    message(STATUS "Qt not found, skipping EPuck robot interface")
endif()

# Installation for AMOSII
if(LPZROBOTS_INSTALL_COMPONENT_LIBS)
    install(TARGETS amosii
        EXPORT lpzrobots-targets
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )
    
    install(FILES ${AMOSII_HEADERS}
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/real_robots/amosii
    )
endif()

# Create a convenience real_robots target that links all robot interfaces
add_library(real_robots INTERFACE)
add_library(lpzrobots::real_robots ALIAS real_robots)

target_link_libraries(real_robots
    INTERFACE
        amosii
)

if(TARGET epuck)
    target_link_libraries(real_robots INTERFACE epuck)
endif()

# Installation for the interface target
if(LPZROBOTS_INSTALL_COMPONENT_LIBS)
    install(TARGETS real_robots
        EXPORT lpzrobots-targets
    )
    
    # Install README files
    install(FILES robots/README utils/README
        DESTINATION ${CMAKE_INSTALL_DATADIR}/lpzrobots/real_robots
    )
endif()