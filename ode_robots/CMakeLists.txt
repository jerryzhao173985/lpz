# ODE Robots - Robot simulation framework

# Main source file
set(ODE_ROBOTS_MAIN_SOURCES
    odeconfig.cpp
    simulation.cpp
)

# Use the unified library creation function
lpzrobots_add_component_library(ode_robots
    SOURCES
        ${ODE_ROBOTS_MAIN_SOURCES}
    SUBDIRS
        agents
        motors
        obstacles
        $<$<BOOL:${LPZROBOTS_HAS_OSG}>:osg>
        robots
        sensors
        utils
    PUBLIC_DEPS
        lpzrobots::selforg
        Threads::Threads
        ${OPENSCENEGRAPH_LIBRARIES}
        $<$<AND:$<PLATFORM_ID:Darwin>,$<BOOL:${OPENGL_LIBRARY}>>:${OPENGL_LIBRARY}>
        $<$<AND:$<NOT:$<PLATFORM_ID:Darwin>>,$<BOOL:${LPZROBOTS_HAS_OPENGL}>>:OpenGL::GL>
        $<$<AND:$<NOT:$<PLATFORM_ID:Darwin>>,$<BOOL:${LPZROBOTS_HAS_OPENGL}>>:OpenGL::GLU>
        $<$<BOOL:${LPZROBOTS_ODE_LIBRARIES}>:${LPZROBOTS_ODE_LIBRARIES}>
    PRIVATE_DEPS
        $<$<BOOL:${READLINE_LIBRARY}>:${READLINE_LIBRARY}>
    DEFINES
        NOCONFIGURATOR  # Always defined as per original Makefile
        OSG_LIBRARY_STATIC
        PREFIX="${CMAKE_INSTALL_PREFIX}"  # For finding data files
)

# Add ode_robots source directory to PUBLIC includes so that includes like <utils/axis.h> work
target_include_directories(ode_robots PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include/ode_robots>  # For flattened includes like "osghandle.h"
    $<INSTALL_INTERFACE:include/ode_robots>  # For both hierarchical and flattened includes after install
)
if(TARGET ode_robots_opt)
    target_include_directories(ode_robots_opt PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include/ode_robots>
        $<INSTALL_INTERFACE:include/ode_robots>
    )
endif()
if(TARGET ode_robots_dbg)
    target_include_directories(ode_robots_dbg PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include/ode_robots>
        $<INSTALL_INTERFACE:include/ode_robots>
    )
endif()

# ODE include handling
if(LPZROBOTS_ODE_INCLUDE_DIRS)
    target_include_directories(ode_robots PUBLIC ${LPZROBOTS_ODE_INCLUDE_DIRS})
    if(TARGET ode_robots_opt)
        target_include_directories(ode_robots_opt PUBLIC ${LPZROBOTS_ODE_INCLUDE_DIRS})
    endif()
    if(TARGET ode_robots_dbg)
        target_include_directories(ode_robots_dbg PUBLIC ${LPZROBOTS_ODE_INCLUDE_DIRS})
    endif()
endif()

# ODE library handling - ensure ODE is linked when using system ODE
if(LPZROBOTS_USE_SYSTEM_ODE AND NOT LPZROBOTS_ODE_LIBRARIES)
    # System ODE was requested but libraries weren't found properly
    # Try to find ODE using pkg-config
    find_package(PkgConfig)
    if(PkgConfig_FOUND)
        pkg_check_modules(ODE REQUIRED ode)
        if(ODE_FOUND)
            target_link_libraries(ode_robots PUBLIC ${ODE_LIBRARIES})
            target_include_directories(ode_robots PUBLIC ${ODE_INCLUDE_DIRS})
            target_link_directories(ode_robots PUBLIC ${ODE_LIBRARY_DIRS})
            if(TARGET ode_robots_opt)
                target_link_libraries(ode_robots_opt PUBLIC ${ODE_LIBRARIES})
                target_include_directories(ode_robots_opt PUBLIC ${ODE_INCLUDE_DIRS})
                target_link_directories(ode_robots_opt PUBLIC ${ODE_LIBRARY_DIRS})
            endif()
            if(TARGET ode_robots_dbg)
                target_link_libraries(ode_robots_dbg PUBLIC ${ODE_LIBRARIES})
                target_include_directories(ode_robots_dbg PUBLIC ${ODE_INCLUDE_DIRS})
                target_link_directories(ode_robots_dbg PUBLIC ${ODE_LIBRARY_DIRS})
            endif()
            message(STATUS "ODE libraries linked via pkg-config: ${ODE_LIBRARIES}")
        endif()
    endif()
endif()

# OpenSceneGraph include directories (use SYSTEM to suppress warnings)
if(OPENSCENEGRAPH_FOUND)
    target_include_directories(ode_robots SYSTEM PUBLIC ${OPENSCENEGRAPH_INCLUDE_DIRS})
    if(TARGET ode_robots_opt)
        target_include_directories(ode_robots_opt SYSTEM PUBLIC ${OPENSCENEGRAPH_INCLUDE_DIRS})
    endif()
    if(TARGET ode_robots_dbg)
        target_include_directories(ode_robots_dbg SYSTEM PUBLIC ${OPENSCENEGRAPH_INCLUDE_DIRS})
    endif()
endif()

# Readline support
if(READLINE_INCLUDE_DIR)
    target_include_directories(ode_robots PRIVATE ${READLINE_INCLUDE_DIR})
    if(TARGET ode_robots_opt)
        target_include_directories(ode_robots_opt PRIVATE ${READLINE_INCLUDE_DIR})
    endif()
    if(TARGET ode_robots_dbg)
        target_include_directories(ode_robots_dbg PRIVATE ${READLINE_INCLUDE_DIR})
    endif()
endif()

# Create header symlinks for development
lpzrobots_create_header_symlinks(ode_robots)

# Ensure configurator support is properly handled
lpzrobots_setup_configurator_support(ode_robots)

# Install textures and other data
install(DIRECTORY textures/
    DESTINATION ${LPZROBOTS_DATA_DIR}/textures
    PATTERN "README" EXCLUDE
)

# Add simulations if enabled
if(BUILD_EXAMPLES)
    add_subdirectory(simulations)
endif()

# Add tests if enabled
if(BUILD_TESTS)
    add_subdirectory(tests)
endif()