# GA Tools Simulations

if(BUILD_EXAMPLES)
    # Include simulation build support
    include(${CMAKE_SOURCE_DIR}/cmake/SimulationBuild.cmake)
    
    # Add GA tools specific simulations here
    # Use the same pattern as ode_robots/simulations
endif()