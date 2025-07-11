# GA Tools library - Genetic Algorithm framework

# Set minimum CMake version if not already set by parent
cmake_minimum_required(VERSION 3.16)

# If this is a standalone build, set up project
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    project(GaTools VERSION 1.0.0 LANGUAGES CXX)
    
    # Set C++ standard
    set(CMAKE_CXX_STANDARD 17)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_CXX_EXTENSIONS OFF)
    
    # Find dependencies
    find_package(Threads REQUIRED)
    
    # Find selforg
    find_package(PkgConfig QUIET)
    if(PkgConfig_FOUND)
        pkg_check_modules(SELFORG selforg)
    endif()
    
    include(GNUInstallDirs)
endif()

# Collect source files from all subdirectories as defined in Makefile.conf
set(GA_TOOLS_DIRS
    .
    FitnessStrategies
    GenerationSizeStrategies
    MutationFactorStrategies
    MutationStrategies
    RandomStrategies
    SelectionStrategies
    Values
)

set(GA_TOOLS_SOURCES)
foreach(dir ${GA_TOOLS_DIRS})
    file(GLOB dir_sources ${dir}/*.cpp)
    list(APPEND GA_TOOLS_SOURCES ${dir_sources})
endforeach()

# Remove any flymake files
list(FILTER GA_TOOLS_SOURCES EXCLUDE REGEX ".*flymake.*")

# Main library sources
set(MAIN_SOURCES
    Gen.cpp
    GenContext.cpp
    GenPrototype.cpp
    Generation.cpp
    Individual.cpp
    SingletonGenAlgAPI.cpp
    SingletonGenEngine.cpp
    SingletonGenFactory.cpp
    SingletonIndividualFactory.cpp
)

# Combine all sources (main sources should already be included by the glob)
list(APPEND GA_TOOLS_SOURCES ${MAIN_SOURCES})
list(REMOVE_DUPLICATES GA_TOOLS_SOURCES)

# Create library
add_library(ga_tools ${GA_TOOLS_SOURCES})
add_library(lpzrobots::ga_tools ALIAS ga_tools)

# Set include directories
target_include_directories(ga_tools
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include>
        $<INSTALL_INTERFACE:include/ga_tools>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/FitnessStrategies
        ${CMAKE_CURRENT_SOURCE_DIR}/GenerationSizeStrategies
        ${CMAKE_CURRENT_SOURCE_DIR}/MutationFactorStrategies
        ${CMAKE_CURRENT_SOURCE_DIR}/MutationStrategies
        ${CMAKE_CURRENT_SOURCE_DIR}/RandomStrategies
        ${CMAKE_CURRENT_SOURCE_DIR}/SelectionStrategies
        ${CMAKE_CURRENT_SOURCE_DIR}/Values
)

# Depend on selforg
if(TARGET lpzrobots::selforg)
    target_link_libraries(ga_tools PUBLIC lpzrobots::selforg)
else()
    # Try to find selforg as external package
    find_package(PkgConfig QUIET)
    if(PkgConfig_FOUND)
        pkg_check_modules(SELFORG selforg REQUIRED)
        target_include_directories(ga_tools PRIVATE ${SELFORG_INCLUDE_DIRS})
        target_link_libraries(ga_tools PUBLIC ${SELFORG_LIBRARIES})
    else()
        message(FATAL_ERROR "Could not find selforg library")
    endif()
endif()

# Other dependencies
target_link_libraries(ga_tools
    PUBLIC
        Threads::Threads
)

# Compiler features
target_compile_features(ga_tools PUBLIC cxx_std_17)

# Platform-specific settings
if(APPLE)
    target_compile_definitions(ga_tools PRIVATE MAC)
elseif(UNIX)
    target_compile_definitions(ga_tools PRIVATE LINUX)
endif()

# Set library properties
set_target_properties(ga_tools PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN ON
    POSITION_INDEPENDENT_CODE ON
    OUTPUT_NAME ga_tools
)

# Create symlinks for headers (mimics original build system)
file(GLOB_RECURSE HEADER_FILES
    *.h
    FitnessStrategies/*.h
    GenerationSizeStrategies/*.h
    MutationFactorStrategies/*.h
    MutationStrategies/*.h
    RandomStrategies/*.h
    SelectionStrategies/*.h
    Values/*.h
)

# Create include/ga_tools directory structure in build directory
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/include/ga_tools)

# Create symlinks for headers so they can be found during build
foreach(header ${HEADER_FILES})
    get_filename_component(header_name ${header} NAME)
    if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.14")
        file(CREATE_LINK
            ${CMAKE_CURRENT_SOURCE_DIR}/${header}
            ${CMAKE_CURRENT_BINARY_DIR}/include/ga_tools/${header_name}
            SYMBOLIC
        )
    else()
        # Fallback for older CMake versions
        execute_process(
            COMMAND ${CMAKE_COMMAND} -E create_symlink
            ${CMAKE_CURRENT_SOURCE_DIR}/${header}
            ${CMAKE_CURRENT_BINARY_DIR}/include/ga_tools/${header_name}
        )
    endif()
endforeach()

# Also create subdirectory structure for headers
foreach(dir ${GA_TOOLS_DIRS})
    if(NOT ${dir} STREQUAL ".")
        file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/include/ga_tools/${dir})
        file(GLOB dir_headers ${dir}/*.h)
        foreach(header ${dir_headers})
            get_filename_component(header_name ${header} NAME)
            if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.14")
                file(CREATE_LINK
                    ${CMAKE_CURRENT_SOURCE_DIR}/${header}
                    ${CMAKE_CURRENT_BINARY_DIR}/include/ga_tools/${dir}/${header_name}
                    SYMBOLIC
                )
            else()
                execute_process(
                    COMMAND ${CMAKE_COMMAND} -E create_symlink
                    ${CMAKE_CURRENT_SOURCE_DIR}/${header}
                    ${CMAKE_CURRENT_BINARY_DIR}/include/ga_tools/${dir}/${header_name}
                )
            endif()
        endforeach()
    endif()
endforeach()

# Add the build directory to include path so symlinks are found
target_include_directories(ga_tools
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
)

# Installation
install(TARGETS ga_tools
    EXPORT LPZRobotsTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ga_tools
)

# Install headers from include directory
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/include/ga_tools)
    install(DIRECTORY include/ga_tools
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        FILES_MATCHING PATTERN "*.h"
    )
endif()

# Install headers from source directories
file(GLOB ROOT_HEADERS *.h)
if(ROOT_HEADERS)
    install(FILES ${ROOT_HEADERS}
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ga_tools
    )
endif()

# Install headers from subdirectories
foreach(subdir ${GA_TOOLS_DIRS})
    if(NOT ${subdir} STREQUAL "." AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${subdir})
        install(DIRECTORY ${subdir}/
            DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ga_tools/${subdir}
            FILES_MATCHING PATTERN "*.h"
        )
    endif()
endforeach()

# Install examples if requested
if(BUILD_EXAMPLES AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/simulations)
    install(DIRECTORY simulations/
        DESTINATION ${CMAKE_INSTALL_DATADIR}/lpzrobots/ga_tools/simulations
        PATTERN "*.o" EXCLUDE
        PATTERN "build*" EXCLUDE
        PATTERN ".git*" EXCLUDE
    )
endif()

# Create ga_tools-config script for compatibility
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    # Only create config script if building standalone
    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/ga_tools-config.m4"
        "${CMAKE_CURRENT_BINARY_DIR}/ga_tools-config"
        @ONLY
    )
    
    install(PROGRAMS "${CMAKE_CURRENT_BINARY_DIR}/ga_tools-config"
        DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
endif()