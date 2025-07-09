# Selforg library - Self-organizing controller framework

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

# Ensure headers are ready before building (moved up from line 144)
add_dependencies(selforg selforg_headers)

# Set include directories
target_include_directories(selforg
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>  # Unified include directory
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}>  # For <selforg/...> includes
        $<INSTALL_INTERFACE:include>
        $<INSTALL_INTERFACE:include/selforg>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
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
    target_include_directories(selforg PRIVATE ${GSL_INCLUDE_DIRS})
    target_link_libraries(selforg PUBLIC ${GSL_LIBRARIES})
    target_compile_options(selforg PRIVATE ${GSL_CFLAGS_OTHER})
else()
    target_compile_definitions(selforg PRIVATE NOGSL)
endif()

# Platform-specific settings
if(APPLE)
    target_compile_definitions(selforg PRIVATE MAC)
elseif(UNIX)
    target_compile_definitions(selforg PRIVATE LINUX)
endif()

# Configurator support (optional for now)
target_compile_definitions(selforg PRIVATE NOCONFIGURATOR)

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
)

# Create flattened header structure - mimic the Makefile's create_header_links
file(GLOB HEADER_FILES_ROOT
    abstractrobot.h
    agent.h
    trackable.h
    wiredcontroller.h
)
file(GLOB_RECURSE HEADER_FILES_SUBDIRS
    controller/*.h
    wirings/*.h
    utils/*.h
    matrix/*.h
    statistictools/*.h
)
set(HEADER_FILES ${HEADER_FILES_ROOT} ${HEADER_FILES_SUBDIRS})

# Create unified include directory at project root (following best practices)
file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/include/selforg)

# Create symlinks in the unified include directory (avoid duplication)
# This needs to happen at configure time so headers are available during compilation
foreach(header ${HEADER_FILES})
    get_filename_component(header_name ${header} NAME)
    # Check if header is already an absolute path
    if(IS_ABSOLUTE ${header})
        set(source_header ${header})
    else()
        set(source_header ${CMAKE_CURRENT_SOURCE_DIR}/${header})
    endif()
    set(dest_header ${CMAKE_SOURCE_DIR}/include/selforg/${header_name})
    
    # Create symlink at configure time
    if(NOT EXISTS ${dest_header})
        execute_process(
            COMMAND ${CMAKE_COMMAND} -E create_symlink ${source_header} ${dest_header}
        )
    endif()
endforeach()

# Create a dummy target for dependency tracking
add_custom_target(selforg_headers)

# Dependency already added above after add_library

# Installation
install(TARGETS selforg
    EXPORT LPZRobotsTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(DIRECTORY include/selforg
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    FILES_MATCHING PATTERN "*.h"
)

# Also install headers from source directories
install(FILES
    abstractrobot.h
    agent.h
    trackable.h
    wiredcontroller.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/selforg
)

install(DIRECTORY controller/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/selforg
    FILES_MATCHING PATTERN "*.h"
)

install(DIRECTORY wirings/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/selforg
    FILES_MATCHING PATTERN "*.h"
)

install(DIRECTORY utils/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/selforg
    FILES_MATCHING PATTERN "*.h"
)

install(DIRECTORY matrix/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/selforg
    FILES_MATCHING PATTERN "*.h"
)

install(DIRECTORY statistictools/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/selforg
    FILES_MATCHING PATTERN "*.h"
)