# LPZRobots Dependency Management
# Handles finding and configuring external dependencies

# Function to find all LPZRobots dependencies
function(lpzrobots_find_dependencies)
    # Parse REQUIRED/OPTIONAL argument lists for future enhancement
    # Usage: lpzrobots_find_dependencies(REQUIRED Threads ODE OPTIONAL Qt OpenSceneGraph)
    set(options)
    set(oneValueArgs)
    set(multiValueArgs REQUIRED OPTIONAL)
    cmake_parse_arguments(FD "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
    
    # Store lists for potential future enforcement
    set(_REQUIRED_DEPS ${FD_REQUIRED})
    set(_OPTIONAL_DEPS ${FD_OPTIONAL})
    
    # For now, we don't enforce but this prepares for future strict checking
    
    # Required dependencies
    find_package(Threads REQUIRED)
    
    # OpenMP (optional, disabled by default on macOS)
    if(ENABLE_OPENMP)
        find_package(OpenMP)
        if(OPENMP_FOUND)
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}" PARENT_SCOPE)
            set(LPZROBOTS_HAS_OPENMP TRUE PARENT_SCOPE)
        else()
            message(WARNING "OpenMP requested but not found - automatically disabling")
            # Auto-downgrade to avoid confusion
            set(ENABLE_OPENMP OFF PARENT_SCOPE)
            set(LPZROBOTS_HAS_OPENMP FALSE PARENT_SCOPE)
        endif()
    endif()
    
    # GSL (GNU Scientific Library)
    find_package(PkgConfig)
    if(PKG_CONFIG_FOUND)
        pkg_check_modules(GSL gsl)
        if(GSL_FOUND)
            set(LPZROBOTS_HAS_GSL TRUE PARENT_SCOPE)
            set(LPZROBOTS_GSL_INCLUDE_DIRS ${GSL_INCLUDE_DIRS} PARENT_SCOPE)
            set(LPZROBOTS_GSL_LIBRARIES ${GSL_LIBRARIES} PARENT_SCOPE)
            set(LPZROBOTS_GSL_LIBRARY_DIRS ${GSL_LIBRARY_DIRS} PARENT_SCOPE)
            set(LPZROBOTS_GSL_CFLAGS ${GSL_CFLAGS_OTHER} PARENT_SCOPE)
            set(LPZROBOTS_GSL_LDFLAGS ${GSL_LDFLAGS_OTHER} PARENT_SCOPE)
        else()
            message(WARNING "GSL not found. Some features will be disabled.")
            set(LPZROBOTS_HAS_GSL FALSE PARENT_SCOPE)
        endif()
    else()
        # Try to find GSL without pkg-config
        find_library(GSL_LIBRARIES gsl)
        find_path(GSL_INCLUDE_DIRS gsl/gsl_math.h)
        if(GSL_LIBRARIES AND GSL_INCLUDE_DIRS)
            set(LPZROBOTS_HAS_GSL TRUE PARENT_SCOPE)
            set(LPZROBOTS_GSL_INCLUDE_DIRS ${GSL_INCLUDE_DIRS} PARENT_SCOPE)
            set(LPZROBOTS_GSL_LIBRARIES ${GSL_LIBRARIES} PARENT_SCOPE)
            # Extract library directory from full path
            get_filename_component(GSL_LIB_DIR ${GSL_LIBRARIES} DIRECTORY)
            set(LPZROBOTS_GSL_LIBRARY_DIRS ${GSL_LIB_DIR} PARENT_SCOPE)
        else()
            message(WARNING "GSL not found. Some features will be disabled.")
            set(LPZROBOTS_HAS_GSL FALSE PARENT_SCOPE)
        endif()
    endif()
    
    # Qt for GUI tools
    if(BUILD_GUI_TOOLS)
        lpzrobots_find_qt()
    endif()
    
    # Testing dependencies
    if(BUILD_TESTS)
        lpzrobots_find_test_dependencies()
    endif()
    
    # OpenSceneGraph
    find_package(OpenSceneGraph COMPONENTS osgDB osgUtil osgViewer osgGA osgShadow osgText)
    if(OPENSCENEGRAPH_FOUND)
        set(LPZROBOTS_HAS_OSG TRUE PARENT_SCOPE)
        set(LPZROBOTS_OSG_INCLUDE_DIRS ${OPENSCENEGRAPH_INCLUDE_DIRS} PARENT_SCOPE)
        set(LPZROBOTS_OSG_LIBRARIES ${OPENSCENEGRAPH_LIBRARIES} PARENT_SCOPE)
    else()
        message(WARNING "OpenSceneGraph not found. Visualization features will be limited.")
        set(LPZROBOTS_HAS_OSG FALSE PARENT_SCOPE)
    endif()
    
    # ODE (Open Dynamics Engine) - Unified handling
    lpzrobots_find_ode()
    
    # Propagate ODE variables to parent scope
    set(LPZROBOTS_HAS_ODE ${LPZROBOTS_HAS_ODE} PARENT_SCOPE)
    set(LPZROBOTS_ODE_INCLUDE_DIRS ${LPZROBOTS_ODE_INCLUDE_DIRS} PARENT_SCOPE)
    set(LPZROBOTS_ODE_LIBRARIES ${LPZROBOTS_ODE_LIBRARIES} PARENT_SCOPE)
    set(LPZROBOTS_ODE_IS_DOUBLE ${LPZROBOTS_ODE_IS_DOUBLE} PARENT_SCOPE)
    set(LPZROBOTS_USE_SYSTEM_ODE ${LPZROBOTS_USE_SYSTEM_ODE} PARENT_SCOPE)
    set(LPZROBOTS_USE_BUNDLED_ODE ${LPZROBOTS_USE_BUNDLED_ODE} PARENT_SCOPE)
    
    # Java for Java tools
    if(BUILD_JAVA_TOOLS)
        find_package(Java COMPONENTS Development)
        if(Java_FOUND)
            set(LPZROBOTS_HAS_JAVA TRUE PARENT_SCOPE)
            include(UseJava)
        else()
            message(WARNING "Java not found. Java tools will not be built.")
            set(LPZROBOTS_HAS_JAVA FALSE PARENT_SCOPE)
            set(BUILD_JAVA_TOOLS OFF PARENT_SCOPE)
        endif()
    endif()
    
    # Readline library
    find_library(READLINE_LIBRARY readline)
    find_path(READLINE_INCLUDE_DIR readline/readline.h)
    if(READLINE_LIBRARY AND READLINE_INCLUDE_DIR)
        set(READLINE_LIBRARY ${READLINE_LIBRARY} PARENT_SCOPE)
        set(READLINE_INCLUDE_DIR ${READLINE_INCLUDE_DIR} PARENT_SCOPE)
        set(LPZROBOTS_HAS_READLINE TRUE PARENT_SCOPE)
    else()
        message(WARNING "Readline not found. Console interaction will be limited.")
        set(LPZROBOTS_HAS_READLINE FALSE PARENT_SCOPE)
    endif()
    
    # Test frameworks for unit testing
    option(LPZROBOTS_USE_DOCTEST "Use doctest instead of GoogleTest" ON)
    
    if(LPZROBOTS_USE_DOCTEST)
        lpzrobots_find_doctest()
        set(LPZROBOTS_HAS_DOCTEST ${LPZROBOTS_HAS_DOCTEST} PARENT_SCOPE)
    else()
        lpzrobots_find_googletest()
        set(LPZROBOTS_HAS_GOOGLETEST ${LPZROBOTS_HAS_GOOGLETEST} PARENT_SCOPE)
    endif()
endfunction()

# Function to find Qt
function(lpzrobots_find_qt)
    # Try Qt6 first, fall back to Qt5
    find_package(Qt6 COMPONENTS Core Widgets OpenGL OpenGLWidgets QUIET)
    
    if(Qt6_FOUND)
        set(LPZROBOTS_QT_VERSION 6 PARENT_SCOPE)
        set(LPZROBOTS_HAS_QT TRUE PARENT_SCOPE)
        set(QT_VERSION_MAJOR 6 PARENT_SCOPE)
        message(STATUS "Found Qt6 for GUI tools")
    else()
        find_package(Qt5 COMPONENTS Core Widgets OpenGL QUIET)
        if(Qt5_FOUND)
            set(LPZROBOTS_QT_VERSION 5 PARENT_SCOPE)
            set(LPZROBOTS_HAS_QT TRUE PARENT_SCOPE)
            set(QT_VERSION_MAJOR 5 PARENT_SCOPE)
            message(STATUS "Found Qt5 for GUI tools")
        else()
            message(WARNING "Neither Qt6 nor Qt5 found. GUI tools will not be built.")
            set(LPZROBOTS_HAS_QT FALSE PARENT_SCOPE)
            set(BUILD_GUI_TOOLS OFF PARENT_SCOPE)
        endif()
    endif()
endfunction()

# Function to find doctest for unit testing
function(lpzrobots_find_doctest)
    if(NOT BUILD_TESTS)
        set(LPZROBOTS_HAS_DOCTEST FALSE PARENT_SCOPE)
        return()
    endif()
    
    # Download doctest single header directly
    message(STATUS "Downloading doctest single header...")
    
    set(DOCTEST_DIR "${CMAKE_BINARY_DIR}/_deps/doctest")
    set(DOCTEST_HEADER "${DOCTEST_DIR}/doctest.h")
    
    if(NOT EXISTS ${DOCTEST_HEADER})
        file(MAKE_DIRECTORY ${DOCTEST_DIR})
        file(DOWNLOAD
            "https://raw.githubusercontent.com/doctest/doctest/v2.4.11/doctest/doctest.h"
            ${DOCTEST_HEADER}
            STATUS DOWNLOAD_STATUS
        )
        
        list(GET DOWNLOAD_STATUS 0 DOWNLOAD_RESULT)
        if(NOT DOWNLOAD_RESULT EQUAL 0)
            message(FATAL_ERROR "Failed to download doctest header")
        endif()
    endif()
    
    # Create an interface library for doctest
    if(NOT TARGET doctest)
        add_library(doctest INTERFACE)
        target_include_directories(doctest INTERFACE ${DOCTEST_DIR})
    endif()
    
    # Add compile definitions for optimization
    target_compile_definitions(doctest INTERFACE
        DOCTEST_CONFIG_SUPER_FAST_ASSERTS  # Faster assertions
        $<$<CONFIG:Release>:DOCTEST_CONFIG_DISABLE>  # Disable in release builds if desired
    )
    
    # Add useful doctest flags
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
        target_compile_options(doctest INTERFACE
            -Wno-unused-variable  # doctest generates some unused variables
        )
    endif()
    
    # Export for script usage
    set(doctest_SOURCE_DIR ${DOCTEST_DIR} PARENT_SCOPE)
    
    set(LPZROBOTS_HAS_DOCTEST TRUE PARENT_SCOPE)
    message(STATUS "doctest configured (single-header testing framework)")
endfunction()

# Function to find GoogleTest for unit testing (kept for compatibility)
function(lpzrobots_find_googletest)
    if(NOT BUILD_TESTS)
        set(LPZROBOTS_HAS_GOOGLETEST FALSE PARENT_SCOPE)
        return()
    endif()
    
    # Try to find system-installed GoogleTest first
    find_package(GTest QUIET)
    
    if(GTest_FOUND)
        set(LPZROBOTS_HAS_GOOGLETEST TRUE PARENT_SCOPE)
        message(STATUS "Found system GoogleTest")
    else()
        # Use FetchContent to download GoogleTest
        include(FetchContent)
        
        message(STATUS "Downloading GoogleTest...")
        FetchContent_Declare(
            googletest
            GIT_REPOSITORY https://github.com/google/googletest.git
            GIT_TAG v1.14.0
            GIT_SHALLOW TRUE
        )
        
        # Prevent GoogleTest from overriding our compiler settings
        set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
        
        FetchContent_MakeAvailable(googletest)
        
        # Disable warnings for GoogleTest
        if(TARGET gtest)
            target_compile_options(gtest PRIVATE 
                $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-w>
            )
        endif()
        if(TARGET gtest_main)
            target_compile_options(gtest_main PRIVATE 
                $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-w>
            )
        endif()
        if(TARGET gmock)
            target_compile_options(gmock PRIVATE 
                $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-w>
            )
        endif()
        if(TARGET gmock_main)
            target_compile_options(gmock_main PRIVATE 
                $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-w>
            )
        endif()
        
        set(LPZROBOTS_HAS_GOOGLETEST TRUE PARENT_SCOPE)
        message(STATUS "GoogleTest downloaded and configured")
    endif()
endfunction()

# Function to find test dependencies
function(lpzrobots_find_test_dependencies)
    # Doctest is handled separately
    if(LPZROBOTS_USE_DOCTEST)
        lpzrobots_find_doctest()
    endif()
    
    # GoogleTest if needed
    if(NOT LPZROBOTS_USE_DOCTEST)
        lpzrobots_find_googletest()
    endif()
endfunction()

# ODE configuration option (must be outside function for command line overrides)
option(LPZROBOTS_USE_SYSTEM_ODE "Use system-installed ODE instead of bundled version" OFF)

# Function to find ODE - Unified logic for all platforms  
function(lpzrobots_find_ode)
    
    if(LPZROBOTS_USE_SYSTEM_ODE)
        message(STATUS "Looking for system ODE...")
        
        # Use the FindODE.cmake module
        set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_LIST_DIR}")
        find_package(ODE)
        
        if(ODE_FOUND)
            set(LPZROBOTS_HAS_ODE TRUE PARENT_SCOPE)
            set(LPZROBOTS_ODE_INCLUDE_DIRS ${ODE_INCLUDE_DIRS} PARENT_SCOPE)
            set(LPZROBOTS_ODE_LIBRARIES ${ODE_LIBRARIES} PARENT_SCOPE)
            set(LPZROBOTS_ODE_IS_DOUBLE ${ODE_IS_DOUBLE} PARENT_SCOPE)
            
            # Create ode-dbl compatibility headers if ODE was found
            lpzrobots_setup_ode_headers("${ODE_INCLUDE_DIRS}")
            
            message(STATUS "Using system ODE (double precision: ${ODE_IS_DOUBLE})")
            
            # Double-check with pkg-config if FindODE didn't find libraries
            if(NOT ODE_LIBRARIES)
                find_package(PkgConfig)
                if(PkgConfig_FOUND)
                    pkg_check_modules(PC_ODE ode)
                    if(PC_ODE_FOUND)
                        set(LPZROBOTS_ODE_LIBRARIES ${PC_ODE_LIBRARIES} PARENT_SCOPE)
                        set(LPZROBOTS_ODE_LIBRARY_DIRS ${PC_ODE_LIBRARY_DIRS} PARENT_SCOPE)
                        message(STATUS "Found ODE via pkg-config: ${PC_ODE_LIBRARIES}")
                    endif()
                endif()
            endif()
        else()
            message(WARNING "System ODE not found. Will use bundled version.")
            set(LPZROBOTS_USE_SYSTEM_ODE OFF PARENT_SCOPE)
            # Call bundled setup and propagate variables
            lpzrobots_setup_bundled_ode()
            set(LPZROBOTS_HAS_ODE TRUE PARENT_SCOPE)
            set(LPZROBOTS_ODE_INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/include/ode-dbl" PARENT_SCOPE)
            set(LPZROBOTS_ODE_LIBRARIES "" PARENT_SCOPE)
            set(LPZROBOTS_ODE_IS_DOUBLE TRUE PARENT_SCOPE)
            set(LPZROBOTS_USE_BUNDLED_ODE TRUE PARENT_SCOPE)
        endif()
    else()
        message(STATUS "Using bundled ODE as requested")
        # Call bundled setup and propagate variables
        lpzrobots_setup_bundled_ode()
        set(LPZROBOTS_HAS_ODE TRUE PARENT_SCOPE)
        set(LPZROBOTS_ODE_INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/include/ode-dbl" PARENT_SCOPE)
        set(LPZROBOTS_ODE_LIBRARIES "" PARENT_SCOPE)
        set(LPZROBOTS_ODE_IS_DOUBLE TRUE PARENT_SCOPE)
        set(LPZROBOTS_USE_BUNDLED_ODE TRUE PARENT_SCOPE)
    endif()
endfunction()

# Function to setup bundled ODE paths
function(lpzrobots_setup_bundled_ode)
    set(LPZROBOTS_HAS_ODE TRUE PARENT_SCOPE)
    set(LPZROBOTS_ODE_INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/include/ode-dbl" PARENT_SCOPE)
    
    # For bundled ODE, we don't link to a library target - it's handled by the legacy Make system
    # The bundled ODE is built separately and included via headers only for CMake builds
    set(LPZROBOTS_ODE_LIBRARIES "" PARENT_SCOPE)
    set(LPZROBOTS_ODE_IS_DOUBLE TRUE PARENT_SCOPE)
    set(LPZROBOTS_USE_BUNDLED_ODE TRUE PARENT_SCOPE)
    
    message(STATUS "Using bundled ODE (double precision, headers only for CMake)")
    message(WARNING "CMake builds with bundled ODE have limited functionality. Consider using system ODE or legacy Make builds for full features.")
endfunction()

# Function to setup ODE headers compatibility
function(lpzrobots_setup_ode_headers ode_include_path)
    if(NOT ode_include_path)
        return()
    endif()
    
    # Create ode-dbl compatibility directory
    set(ODE_DBL_DIR "${CMAKE_BINARY_DIR}/include/ode-dbl")
    file(MAKE_DIRECTORY ${ODE_DBL_DIR})
    
    # Find ODE headers and create symlinks
    if(EXISTS "${ode_include_path}/ode")
        file(GLOB ODE_HEADERS "${ode_include_path}/ode/*.h")
        foreach(header ${ODE_HEADERS})
            get_filename_component(header_name ${header} NAME)
            set(symlink_target "${ODE_DBL_DIR}/${header_name}")
            
            # Remove existing symlink/file if it exists
            if(EXISTS ${symlink_target})
                file(REMOVE ${symlink_target})
            endif()
            
            # Create symlink
            execute_process(
                COMMAND ${CMAKE_COMMAND} -E create_symlink 
                    ${header} 
                    ${symlink_target}
                RESULT_VARIABLE SYMLINK_RESULT
            )
            
            if(NOT SYMLINK_RESULT EQUAL 0)
                # Fall back to copying if symlink fails
                file(COPY ${header} DESTINATION ${ODE_DBL_DIR})
            endif()
        endforeach()
        
        message(STATUS "Created ODE header compatibility layer at ${ODE_DBL_DIR}")
        
        # Also add this to the include path
        set(LPZROBOTS_ODE_INCLUDE_DIRS "${CMAKE_BINARY_DIR}/include/ode-dbl" PARENT_SCOPE)
    else()
        message(WARNING "ODE headers not found at ${ode_include_path}/ode")
    endif()
endfunction()

# Function to apply dependency settings to a target
function(lpzrobots_apply_dependencies target)
    cmake_parse_arguments(PARSE_ARGV 1 DEPS "GSL;OSG;ODE;QT" "" "")
    
    # GSL
    if(DEPS_GSL AND LPZROBOTS_HAS_GSL)
        target_include_directories(${target} SYSTEM PRIVATE ${LPZROBOTS_GSL_INCLUDE_DIRS})
        if(LPZROBOTS_GSL_LIBRARY_DIRS)
            target_link_directories(${target} PUBLIC ${LPZROBOTS_GSL_LIBRARY_DIRS})
        endif()
        target_link_libraries(${target} PUBLIC ${LPZROBOTS_GSL_LIBRARIES})
        if(LPZROBOTS_GSL_CFLAGS)
            target_compile_options(${target} PRIVATE ${LPZROBOTS_GSL_CFLAGS})
        endif()
    elseif(DEPS_GSL)
        target_compile_definitions(${target} PRIVATE NO_GSL)
    endif()
    
    # OpenSceneGraph
    if(DEPS_OSG AND LPZROBOTS_HAS_OSG)
        # Use SYSTEM to suppress warnings from OSG headers
        target_include_directories(${target} SYSTEM PUBLIC ${LPZROBOTS_OSG_INCLUDE_DIRS})
        target_link_libraries(${target} PUBLIC ${LPZROBOTS_OSG_LIBRARIES})
    endif()
    
    # ODE
    if(DEPS_ODE AND LPZROBOTS_HAS_ODE)
        target_include_directories(${target} SYSTEM PUBLIC ${LPZROBOTS_ODE_INCLUDE_DIRS})
        target_link_libraries(${target} PUBLIC ${LPZROBOTS_ODE_LIBRARIES})
    endif()
    
    # Qt
    if(DEPS_QT AND LPZROBOTS_HAS_QT)
        if(LPZROBOTS_QT_VERSION EQUAL 6)
            target_link_libraries(${target} PUBLIC 
                Qt6::Core Qt6::Widgets Qt6::OpenGL Qt6::OpenGLWidgets
            )
        else()
            target_link_libraries(${target} PUBLIC 
                Qt5::Core Qt5::Widgets Qt5::OpenGL
            )
        endif()
    endif()
endfunction()

# Function to print dependency summary
function(lpzrobots_print_dependencies)
    message(STATUS "")
    message(STATUS "LPZRobots Dependencies:")
    message(STATUS "  Threads:         FOUND")
    message(STATUS "  OpenMP:          ${LPZROBOTS_HAS_OPENMP}")
    message(STATUS "  GSL:             ${LPZROBOTS_HAS_GSL}")
    message(STATUS "  OpenSceneGraph:  ${LPZROBOTS_HAS_OSG}")
    if(LPZROBOTS_HAS_ODE)
        if(LPZROBOTS_USE_SYSTEM_ODE)
            message(STATUS "  ODE:             FOUND (system, double: ${LPZROBOTS_ODE_IS_DOUBLE})")
        else()
            message(STATUS "  ODE:             FOUND (bundled, double precision)")
        endif()
    else()
        message(STATUS "  ODE:             NOT FOUND")
    endif()
    message(STATUS "  Qt:              ${LPZROBOTS_HAS_QT}")
    if(LPZROBOTS_HAS_QT)
        message(STATUS "    Qt Version:    ${LPZROBOTS_QT_VERSION}")
    endif()
    message(STATUS "  Readline:        ${LPZROBOTS_HAS_READLINE}")
    message(STATUS "  Java:            ${LPZROBOTS_HAS_JAVA}")
    message(STATUS "  GoogleTest:      ${LPZROBOTS_HAS_GOOGLETEST}")
    message(STATUS "")
endfunction()