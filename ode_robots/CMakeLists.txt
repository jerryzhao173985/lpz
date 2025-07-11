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
        ${LPZROBOTS_ODE_LIBRARIES}
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