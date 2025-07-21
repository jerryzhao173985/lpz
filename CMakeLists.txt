cmake_minimum_required(VERSION 3.20)

# Read version from VERSION file for single source of truth
file(READ "${CMAKE_CURRENT_SOURCE_DIR}/VERSION.txt" VERSION_CONTENTS)
string(STRIP "${VERSION_CONTENTS}" VERSION_CONTENTS)

project(lpzrobots VERSION ${VERSION_CONTENTS} LANGUAGES CXX C)

# Modern C++ standards
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Include custom cmake modules
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(LPZRobotsCore)
include(LPZRobotsCompiler)
include(LPZRobotsPlatform)
include(LPZRobotsLibrary)
include(LPZRobotsDependencies)
include(LPZRobotsTargets)

# ------------------------------------------------------------------
# Initialise LPZRobots core (defines install paths, compiler flags,
# platform fixes, dependency variables, etc.)
# ------------------------------------------------------------------
lpzrobots_configure()
lpzrobots_print_summary()

# Include platform-specific fixes
if(APPLE)
    include(LPZRobotsMacOSFixes)
endif()

# Options
option(BUILD_SHARED_LIBS "Build shared libraries" ON)
option(BUILD_EXAMPLES "Build example simulations" ON)
option(BUILD_TESTS "Build unit tests" OFF)
option(BUILD_GUI_TOOLS "Build GUI tools (requires Qt6)" ON)
option(ENABLE_SIMD "Enable SIMD optimizations" ON)
option(ENABLE_OPENMP "Enable OpenMP parallelization" ON)

# Testing and Quality Options
option(LPZROBOTS_ENABLE_COVERAGE "Enable code coverage collection" OFF)
option(LPZROBOTS_ENABLE_BENCHMARKS "Enable benchmark tests" OFF)
option(LPZROBOTS_ENABLE_SANITIZERS "Enable sanitizers" OFF)
option(LPZROBOTS_SANITIZER_ADDRESS "Enable AddressSanitizer" OFF)
option(LPZROBOTS_SANITIZER_THREAD "Enable ThreadSanitizer" OFF)
option(LPZROBOTS_SANITIZER_UNDEFINED "Enable UndefinedBehaviorSanitizer" OFF)
option(LPZROBOTS_SANITIZER_MEMORY "Enable MemorySanitizer" OFF)

# Static Analysis Options
option(LPZROBOTS_ENABLE_CLANG_TIDY "Enable clang-tidy" OFF)
option(LPZROBOTS_ENABLE_CPPCHECK "Enable cppcheck" OFF)
option(LPZROBOTS_ENABLE_IWYU "Enable include-what-you-use" OFF)

# Set default build type
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "RelWithDebInfo" "MinSizeRel")
endif()

# Compiler warnings are handled centrally in cmake/LPZRobotsCompiler.cmake
# Components receive warnings via lpzrobots_apply_compiler_settings()
# This avoids duplicate/conflicting warning configurations

# Coverage configuration
if(LPZROBOTS_ENABLE_COVERAGE)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
        message(STATUS "Enabling code coverage collection")
        add_compile_options(-fprofile-arcs -ftest-coverage)
        add_link_options(-fprofile-arcs -ftest-coverage)

        # Find required programs
        find_program(LCOV lcov)
        find_program(GENHTML genhtml)

        if(LCOV AND GENHTML)
            message(STATUS "Found lcov and genhtml for coverage reports")
        else()
            message(WARNING "lcov/genhtml not found - coverage reports will not be available")
        endif()
    else()
        message(WARNING "Coverage is only supported with GCC/Clang compilers")
        set(LPZROBOTS_ENABLE_COVERAGE OFF)
    endif()
endif()

# Sanitizer configuration
if(LPZROBOTS_ENABLE_SANITIZERS)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
        set(SANITIZER_FLAGS "")

        # Auto-enable individual sanitizers if main option is ON
        if(NOT LPZROBOTS_SANITIZER_ADDRESS AND NOT LPZROBOTS_SANITIZER_THREAD
            AND NOT LPZROBOTS_SANITIZER_UNDEFINED AND NOT LPZROBOTS_SANITIZER_MEMORY)
            set(LPZROBOTS_SANITIZER_ADDRESS ON)
            set(LPZROBOTS_SANITIZER_UNDEFINED ON)
        endif()

        if(LPZROBOTS_SANITIZER_ADDRESS)
            message(STATUS "Enabling AddressSanitizer")
            list(APPEND SANITIZER_FLAGS "address")
        endif()

        if(LPZROBOTS_SANITIZER_THREAD)
            if(LPZROBOTS_SANITIZER_ADDRESS OR LPZROBOTS_SANITIZER_MEMORY)
                message(FATAL_ERROR "ThreadSanitizer cannot be used with Address or Memory sanitizers")
            endif()

            message(STATUS "Enabling ThreadSanitizer")
            list(APPEND SANITIZER_FLAGS "thread")
        endif()

        if(LPZROBOTS_SANITIZER_UNDEFINED)
            message(STATUS "Enabling UndefinedBehaviorSanitizer")
            list(APPEND SANITIZER_FLAGS "undefined")
        endif()

        if(LPZROBOTS_SANITIZER_MEMORY)
            if(LPZROBOTS_SANITIZER_ADDRESS OR LPZROBOTS_SANITIZER_THREAD)
                message(FATAL_ERROR "MemorySanitizer cannot be used with Address or Thread sanitizers")
            endif()

            message(STATUS "Enabling MemorySanitizer")
            list(APPEND SANITIZER_FLAGS "memory")
        endif()

        if(SANITIZER_FLAGS)
            string(JOIN "," SANITIZER_FLAGS_STR ${SANITIZER_FLAGS})
            add_compile_options(-fsanitize=${SANITIZER_FLAGS_STR} -fno-omit-frame-pointer)
            add_link_options(-fsanitize=${SANITIZER_FLAGS_STR})
        endif()
    else()
        message(WARNING "Sanitizers are only supported with GCC/Clang compilers")
        set(LPZROBOTS_ENABLE_SANITIZERS OFF)
    endif()
endif()

# Benchmark configuration
if(LPZROBOTS_ENABLE_BENCHMARKS)
    message(STATUS "Enabling benchmark tests")

    # Find Google Benchmark
    find_package(benchmark QUIET)

    if(NOT benchmark_FOUND)
        message(STATUS "Google Benchmark not found - will fetch during build if needed")
    endif()
endif()

# Static Analysis configuration
if(LPZROBOTS_ENABLE_CLANG_TIDY)
    find_program(CLANG_TIDY_EXE NAMES "clang-tidy" "clang-tidy-15" "clang-tidy-14")

    if(CLANG_TIDY_EXE)
        message(STATUS "Found clang-tidy: ${CLANG_TIDY_EXE}")
        set(CMAKE_CXX_CLANG_TIDY "${CLANG_TIDY_EXE}")
    else()
        message(WARNING "clang-tidy requested but not found")
        set(LPZROBOTS_ENABLE_CLANG_TIDY OFF)
    endif()
endif()

if(LPZROBOTS_ENABLE_CPPCHECK)
    find_program(CPPCHECK_EXE NAMES "cppcheck")

    if(CPPCHECK_EXE)
        message(STATUS "Found cppcheck: ${CPPCHECK_EXE}")
        set(CMAKE_CXX_CPPCHECK "${CPPCHECK_EXE};--enable=warning,style,performance,portability;--inline-suppr;--quiet")
    else()
        message(WARNING "cppcheck requested but not found")
        set(LPZROBOTS_ENABLE_CPPCHECK OFF)
    endif()
endif()

if(LPZROBOTS_ENABLE_IWYU)
    find_program(IWYU_EXE NAMES "include-what-you-use" "iwyu")

    if(IWYU_EXE)
        message(STATUS "Found include-what-you-use: ${IWYU_EXE}")
        set(CMAKE_CXX_INCLUDE_WHAT_YOU_USE "${IWYU_EXE}")
    else()
        message(WARNING "include-what-you-use requested but not found")
        set(LPZROBOTS_ENABLE_IWYU OFF)
    endif()
endif()

# macOS specific settings
if(APPLE)
    set(CMAKE_MACOSX_RPATH ON)
    set(CMAKE_INSTALL_RPATH "@loader_path/../lib")
endif()

# Find dependencies using unified function with categorization
lpzrobots_find_dependencies(
    REQUIRED Threads ODE
    OPTIONAL OpenSceneGraph Qt GSL
)

# OpenGL (needed for graphics)
if(APPLE)
    find_library(OPENGL_LIBRARY OpenGL)

    if(NOT OPENGL_LIBRARY)
        message(WARNING "OpenGL framework not found on macOS - graphics disabled")
        set(LPZROBOTS_HAS_OPENGL FALSE)
    else()
        set(LPZROBOTS_HAS_OPENGL TRUE)
    endif()
else()
    find_package(OpenGL)

    if(OpenGL_FOUND)
        set(LPZROBOTS_HAS_OPENGL TRUE)
    else()
        message(WARNING "OpenGL not found - graphics disabled")
        set(LPZROBOTS_HAS_OPENGL FALSE)
    endif()
endif()

# Print dependency summary
lpzrobots_print_dependencies()

# Export OSG libraries for use in subdirectories
if(LPZROBOTS_HAS_OSG)
    set(OPENSCENEGRAPH_FOUND TRUE CACHE INTERNAL "OSG found")
    set(OPENSCENEGRAPH_INCLUDE_DIRS ${LPZROBOTS_OSG_INCLUDE_DIRS} CACHE INTERNAL "OSG includes")
    set(OPENSCENEGRAPH_LIBRARIES ${LPZROBOTS_OSG_LIBRARIES} CACHE INTERNAL "OSG libraries")
endif()

# Export ODE libraries for use in subdirectories
if(LPZROBOTS_HAS_ODE)
    set(LPZROBOTS_ODE_INCLUDE_DIRS ${LPZROBOTS_ODE_INCLUDE_DIRS} CACHE INTERNAL "ODE includes")
    set(LPZROBOTS_ODE_LIBRARIES ${LPZROBOTS_ODE_LIBRARIES} CACHE INTERNAL "ODE libraries")
    set(LPZROBOTS_USE_SYSTEM_ODE ${LPZROBOTS_USE_SYSTEM_ODE} CACHE INTERNAL "Using system ODE")
endif()

# Export compile commands for clang-tidy
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Installation directories
include(GNUInstallDirs)
set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}")

# Add subdirectories
# Add the unified include directory for all components
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/include # Unified include directory
)

add_subdirectory(selforg)

# Build ODE compatibility layer for system ODE only
# Bundled ODE is headers-only and doesn't need to be built via CMake
if(LPZROBOTS_USE_SYSTEM_ODE)
    add_subdirectory(opende)
endif()

if(LPZROBOTS_HAS_OSG)
    add_subdirectory(ode_robots)
else()
    message(STATUS "Skipping ode_robots (requires OpenSceneGraph)")
endif()

if(BUILD_GUI_TOOLS)
    add_subdirectory(guilogger)
    add_subdirectory(matrixviz)
    add_subdirectory(configurator)
endif()

add_subdirectory(ga_tools)
add_subdirectory(ecbrobots)
add_subdirectory(real_robots)

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

# ------------------------------------------------------------------
# Generate lpzrobotsConfig.cmake so that install step can find it
# ------------------------------------------------------------------
configure_package_config_file(
    "${CMAKE_SOURCE_DIR}/cmake/LPZRobotsConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/lpzrobotsConfig.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/lpzrobots
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
message(STATUS "  GSL:             ${LPZROBOTS_HAS_GSL}")
message(STATUS "  OpenSceneGraph:  ${OPENSCENEGRAPH_FOUND}")
message(STATUS "  OpenMP:          ${OPENMP_FOUND}")
message(STATUS "")
message(STATUS "Install prefix:    ${CMAKE_INSTALL_PREFIX}")
message(STATUS "=====================================")
