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

# Collect ODE source files
file(GLOB_RECURSE ODE_SOURCES
    ode/src/*.cpp
    ode/src/*.c
    OPCODE/*.cpp
    OPCODE/*.c
    ou/src/*.cpp
    GIMPACT/*.cpp
)

# Remove any test or demo files
list(FILTER ODE_SOURCES EXCLUDE REGEX ".*test.*")
list(FILTER ODE_SOURCES EXCLUDE REGEX ".*demo.*")
list(FILTER ODE_SOURCES EXCLUDE REGEX ".*example.*")

# Create the library
add_library(ode ${ODE_SOURCES})
add_library(lpzrobots::ode ALIAS ode)

# Set target properties
set_target_properties(ode PROPERTIES
    OUTPUT_NAME ode
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    POSITION_INDEPENDENT_CODE ON
)

# Include directories
target_include_directories(ode
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/ode/src
        ${CMAKE_CURRENT_SOURCE_DIR}/OPCODE
        ${CMAKE_CURRENT_SOURCE_DIR}/ou/include
        ${CMAKE_CURRENT_SOURCE_DIR}/GIMPACT/include
)

# Compiler definitions
target_compile_definitions(ode PRIVATE
    dIDEDOUBLE
    CCD_IDEDOUBLE
    dTRIMESH_ENABLED
    dTRIMESH_OPCODE
)

# Platform-specific settings
if(APPLE)
    target_compile_definitions(ode PRIVATE MAC)
elseif(UNIX)
    target_compile_definitions(ode PRIVATE LINUX)
    # Link math library on Linux
    target_link_libraries(ode PUBLIC m)
endif()

# Export the target for parent project
if(NOT CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    # We're being built as part of LPZRobots
    # The target will be exported by the parent project
    set_target_properties(ode PROPERTIES EXPORT_NAME ode)
endif()

# Installation (only for standalone builds)
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    install(TARGETS ode
        EXPORT OpenDETargets
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    )
    
    # Install headers
    install(DIRECTORY include/ode
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        FILES_MATCHING PATTERN "*.h"
    )
    
    # Export targets
    install(EXPORT OpenDETargets
        FILE OpenDETargets.cmake
        NAMESPACE OpenDE::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/OpenDE
    )
endif()