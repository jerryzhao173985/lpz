# ODE (Open Dynamics Engine) Integration
# This CMakeLists.txt provides a compatibility layer for using system ODE with lpzrobots

cmake_minimum_required(VERSION 3.20)
project(opende_compat)

# Find system ODE
find_package(PkgConfig REQUIRED)
pkg_check_modules(ODE REQUIRED ode)

# Create interface library for ODE compatibility
add_library(ode_compat INTERFACE)

# Add include directories
target_include_directories(ode_compat INTERFACE
    ${ODE_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_BINARY_DIR}/include
)

# Link libraries
target_link_libraries(ode_compat INTERFACE
    ${ODE_LIBRARIES}
)

# Add compile definitions
target_compile_definitions(ode_compat INTERFACE
    dDOUBLE
)

# Create ode-dbl compatibility headers
set(ODE_DBL_DIR "${CMAKE_CURRENT_BINARY_DIR}/include/ode-dbl")
file(MAKE_DIRECTORY ${ODE_DBL_DIR})

# Create symlinks for ODE headers with -dbl suffix
if(ODE_INCLUDE_DIRS)
    file(GLOB ODE_HEADERS "${ODE_INCLUDE_DIRS}/ode/*.h")
    foreach(header ${ODE_HEADERS})
        get_filename_component(header_name ${header} NAME)
        execute_process(
            COMMAND ${CMAKE_COMMAND} -E create_symlink ${header} ${ODE_DBL_DIR}/${header_name}
        )
    endforeach()
endif()

# Export targets
install(TARGETS ode_compat
    EXPORT lpzrobots-targets
)

# Install compatibility headers
install(DIRECTORY ${ODE_DBL_DIR}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# Create alias for consistency
add_library(lpzrobots::ode ALIAS ode_compat)