# FindODE.cmake - Find Open Dynamics Engine (ODE)
# This module searches for ODE, preferring the double-precision version
#
# This module defines:
#  ODE_FOUND        - System has ODE
#  ODE_INCLUDE_DIRS - The ODE include directories
#  ODE_LIBRARIES    - The libraries needed to use ODE
#  ODE_DEFINITIONS  - Compiler definitions for ODE
#  ODE_VERSION      - Version string of ODE
#  ODE_IS_DOUBLE    - TRUE if ODE uses double precision
#
# This module accepts the following variables:
#  ODE_ROOT         - Root directory to search for ODE
#  ODE_USE_DOUBLE   - Prefer double precision version (default: ON)

# Set default preference for double precision
if(NOT DEFINED ODE_USE_DOUBLE)
    set(ODE_USE_DOUBLE ON)
endif()

# Find using pkg-config first
find_package(PkgConfig QUIET)
if(PKG_CONFIG_FOUND)
    # Try double precision version first
    if(ODE_USE_DOUBLE)
        pkg_check_modules(PC_ODE_DBL QUIET ode-dbl)
        if(PC_ODE_DBL_FOUND)
            set(PC_ODE ${PC_ODE_DBL})
            set(ODE_IS_DOUBLE TRUE)
        endif()
    endif()
    
    # Fall back to regular ODE
    if(NOT PC_ODE_DBL_FOUND)
        pkg_check_modules(PC_ODE QUIET ode)
        if(PC_ODE_FOUND)
            # Check if this ODE is double precision
            execute_process(
                COMMAND ${PKG_CONFIG_EXECUTABLE} --variable=precision ode
                OUTPUT_VARIABLE ODE_PRECISION
                OUTPUT_STRIP_TRAILING_WHITESPACE
                ERROR_QUIET
            )
            if(ODE_PRECISION STREQUAL "double")
                set(ODE_IS_DOUBLE TRUE)
            else()
                set(ODE_IS_DOUBLE FALSE)
            endif()
        endif()
    endif()
endif()

# Find include directory
find_path(ODE_INCLUDE_DIR
    NAMES ode/ode.h
    HINTS
        ${ODE_ROOT}/include
        ${PC_ODE_INCLUDEDIR}
        ${PC_ODE_INCLUDE_DIRS}
    PATHS
        /usr/include
        /usr/local/include
        /opt/local/include
        /opt/homebrew/include
)

# Find library
if(ODE_USE_DOUBLE)
    # Try to find double precision library first
    find_library(ODE_LIBRARY_DBL
        NAMES ode_double ode-dbl oded
        HINTS
            ${ODE_ROOT}/lib
            ${PC_ODE_LIBDIR}
            ${PC_ODE_LIBRARY_DIRS}
        PATHS
            /usr/lib
            /usr/local/lib
            /opt/local/lib
            /opt/homebrew/lib
    )
    if(ODE_LIBRARY_DBL)
        set(ODE_LIBRARY ${ODE_LIBRARY_DBL})
        set(ODE_IS_DOUBLE TRUE)
    endif()
endif()

# Fall back to regular ODE library
if(NOT ODE_LIBRARY)
    find_library(ODE_LIBRARY
        NAMES ode
        HINTS
            ${ODE_ROOT}/lib
            ${PC_ODE_LIBDIR}
            ${PC_ODE_LIBRARY_DIRS}
        PATHS
            /usr/lib
            /usr/local/lib
            /opt/local/lib
            /opt/homebrew/lib
    )
    
    # If we found regular ODE, check if it's double precision
    if(ODE_LIBRARY AND ODE_INCLUDE_DIR)
        # Try to detect precision from headers
        if(EXISTS "${ODE_INCLUDE_DIR}/ode/config.h")
            file(STRINGS "${ODE_INCLUDE_DIR}/ode/config.h" ODE_CONFIG_H REGEX "dDOUBLE|dSINGLE")
            if(ODE_CONFIG_H MATCHES "dDOUBLE")
                set(ODE_IS_DOUBLE TRUE)
            else()
                set(ODE_IS_DOUBLE FALSE)
            endif()
        endif()
    endif()
endif()

# Set the results
set(ODE_LIBRARIES ${ODE_LIBRARY})
set(ODE_INCLUDE_DIRS ${ODE_INCLUDE_DIR})

# Get version
if(PC_ODE_VERSION)
    set(ODE_VERSION ${PC_ODE_VERSION})
elseif(ODE_INCLUDE_DIR AND EXISTS "${ODE_INCLUDE_DIR}/ode/version.h")
    file(STRINGS "${ODE_INCLUDE_DIR}/ode/version.h" ODE_VERSION_H REGEX "^#define ODE_VERSION")
    string(REGEX REPLACE "^#define ODE_VERSION \"([^\"]*)\".*" "\\1" ODE_VERSION "${ODE_VERSION_H}")
endif()

# Set definitions
set(ODE_DEFINITIONS "")
if(ODE_IS_DOUBLE)
    list(APPEND ODE_DEFINITIONS -DdDOUBLE)
else()
    list(APPEND ODE_DEFINITIONS -DdSINGLE)
endif()

# Handle the QUIETLY and REQUIRED arguments
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(ODE
    REQUIRED_VARS ODE_LIBRARY ODE_INCLUDE_DIR
    VERSION_VAR ODE_VERSION
)

# Create imported target
if(ODE_FOUND AND NOT TARGET ODE::ODE)
    add_library(ODE::ODE UNKNOWN IMPORTED)
    set_target_properties(ODE::ODE PROPERTIES
        IMPORTED_LOCATION "${ODE_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${ODE_INCLUDE_DIRS}"
        INTERFACE_COMPILE_DEFINITIONS "${ODE_DEFINITIONS}"
    )
endif()

# Compatibility variables
set(ODE_INCLUDE_DIRS ${ODE_INCLUDE_DIRS})
set(ODE_LIBRARIES ${ODE_LIBRARIES})

mark_as_advanced(ODE_INCLUDE_DIR ODE_LIBRARY ODE_LIBRARY_DBL)