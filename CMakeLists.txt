cmake_minimum_required(VERSION 3.16)
project(LPZRobots VERSION 0.8.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Options
option(BUILD_SHARED_LIBS "Build shared libraries" OFF)
option(BUILD_GUI_TOOLS "Build GUI tools (requires Qt6)" ON)
option(ENABLE_ARM64_NEON "Enable ARM64 NEON optimizations" ON)
option(BUILD_EXAMPLES "Build example simulations" ON)

# Detect platform
if(APPLE)
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "arm64")
        message(STATUS "Building for macOS ARM64")
        if(ENABLE_ARM64_NEON)
            add_compile_options(-march=armv8-a+simd -mtune=native)
            add_compile_definitions(__ARM_NEON)
        endif()
    endif()
endif()

# Find dependencies
find_package(Threads REQUIRED)

# GSL (optional but recommended)
find_package(PkgConfig)
if(PkgConfig_FOUND)
    pkg_check_modules(GSL gsl)
    if(GSL_FOUND)
        message(STATUS "Found GSL: ${GSL_VERSION}")
    else()
        message(WARNING "GSL not found, some features will be disabled")
        add_compile_definitions(NOGSL)
    endif()
endif()

# OpenSceneGraph for ode_robots
find_package(OpenSceneGraph COMPONENTS osg osgDB osgGA osgViewer osgUtil osgShadow osgText)
if(OpenSceneGraph_FOUND)
    message(STATUS "Found OpenSceneGraph: ${OPENSCENEGRAPH_VERSION}")
else()
    message(WARNING "OpenSceneGraph not found, ode_robots will not be built")
endif()

# Qt6 for GUI tools
if(BUILD_GUI_TOOLS)
    find_package(Qt6 COMPONENTS Core Widgets OpenGL OpenGLWidgets Xml)
    if(Qt6_FOUND)
        message(STATUS "Found Qt6: ${Qt6_VERSION}")
        set(CMAKE_AUTOMOC ON)
    else()
        message(WARNING "Qt6 not found, GUI tools will not be built")
        set(BUILD_GUI_TOOLS OFF)
    endif()
endif()

# Common compile options
add_compile_options(
    -Wall
    -Wextra
    -Wno-unused-parameter
    -Wno-deprecated
    $<$<CONFIG:Debug>:-g -O0>
    $<$<CONFIG:Release>:-O3 -DNDEBUG>
)

# Add subdirectories
add_subdirectory(selforg)

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/opende/CMakeLists.txt")
    add_subdirectory(opende)
endif()

if(OpenSceneGraph_FOUND AND TARGET selforg)
    add_subdirectory(ode_robots)
endif()

if(BUILD_GUI_TOOLS AND Qt6_FOUND)
    add_subdirectory(guilogger)
    add_subdirectory(matrixviz)
    add_subdirectory(configurator)
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/ga_tools/CMakeLists.txt")
    add_subdirectory(ga_tools)
endif()

# Installation
include(GNUInstallDirs)

# Export targets
install(EXPORT LPZRobotsTargets
    FILE LPZRobotsTargets.cmake
    NAMESPACE LPZRobots::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/LPZRobots
)

# Create config file
include(CMakePackageConfigHelpers)
configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/LPZRobotsConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/LPZRobotsConfig.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/LPZRobots
)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/LPZRobotsConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/LPZRobotsConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/LPZRobotsConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/LPZRobots
)

# Summary
message(STATUS "")
message(STATUS "LPZRobots configuration summary:")
message(STATUS "  Version: ${PROJECT_VERSION}")
message(STATUS "  C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  Compiler: ${CMAKE_CXX_COMPILER}")
message(STATUS "  Platform: ${CMAKE_SYSTEM_NAME} ${CMAKE_SYSTEM_PROCESSOR}")
message(STATUS "  GSL: ${GSL_FOUND}")
message(STATUS "  OpenSceneGraph: ${OpenSceneGraph_FOUND}")
message(STATUS "  Qt6: ${Qt6_FOUND}")
message(STATUS "  ARM64 NEON: ${ENABLE_ARM64_NEON}")
message(STATUS "  Build GUI tools: ${BUILD_GUI_TOOLS}")
message(STATUS "  Build examples: ${BUILD_EXAMPLES}")
message(STATUS "  Install prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "")