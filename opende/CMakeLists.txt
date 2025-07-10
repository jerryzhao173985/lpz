cmake_minimum_required(VERSION 3.20)
project(opende VERSION 0.16.2 LANGUAGES CXX C)

# Set C++ standard for ODE
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Configuration options
option(ODE_ENABLE_GIMPACT "Enable GIMPACT collision detection" ON)
option(ODE_ENABLE_OPCODE "Enable OPCODE collision detection" ON)
option(ODE_ENABLE_OU "Enable OU utility library" ON)

# Platform-specific settings
if(WIN32)
    add_definitions(-DWIN32 -D_CRT_SECURE_NO_WARNINGS)
elseif(APPLE)
    set(CMAKE_MACOSX_RPATH ON)
elseif(UNIX)
    find_package(Threads REQUIRED)
endif()

# Always use double precision for LPZRobots
add_definitions(-DdDOUBLE)

# Find required dependencies
find_package(Threads REQUIRED)

# Collect ODE core source files
file(GLOB ODE_CORE_SOURCES
    ode/src/*.cpp
    ode/src/*.c
)

# Collect joint source files  
file(GLOB ODE_JOINT_SOURCES
    ode/src/joints/*.cpp
)

# OPCODE sources (if enabled)
if(ODE_ENABLE_OPCODE)
    file(GLOB_RECURSE OPCODE_SOURCES
        OPCODE/*.cpp
        OPCODE/*.c
    )
    add_definitions(-DdTRIMESH_ENABLED -DdTRIMESH_OPCODE)
endif()

# GIMPACT sources (if enabled)  
if(ODE_ENABLE_GIMPACT)
    file(GLOB_RECURSE GIMPACT_SOURCES
        GIMPACT/src/*.cpp
        GIMPACT/src/*.c
    )
    add_definitions(-DdTRIMESH_GIMPACT)
endif()

# OU utility library sources (if enabled)
if(ODE_ENABLE_OU)
    file(GLOB_RECURSE OU_SOURCES
        ou/src/*.cpp
        ou/src/*.c
    )
endif()

# Combine all sources
set(ALL_ODE_SOURCES
    ${ODE_CORE_SOURCES}
    ${ODE_JOINT_SOURCES}
    ${OPCODE_SOURCES}
    ${GIMPACT_SOURCES}
    ${OU_SOURCES}
)

# Filter out any test or demo files
list(FILTER ALL_ODE_SOURCES EXCLUDE REGEX ".*/test/.*")
list(FILTER ALL_ODE_SOURCES EXCLUDE REGEX ".*/demo/.*")
list(FILTER ALL_ODE_SOURCES EXCLUDE REGEX ".*/tests/.*")

# Create the ODE library
add_library(ode ${ALL_ODE_SOURCES})

# Set up include directories
target_include_directories(ode PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/ode/src>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/OPCODE>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/OPCODE/Ice>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/GIMPACT/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/ou/include>
    $<INSTALL_INTERFACE:include>
)

# Set library properties
set_target_properties(ode PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION 0
    OUTPUT_NAME ode
    DEBUG_POSTFIX ""
)

# Link with threads if needed
if(Threads_FOUND)
    target_link_libraries(ode PUBLIC Threads::Threads)
endif()

# Platform-specific linking
if(UNIX AND NOT APPLE)
    target_link_libraries(ode PUBLIC m) # Math library
endif()

# Compiler-specific settings
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
    target_compile_options(ode PRIVATE
        -Wall -Wno-unused-parameter -Wno-unused-variable
        -fPIC
    )
    # Suppress warnings for older C++ code
    target_compile_options(ode PRIVATE
        -Wno-deprecated-declarations
        -Wno-sign-compare
        -Wno-unused-but-set-variable
    )
endif()

# Create ode-dbl headers directory and symlinks
set(ODE_DBL_INCLUDE_DIR "${CMAKE_CURRENT_BINARY_DIR}/include/ode-dbl")
file(MAKE_DIRECTORY ${ODE_DBL_INCLUDE_DIR})

# Copy main ODE headers and create ode-dbl compatibility layer
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/include/ode")
    file(GLOB ODE_MAIN_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/include/ode/*.h")
    foreach(header ${ODE_MAIN_HEADERS})
        get_filename_component(header_name ${header} NAME)
        configure_file(${header} ${ODE_DBL_INCLUDE_DIR}/${header_name} COPYONLY)
    endforeach()
else()
    # Create basic ode.h if it doesn't exist
    file(WRITE ${ODE_DBL_INCLUDE_DIR}/ode.h
        "#ifndef _ODE_H_\n"
        "#define _ODE_H_\n"
        "#ifdef __cplusplus\n"
        "extern \"C\" {\n"
        "#endif\n"
        "#include <ode/common.h>\n"
        "#include <ode/collision.h>\n"
        "#include <ode/collision_space.h>\n"
        "#include <ode/contact.h>\n"
        "#include <ode/error.h>\n"
        "#include <ode/matrix.h>\n"
        "#include <ode/memory.h>\n"
        "#include <ode/misc.h>\n"
        "#include <ode/objects.h>\n"
        "#include <ode/rotation.h>\n"
        "#include <ode/timer.h>\n"
        "#ifdef __cplusplus\n"
        "}\n"
        "#endif\n"
        "#endif\n"
    )
endif()

# Generate ode-dbl-config script
set(ODE_DBL_CONFIG_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/ode-dbl-config")
file(WRITE ${ODE_DBL_CONFIG_SCRIPT}
    "#!/bin/sh\n"
    "# Generated ode-dbl-config for bundled ODE\n"
    "\n"
    "prefix=\"${CMAKE_CURRENT_BINARY_DIR}\"\n"
    "exec_prefix=\"\${prefix}\"\n"
    "includedir=\"\${prefix}/include\"\n"
    "libdir=\"\${prefix}\"\n"
    "\n"
    "usage() {\n"
    "    cat <<EOH\n"
    "Usage: ode-dbl-config [OPTIONS]\n"
    "Options:\n"
    "    [--prefix]\n"
    "    [--exec-prefix]\n"
    "    [--version]\n"
    "    [--cflags]\n"
    "    [--libs]\n"
    "EOH\n"
    "    exit \$1\n"
    "}\n"
    "\n"
    "if test \$# -eq 0; then\n"
    "    usage 1\n"
    "fi\n"
    "\n"
    "while test \$# -gt 0; do\n"
    "    case \"\$1\" in\n"
    "    --prefix)\n"
    "        echo \"\$prefix\"\n"
    "        ;;\n"
    "    --exec-prefix)\n"
    "        echo \"\$exec_prefix\"\n"
    "        ;;\n"
    "    --version)\n"
    "        echo \"${PROJECT_VERSION}\"\n"
    "        ;;\n"
    "    --cflags)\n"
    "        echo \"-I\${includedir}/ode-dbl -DdDOUBLE\"\n"
    "        ;;\n"
    "    --libs)\n"
    "        echo \"-L\${libdir} -lode\"\n"
    "        ;;\n"
    "    *)\n"
    "        usage 1\n"
    "        ;;\n"
    "    esac\n"
    "    shift\n"
    "done\n"
    "\n"
    "exit 0\n"
)

# Make the script executable
file(COPY ${ODE_DBL_CONFIG_SCRIPT} 
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR}
     FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                      GROUP_READ GROUP_EXECUTE
                      WORLD_READ WORLD_EXECUTE)

# Create symlink in parent include directory for compatibility
set(PARENT_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/include")
file(MAKE_DIRECTORY ${PARENT_INCLUDE_DIR})

# Create symlink for ode-dbl headers
execute_process(
    COMMAND ${CMAKE_COMMAND} -E create_symlink
        ${CMAKE_CURRENT_BINARY_DIR}/include/ode-dbl
        ${PARENT_INCLUDE_DIR}/ode-dbl
    RESULT_VARIABLE SYMLINK_RESULT
)

if(NOT SYMLINK_RESULT EQUAL 0)
    # Fallback to copying if symlink fails
    file(COPY ${CMAKE_CURRENT_BINARY_DIR}/include/ode-dbl
         DESTINATION ${PARENT_INCLUDE_DIR})
endif()

# Export variables for parent scope
set(ODE_FOUND TRUE PARENT_SCOPE)
set(ODE_LIBRARIES ode PARENT_SCOPE) 
set(ODE_INCLUDE_DIRS ${ODE_DBL_INCLUDE_DIR} PARENT_SCOPE)
set(ODE_IS_DOUBLE TRUE PARENT_SCOPE)

# Installation
install(TARGETS ode
    EXPORT lpzrobots-targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/include/ode-dbl
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(PROGRAMS ${ODE_DBL_CONFIG_SCRIPT}
    DESTINATION ${CMAKE_INSTALL_BINDIR}
)

message(STATUS "Bundled ODE will be built with:")
message(STATUS "  Version: ${PROJECT_VERSION}")
message(STATUS "  Double precision: ON")
message(STATUS "  GIMPACT: ${ODE_ENABLE_GIMPACT}")
message(STATUS "  OPCODE: ${ODE_ENABLE_OPCODE}")
message(STATUS "  OU: ${ODE_ENABLE_OU}")
message(STATUS "  Library: ${CMAKE_CURRENT_BINARY_DIR}/libode.a")
message(STATUS "  Headers: ${ODE_DBL_INCLUDE_DIR}")
message(STATUS "  Config script: ${ODE_DBL_CONFIG_SCRIPT}")