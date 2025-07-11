# ODE Robots library - Physics simulation framework

# Set minimum CMake version if not already set by parent
cmake_minimum_required(VERSION 3.16)

# If this is a standalone build, set up project
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    project(OdeRobots VERSION 1.0.0 LANGUAGES CXX)
    
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
set(ODE_ROBOTS_DIRS
    .
    agents
    utils
    osg
    robots
    obstacles
    sensors
    motors
)

set(ODE_ROBOTS_SOURCES)
foreach(dir ${ODE_ROBOTS_DIRS})
    file(GLOB dir_sources ${dir}/*.cpp)
    list(APPEND ODE_ROBOTS_SOURCES ${dir_sources})
endforeach()

# Remove any flymake files
list(FILTER ODE_ROBOTS_SOURCES EXCLUDE REGEX ".*flymake.*")

# Main library sources
set(MAIN_SOURCES
    simulation.cpp
    odeconfig.cpp
)

# Combine all sources
list(APPEND ODE_ROBOTS_SOURCES ${MAIN_SOURCES})

# Create library
add_library(ode_robots ${ODE_ROBOTS_SOURCES})
add_library(lpzrobots::ode_robots ALIAS ode_robots)

# Set include directories
target_include_directories(ode_robots
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include>
        $<INSTALL_INTERFACE:include/ode_robots>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/agents
        ${CMAKE_CURRENT_SOURCE_DIR}/utils
        ${CMAKE_CURRENT_SOURCE_DIR}/osg
        ${CMAKE_CURRENT_SOURCE_DIR}/robots
        ${CMAKE_CURRENT_SOURCE_DIR}/obstacles
        ${CMAKE_CURRENT_SOURCE_DIR}/sensors
        ${CMAKE_CURRENT_SOURCE_DIR}/motors
)

# Depend on selforg
if(TARGET lpzrobots::selforg)
    target_link_libraries(ode_robots PUBLIC lpzrobots::selforg)
else()
    # Try to find selforg as external package
    find_package(PkgConfig QUIET)
    if(PkgConfig_FOUND)
        pkg_check_modules(SELFORG selforg REQUIRED)
        target_include_directories(ode_robots PRIVATE ${SELFORG_INCLUDE_DIRS})
        target_link_libraries(ode_robots PUBLIC ${SELFORG_LIBRARIES})
    else()
        message(FATAL_ERROR "Could not find selforg library")
    endif()
endif()

# ODE dependency
if(TARGET ode)
    # Using bundled ODE
    target_link_libraries(ode_robots PUBLIC ode)
    target_include_directories(ode_robots PRIVATE 
        $<TARGET_PROPERTY:ode,INTERFACE_INCLUDE_DIRECTORIES>
    )
elseif(ODE_FOUND)
    # Using system ODE
    target_include_directories(ode_robots PRIVATE ${ODE_INCLUDE_DIRS})
    target_link_libraries(ode_robots PUBLIC ${ODE_LIBRARIES})
else()
    message(FATAL_ERROR "ODE library not found")
endif()

# Other dependencies
target_link_libraries(ode_robots
    PUBLIC
        Threads::Threads
)

# Compiler features
target_compile_features(ode_robots PUBLIC cxx_std_17)

# Platform-specific settings
if(APPLE)
    target_compile_definitions(ode_robots PRIVATE MAC)
    # macOS specific libraries (OpenGL/GLUT - optional)
    find_package(OpenGL QUIET)
    if(OpenGL_FOUND)
        target_link_libraries(ode_robots PUBLIC "-framework OpenGL" "-framework GLUT")
        target_compile_definitions(ode_robots PRIVATE USE_OPENGL)
    endif()
elseif(UNIX)
    target_compile_definitions(ode_robots PRIVATE LINUX)
    # Linux specific libraries (OpenGL/GLUT - optional)
    find_package(OpenGL QUIET)
    find_package(GLUT QUIET)
    if(OpenGL_FOUND AND GLUT_FOUND)
        target_link_libraries(ode_robots PUBLIC OpenGL::GL OpenGL::GLU GLUT::GLUT)
        target_compile_definitions(ode_robots PRIVATE USE_OPENGL)
    else()
        message(WARNING "OpenGL/GLUT not found. Building without graphics support.")
        target_compile_definitions(ode_robots PRIVATE NO_GRAPHICS)
    endif()
endif()

# Optional OpenSceneGraph support
find_package(OpenSceneGraph QUIET COMPONENTS osgDB osgUtil osgViewer osgGA)
if(OPENSCENEGRAPH_FOUND)
    target_compile_definitions(ode_robots PRIVATE USE_OSG)
    target_include_directories(ode_robots PRIVATE ${OPENSCENEGRAPH_INCLUDE_DIRS})
    target_link_libraries(ode_robots PUBLIC ${OPENSCENEGRAPH_LIBRARIES})
endif()

# Set library properties
set_target_properties(ode_robots PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN ON
    POSITION_INDEPENDENT_CODE ON
    OUTPUT_NAME ode_robots
)

# Create symlinks for headers (mimics original build system)
file(GLOB_RECURSE HEADER_FILES
    simulation.h
    odeconfig.h
    agents/*.h
    utils/*.h
    osg/*.h
    robots/*.h
    obstacles/*.h
    sensors/*.h
    motors/*.h
)

# Create include/ode_robots directory structure in build directory
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/include/ode_robots)

# Create symlinks for headers so they can be found during build
foreach(header ${HEADER_FILES})
    get_filename_component(header_name ${header} NAME)
    if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.14")
        file(CREATE_LINK
            ${CMAKE_CURRENT_SOURCE_DIR}/${header}
            ${CMAKE_CURRENT_BINARY_DIR}/include/ode_robots/${header_name}
            SYMBOLIC
        )
    else()
        # Fallback for older CMake versions
        execute_process(
            COMMAND ${CMAKE_COMMAND} -E create_symlink
            ${CMAKE_CURRENT_SOURCE_DIR}/${header}
            ${CMAKE_CURRENT_BINARY_DIR}/include/ode_robots/${header_name}
        )
    endif()
endforeach()

# Also create subdirectory structure for headers
foreach(dir ${ODE_ROBOTS_DIRS})
    if(NOT ${dir} STREQUAL ".")
        file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/include/ode_robots/${dir})
        file(GLOB dir_headers ${dir}/*.h)
        foreach(header ${dir_headers})
            get_filename_component(header_name ${header} NAME)
            if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.14")
                file(CREATE_LINK
                    ${CMAKE_CURRENT_SOURCE_DIR}/${header}
                    ${CMAKE_CURRENT_BINARY_DIR}/include/ode_robots/${dir}/${header_name}
                    SYMBOLIC
                )
            else()
                execute_process(
                    COMMAND ${CMAKE_COMMAND} -E create_symlink
                    ${CMAKE_CURRENT_SOURCE_DIR}/${header}
                    ${CMAKE_CURRENT_BINARY_DIR}/include/ode_robots/${dir}/${header_name}
                )
            endif()
        endforeach()
    endif()
endforeach()

# Add the build directory to include path so symlinks are found
target_include_directories(ode_robots
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
)

# Installation
install(TARGETS ode_robots
    EXPORT LPZRobotsTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ode_robots
)

# Install headers from include directory
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/include/ode_robots)
    install(DIRECTORY include/ode_robots
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        FILES_MATCHING PATTERN "*.h"
    )
endif()

# Install headers from source directories
install(FILES
    simulation.h
    odeconfig.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ode_robots
)

# Install headers from subdirectories
foreach(subdir ${ODE_ROBOTS_DIRS})
    if(NOT ${subdir} STREQUAL "." AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${subdir})
        install(DIRECTORY ${subdir}/
            DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ode_robots/${subdir}
            FILES_MATCHING PATTERN "*.h"
        )
    endif()
endforeach()

# Install utility scripts
install(PROGRAMS
    utils/feedfile.pl
    utils/feedfile_with_videocommands.pl
    utils/encodevideo.sh
    utils/writeTextToFrames.sh
    utils/selectcolumns.pl
    DESTINATION ${CMAKE_INSTALL_BINDIR}
    OPTIONAL
)

# Install OSG data directory
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/osg/data)
    install(DIRECTORY osg/data/
        DESTINATION ${CMAKE_INSTALL_DATADIR}/lpzrobots/data
    )
endif()

# Install textures directory
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/textures)
    install(DIRECTORY textures/
        DESTINATION ${CMAKE_INSTALL_DATADIR}/lpzrobots/textures
    )
endif()

# Install examples if requested
if(BUILD_EXAMPLES AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/simulations)
    install(DIRECTORY simulations/
        DESTINATION ${CMAKE_INSTALL_DATADIR}/lpzrobots/ode_robots/simulations
        PATTERN "*.o" EXCLUDE
        PATTERN "build*" EXCLUDE
        PATTERN ".git*" EXCLUDE
    )
endif()

# Create ode_robots-config script for compatibility
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    # Only create config script if building standalone
    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/ode_robots-config.m4"
        "${CMAKE_CURRENT_BINARY_DIR}/ode_robots-config"
        @ONLY
    )
    
    install(PROGRAMS "${CMAKE_CURRENT_BINARY_DIR}/ode_robots-config"
        DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
endif()