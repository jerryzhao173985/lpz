# OpenDE - Open Dynamics Engine (bundled version)

# Set minimum CMake version if not already set by parent
if(NOT CMAKE_VERSION)
    cmake_minimum_required(VERSION 3.16)
endif()

# If this is a standalone build, set up project
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    project(OpenDE VERSION 0.16.0 LANGUAGES CXX C)
    
    # Set C++ standard
    set(CMAKE_CXX_STANDARD 17)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_CXX_EXTENSIONS OFF)
    
    include(GNUInstallDirs)
endif()

# Note: This is a simplified CMake build for OpenDE
# For production use, consider using the full autotools build or official CMake support

# Check if we have the ODE source structure
if(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/ode)
    message(WARNING "OpenDE source directory 'ode' not found. Skipping bundled ODE build.")
    return()
endif()

# Collect core ODE source files (excluding problematic OPCODE for now)
file(GLOB_RECURSE ODE_CORE_SOURCES
    ode/src/*.cpp
    ode/src/*.c
)

# Collect OU (utilities) sources
file(GLOB_RECURSE OU_SOURCES
    ou/src/*.cpp
)

# Start with core sources
set(ODE_SOURCES ${ODE_CORE_SOURCES} ${OU_SOURCES})

# Remove any test or demo files
list(FILTER ODE_SOURCES EXCLUDE REGEX ".*test.*")
list(FILTER ODE_SOURCES EXCLUDE REGEX ".*demo.*")
list(FILTER ODE_SOURCES EXCLUDE REGEX ".*example.*")

# Skip OPCODE for now to avoid compilation issues
# TODO: Fix OPCODE build issues
message(STATUS "Skipping OPCODE build due to compilation issues - trimesh support will be limited")

# Create the library if we have sources
if(ODE_SOURCES)
    add_library(ode ${ODE_SOURCES})
    add_library(lpzrobots::ode ALIAS ode)
    
    # Set target properties
    set_target_properties(ode PROPERTIES
        VERSION 0.16.0
        SOVERSION 0
        PUBLIC_HEADER "${CMAKE_CURRENT_SOURCE_DIR}/config.h"
    )
    
    # Include directories
    target_include_directories(ode
        PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
            $<INSTALL_INTERFACE:include>
    )
    
    # Link with math library
    target_link_libraries(ode PUBLIC m)
    
    # Platform-specific settings
    if(WIN32)
        target_compile_definitions(ode PRIVATE WIN32)
    endif()
    
    # Install targets
    install(TARGETS ode
        EXPORT LPZRobotsTargets
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )
    
    message(STATUS "Bundled ODE library configured successfully")
    set(ODE_LIBRARIES ode PARENT_SCOPE)
    set(ODE_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR} PARENT_SCOPE)
else()
    message(WARNING "ODE sources not found or incomplete")
endif()