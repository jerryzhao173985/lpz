cmake_minimum_required(VERSION 3.20)
project(lpzrobots VERSION 2.0.0 LANGUAGES CXX C)

# Modern C++ standards
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Options
option(BUILD_SHARED_LIBS "Build shared libraries" ON)
option(BUILD_EXAMPLES "Build example simulations" ON)
option(BUILD_TESTS "Build unit tests" OFF)
option(BUILD_GUI_TOOLS "Build GUI tools (requires Qt6)" ON)
option(ENABLE_SIMD "Enable SIMD optimizations" ON)
option(ENABLE_OPENMP "Enable OpenMP parallelization" ON)

# Set default build type
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "RelWithDebInfo" "MinSizeRel")
endif()

# Compiler warnings
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
    add_compile_options(
        -Wall -Wextra -Wpedantic
        -Wcast-align -Wcast-qual 
        -Wconversion -Wsign-conversion
        -Wformat=2 -Wuninitialized
        -Wunused -Wunused-function
        -Wunused-label -Wunused-value
        -Wunused-variable -Wunused-parameter
        -Wwrite-strings -Wpointer-arith
        -Wredundant-decls -Woverloaded-virtual
        -Wsign-promo -Wformat-security
        -Wnon-virtual-dtor -Wold-style-cast
        -Wzero-as-null-pointer-constant
        -Wno-unused-parameter  # Too many false positives
        -Wno-sign-conversion   # Too noisy for now
    )
    
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
        add_compile_options(-Wlogical-op -Wnoexcept)
    endif()
endif()

# macOS specific settings
if(APPLE)
    set(CMAKE_MACOSX_RPATH ON)
    set(CMAKE_INSTALL_RPATH "@loader_path/../lib")
endif()

# Find dependencies
find_package(Threads REQUIRED)

# Optional dependencies
find_package(OpenMP)
if(OPENMP_FOUND AND ENABLE_OPENMP)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()

# GSL (GNU Scientific Library)
find_package(PkgConfig)
if(PKG_CONFIG_FOUND)
    pkg_check_modules(GSL gsl)
endif()

# Qt6 for GUI tools
if(BUILD_GUI_TOOLS)
    find_package(Qt6 COMPONENTS Core Widgets OpenGL QUIET)
    if(NOT Qt6_FOUND)
        message(WARNING "Qt6 not found. GUI tools will not be built.")
        set(BUILD_GUI_TOOLS OFF)
    endif()
endif()

# OpenSceneGraph
find_package(OpenSceneGraph COMPONENTS osgDB osgUtil osgViewer osgGA)

# Export compile commands for clang-tidy
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Installation directories
include(GNUInstallDirs)
set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}")

# Add subdirectories
add_subdirectory(selforg)
add_subdirectory(ode_robots)
add_subdirectory(opende)

if(BUILD_GUI_TOOLS)
    add_subdirectory(guilogger)
    add_subdirectory(matrixviz)
    add_subdirectory(configurator)
endif()

add_subdirectory(ga_tools)
add_subdirectory(ecbrobots)

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# Package configuration
include(CMakePackageConfigHelpers)

# Create package version file
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/lpzrobotsConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

# Install package configuration
install(
    FILES
        "${CMAKE_CURRENT_BINARY_DIR}/lpzrobotsConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/lpzrobotsConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/lpzrobots
)

# CPack configuration for creating packages
set(CPACK_PACKAGE_NAME "lpzrobots")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "A versatile simulator for robotic research on self-organization of behavior")
set(CPACK_PACKAGE_VENDOR "Leipzig Robot Group")
set(CPACK_PACKAGE_CONTACT "martius@informatik.uni-leipzig.de")

if(APPLE)
    set(CPACK_GENERATOR "DragNDrop")
    set(CPACK_DMG_VOLUME_NAME "LPZRobots ${PROJECT_VERSION}")
elseif(UNIX)
    set(CPACK_GENERATOR "TGZ;DEB")
    set(CPACK_DEBIAN_PACKAGE_DEPENDS "libqt6-dev, libgsl-dev")
endif()

include(CPack)

# Summary
message(STATUS "")
message(STATUS "LPZRobots ${PROJECT_VERSION} Configuration Summary")
message(STATUS "=====================================")
message(STATUS "Build type:        ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ Standard:      ${CMAKE_CXX_STANDARD}")
message(STATUS "Compiler:          ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "")
message(STATUS "Options:")
message(STATUS "  Shared libs:     ${BUILD_SHARED_LIBS}")
message(STATUS "  Examples:        ${BUILD_EXAMPLES}")
message(STATUS "  Tests:           ${BUILD_TESTS}")
message(STATUS "  GUI tools:       ${BUILD_GUI_TOOLS}")
message(STATUS "  SIMD:            ${ENABLE_SIMD}")
message(STATUS "  OpenMP:          ${ENABLE_OPENMP}")
message(STATUS "")
message(STATUS "Dependencies:")
message(STATUS "  Qt6:             ${Qt6_FOUND}")
message(STATUS "  GSL:             ${GSL_FOUND}")
message(STATUS "  OpenSceneGraph:  ${OPENSCENEGRAPH_FOUND}")
message(STATUS "  OpenMP:          ${OPENMP_FOUND}")
message(STATUS "")
message(STATUS "Install prefix:    ${CMAKE_INSTALL_PREFIX}")
message(STATUS "=====================================")