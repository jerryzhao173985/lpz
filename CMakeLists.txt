cmake_minimum_required(VERSION 3.16)

project(LPZRobots
    VERSION 1.0.0
    DESCRIPTION "LPZRobots - Robot simulation and self-organization framework"
    LANGUAGES CXX
)

# Set modern C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Modern CMake policies
cmake_policy(SET CMP0077 NEW)  # option() honors normal variables
cmake_policy(SET CMP0079 NEW)  # target_link_libraries() can link to targets in other directories

# Include GNUInstallDirs for standard installation directories
include(GNUInstallDirs)

# Options for controlling build
option(BUILD_SHARED_LIBS "Build shared libraries" ON)
option(BUILD_TESTS "Build tests" OFF)
option(BUILD_EXAMPLES "Build example simulations" ON)
option(BUILD_UTILS "Build utility tools (guilogger, matrixviz, etc.)" ON)
option(ENABLE_SIMD "Enable SIMD optimizations" ON)
option(USE_SYSTEM_ODE "Use system-installed ODE library" ON)

# Platform detection
if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set(APPLE TRUE)
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "arm64|aarch64")
        set(APPLE_ARM64 TRUE)
    endif()
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(LINUX TRUE)
endif()

# Find required dependencies
find_package(Threads REQUIRED)

# Optional dependencies
find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
    pkg_check_modules(GSL gsl)
endif()

if(NOT GSL_FOUND)
    find_package(GSL QUIET)
endif()

# ODE dependency handling
if(USE_SYSTEM_ODE)
    if(APPLE)
        # On macOS, try to find ODE via Homebrew
        execute_process(
            COMMAND brew --prefix ode
            OUTPUT_VARIABLE ODE_PREFIX
            OUTPUT_STRIP_TRAILING_WHITESPACE
            ERROR_QUIET
        )
        
        if(ODE_PREFIX)
            set(ODE_INCLUDE_DIRS "${ODE_PREFIX}/include")
            set(ODE_LIBRARIES "${ODE_PREFIX}/lib/libode.dylib")
            set(ODE_FOUND TRUE)
            message(STATUS "Found ODE via Homebrew: ${ODE_PREFIX}")
        else()
            message(WARNING "ODE not found via Homebrew. Install with: brew install ode")
        endif()
    else()
        # On Linux, try pkg-config first
        if(PkgConfig_FOUND)
            pkg_check_modules(ODE ode)
        endif()
        
        if(NOT ODE_FOUND)
            find_path(ODE_INCLUDE_DIRS NAMES ode/ode.h)
            find_library(ODE_LIBRARIES NAMES ode)
            if(ODE_INCLUDE_DIRS AND ODE_LIBRARIES)
                set(ODE_FOUND TRUE)
            endif()
        endif()
    endif()
    
    if(NOT ODE_FOUND)
        message(FATAL_ERROR "System ODE not found. Please install ODE development package or set USE_SYSTEM_ODE=OFF")
    endif()
else()
    message(STATUS "Using bundled OpenDE library")
    add_subdirectory(opende)
    set(ODE_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/opende/include")
    set(ODE_LIBRARIES ode)
    set(ODE_FOUND TRUE)
    set(USING_BUNDLED_ODE TRUE)
endif()

# Qt dependency for GUI components
if(BUILD_UTILS)
    find_package(Qt6 QUIET COMPONENTS Core Widgets Xml OpenGL OpenGLWidgets)
    if(NOT Qt6_FOUND)
        find_package(Qt5 REQUIRED COMPONENTS Core Widgets Xml OpenGL)
        set(QT_VERSION_MAJOR 5)
    else()
        set(QT_VERSION_MAJOR 6)
    endif()
endif()

# Configure installation paths
set(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT TRUE)
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    if(APPLE OR CMAKE_SYSTEM_NAME STREQUAL "Darwin")
        set(CMAKE_INSTALL_PREFIX "/usr/local" CACHE PATH "Install path prefix" FORCE)
    else()
        set(CMAKE_INSTALL_PREFIX "/usr/local" CACHE PATH "Install path prefix" FORCE)
    endif()
endif()

# Create install prefix configuration
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/install_prefix.conf"
    "# installation type: user or devel\n"
    "INSTALLTYPE=USER\n"
    "\n"
    "# installation prefix:\n"
    "# e.g. /usr/local or /home/theuser\n"
    "PREFIX=${CMAKE_INSTALL_PREFIX}\n"
)

# Compiler flags
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    # Common flags for GCC and Clang
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pipe")
    
    # Release flags
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -DNDEBUG")
    
    # Debug flags
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -g -DDEBUG")
    
    # Position independent code
    set(CMAKE_POSITION_INDEPENDENT_CODE ON)
endif()

# Create config files
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/install_prefix.conf"
    "${CMAKE_CURRENT_BINARY_DIR}/install_prefix.conf"
    COPYONLY
)

# Add subdirectories in dependency order
add_subdirectory(selforg)

if(BUILD_UTILS)
    add_subdirectory(configurator)
endif()

add_subdirectory(ode_robots)
add_subdirectory(ga_tools)

# GUI utilities (optional)
if(BUILD_UTILS AND Qt${QT_VERSION_MAJOR}_FOUND)
    add_subdirectory(guilogger)
    add_subdirectory(matrixviz)
endif()

# Create package config files
include(CMakePackageConfigHelpers)

# Create LPZRobotsConfig.cmake
configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/LPZRobotsConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/LPZRobotsConfig.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/LPZRobots
    PATH_VARS CMAKE_INSTALL_INCLUDEDIR CMAKE_INSTALL_LIBDIR
)

# Create version file
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/LPZRobotsConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

# Install package config files
install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/LPZRobotsConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/LPZRobotsConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/LPZRobots
)

# Install other configuration files
install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/install_prefix.conf"
    DESTINATION ${CMAKE_INSTALL_DATADIR}/lpzrobots
)

# Export targets for installed tree
# Include bundled ODE if we're using it
set(EXPORT_TARGETS LPZRobotsTargets)
if(USING_BUNDLED_ODE)
    install(TARGETS ode
        EXPORT ${EXPORT_TARGETS}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    )
endif()

install(EXPORT ${EXPORT_TARGETS}
    FILE LPZRobotsTargets.cmake
    NAMESPACE lpzrobots::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/LPZRobots
)

# Export targets for build tree
if(USING_BUNDLED_ODE)
    export(TARGETS ode selforg ode_robots ga_tools
        FILE "${CMAKE_CURRENT_BINARY_DIR}/LPZRobotsTargets.cmake"
        NAMESPACE lpzrobots::
    )
else()
    export(EXPORT ${EXPORT_TARGETS}
        FILE "${CMAKE_CURRENT_BINARY_DIR}/LPZRobotsTargets.cmake"
        NAMESPACE lpzrobots::
    )
endif()

# Summary
message(STATUS "")
message(STATUS "=== LPZRobots Build Configuration ===")
message(STATUS "Version: ${PROJECT_VERSION}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Install prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "")
message(STATUS "Build options:")
message(STATUS "  BUILD_SHARED_LIBS: ${BUILD_SHARED_LIBS}")
message(STATUS "  BUILD_TESTS: ${BUILD_TESTS}")
message(STATUS "  BUILD_EXAMPLES: ${BUILD_EXAMPLES}")
message(STATUS "  BUILD_UTILS: ${BUILD_UTILS}")
message(STATUS "  ENABLE_SIMD: ${ENABLE_SIMD}")
message(STATUS "  USE_SYSTEM_ODE: ${USE_SYSTEM_ODE}")
message(STATUS "")
message(STATUS "Dependencies:")
message(STATUS "  GSL: ${GSL_FOUND}")
message(STATUS "  ODE: ${ODE_FOUND}")
if(BUILD_UTILS)
    message(STATUS "  Qt${QT_VERSION_MAJOR}: ${Qt${QT_VERSION_MAJOR}_FOUND}")
endif()
message(STATUS "")