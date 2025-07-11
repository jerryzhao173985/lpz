# Selforg library - Self-organizing controller framework

# Set minimum CMake version if not already set by parent
cmake_minimum_required(VERSION 3.16)

# If this is a standalone build, set up project
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    project(Selforg VERSION 1.0.0 LANGUAGES CXX)
    
    # Set C++ standard
    set(CMAKE_CXX_STANDARD 17)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_CXX_EXTENSIONS OFF)
    
    # Find dependencies
    find_package(Threads REQUIRED)
    
    # Optional GSL
    find_package(PkgConfig QUIET)
    if(PkgConfig_FOUND)
        pkg_check_modules(GSL gsl)
    endif()
    if(NOT GSL_FOUND)
        find_package(GSL QUIET)
    endif()
    
    include(GNUInstallDirs)
endif()

# Collect source files
file(GLOB CONTROLLER_SOURCES controller/*.cpp)
file(GLOB WIRING_SOURCES wirings/*.cpp)
file(GLOB UTILS_SOURCES utils/*.cpp)
file(GLOB MATRIX_SOURCES matrix/*.cpp)
file(GLOB STATS_SOURCES 
    statistictools/*.cpp
    statistictools/measure/*.cpp
    statistictools/dataanalysation/*.cpp
)

# Main library sources
set(SELFORG_SOURCES
    agent.cpp
    wiredcontroller.cpp
    ${CONTROLLER_SOURCES}
    ${WIRING_SOURCES}
    ${UTILS_SOURCES}
    ${MATRIX_SOURCES}
    ${STATS_SOURCES}
)

# Create library
add_library(selforg ${SELFORG_SOURCES})
add_library(lpzrobots::selforg ALIAS selforg)

# Add compile definitions
target_compile_definitions(selforg PUBLIC NOCONFIGURATOR)

# Set include directories
target_include_directories(selforg
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
        $<INSTALL_INTERFACE:include>
        $<INSTALL_INTERFACE:include/selforg>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/controller
        ${CMAKE_CURRENT_SOURCE_DIR}/wirings
        ${CMAKE_CURRENT_SOURCE_DIR}/utils
        ${CMAKE_CURRENT_SOURCE_DIR}/matrix
        ${CMAKE_CURRENT_SOURCE_DIR}/statistictools
        ${CMAKE_CURRENT_SOURCE_DIR}/statistictools/measure
        ${CMAKE_CURRENT_SOURCE_DIR}/statistictools/dataanalysation
)

# Link dependencies
target_link_libraries(selforg
    PUBLIC
        Threads::Threads
)

# Compiler features
target_compile_features(selforg PUBLIC cxx_std_17)

# GSL support (optional)
if(GSL_FOUND)
    if(TARGET GSL::gsl)
        target_link_libraries(selforg PUBLIC GSL::gsl)
    else()
        target_include_directories(selforg PRIVATE ${GSL_INCLUDE_DIRS})
        target_link_libraries(selforg PUBLIC ${GSL_LIBRARIES})
        if(GSL_CFLAGS_OTHER)
            target_compile_options(selforg PRIVATE ${GSL_CFLAGS_OTHER})
        endif()
    endif()
else()
    target_compile_definitions(selforg PUBLIC NO_GSL)
endif()

# Platform-specific settings
if(APPLE)
    target_compile_definitions(selforg PRIVATE MAC)
elseif(UNIX)
    target_compile_definitions(selforg PRIVATE LINUX)
endif()

# SIMD optimizations
if(ENABLE_SIMD)
    include(CheckCXXCompilerFlag)
    
    # Check for ARM NEON (Apple Silicon)
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "arm64|aarch64")
        check_cxx_compiler_flag("-march=armv8-a+simd" HAS_ARM_NEON)
        if(HAS_ARM_NEON)
            target_compile_options(selforg PRIVATE -march=armv8-a+simd)
            target_compile_definitions(selforg PRIVATE USE_ARM_NEON)
            message(STATUS "Enabling ARM NEON SIMD optimizations for selforg")
        endif()
    # Check for x86 SIMD
    elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|i686")
        check_cxx_compiler_flag("-mavx2" HAS_AVX2)
        if(HAS_AVX2)
            target_compile_options(selforg PRIVATE -mavx2)
            target_compile_definitions(selforg PRIVATE USE_AVX2)
            message(STATUS "Enabling AVX2 SIMD optimizations for selforg")
        endif()
    endif()
endif()

# Set library properties
set_target_properties(selforg PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN ON
    POSITION_INDEPENDENT_CODE ON
    OUTPUT_NAME selforg
)

# Create symlinks for headers (mimics original build system)
file(GLOB_RECURSE HEADER_FILES
    RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    abstractrobot.h
    agent.h
    trackable.h
    wiredcontroller.h
    controller/*.h
    wirings/*.h
    utils/*.h
    matrix/*.h
    statistictools/*.h
    statistictools/measure/*.h
    statistictools/dataanalysation/*.h
)

# Create include/selforg directory structure in build directory
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/include/selforg)

# Create symlinks for headers so they can be found during build
foreach(header ${HEADER_FILES})
    get_filename_component(header_name ${header} NAME)
    get_filename_component(header_dir ${header} DIRECTORY)
    
    # Create the source path (absolute)
    set(source_path "${CMAKE_CURRENT_SOURCE_DIR}/${header}")
    
    # Preserve directory structure to avoid name collisions
    if(header_dir)
        set(dest_path "${CMAKE_CURRENT_BINARY_DIR}/include/selforg/${header_dir}/${header_name}")
        file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/include/selforg/${header_dir}")
    else()
        set(dest_path "${CMAKE_CURRENT_BINARY_DIR}/include/selforg/${header_name}")
    endif()
    
    # Create symlink with absolute paths to avoid double path issues
    if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.14")
        file(CREATE_LINK
            ${source_path}
            ${dest_path}
            SYMBOLIC
        )
    else()
        # Fallback for older CMake versions
        execute_process(
            COMMAND ${CMAKE_COMMAND} -E create_symlink
            ${source_path}
            ${dest_path}
        )
    endif()
endforeach()

# Installation
install(TARGETS selforg
    EXPORT LPZRobotsTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/selforg
)

# Install headers from include directory
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/include/selforg)
    install(DIRECTORY include/selforg
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        FILES_MATCHING PATTERN "*.h"
    )
endif()

# Install headers from source directories
install(FILES
    abstractrobot.h
    agent.h
    trackable.h
    wiredcontroller.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/selforg
)

# Install headers from subdirectories
foreach(subdir controller wirings utils matrix statistictools)
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${subdir})
        install(DIRECTORY ${subdir}/
            DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/selforg
            FILES_MATCHING PATTERN "*.h"
        )
    endif()
endforeach()

# Install measure and dataanalysation subdirectories if they exist
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/statistictools/measure)
    install(DIRECTORY statistictools/measure/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/selforg/measure
        FILES_MATCHING PATTERN "*.h"
    )
endif()

if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/statistictools/dataanalysation)
    install(DIRECTORY statistictools/dataanalysation/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/selforg/dataanalysation
        FILES_MATCHING PATTERN "*.h"
    )
endif()

# Install examples if requested
if(BUILD_EXAMPLES AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/simulations)
    install(DIRECTORY simulations/
        DESTINATION ${CMAKE_INSTALL_DATADIR}/lpzrobots/selforg/simulations
        PATTERN "*.o" EXCLUDE
        PATTERN "build*" EXCLUDE
        PATTERN ".git*" EXCLUDE
    )
endif()

# Create selforg-config script for compatibility
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    # Only create config script if building standalone
    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/selforg-config.m4"
        "${CMAKE_CURRENT_BINARY_DIR}/selforg-config"
        @ONLY
    )
    
    install(PROGRAMS "${CMAKE_CURRENT_BINARY_DIR}/selforg-config"
        DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
endif()