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

# Find ODE (Open Dynamics Engine)
if(USE_SYSTEM_ODE)
    pkg_check_modules(ODE ode)
    if(NOT ODE_FOUND)
        find_package(PkgConfig QUIET)
        if(PkgConfig_FOUND)
            pkg_check_modules(ODE ode)
        endif()
    endif()
    if(ODE_FOUND)
        message(STATUS "Found system ODE: ${ODE_VERSION}")
    else()
        message(WARNING "System ODE not found. ODE-dependent components will be disabled.")
    endif()
else()
    # Check for bundled ODE
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/opende/ode)
        message(STATUS "Using bundled ODE")
        set(ODE_FOUND TRUE)
        add_subdirectory(opende)
    else()
        message(WARNING "Bundled ODE not found. ODE-dependent components will be disabled.")
        set(ODE_FOUND FALSE)
    endif()
endif()

# Find Qt5/Qt6 for GUI components (optional)
if(BUILD_UTILS)
    find_package(Qt5 QUIET COMPONENTS Core Widgets Gui OpenGL)
    if(NOT Qt5_FOUND)
        find_package(Qt6 QUIET COMPONENTS Core Widgets Gui OpenGL)
        if(NOT Qt6_FOUND)
            message(WARNING "Qt5/Qt6 not found. GUI utilities (guilogger, matrixviz) will be disabled.")
            set(BUILD_UTILS OFF)
        endif()
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

# Add subdirectories for components
add_subdirectory(selforg)

# Add configurator (if building utils)
if(BUILD_UTILS AND (Qt5_FOUND OR Qt6_FOUND))
    add_subdirectory(configurator)
endif()

# Add genetic algorithm tools
add_subdirectory(ga_tools)

# Add ode_robots only if ODE is available and properly configured
if(ODE_FOUND OR (NOT USE_SYSTEM_ODE AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/opende/ode))
    add_subdirectory(ode_robots)
else()
    message(WARNING "ODE not found. ode_robots component will be skipped.")
endif()

# Add utilities if requested and Qt is available
if(BUILD_UTILS AND (Qt5_FOUND OR Qt6_FOUND))
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