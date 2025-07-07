# ODE Robots Simulations

# Include simulation build support
include(${CMAKE_SOURCE_DIR}/cmake/SimulationBuild.cmake)

# Only build simulations if we're building examples
if(NOT BUILD_EXAMPLES)
    return()
endif()

# Function to add all simulations in a directory
function(add_simulation_directory dir)
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${dir}/main.cpp)
        # Check for Makefile.conf to get additional files
        if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${dir}/Makefile.conf)
            file(STRINGS ${CMAKE_CURRENT_SOURCE_DIR}/${dir}/Makefile.conf conf_lines)
            foreach(line ${conf_lines})
                if(line MATCHES "^FILES[ \t]*=[ \t]*(.+)")
                    string(REGEX REPLACE "^FILES[ \t]*=[ \t]*" "" files_list "${line}")
                    string(REPLACE " " ";" files_list "${files_list}")
                    set(extra_sources)
                    foreach(file ${files_list})
                        if(NOT file STREQUAL "main")
                            list(APPEND extra_sources ${CMAKE_CURRENT_SOURCE_DIR}/${dir}/${file}.cpp)
                        endif()
                    endforeach()
                endif()
            endforeach()
        endif()
        
        # Create simulation
        lpzrobots_add_simulation(${dir}_sim
            SOURCES 
                ${CMAKE_CURRENT_SOURCE_DIR}/${dir}/main.cpp
                ${extra_sources}
        )
        
        # Set output name to 'start' to match original
        set_target_properties(${dir}_sim PROPERTIES
            OUTPUT_NAME start
            RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${dir}
        )
        
        # Generate backward-compatible Makefile
        lpzrobots_generate_simulation_makefile()
    endif()
endfunction()

# List of example simulations to build
# For now, just build the templates and a few key examples
# Users can uncomment or add more as needed
set(EXAMPLE_SIMULATIONS
    template_sphererobot
    template_schlange
    template_hexapod
    template_onerobot
    sphererobot_novel_controllers  # Demonstrates novel homeokinetic algorithms
    hexapod_novel_controllers      # Hexapod with novel algorithms
    
    # Core robot examples (uncomment to build)
    # sphericalrobot
    # hexapod
    # wheelie
    # nimm2_arena
    # barrel
    # humanoid
    
    # Advanced examples (uncomment to build)
    # multiarm
    # octopus
    # caterpillar
    # hand
    
    # Research simulations (uncomment to build)
    # homeok_vs_static
    # reinforcement
    # teaching
    # entropy
    
    # Add any other simulations from the full list as needed
)

# Add each simulation
foreach(sim ${EXAMPLE_SIMULATIONS})
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${sim})
        # Check if the simulation has its own CMakeLists.txt
        if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${sim}/CMakeLists.txt)
            add_subdirectory(${sim})
        else()
            add_simulation_directory(${sim})
        endif()
    endif()
endforeach()

# Install simulation sources for users to modify
if(LPZROBOTS_INSTALL_TYPE STREQUAL "USER")
    install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/
        DESTINATION ${LPZROBOTS_SIMULATIONS_DIR}
        FILES_MATCHING 
        PATTERN "*.cpp"
        PATTERN "*.h"
        PATTERN "Makefile.conf"
        PATTERN "*.cfg"
        PATTERN "CMakeLists.txt" EXCLUDE
        PATTERN "build" EXCLUDE
    )
endif()