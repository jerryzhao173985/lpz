# LPZRobots Library Management
# Provides unified functions for creating component libraries with minimal code duplication

# Main function to create a LPZRobots component library
function(lpzrobots_add_component_library name)
    cmake_parse_arguments(PARSE_ARGV 1 ARG
        "HEADER_ONLY;NO_INSTALL;NO_EXPORT"
        "VERSION"
        "SOURCES;SUBDIRS;PUBLIC_DEPS;PRIVATE_DEPS;DEFINES;PUBLIC_DEFINES"
    )
    
    # Collect sources from subdirectories if specified
    if(ARG_SUBDIRS)
        foreach(dir ${ARG_SUBDIRS})
            file(GLOB DIR_SOURCES 
                ${CMAKE_CURRENT_SOURCE_DIR}/${dir}/*.cpp
                ${CMAKE_CURRENT_SOURCE_DIR}/${dir}/*.c
            )
            # Filter out backup files
            list(FILTER DIR_SOURCES EXCLUDE REGEX ".*\\.(bak|restore).*")
            list(FILTER DIR_SOURCES EXCLUDE REGEX ".*\\.!.*")
            list(APPEND ARG_SOURCES ${DIR_SOURCES})
        endforeach()
    endif()
    
    # Create the library
    if(ARG_HEADER_ONLY)
        add_library(${name} INTERFACE)
        add_library(lpzrobots::${name} ALIAS ${name})
    else()
        add_library(${name} STATIC ${ARG_SOURCES})
        add_library(lpzrobots::${name} ALIAS ${name})
        
        # Set C++ standard
        target_compile_features(${name} PUBLIC cxx_std_17)
        
        # Apply base compiler settings
        lpzrobots_apply_compiler_settings(${name})
        
        # Apply configuration-specific flags using generator expressions
        target_compile_options(${name} PRIVATE
            $<$<CONFIG:Debug>:-g -O0>
            $<$<CONFIG:Release>:-O3 -ffast-math>
            $<$<CONFIG:RelWithDebInfo>:-O2 -g>
            $<$<CONFIG:MinSizeRel>:-Os>
        )
        
        target_compile_definitions(${name} PRIVATE
            $<$<CONFIG:Release>:NDEBUG>
            $<$<CONFIG:RelWithDebInfo>:NDEBUG>
            $<$<CONFIG:MinSizeRel>:NDEBUG>
        )
    endif()
    
    # Set up include directories using modern CMake patterns
    if(ARG_HEADER_ONLY)
        target_include_directories(${name} INTERFACE
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/${name}>
            $<INSTALL_INTERFACE:include>
            $<INSTALL_INTERFACE:include/${name}>
        )
    else()
        target_include_directories(${name}
            PUBLIC
                $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/${name}>
                $<INSTALL_INTERFACE:include>
                $<INSTALL_INTERFACE:include/${name}>
            PRIVATE
                ${CMAKE_CURRENT_SOURCE_DIR}
        )
        
        # Add subdirectories as private includes
        if(ARG_SUBDIRS)
            foreach(dir ${ARG_SUBDIRS})
                target_include_directories(${name} PRIVATE 
                    ${CMAKE_CURRENT_SOURCE_DIR}/${dir}
                )
            endforeach()
        endif()
    endif()
    
    # Dependencies
    if(ARG_PUBLIC_DEPS)
        target_link_libraries(${name} PUBLIC ${ARG_PUBLIC_DEPS})
    endif()
    if(ARG_PRIVATE_DEPS)
        target_link_libraries(${name} PRIVATE ${ARG_PRIVATE_DEPS})
    endif()
    
    # Compile definitions
    if(ARG_DEFINES)
        target_compile_definitions(${name} PRIVATE ${ARG_DEFINES})
    endif()
    if(ARG_PUBLIC_DEFINES)
        target_compile_definitions(${name} PUBLIC ${ARG_PUBLIC_DEFINES})
    endif()
    
    # Platform-specific settings
    lpzrobots_apply_platform_settings(${name})
    
    # Set library properties
    if(NOT ARG_HEADER_ONLY)
        set_target_properties(${name} PROPERTIES
            VERSION ${PROJECT_VERSION}
            SOVERSION ${PROJECT_VERSION_MAJOR}
            CXX_VISIBILITY_PRESET hidden
            VISIBILITY_INLINES_HIDDEN ON
            POSITION_INDEPENDENT_CODE ON
            DEBUG_POSTFIX "_dbg"
            RELEASE_POSTFIX "_opt"
            RELWITHDEBINFO_POSTFIX "_reldbg"
            MINSIZEREL_POSTFIX "_min"
        )
    endif()
    
    # Create legacy library variants for backward compatibility (if enabled)
    if(NOT ARG_HEADER_ONLY AND LPZROBOTS_LEGACY_VARIANTS)
        lpzrobots_create_legacy_variants(${name} "${ARG_SOURCES}" "${ARG_SUBDIRS}")
    endif()
    
    # Installation
    if(NOT ARG_NO_INSTALL)
        lpzrobots_install_component(${name} ${ARG_NO_EXPORT})
    endif()
    
    # Generate config script
    lpzrobots_generate_config_script(${name})
endfunction()

# Function to create legacy library variants (_opt and _dbg) for backward compatibility
function(lpzrobots_create_legacy_variants name sources subdirs)
    message(STATUS "Creating legacy library variants for ${name} (deprecated)")
    
    # Create optimized variant
    if(BUILD_OPT_LIBRARIES)
        add_library(${name}_opt STATIC ${sources})
        add_library(lpzrobots::${name}_opt ALIAS ${name}_opt)
        
        # Copy all properties from main target
        lpzrobots_copy_target_properties(${name} ${name}_opt)
        
        # Override with optimization flags
        target_compile_options(${name}_opt PRIVATE -O3 -ffast-math)
        target_compile_definitions(${name}_opt PRIVATE NDEBUG)
        
        # SIMD optimizations
        if(ENABLE_SIMD)
            lpzrobots_enable_simd(${name}_opt)
        endif()
        
        # Install
        install(TARGETS ${name}_opt
            EXPORT LPZRobotsTargets
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        )
    endif()
    
    # Create debug variant
    if(BUILD_DBG_LIBRARIES)
        add_library(${name}_dbg STATIC ${sources})
        add_library(lpzrobots::${name}_dbg ALIAS ${name}_dbg)
        
        # Copy all properties from main target
        lpzrobots_copy_target_properties(${name} ${name}_dbg)
        
        # Override with debug flags
        target_compile_options(${name}_dbg PRIVATE -g -O0)
        
        # Install
        install(TARGETS ${name}_dbg
            EXPORT LPZRobotsTargets
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        )
    endif()
endfunction()

# Helper function to copy target properties
function(lpzrobots_copy_target_properties from_target to_target)
    # Get include directories
    get_target_property(includes ${from_target} INCLUDE_DIRECTORIES)
    if(includes)
        set_property(TARGET ${to_target} PROPERTY INCLUDE_DIRECTORIES ${includes})
    endif()
    
    get_target_property(interface_includes ${from_target} INTERFACE_INCLUDE_DIRECTORIES)
    if(interface_includes)
        set_property(TARGET ${to_target} PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${interface_includes})
    endif()
    
    # Get link libraries
    get_target_property(link_libs ${from_target} LINK_LIBRARIES)
    if(link_libs)
        # Fix library references for variants
        set(new_link_libs)
        foreach(lib ${link_libs})
            if(lib MATCHES "^lpzrobots::")
                string(REPLACE "lpzrobots::" "" lib_name ${lib})
                string(REGEX MATCH "_opt$" is_opt ${to_target})
                string(REGEX MATCH "_dbg$" is_dbg ${to_target})
                if(is_opt AND TARGET lpzrobots::${lib_name}_opt)
                    list(APPEND new_link_libs lpzrobots::${lib_name}_opt)
                elseif(is_dbg AND TARGET lpzrobots::${lib_name}_dbg)
                    list(APPEND new_link_libs lpzrobots::${lib_name}_dbg)
                else()
                    list(APPEND new_link_libs ${lib})
                endif()
            else()
                list(APPEND new_link_libs ${lib})
            endif()
        endforeach()
        set_property(TARGET ${to_target} PROPERTY LINK_LIBRARIES ${new_link_libs})
    endif()
    
    get_target_property(interface_link_libs ${from_target} INTERFACE_LINK_LIBRARIES)
    if(interface_link_libs)
        set_property(TARGET ${to_target} PROPERTY INTERFACE_LINK_LIBRARIES ${interface_link_libs})
    endif()
    
    # Get compile definitions
    get_target_property(compile_defs ${from_target} COMPILE_DEFINITIONS)
    if(compile_defs)
        set_property(TARGET ${to_target} PROPERTY COMPILE_DEFINITIONS ${compile_defs})
    endif()
    
    get_target_property(interface_compile_defs ${from_target} INTERFACE_COMPILE_DEFINITIONS)
    if(interface_compile_defs)
        set_property(TARGET ${to_target} PROPERTY INTERFACE_COMPILE_DEFINITIONS ${interface_compile_defs})
    endif()
    
    # Get compile features
    get_target_property(compile_features ${from_target} COMPILE_FEATURES)
    if(compile_features)
        set_property(TARGET ${to_target} PROPERTY COMPILE_FEATURES ${compile_features})
    endif()
    
    get_target_property(interface_compile_features ${from_target} INTERFACE_COMPILE_FEATURES)
    if(interface_compile_features)
        set_property(TARGET ${to_target} PROPERTY INTERFACE_COMPILE_FEATURES ${interface_compile_features})
    endif()
    
    # Copy other properties
    set_target_properties(${to_target} PROPERTIES
        VERSION ${PROJECT_VERSION}
        SOVERSION ${PROJECT_VERSION_MAJOR}
        CXX_VISIBILITY_PRESET hidden
        VISIBILITY_INLINES_HIDDEN ON
        POSITION_INDEPENDENT_CODE ON
    )
    
    # Apply platform settings
    lpzrobots_apply_platform_settings(${to_target})
endfunction()

# Function to apply platform-specific settings
function(lpzrobots_apply_platform_settings target)
    if(APPLE)
        target_compile_definitions(${target} PRIVATE MAC)
        if(CMAKE_SYSTEM_PROCESSOR MATCHES "arm64|aarch64")
            target_compile_definitions(${target} PRIVATE MAC_ARM64)
        endif()
    elseif(UNIX)
        target_compile_definitions(${target} PRIVATE LINUX)
    elseif(WIN32)
        target_compile_definitions(${target} PRIVATE WINDOWS)
    endif()
endfunction()

# Function to create header symlinks in unified include directory
# This function creates both hierarchical and flattened symlinks to support
# both include styles: <component/subdir/header.h> and <component/header.h>
function(lpzrobots_create_header_symlinks component)
    # Create component include directory in source tree's unified include dir
    set(INCLUDE_DIR ${CMAKE_SOURCE_DIR}/include/${component})
    file(MAKE_DIRECTORY ${INCLUDE_DIR})
    
    # Collect all header files
    file(GLOB_RECURSE HEADER_FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/*.h
        ${CMAKE_CURRENT_SOURCE_DIR}/*.hpp
    )
    
    # Filter out certain directories if needed
    list(FILTER HEADER_FILES EXCLUDE REGEX ".*/test/.*")
    list(FILTER HEADER_FILES EXCLUDE REGEX ".*/tests/.*")
    list(FILTER HEADER_FILES EXCLUDE REGEX ".*/obj/.*")
    list(FILTER HEADER_FILES EXCLUDE REGEX ".*/build.*/.*")
    list(FILTER HEADER_FILES EXCLUDE REGEX ".*/include/.*")  # Avoid recursion
    
    # Create symlinks at configure time
    foreach(header ${HEADER_FILES})
        # Get relative path from component root
        file(RELATIVE_PATH rel_path ${CMAKE_CURRENT_SOURCE_DIR} ${header})
        get_filename_component(header_name ${header} NAME)
        get_filename_component(header_dir ${rel_path} DIRECTORY)
        
        # Create hierarchical symlink (preserving subdirectory structure)
        if(NOT "${header_dir}" STREQUAL "")
            file(MAKE_DIRECTORY ${INCLUDE_DIR}/${header_dir})
            set(hierarchical_link ${INCLUDE_DIR}/${rel_path})
            if(NOT EXISTS ${hierarchical_link})
                execute_process(
                    COMMAND ${CMAKE_COMMAND} -E create_symlink ${header} ${hierarchical_link}
                    RESULT_VARIABLE result
                    ERROR_QUIET
                )
            endif()
        endif()
        
        # Create flattened symlink (just the filename in component root)
        set(flattened_link ${INCLUDE_DIR}/${header_name})
        if(NOT EXISTS ${flattened_link})
            execute_process(
                COMMAND ${CMAKE_COMMAND} -E create_symlink ${header} ${flattened_link}
                RESULT_VARIABLE result
                ERROR_QUIET
            )
        endif()
    endforeach()
    
    # Add the unified include directory to this target's interface
    if(TARGET ${component})
        target_include_directories(${component} PUBLIC
            $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
        )
    endif()
endfunction()

# Function to install component
function(lpzrobots_install_component target no_export)
    if(NOT no_export)
        install(TARGETS ${target}
            EXPORT LPZRobotsTargets
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
            INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        )
    else()
        install(TARGETS ${target}
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        )
    endif()
    
    # Install headers
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/include/${target})
        install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/${target}
            DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
            FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp"
        )
    endif()
    
    # Also install any headers in the root that should be public
    file(GLOB ROOT_HEADERS
        ${CMAKE_CURRENT_SOURCE_DIR}/*.h
        ${CMAKE_CURRENT_SOURCE_DIR}/*.hpp
    )
    if(ROOT_HEADERS)
        install(FILES ${ROOT_HEADERS}
            DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${target}
        )
    endif()
endfunction()

# Function to enable SIMD optimizations
function(lpzrobots_enable_simd target)
    include(CheckCXXCompilerFlag)
    
    # ARM NEON (Apple Silicon and ARM64)
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "arm64|aarch64")
        check_cxx_compiler_flag("-march=armv8-a+simd" HAS_ARM_NEON)
        if(HAS_ARM_NEON)
            target_compile_options(${target} PRIVATE -march=armv8-a+simd)
            target_compile_definitions(${target} PRIVATE USE_ARM_NEON)
            message(STATUS "Enabling ARM NEON SIMD optimizations for ${target}")
        endif()
    # x86 SIMD
    elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|i686|AMD64")
        check_cxx_compiler_flag("-mavx2" HAS_AVX2)
        if(HAS_AVX2)
            target_compile_options(${target} PRIVATE -mavx2)
            target_compile_definitions(${target} PRIVATE USE_AVX2)
            message(STATUS "Enabling AVX2 SIMD optimizations for ${target}")
        endif()
    endif()
endfunction()

# Function to setup configurator support properly
function(lpzrobots_setup_configurator_support target)
    # Components should ALWAYS compile with NOCONFIGURATOR
    # Only simulations may link to configurator
    target_compile_definitions(${target} PRIVATE NOCONFIGURATOR)
    
    # The legacy variants also need NOCONFIGURATOR
    if(TARGET ${target}_opt)
        target_compile_definitions(${target}_opt PRIVATE NOCONFIGURATOR)
    endif()
    if(TARGET ${target}_dbg)
        target_compile_definitions(${target}_dbg PRIVATE NOCONFIGURATOR)
    endif()
endfunction()