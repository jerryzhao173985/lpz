# Selforg Simulations
# These are simulations that don't require ode_robots (no physics)

# Include simulation build support
include(${CMAKE_SOURCE_DIR}/cmake/SimulationBuild.cmake)

# Only build simulations if we're building examples
if(NOT BUILD_EXAMPLES)
    return()
endif()

# Function to add a selforg simulation
function(add_selforg_simulation name)
    set(sources)
    
    # Check if there's a main.cpp or other main file
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${name}/main.cpp)
        list(APPEND sources ${CMAKE_CURRENT_SOURCE_DIR}/${name}/main.cpp)
    endif()
    
    # Add cmdline and console if they exist
    foreach(file cmdline console)
        if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${name}/${file}.cpp)
            list(APPEND sources ${CMAKE_CURRENT_SOURCE_DIR}/${name}/${file}.cpp)
        endif()
    endforeach()
    
    # Check for additional source files in Makefile.conf
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${name}/Makefile.conf)
        file(STRINGS ${CMAKE_CURRENT_SOURCE_DIR}/${name}/Makefile.conf conf_lines)
        foreach(line ${conf_lines})
            if(line MATCHES "^FILES[ \t]*=[ \t]*(.+)")
                string(REGEX REPLACE "^FILES[ \t]*=[ \t]*" "" files_list "${line}")
                string(REPLACE " " ";" files_list "${files_list}")
                foreach(file ${files_list})
                    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${name}/${file}.cpp)
                        list(APPEND sources ${CMAKE_CURRENT_SOURCE_DIR}/${name}/${file}.cpp)
                    endif()
                endforeach()
                # Clear sources and rebuild from FILES list
                set(sources)
                foreach(file ${files_list})
                    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${name}/${file}.cpp)
                        list(APPEND sources ${CMAKE_CURRENT_SOURCE_DIR}/${name}/${file}.cpp)
                    endif()
                endforeach()
            endif()
        endforeach()
    endif()
    
    if(sources)
        # Create executable
        add_executable(${name}_sim ${sources})
        
        # Link to selforg
        target_link_libraries(${name}_sim PRIVATE
            lpzrobots::selforg
        )
        
        # Check if console.cpp is used (needs readline)
        foreach(src ${sources})
            if(src MATCHES "console\\.cpp$")
                find_library(READLINE_LIB readline)
                find_library(HISTORY_LIB history)
                if(READLINE_LIB)
                    target_link_libraries(${name}_sim PRIVATE ${READLINE_LIB})
                endif()
                if(HISTORY_LIB)
                    target_link_libraries(${name}_sim PRIVATE ${HISTORY_LIB})
                endif()
                # If history lib not found separately, it might be part of readline
                if(READLINE_LIB AND NOT HISTORY_LIB)
                    target_link_libraries(${name}_sim PRIVATE ${READLINE_LIB})
                endif()
                break()
            endif()
        endforeach()
        
        # Include directories
        target_include_directories(${name}_sim PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/${name}
        )
        
        # Set output name to match original
        set_target_properties(${name}_sim PROPERTIES
            OUTPUT_NAME ${name}
            RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${name}
        )
        
        # Set C++ standard
        target_compile_features(${name}_sim PRIVATE cxx_std_17)
    endif()
endfunction()

# List of selforg simulations
set(SELFORG_SIMULATIONS
    dblcontingency
    dblcontingency_direct
    simplesystems
    pendulum
    # tcpcommunication  # Requires special handling for Socket class
    # manipuexperts    # May require additional dependencies
)

# Add each simulation
foreach(sim ${SELFORG_SIMULATIONS})
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${sim})
        add_selforg_simulation(${sim})
    endif()
endforeach()

# Special handling for tcpcommunication if needed
# DISABLED: Socket.cpp has major issues that need fixing
# if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/tcpcommunication)
#     add_selforg_simulation(tcpcommunication)
#     if(TARGET tcpcommunication_sim)
#         # Add Socket.cpp to the sources
#         target_sources(tcpcommunication_sim PRIVATE
#             ${CMAKE_CURRENT_SOURCE_DIR}/tcpcommunication/Socket.cpp
#         )
#     endif()
# endif()

# Install simulation sources for users to modify
if(LPZROBOTS_INSTALL_TYPE STREQUAL "USER")
    install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/
        DESTINATION ${LPZROBOTS_DATA_DIR}/selforg_simulations
        FILES_MATCHING 
        PATTERN "*.cpp"
        PATTERN "*.h"
        PATTERN "Makefile.conf"
        PATTERN "*.cfg"
        PATTERN "CMakeLists.txt" EXCLUDE
        PATTERN "build" EXCLUDE
    )
endif()