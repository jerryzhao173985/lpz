# Platform Detection Module for lpzrobots
# Handles Linux, macOS (Intel and ARM64), and Windows

# platform_detection.cmake
cmake_minimum_required(VERSION 3.16)

# Function to detect platform and architecture
function(detect_platform)
    # Initialize variables
    set(LPZROBOTS_PLATFORM "" PARENT_SCOPE)
    set(LPZROBOTS_ARCH "" PARENT_SCOPE)
    set(LPZROBOTS_PLATFORM_STRING "" PARENT_SCOPE)
    
    # Detect OS
    if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
        set(LPZROBOTS_OS "macOS" PARENT_SCOPE)
        
        # Detect macOS architecture
        execute_process(
            COMMAND uname -m
            OUTPUT_VARIABLE MACHINE_ARCH
            OUTPUT_STRIP_TRAILING_WHITESPACE
            ERROR_QUIET
        )
        
        # Also check CMAKE_OSX_ARCHITECTURES for universal builds
        if(CMAKE_OSX_ARCHITECTURES)
            set(LPZROBOTS_ARCH "${CMAKE_OSX_ARCHITECTURES}" PARENT_SCOPE)
            if(CMAKE_OSX_ARCHITECTURES MATCHES "arm64;x86_64" OR 
               CMAKE_OSX_ARCHITECTURES MATCHES "x86_64;arm64")
                set(LPZROBOTS_PLATFORM "macOS_Universal" PARENT_SCOPE)
                set(LPZROBOTS_PLATFORM_STRING "macOS Universal Binary" PARENT_SCOPE)
            elseif(CMAKE_OSX_ARCHITECTURES STREQUAL "arm64")
                set(LPZROBOTS_PLATFORM "macOS_ARM64" PARENT_SCOPE)
                set(LPZROBOTS_PLATFORM_STRING "macOS ARM64 (Apple Silicon)" PARENT_SCOPE)
            elseif(CMAKE_OSX_ARCHITECTURES STREQUAL "x86_64")
                set(LPZROBOTS_PLATFORM "macOS_x86_64" PARENT_SCOPE)
                set(LPZROBOTS_PLATFORM_STRING "macOS x86_64 (Intel)" PARENT_SCOPE)
            endif()
        elseif(MACHINE_ARCH STREQUAL "arm64")
            set(LPZROBOTS_ARCH "arm64" PARENT_SCOPE)
            set(LPZROBOTS_PLATFORM "macOS_ARM64" PARENT_SCOPE)
            set(LPZROBOTS_PLATFORM_STRING "macOS ARM64 (Apple Silicon)" PARENT_SCOPE)
        elseif(MACHINE_ARCH STREQUAL "x86_64")
            set(LPZROBOTS_ARCH "x86_64" PARENT_SCOPE)
            set(LPZROBOTS_PLATFORM "macOS_x86_64" PARENT_SCOPE)
            set(LPZROBOTS_PLATFORM_STRING "macOS x86_64 (Intel)" PARENT_SCOPE)
        else()
            set(LPZROBOTS_ARCH "${MACHINE_ARCH}" PARENT_SCOPE)
            set(LPZROBOTS_PLATFORM "macOS_${MACHINE_ARCH}" PARENT_SCOPE)
            set(LPZROBOTS_PLATFORM_STRING "macOS ${MACHINE_ARCH}" PARENT_SCOPE)
        endif()
        
    elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
        set(LPZROBOTS_OS "Linux" PARENT_SCOPE)
        
        # Detect Linux architecture
        execute_process(
            COMMAND uname -m
            OUTPUT_VARIABLE MACHINE_ARCH
            OUTPUT_STRIP_TRAILING_WHITESPACE
            ERROR_QUIET
        )
        
        set(LPZROBOTS_ARCH "${MACHINE_ARCH}" PARENT_SCOPE)
        
        if(MACHINE_ARCH STREQUAL "x86_64")
            set(LPZROBOTS_PLATFORM "Linux_x86_64" PARENT_SCOPE)
            set(LPZROBOTS_PLATFORM_STRING "Linux x86_64" PARENT_SCOPE)
        elseif(MACHINE_ARCH STREQUAL "aarch64")
            set(LPZROBOTS_PLATFORM "Linux_ARM64" PARENT_SCOPE)
            set(LPZROBOTS_PLATFORM_STRING "Linux ARM64" PARENT_SCOPE)
        elseif(MACHINE_ARCH MATCHES "arm")
            set(LPZROBOTS_PLATFORM "Linux_ARM" PARENT_SCOPE)
            set(LPZROBOTS_PLATFORM_STRING "Linux ARM" PARENT_SCOPE)
        else()
            set(LPZROBOTS_PLATFORM "Linux_${MACHINE_ARCH}" PARENT_SCOPE)
            set(LPZROBOTS_PLATFORM_STRING "Linux ${MACHINE_ARCH}" PARENT_SCOPE)
        endif()
        
    elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
        set(LPZROBOTS_OS "Windows" PARENT_SCOPE)
        
        # Detect Windows architecture
        if(CMAKE_SIZEOF_VOID_P EQUAL 8)
            set(LPZROBOTS_ARCH "x64" PARENT_SCOPE)
            set(LPZROBOTS_PLATFORM "Windows_x64" PARENT_SCOPE)
            set(LPZROBOTS_PLATFORM_STRING "Windows 64-bit" PARENT_SCOPE)
        else()
            set(LPZROBOTS_ARCH "x86" PARENT_SCOPE)
            set(LPZROBOTS_PLATFORM "Windows_x86" PARENT_SCOPE)
            set(LPZROBOTS_PLATFORM_STRING "Windows 32-bit" PARENT_SCOPE)
        endif()
        
    else()
        set(LPZROBOTS_OS "${CMAKE_SYSTEM_NAME}" PARENT_SCOPE)
        set(LPZROBOTS_ARCH "${CMAKE_SYSTEM_PROCESSOR}" PARENT_SCOPE)
        set(LPZROBOTS_PLATFORM "${CMAKE_SYSTEM_NAME}_${CMAKE_SYSTEM_PROCESSOR}" PARENT_SCOPE)
        set(LPZROBOTS_PLATFORM_STRING "${CMAKE_SYSTEM_NAME} ${CMAKE_SYSTEM_PROCESSOR}" PARENT_SCOPE)
    endif()
    
    # Set compiler-specific flags
    set(LPZROBOTS_COMPILER_ID "${CMAKE_CXX_COMPILER_ID}" PARENT_SCOPE)
    set(LPZROBOTS_COMPILER_VERSION "${CMAKE_CXX_COMPILER_VERSION}" PARENT_SCOPE)
endfunction()

# Function to set platform-specific compiler flags
function(set_platform_flags target)
    detect_platform()
    
    # Common flags for all platforms
    target_compile_features(${target} PUBLIC cxx_std_17)
    
    if(LPZROBOTS_OS STREQUAL "macOS")
        # macOS specific flags
        target_compile_definitions(${target} PRIVATE MACOS)
        
        if(LPZROBOTS_PLATFORM STREQUAL "macOS_ARM64")
            # Apple Silicon specific optimizations
            target_compile_options(${target} PRIVATE
                -mcpu=apple-m1
                -mtune=native
            )
            target_compile_definitions(${target} PRIVATE MACOS_ARM64)
            
        elseif(LPZROBOTS_PLATFORM STREQUAL "macOS_x86_64")
            # Intel Mac specific optimizations
            if(CMAKE_BUILD_TYPE STREQUAL "Release")
                target_compile_options(${target} PRIVATE
                    -march=haswell  # Minimum for modern Intel Macs
                    -mtune=native
                )
            endif()
            target_compile_definitions(${target} PRIVATE MACOS_X86_64)
            
        elseif(LPZROBOTS_PLATFORM STREQUAL "macOS_Universal")
            # Universal binary settings
            set_target_properties(${target} PROPERTIES
                OSX_ARCHITECTURES "arm64;x86_64"
            )
            target_compile_definitions(${target} PRIVATE MACOS_UNIVERSAL)
        endif()
        
        # Framework paths for macOS
        target_link_options(${target} PRIVATE
            -framework Cocoa
            -framework IOKit
            -framework CoreVideo
        )
        
    elseif(LPZROBOTS_OS STREQUAL "Linux")
        # Linux specific flags
        target_compile_definitions(${target} PRIVATE LINUX)
        
        if(LPZROBOTS_ARCH STREQUAL "x86_64")
            target_compile_definitions(${target} PRIVATE LINUX_X86_64)
            if(CMAKE_BUILD_TYPE STREQUAL "Release")
                target_compile_options(${target} PRIVATE
                    -march=x86-64-v2  # Modern baseline
                    -mtune=generic
                )
            endif()
            
        elseif(LPZROBOTS_ARCH STREQUAL "aarch64")
            target_compile_definitions(${target} PRIVATE LINUX_ARM64)
            if(CMAKE_BUILD_TYPE STREQUAL "Release")
                target_compile_options(${target} PRIVATE
                    -march=armv8-a
                    -mtune=generic
                )
            endif()
        endif()
        
        # Linux-specific libraries
        find_package(Threads REQUIRED)
        target_link_libraries(${target} PRIVATE Threads::Threads)
        
    elseif(LPZROBOTS_OS STREQUAL "Windows")
        # Windows specific flags
        target_compile_definitions(${target} PRIVATE 
            WINDOWS
            _CRT_SECURE_NO_WARNINGS
            NOMINMAX  # Prevent min/max macros
        )
        
        if(MSVC)
            target_compile_options(${target} PRIVATE
                /W4           # Warning level 4
                /permissive-  # Conformance mode
                /Zc:__cplusplus  # Correct __cplusplus macro
            )
            
            if(CMAKE_BUILD_TYPE STREQUAL "Release")
                target_compile_options(${target} PRIVATE
                    /O2       # Optimize for speed
                    /GL       # Whole program optimization
                )
                target_link_options(${target} PRIVATE
                    /LTCG     # Link time code generation
                )
            endif()
        endif()
    endif()
    
    # Set preprocessor definitions for architecture
    target_compile_definitions(${target} PRIVATE
        LPZROBOTS_PLATFORM="${LPZROBOTS_PLATFORM}"
        LPZROBOTS_ARCH="${LPZROBOTS_ARCH}"
    )
endfunction()

# Function to print platform information
function(print_platform_info)
    detect_platform()
    
    message(STATUS "")
    message(STATUS "Platform Detection Results:")
    message(STATUS "  Operating System: ${LPZROBOTS_OS}")
    message(STATUS "  Architecture: ${LPZROBOTS_ARCH}")
    message(STATUS "  Platform String: ${LPZROBOTS_PLATFORM_STRING}")
    message(STATUS "  Compiler: ${LPZROBOTS_COMPILER_ID} ${LPZROBOTS_COMPILER_VERSION}")
    message(STATUS "  CMake Version: ${CMAKE_VERSION}")
    
    # Additional system information
    if(LPZROBOTS_OS STREQUAL "macOS")
        execute_process(
            COMMAND sw_vers -productVersion
            OUTPUT_VARIABLE MACOS_VERSION
            OUTPUT_STRIP_TRAILING_WHITESPACE
            ERROR_QUIET
        )
        if(MACOS_VERSION)
            message(STATUS "  macOS Version: ${MACOS_VERSION}")
        endif()
        
        # Check for Rosetta 2
        execute_process(
            COMMAND sysctl -n sysctl.proc_translated
            OUTPUT_VARIABLE PROC_TRANSLATED
            OUTPUT_STRIP_TRAILING_WHITESPACE
            ERROR_QUIET
        )
        if(PROC_TRANSLATED STREQUAL "1")
            message(STATUS "  Running under Rosetta 2: YES")
        endif()
    endif()
    
    message(STATUS "")
endfunction()

# Usage example in CMakeLists.txt:
# include(platform_detection.cmake)
# print_platform_info()
# add_library(mylib ...)
# set_platform_flags(mylib)