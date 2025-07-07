# LPZRobots Compiler Settings
# Handles compiler detection, warning flags, and optimization settings

# Function to configure compiler settings
function(lpzrobots_configure_compiler)
    # Set C++ standard
    set(CMAKE_CXX_STANDARD 17 PARENT_SCOPE)
    set(CMAKE_CXX_STANDARD_REQUIRED ON PARENT_SCOPE)
    set(CMAKE_CXX_EXTENSIONS OFF PARENT_SCOPE)
    
    # Export compiler info for summary
    set(LPZROBOTS_COMPILER_ID "${CMAKE_CXX_COMPILER_ID}" PARENT_SCOPE)
    set(LPZROBOTS_COMPILER_VERSION "${CMAKE_CXX_COMPILER_VERSION}" PARENT_SCOPE)
    
    # Compiler detection
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
        # Base warnings for all code
        set(LPZROBOTS_BASE_WARNINGS
            -Wall -Wextra -Wpedantic
            -Wformat=2 -Wuninitialized
            -Wunused-function -Wunused-variable
            -Wpointer-arith -Woverloaded-virtual
            -Wnon-virtual-dtor
            PARENT_SCOPE
        )
        
        # Extra warnings only for our code (not external dependencies)
        set(LPZROBOTS_EXTRA_WARNINGS
            -Wcast-align -Wcast-qual
            -Wconversion -Wsign-conversion
            -Wredundant-decls -Wsign-promo
            -Wold-style-cast -Wzero-as-null-pointer-constant
            PARENT_SCOPE
        )
        
        # Warnings to suppress globally
        set(LPZROBOTS_SUPPRESS_WARNINGS
            -Wno-unused-parameter
            -Wno-sign-conversion
            PARENT_SCOPE
        )
        
        # GNU-specific warnings
        if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
            set(LPZROBOTS_GNU_WARNINGS -Wlogical-op -Wnoexcept PARENT_SCOPE)
        endif()
        
        # Optimization flags for different build types
        set(LPZROBOTS_OPT_FLAGS -O3 -ffast-math -DNDEBUG PARENT_SCOPE)
        set(LPZROBOTS_DBG_FLAGS -g -O0 PARENT_SCOPE)
        
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        # MSVC warnings
        set(LPZROBOTS_BASE_WARNINGS
            /W4 /WX- /permissive-
            /wd4100 # unreferenced formal parameter
            /wd4458 # declaration hides class member
            /wd4459 # declaration hides global
            PARENT_SCOPE
        )
        
        set(LPZROBOTS_OPT_FLAGS /O2 /DNDEBUG PARENT_SCOPE)
        set(LPZROBOTS_DBG_FLAGS /Od /Zi PARENT_SCOPE)
    endif()
    
    # Export compiler info
    set(LPZROBOTS_COMPILER_ID ${CMAKE_CXX_COMPILER_ID} PARENT_SCOPE)
    set(LPZROBOTS_COMPILER_VERSION ${CMAKE_CXX_COMPILER_VERSION} PARENT_SCOPE)
endfunction()

# Function to apply compiler settings to a target
function(lpzrobots_apply_compiler_settings target)
    cmake_parse_arguments(PARSE_ARGV 1 SETTINGS "EXTRA_WARNINGS;NO_WARNINGS" "" "")
    
    # Apply base warnings
    if(NOT SETTINGS_NO_WARNINGS)
        target_compile_options(${target} PRIVATE ${LPZROBOTS_BASE_WARNINGS})
        target_compile_options(${target} PRIVATE ${LPZROBOTS_SUPPRESS_WARNINGS})
        
        # Apply extra warnings if requested
        if(SETTINGS_EXTRA_WARNINGS)
            target_compile_options(${target} PRIVATE ${LPZROBOTS_EXTRA_WARNINGS})
        endif()
        
        # Apply GNU-specific warnings
        if(DEFINED LPZROBOTS_GNU_WARNINGS)
            target_compile_options(${target} PRIVATE ${LPZROBOTS_GNU_WARNINGS})
        endif()
    endif()
    
    # Apply C++ standard
    target_compile_features(${target} PUBLIC cxx_std_17)
    
    # Apply sanitizers if enabled
    if(LPZROBOTS_ENABLE_SANITIZERS AND NOT target MATCHES "_opt$")
        lpzrobots_add_sanitizers(${target})
    endif()
    
    # Apply static analysis if enabled
    lpzrobots_apply_static_analysis(${target})
endfunction()

# Function to apply static analysis tools
function(lpzrobots_apply_static_analysis target)
    # clang-tidy
    if(LPZROBOTS_ENABLE_CLANG_TIDY)
        find_program(CLANG_TIDY_EXE NAMES clang-tidy)
        if(CLANG_TIDY_EXE)
            set(CLANG_TIDY_COMMAND
                ${CLANG_TIDY_EXE}
                -checks=-*,readability-*,modernize-*,performance-*,cppcoreguidelines-*,bugprone-*
                -header-filter=.*
                -warnings-as-errors=*
            )
            set_target_properties(${target} PROPERTIES CXX_CLANG_TIDY "${CLANG_TIDY_COMMAND}")
        else()
            message(WARNING "clang-tidy requested but not found")
        endif()
    endif()
    
    # cppcheck
    if(LPZROBOTS_ENABLE_CPPCHECK)
        find_program(CPPCHECK_EXE NAMES cppcheck)
        if(CPPCHECK_EXE)
            set(CPPCHECK_COMMAND
                ${CPPCHECK_EXE}
                --enable=warning,style,performance,portability
                --suppress=missingIncludeSystem
                --error-exitcode=1
                --inline-suppr
                --std=c++17
            )
            set_target_properties(${target} PROPERTIES CXX_CPPCHECK "${CPPCHECK_COMMAND}")
        else()
            message(WARNING "cppcheck requested but not found")
        endif()
    endif()
    
    # include-what-you-use
    if(LPZROBOTS_ENABLE_IWYU)
        find_program(IWYU_EXE NAMES include-what-you-use iwyu)
        if(IWYU_EXE)
            set(IWYU_COMMAND ${IWYU_EXE} -Xiwyu --cxx17ns)
            set_target_properties(${target} PROPERTIES CXX_INCLUDE_WHAT_YOU_USE "${IWYU_COMMAND}")
        else()
            message(WARNING "include-what-you-use requested but not found")
        endif()
    endif()
endfunction()

# Function to create optimized and debug variants
function(lpzrobots_create_variants target)
    if(NOT BUILD_OPT_LIBRARIES AND NOT BUILD_DBG_LIBRARIES)
        return()
    endif()
    
    # Get the base target's sources
    get_target_property(SOURCES ${target} SOURCES)
    if(NOT SOURCES)
        message(WARNING "Cannot get sources from target ${target}")
        return()
    endif()
    
    # Helper function to copy target properties
    function(copy_target_properties from_target to_target)
        # Copy include directories
        foreach(prop INCLUDE_DIRECTORIES INTERFACE_INCLUDE_DIRECTORIES)
            get_target_property(value ${from_target} ${prop})
            if(value)
                set_property(TARGET ${to_target} PROPERTY ${prop} ${value})
            endif()
        endforeach()
        
        # Copy link libraries
        foreach(prop LINK_LIBRARIES INTERFACE_LINK_LIBRARIES)
            get_target_property(value ${from_target} ${prop})
            if(value)
                set_property(TARGET ${to_target} PROPERTY ${prop} ${value})
            endif()
        endforeach()
        
        # Copy compile definitions
        foreach(prop COMPILE_DEFINITIONS INTERFACE_COMPILE_DEFINITIONS)
            get_target_property(value ${from_target} ${prop})
            if(value)
                set_property(TARGET ${to_target} PROPERTY ${prop} ${value})
            endif()
        endforeach()
        
        # Copy compile features
        foreach(prop COMPILE_FEATURES INTERFACE_COMPILE_FEATURES)
            get_target_property(value ${from_target} ${prop})
            if(value)
                set_property(TARGET ${to_target} PROPERTY ${prop} ${value})
            endif()
        endforeach()
    endfunction()
    
    # Create optimized variant
    if(BUILD_OPT_LIBRARIES)
        add_library(${target}_opt STATIC ${SOURCES})
        add_library(lpzrobots::${target}_opt ALIAS ${target}_opt)
        
        # Copy properties from base target
        copy_target_properties(${target} ${target}_opt)
        
        # Add optimization flags
        target_compile_options(${target}_opt PRIVATE ${LPZROBOTS_OPT_FLAGS})
        set_target_properties(${target}_opt PROPERTIES
            POSITION_INDEPENDENT_CODE ON
        )
        
        # Install and export
        if(LPZROBOTS_INSTALL_TYPE STREQUAL "USER")
            install(TARGETS ${target}_opt
                EXPORT LPZRobotsTargets
                ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
            )
        else()
            install(TARGETS ${target}_opt EXPORT LPZRobotsTargets)
        endif()
    endif()
    
    # Create debug variant
    if(BUILD_DBG_LIBRARIES)
        add_library(${target}_dbg STATIC ${SOURCES})
        add_library(lpzrobots::${target}_dbg ALIAS ${target}_dbg)
        
        # Copy properties from base target
        copy_target_properties(${target} ${target}_dbg)
        
        # Add debug flags
        target_compile_options(${target}_dbg PRIVATE ${LPZROBOTS_DBG_FLAGS})
        set_target_properties(${target}_dbg PROPERTIES
            POSITION_INDEPENDENT_CODE ON
        )
        
        # Install and export
        if(LPZROBOTS_INSTALL_TYPE STREQUAL "USER")
            install(TARGETS ${target}_dbg
                EXPORT LPZRobotsTargets
                ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
            )
        else()
            install(TARGETS ${target}_dbg EXPORT LPZRobotsTargets)
        endif()
    endif()
endfunction()

# Function to handle SIMD optimizations
function(lpzrobots_enable_simd target)
    if(NOT ENABLE_SIMD)
        return()
    endif()
    
    include(CheckCXXCompilerFlag)
    
    # ARM NEON (Apple Silicon)
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "arm64|aarch64")
        check_cxx_compiler_flag("-march=armv8-a+simd" HAS_ARM_NEON)
        if(HAS_ARM_NEON)
            target_compile_options(${target} PRIVATE -march=armv8-a+simd)
            target_compile_definitions(${target} PRIVATE USE_ARM_NEON)
            message(STATUS "Enabling ARM NEON SIMD for ${target}")
        endif()
    # x86 SIMD
    elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|i686")
        check_cxx_compiler_flag("-mavx2" HAS_AVX2)
        if(HAS_AVX2)
            target_compile_options(${target} PRIVATE -mavx2)
            target_compile_definitions(${target} PRIVATE USE_AVX2)
            message(STATUS "Enabling AVX2 SIMD for ${target}")
        endif()
    endif()
endfunction()