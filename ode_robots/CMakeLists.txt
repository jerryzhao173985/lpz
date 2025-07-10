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
        osg
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

# ODE include and library handling
if(LPZROBOTS_USE_SYSTEM_ODE AND LPZROBOTS_ODE_INCLUDE_DIRS)
    # Use system ODE 
    target_include_directories(ode_robots PUBLIC ${LPZROBOTS_ODE_INCLUDE_DIRS})
    if(TARGET ode_robots_opt)
        target_include_directories(ode_robots_opt PUBLIC ${LPZROBOTS_ODE_INCLUDE_DIRS})
    endif()
    if(TARGET ode_robots_dbg)
        target_include_directories(ode_robots_dbg PUBLIC ${LPZROBOTS_ODE_INCLUDE_DIRS})
    endif()
    
    if(LPZROBOTS_ODE_LIBRARIES)
        target_link_libraries(ode_robots PUBLIC ${LPZROBOTS_ODE_LIBRARIES})
        if(TARGET ode_robots_opt)
            target_link_libraries(ode_robots_opt PUBLIC ${LPZROBOTS_ODE_LIBRARIES})
        endif()
        if(TARGET ode_robots_dbg)
            target_link_libraries(ode_robots_dbg PUBLIC ${LPZROBOTS_ODE_LIBRARIES})
        endif()
    endif()
else()
    # Use bundled ODE headers
    if(EXISTS "${CMAKE_SOURCE_DIR}/include/ode-dbl")
        target_include_directories(ode_robots PUBLIC
            $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include/ode-dbl>
            $<INSTALL_INTERFACE:include/ode-dbl>
        )
        if(TARGET ode_robots_opt)
            target_include_directories(ode_robots_opt PUBLIC
                $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include/ode-dbl>
                $<INSTALL_INTERFACE:include/ode-dbl>
            )
        endif()
        if(TARGET ode_robots_dbg)
            target_include_directories(ode_robots_dbg PUBLIC
                $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include/ode-dbl>
                $<INSTALL_INTERFACE:include/ode-dbl>
            )
        endif()
    endif()
    
    # For bundled ODE, the library is built by the legacy Make system
    # CMake builds will need to ensure the bundled ODE is pre-built
    message(STATUS "Using bundled ODE headers (library must be pre-built)")
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