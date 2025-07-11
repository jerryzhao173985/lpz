# LPZRobots Simulation Build Support
# This file provides functions to build simulations using modern CMake

# Function to add a simulation executable
function(lpzrobots_add_simulation name)
    # Parse arguments
    set(options)
    set(oneValueArgs)
    set(multiValueArgs SOURCES EXTRA_LIBS)
    cmake_parse_arguments(SIM "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
    
    # Default to main.cpp if no sources specified
    if(NOT SIM_SOURCES)
        set(SIM_SOURCES main.cpp)
    endif()
    
    # Create executable
    add_executable(${name} ${SIM_SOURCES})
    
    # Set C++ standard
    target_compile_features(${name} PRIVATE cxx_std_17)
    
    # Check if we're in development mode (building within lpzrobots tree)
    set(DEVELOPMENT_MODE FALSE)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../../../selforg/CMakeLists.txt")
        set(DEVELOPMENT_MODE TRUE)
    endif()
    
    # Link against lpzrobots components
    if(TARGET lpzrobots::selforg)
        target_link_libraries(${name} PRIVATE lpzrobots::selforg)
    elseif(TARGET selforg)
        target_link_libraries(${name} PRIVATE selforg)
    elseif(DEVELOPMENT_MODE)
        # In development mode, manually add include paths and libraries
        target_include_directories(${name} PRIVATE 
            "${CMAKE_CURRENT_SOURCE_DIR}/../../../selforg"
            "${CMAKE_CURRENT_SOURCE_DIR}/../../../selforg/include"
        )
        target_link_directories(${name} PRIVATE 
            "${CMAKE_CURRENT_SOURCE_DIR}/../../../selforg"
        )
        target_link_libraries(${name} PRIVATE selforg)
        
        # Add GSL if available
        if(LPZROBOTS_HAS_GSL)
            target_compile_definitions(${name} PRIVATE HAVE_GSL)
            target_include_directories(${name} PRIVATE ${LPZROBOTS_GSL_INCLUDE_DIRS})
            target_link_libraries(${name} PRIVATE ${LPZROBOTS_GSL_LIBRARIES})
        endif()
    else()
        message(FATAL_ERROR "Cannot find selforg library")
    endif()
    
    if(TARGET lpzrobots::ode_robots)
        target_link_libraries(${name} PRIVATE lpzrobots::ode_robots)
    elseif(TARGET ode_robots)
        target_link_libraries(${name} PRIVATE ode_robots)
        # Ensure simulations can find ode_robots headers
        get_target_property(ODE_ROBOTS_SOURCE_DIR ode_robots SOURCE_DIR)
        if(ODE_ROBOTS_SOURCE_DIR)
            target_include_directories(${name} PRIVATE ${ODE_ROBOTS_SOURCE_DIR})
        endif()
    elseif(DEVELOPMENT_MODE)
        # In development mode, manually add include paths and libraries
        target_include_directories(${name} PRIVATE 
            "${CMAKE_CURRENT_SOURCE_DIR}/../.."
            "${CMAKE_CURRENT_SOURCE_DIR}/../../include"
            "${CMAKE_CURRENT_SOURCE_DIR}/../../robots"
            "${CMAKE_CURRENT_SOURCE_DIR}/../../obstacles"
            "${CMAKE_CURRENT_SOURCE_DIR}/../../utils"
        )
        target_link_directories(${name} PRIVATE 
            "${CMAKE_CURRENT_SOURCE_DIR}/../.."
        )
        target_link_libraries(${name} PRIVATE ode_robots)
        
        # Add OpenSceneGraph
        if(OPENSCENEGRAPH_FOUND)
            target_include_directories(${name} PRIVATE ${OPENSCENEGRAPH_INCLUDE_DIRS})
            target_link_libraries(${name} PRIVATE ${OPENSCENEGRAPH_LIBRARIES})
        endif()
        
        # Add ODE - use system ODE if configured
        if(LPZROBOTS_USE_SYSTEM_ODE AND LPZROBOTS_ODE_LIBRARIES)
            target_link_libraries(${name} PRIVATE ${LPZROBOTS_ODE_LIBRARIES})
        elseif(LPZROBOTS_USE_SYSTEM_ODE AND NOT LPZROBOTS_ODE_LIBRARIES)
            # System ODE was requested but libraries weren't set properly
            # Try to find ODE directly
            find_package(PkgConfig)
            if(PkgConfig_FOUND)
                pkg_check_modules(ODE ode)
                if(ODE_FOUND)
                    target_link_libraries(${name} PRIVATE ${ODE_LIBRARIES})
                    target_include_directories(${name} PRIVATE ${ODE_INCLUDE_DIRS})
                    target_link_directories(${name} PRIVATE ${ODE_LIBRARY_DIRS})
                else()
                    message(FATAL_ERROR "System ODE requested but not found")
                endif()
            else()
                message(FATAL_ERROR "System ODE requested but pkg-config not available")
            endif()
        else()
            # Use bundled ODE
            target_include_directories(${name} PRIVATE 
                "${CMAKE_CURRENT_SOURCE_DIR}/../../../opende/include"
            )
            target_link_directories(${name} PRIVATE 
                "${CMAKE_CURRENT_SOURCE_DIR}/../../../opende/ode/src/.libs"
            )
            target_link_libraries(${name} PRIVATE ode_dbl)
        endif()
    else()
        message(FATAL_ERROR "Cannot find ode_robots library")
    endif()
    
    # Add extra libraries
    if(SIM_EXTRA_LIBS)
        target_link_libraries(${name} PRIVATE ${SIM_EXTRA_LIBS})
    endif()
    
    # Add configurator if GUI tools are enabled and configurator exists
    if(BUILD_GUI_TOOLS AND TARGET configurator)
        target_link_libraries(${name} PRIVATE configurator)
    endif()
    
    # Add OpenSceneGraph libraries (required for ode_robots)
    if(NOT DEVELOPMENT_MODE)
        # When using installed lpzrobots, OSG should be handled by the targets
        # But we may need to add them explicitly if not included
        find_package(OpenSceneGraph COMPONENTS osgDB osgUtil osgViewer osgGA osgShadow osgText QUIET)
        if(OPENSCENEGRAPH_FOUND)
            target_link_libraries(${name} PRIVATE ${OPENSCENEGRAPH_LIBRARIES})
        endif()
    endif()
    
    # Add platform-specific libraries
    if(APPLE)
        find_library(OPENGL_LIBRARY OpenGL)
        find_library(GLUT_LIBRARY GLUT)
        if(OPENGL_LIBRARY AND GLUT_LIBRARY)
            target_link_libraries(${name} PRIVATE ${OPENGL_LIBRARY} ${GLUT_LIBRARY})
        endif()
    else()
        find_package(OpenGL)
        find_package(GLUT)
        if(OPENGL_FOUND)
            target_link_libraries(${name} PRIVATE OpenGL::GL OpenGL::GLU)
        endif()
        if(GLUT_FOUND)
            target_link_libraries(${name} PRIVATE GLUT::GLUT)
        endif()
    endif()
    
    # Add readline if available
    if(READLINE_LIBRARY)
        target_link_libraries(${name} PRIVATE ${READLINE_LIBRARY})
    endif()
    
    # On macOS, apply AGL filter to prevent linking with deprecated framework
    if(APPLE)
        set_property(TARGET ${name} PROPERTY RULE_LAUNCH_LINK 
            "${CMAKE_SOURCE_DIR}/cmake/filter_agl_link.sh")
    endif()
    
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
    
    # Create legacy variants only if requested
    if(LPZROBOTS_LEGACY_VARIANTS AND DEVELOPMENT_MODE)
        # Create _opt variant
        add_executable(${name}_opt ${SIM_SOURCES})
        target_compile_options(${name}_opt PRIVATE -O3 -DNDEBUG -ffast-math)
        target_compile_features(${name}_opt PRIVATE cxx_std_17)
        
        # Apply AGL filter on macOS
        if(APPLE)
            set_property(TARGET ${name}_opt PROPERTY RULE_LAUNCH_LINK 
                "${CMAKE_SOURCE_DIR}/cmake/filter_agl_link.sh")
        endif()
        
        # Link to optimized library variants if available
        if(TARGET lpzrobots::selforg_opt AND TARGET lpzrobots::ode_robots_opt)
            target_link_libraries(${name}_opt PRIVATE 
                lpzrobots::ode_robots_opt 
                lpzrobots::selforg_opt
            )
        else()
            # Fall back to regular libraries
            target_link_libraries(${name}_opt PRIVATE 
                lpzrobots::ode_robots 
                lpzrobots::selforg
            )
        endif()
        
        # Add extra libraries
        if(SIM_EXTRA_LIBS)
            target_link_libraries(${name}_opt PRIVATE ${SIM_EXTRA_LIBS})
        endif()
        
        # Add configurator if needed
        if(BUILD_GUI_TOOLS AND TARGET configurator)
            target_link_libraries(${name}_opt PRIVATE configurator)
        endif()
        
        # Create _dbg variant
        add_executable(${name}_dbg ${SIM_SOURCES})
        target_compile_options(${name}_dbg PRIVATE -g -O0)
        target_compile_features(${name}_dbg PRIVATE cxx_std_17)
        
        # Apply AGL filter on macOS
        if(APPLE)
            set_property(TARGET ${name}_dbg PROPERTY RULE_LAUNCH_LINK 
                "${CMAKE_SOURCE_DIR}/cmake/filter_agl_link.sh")
        endif()
        
        # Link to debug library variants if available
        if(TARGET lpzrobots::selforg_dbg AND TARGET lpzrobots::ode_robots_dbg)
            target_link_libraries(${name}_dbg PRIVATE 
                lpzrobots::ode_robots_dbg 
                lpzrobots::selforg_dbg
            )
        else()
            # Fall back to regular libraries
            target_link_libraries(${name}_dbg PRIVATE 
                lpzrobots::ode_robots 
                lpzrobots::selforg
            )
        endif()
        
        # Add extra libraries
        if(SIM_EXTRA_LIBS)
            target_link_libraries(${name}_dbg PRIVATE ${SIM_EXTRA_LIBS})
        endif()
        
        # Add configurator if needed
        if(BUILD_GUI_TOOLS AND TARGET configurator)
            target_link_libraries(${name}_dbg PRIVATE configurator)
        endif()
    endif()
endfunction()

# Function to generate Makefile for backward compatibility
function(lpzrobots_generate_simulation_makefile)
    # Only generate if it doesn't exist
    if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/Makefile")
        file(WRITE "${CMAKE_CURRENT_SOURCE_DIR}/Makefile"
"# Auto-generated Makefile for backward compatibility
# You can still use 'make' to build this simulation

.PHONY: all clean

all:
\t@echo \"Building with CMake...\"
\t@cmake -B build . && cmake --build build

clean:
\t@rm -rf build

# Legacy targets
normal: all
opt: all
dbg: all
")
    endif()
endfunction()

# Set up include paths for simulations
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../../
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../ode_robots
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../selforg
)

# Look for ode-dbl compatibility headers
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../../../include/ode-dbl")
    include_directories(SYSTEM ${CMAKE_CURRENT_SOURCE_DIR}/../../../include/ode-dbl)
elseif(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../../include/ode-dbl")
    include_directories(SYSTEM ${CMAKE_CURRENT_SOURCE_DIR}/../../include/ode-dbl)
endif()