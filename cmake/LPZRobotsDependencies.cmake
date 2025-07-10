# LPZRobots Dependency Management
# Handles finding and configuring external dependencies

# Function to find all LPZRobots dependencies
function(lpzrobots_find_dependencies)
    # Required dependencies
    find_package(Threads REQUIRED)
    
    # OpenMP (optional, disabled by default on macOS)
    if(ENABLE_OPENMP)
        find_package(OpenMP)
        if(OPENMP_FOUND)
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}" PARENT_SCOPE)
            set(LPZROBOTS_HAS_OPENMP TRUE PARENT_SCOPE)
        else()
            message(WARNING "OpenMP requested but not found")
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
    
    # ODE (Open Dynamics Engine)
    if(LPZROBOTS_USE_SYSTEM_ODE)
        lpzrobots_find_ode()
    else()
        # Use compatibility layer
        lpzrobots_setup_ode_compat()
    endif()
    
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
    
    # ODE (Open Dynamics Engine)
    option(LPZROBOTS_USE_SYSTEM_ODE "Use system-installed ODE instead of bundled version" OFF)
    
    if(LPZROBOTS_USE_SYSTEM_ODE)
        set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_LIST_DIR}")
        find_package(ODE)
        if(ODE_FOUND)
            set(LPZROBOTS_HAS_ODE TRUE PARENT_SCOPE)
            set(LPZROBOTS_ODE_INCLUDE_DIRS ${ODE_INCLUDE_DIRS} PARENT_SCOPE)
            set(LPZROBOTS_ODE_LIBRARIES ${ODE_LIBRARIES} PARENT_SCOPE)
            set(LPZROBOTS_ODE_IS_DOUBLE ${ODE_IS_DOUBLE} PARENT_SCOPE)
            
            # Create ode-dbl compatibility headers
            file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/include/ode-dbl)
            if(ODE_INCLUDE_DIRS)
                file(GLOB ODE_HEADERS "${ODE_INCLUDE_DIRS}/ode/*.h")
                foreach(header ${ODE_HEADERS})
                    get_filename_component(header_name ${header} NAME)
                    execute_process(
                        COMMAND ${CMAKE_COMMAND} -E create_symlink 
                            ${header} 
                            ${CMAKE_BINARY_DIR}/include/ode-dbl/${header_name}
                    )
                endforeach()
            endif()
            
            message(STATUS "Using system ODE (double precision: ${ODE_IS_DOUBLE})")
        else()
            message(WARNING "System ODE not found. Will use bundled version.")
            set(LPZROBOTS_USE_SYSTEM_ODE OFF PARENT_SCOPE)
        endif()
    endif()
    
    if(NOT LPZROBOTS_USE_SYSTEM_ODE)
        # Use bundled ODE
        set(LPZROBOTS_HAS_ODE TRUE PARENT_SCOPE)
        set(LPZROBOTS_ODE_INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/include/ode-dbl" PARENT_SCOPE)
        set(LPZROBOTS_ODE_LIBRARIES "ode_dbl" PARENT_SCOPE)
        set(LPZROBOTS_ODE_IS_DOUBLE TRUE PARENT_SCOPE)
        message(STATUS "Using bundled ODE (double precision)")
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

# Function to find ODE
function(lpzrobots_find_ode)
    if(PKG_CONFIG_FOUND)
        pkg_check_modules(ODE ode)
        if(ODE_FOUND)
            set(LPZROBOTS_HAS_ODE TRUE PARENT_SCOPE)
            set(LPZROBOTS_ODE_INCLUDE_DIRS ${ODE_INCLUDE_DIRS} PARENT_SCOPE)
            set(LPZROBOTS_ODE_LIBRARIES ${ODE_LIBRARIES} PARENT_SCOPE)
            return()
        endif()
    endif()
    
    # Fallback: try to find ODE manually
    find_library(ODE_LIBRARIES ode)
    find_path(ODE_INCLUDE_DIRS ode/ode.h)
    
    if(ODE_LIBRARIES AND ODE_INCLUDE_DIRS)
        set(LPZROBOTS_HAS_ODE TRUE PARENT_SCOPE)
        set(LPZROBOTS_ODE_INCLUDE_DIRS ${ODE_INCLUDE_DIRS} PARENT_SCOPE)
        set(LPZROBOTS_ODE_LIBRARIES ${ODE_LIBRARIES} PARENT_SCOPE)
    else()
        message(FATAL_ERROR "System ODE requested but not found")
    endif()
endfunction()

# Function to setup ODE compatibility layer
function(lpzrobots_setup_ode_compat)
    if(APPLE)
        # On macOS, find system ODE
        find_library(ODE_LIBRARY NAMES ode PATHS /opt/homebrew/lib /usr/local/lib)
        if(NOT ODE_LIBRARY)
            message(FATAL_ERROR "ODE not found. Please install: brew install ode")
        endif()
        set(LPZROBOTS_ODE_LIBRARIES ${ODE_LIBRARY} PARENT_SCOPE)
    else()
        # On Linux, use system ODE
        find_library(ODE_LIBRARY NAMES ode)
        if(NOT ODE_LIBRARY)
            message(FATAL_ERROR "ODE not found. Please install ODE development package")
        endif()
        set(LPZROBOTS_ODE_LIBRARIES ${ODE_LIBRARY} PARENT_SCOPE)
    endif()
    
    # Set include path for compatibility headers
    if(EXISTS "${CMAKE_SOURCE_DIR}/include/ode-dbl")
        set(LPZROBOTS_ODE_INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/include/ode-dbl" PARENT_SCOPE)
    else()
        # Use system headers
        find_path(ODE_INCLUDE_DIR ode/ode.h)
        set(LPZROBOTS_ODE_INCLUDE_DIRS ${ODE_INCLUDE_DIR} PARENT_SCOPE)
    endif()
    
    set(LPZROBOTS_HAS_ODE TRUE PARENT_SCOPE)
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
        target_compile_definitions(${target} PRIVATE NOGSL)
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