# FindODE.cmake - Find Open Dynamics Engine
#
# This module defines:
#  ODE_FOUND - system has ODE
#  ODE_INCLUDE_DIRS - the ODE include directories
#  ODE_LIBRARIES - link these to use ODE
#  ODE_VERSION - version of ODE
#  ODE_IS_DOUBLE - TRUE if ODE was built with double precision

find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
    pkg_check_modules(PC_ODE QUIET ode)
endif()

# Find include directory
find_path(ODE_INCLUDE_DIR
    NAMES ode/ode.h
    HINTS
        ${PC_ODE_INCLUDEDIR}
        ${PC_ODE_INCLUDE_DIRS}
    PATHS
        /usr/include
        /usr/local/include
        /opt/local/include
        /opt/homebrew/include
)

# Find library
find_library(ODE_LIBRARY
    NAMES ode
    HINTS
        ${PC_ODE_LIBDIR}
        ${PC_ODE_LIBRARY_DIRS}
    PATHS
        /usr/lib
        /usr/local/lib
        /opt/local/lib
        /opt/homebrew/lib
        /usr/lib/x86_64-linux-gnu
        /usr/lib/aarch64-linux-gnu
)

# Extract version
if(PC_ODE_VERSION)
    set(ODE_VERSION ${PC_ODE_VERSION})
elseif(ODE_INCLUDE_DIR AND EXISTS "${ODE_INCLUDE_DIR}/ode/version.h")
    file(STRINGS "${ODE_INCLUDE_DIR}/ode/version.h" ODE_VERSION_LINE REGEX "^#define ODE_VERSION \"[^\"]*\"")
    if(ODE_VERSION_LINE)
        string(REGEX REPLACE "^#define ODE_VERSION \"([^\"]*)\".*" "\\1" ODE_VERSION "${ODE_VERSION_LINE}")
    endif()
endif()

# Check if double precision
set(ODE_IS_DOUBLE FALSE)
if(ODE_INCLUDE_DIR AND EXISTS "${ODE_INCLUDE_DIR}/ode/config.h")
    file(STRINGS "${ODE_INCLUDE_DIR}/ode/config.h" ODE_DOUBLE_LINE REGEX "^#define dDOUBLE")
    if(ODE_DOUBLE_LINE)
        set(ODE_IS_DOUBLE TRUE)
    endif()
endif()

# Handle find_package arguments
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(ODE
    REQUIRED_VARS ODE_LIBRARY ODE_INCLUDE_DIR
    VERSION_VAR ODE_VERSION
)

if(ODE_FOUND)
    set(ODE_INCLUDE_DIRS ${ODE_INCLUDE_DIR})
    set(ODE_LIBRARIES ${ODE_LIBRARY})
    
    # For compatibility with different naming conventions
    set(ODE_INCLUDE_PATH ${ODE_INCLUDE_DIR})
    set(ODE_LIB ${ODE_LIBRARY})
endif()

mark_as_advanced(ODE_INCLUDE_DIR ODE_LIBRARY)