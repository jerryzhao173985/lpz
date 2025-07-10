# LPZRobots macOS Build Fixes
# Comprehensive fixes for macOS 15, Xcode 15+, Qt6, and OpenSceneGraph issues

# Function to apply macOS-specific build fixes
function(lpzrobots_apply_macos_fixes)
    if(NOT APPLE)
        return()
    endif()
    
    message(STATUS "Applying macOS build fixes...")
    
    # Get macOS version
    execute_process(
        COMMAND sw_vers -productVersion
        OUTPUT_VARIABLE MACOS_VERSION
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    
    # Get Xcode version 
    execute_process(
        COMMAND xcodebuild -version
        OUTPUT_VARIABLE XCODE_VERSION_OUTPUT
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )
    
    message(STATUS "macOS Version: ${MACOS_VERSION}")
    if(XCODE_VERSION_OUTPUT)
        message(STATUS "Xcode Version: ${XCODE_VERSION_OUTPUT}")
    endif()
    
    # Fix 1: Set proper SDK and deployment target for macOS 15+
    if(MACOS_VERSION VERSION_GREATER_EQUAL "15.0")
        lpzrobots_fix_macos_15_sdk()
    endif()
    
    # Fix 2: Handle Qt6 compatibility issues
    # Apply Qt6 fixes unconditionally on macOS since Qt6 is commonly used
    lpzrobots_fix_qt6_macos()
    
    # Fix 3: Handle OpenSceneGraph Homebrew issues
    # Apply OSG fixes unconditionally on macOS since OSG is commonly used
    lpzrobots_fix_osg_macos()
    
    # Fix 4: Handle Homebrew vs system library conflicts
    lpzrobots_fix_homebrew_conflicts()
    
    # Fix 5: Fix linker issues with newer Xcode
    lpzrobots_fix_xcode_15_linking()
    
    # Fix 6: Handle C++ standard library issues
    lpzrobots_fix_cpp_stdlib_macos()
    
endfunction()

# Fix macOS 15+ SDK issues
function(lpzrobots_fix_macos_15_sdk)
    message(STATUS "Applying macOS 15+ SDK fixes...")
    
    # Set minimum deployment target
    set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0" PARENT_SCOPE)
    
    # Explicitly set SDK root if not set
    if(NOT CMAKE_OSX_SYSROOT)
        execute_process(
            COMMAND xcrun --show-sdk-path
            OUTPUT_VARIABLE SDK_PATH
            OUTPUT_STRIP_TRAILING_WHITESPACE
            ERROR_QUIET
        )
        if(SDK_PATH)
            set(CMAKE_OSX_SYSROOT "${SDK_PATH}" PARENT_SCOPE)
            message(STATUS "Set CMAKE_OSX_SYSROOT to: ${SDK_PATH}")
        endif()
    endif()
    
    # Force use of libc++ (not libstdc++)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++" PARENT_SCOPE)
    
    # Add compatibility flags for newer SDK
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-enum-constexpr-conversion" PARENT_SCOPE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated-declarations" PARENT_SCOPE)
    
endfunction()

# Fix Qt6 macOS compatibility issues
function(lpzrobots_fix_qt6_macos)
    message(STATUS "Applying Qt6 macOS fixes...")
    
    # Try to find Qt6 via environment variable first (set by CI)
    if(DEFINED ENV{CMAKE_PREFIX_PATH})
        set(ENV_PREFIX_PATH $ENV{CMAKE_PREFIX_PATH})
        string(REPLACE ":" ";" ENV_PREFIX_PATHS "${ENV_PREFIX_PATH}")
        foreach(path ${ENV_PREFIX_PATHS})
            if(path MATCHES "qt@6" OR path MATCHES "Qt6")
                message(STATUS "Found Qt6 in environment: ${path}")
                list(APPEND CMAKE_PREFIX_PATH "${path}")
                set(CMAKE_PREFIX_PATH "${CMAKE_PREFIX_PATH}" PARENT_SCOPE)
                break()
            endif()
        endforeach()
    endif()
    
    # Try common Homebrew Qt6 locations
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "arm64")
        set(HOMEBREW_QT_PATH "/opt/homebrew/opt/qt@6")
    else()
        set(HOMEBREW_QT_PATH "/usr/local/opt/qt@6")
    endif()
    
    if(EXISTS "${HOMEBREW_QT_PATH}")
        message(STATUS "Found Qt6 at: ${HOMEBREW_QT_PATH}")
        list(APPEND CMAKE_PREFIX_PATH "${HOMEBREW_QT_PATH}")
        set(CMAKE_PREFIX_PATH "${CMAKE_PREFIX_PATH}" PARENT_SCOPE)
        
        # Set Qt6-specific flags
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DQT_DISABLE_DEPRECATED_BEFORE=0x060000" PARENT_SCOPE)
    endif()
    
    # Handle AGL framework deprecation issue
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DGL_SILENCE_DEPRECATION" PARENT_SCOPE)
    
endfunction()

# Fix OpenSceneGraph macOS issues
function(lpzrobots_fix_osg_macos)
    message(STATUS "Applying OpenSceneGraph macOS fixes...")
    
    # Apply OSG-specific fixes for common Homebrew installation paths
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "arm64")
        set(HOMEBREW_OSG_PATH "/opt/homebrew/include/osg")
    else()
        set(HOMEBREW_OSG_PATH "/usr/local/include/osg")
    endif()
    
    if(EXISTS "${HOMEBREW_OSG_PATH}")
        message(STATUS "Detected Homebrew OpenSceneGraph installation at: ${HOMEBREW_OSG_PATH}")
    endif()
    
    # Add Homebrew OpenGL framework path explicitly
    find_library(OPENGL_LIBRARY OpenGL REQUIRED)
    if(OPENGL_LIBRARY)
        message(STATUS "Found OpenGL framework: ${OPENGL_LIBRARY}")
    endif()
    
    # Set OSG-specific compiler flags unconditionally for macOS
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DOSG_GL_SILENCE_DEPRECATION" PARENT_SCOPE)
    
    # Handle OSG viewer threading issues on macOS
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DOSG_VIEWER_SINGLE_THREADED" PARENT_SCOPE)
    
endfunction()

# Fix Homebrew vs system library conflicts
function(lpzrobots_fix_homebrew_conflicts)
    message(STATUS "Applying Homebrew conflict fixes...")
    
    # Prioritize Homebrew paths for arm64, /usr/local for x86_64
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "arm64")
        set(HOMEBREW_PREFIX "/opt/homebrew")
    else()
        set(HOMEBREW_PREFIX "/usr/local")
    endif()
    
    # Add Homebrew paths in proper order
    if(EXISTS "${HOMEBREW_PREFIX}")
        list(PREPEND CMAKE_PREFIX_PATH "${HOMEBREW_PREFIX}")
        list(PREPEND CMAKE_LIBRARY_PATH "${HOMEBREW_PREFIX}/lib")
        list(PREPEND CMAKE_INCLUDE_PATH "${HOMEBREW_PREFIX}/include")
        
        set(CMAKE_PREFIX_PATH "${CMAKE_PREFIX_PATH}" PARENT_SCOPE)
        set(CMAKE_LIBRARY_PATH "${CMAKE_LIBRARY_PATH}" PARENT_SCOPE)
        set(CMAKE_INCLUDE_PATH "${CMAKE_INCLUDE_PATH}" PARENT_SCOPE)
        
        message(STATUS "Added Homebrew prefix: ${HOMEBREW_PREFIX}")
    endif()
    
    # Set PKG_CONFIG_PATH for Homebrew
    set(ENV{PKG_CONFIG_PATH} "${HOMEBREW_PREFIX}/lib/pkgconfig:$ENV{PKG_CONFIG_PATH}")
    
endfunction()

# Fix Xcode 15+ linking issues
function(lpzrobots_fix_xcode_15_linking)
    message(STATUS "Applying Xcode 15+ linking fixes...")
    
    # Add linker flags to handle newer Xcode issues
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-no_warn_duplicate_libraries" PARENT_SCOPE)
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,-no_warn_duplicate_libraries" PARENT_SCOPE)
    
    # Handle weak symbol imports
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-no_weak_imports" PARENT_SCOPE)
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,-no_weak_imports" PARENT_SCOPE)
    
    # Fix undefined symbols issues
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-undefined,dynamic_lookup" PARENT_SCOPE)
    
endfunction()

# Fix C++ standard library issues on macOS
function(lpzrobots_fix_cpp_stdlib_macos)
    message(STATUS "Applying C++ stdlib macOS fixes...")
    
    # Ensure we're using libc++ (not libstdc++)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++" PARENT_SCOPE)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -stdlib=libc++" PARENT_SCOPE)
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -stdlib=libc++" PARENT_SCOPE)
    
    # Add flags to handle Boost compatibility issues with newer clang
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DBOOST_NO_CXX98_FUNCTION_BASE" PARENT_SCOPE)
    
    # Handle newer C++ standard requirements
    if(CMAKE_CXX_STANDARD GREATER_EQUAL 17)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated-copy" PARENT_SCOPE)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated-copy-dtor" PARENT_SCOPE)
    endif()
    
endfunction()

# Function to configure target for macOS
function(lpzrobots_configure_target_macos target)
    if(NOT APPLE)
        return()
    endif()
    
    # Apply Qt macOS configuration if Qt target
    get_target_property(TARGET_LINK_LIBS ${target} LINK_LIBRARIES)
    if(TARGET_LINK_LIBS AND TARGET_LINK_LIBS MATCHES "Qt[56]::")
        lpzrobots_configure_qt_macos(${target})
    endif()
    
    # Set macOS-specific target properties
    set_target_properties(${target} PROPERTIES
        MACOSX_RPATH ON
        INSTALL_RPATH "@loader_path/../lib"
    )
    
    # Add macOS-specific compile definitions
    target_compile_definitions(${target} PRIVATE
        GL_SILENCE_DEPRECATION
        OSG_GL_SILENCE_DEPRECATION
    )
    
    # Handle framework linking properly
    if(TARGET_LINK_LIBS MATCHES "OpenGL")
        target_link_libraries(${target} PRIVATE "-framework OpenGL")
    endif()
    
endfunction()

# Apply fixes automatically when this file is included, but only core system fixes
# Target-specific fixes will be applied later
if(APPLE)
    # Get macOS version for conditional fixes
    execute_process(
        COMMAND sw_vers -productVersion
        OUTPUT_VARIABLE MACOS_VERSION
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )
    
    message(STATUS "Applying macOS build fixes for version: ${MACOS_VERSION}")
    
    # Apply basic system fixes immediately
    if(MACOS_VERSION VERSION_GREATER_EQUAL "15.0")
        lpzrobots_fix_macos_15_sdk()
    else()
        # Apply basic macOS compatibility for older versions
        set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0" PARENT_SCOPE)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++" PARENT_SCOPE)
    endif()
    
    lpzrobots_fix_homebrew_conflicts()
    lpzrobots_fix_xcode_15_linking()
    lpzrobots_fix_cpp_stdlib_macos()
    
    # Apply Qt6 and OSG fixes (safe to apply early now)
    lpzrobots_fix_qt6_macos()
    lpzrobots_fix_osg_macos()
    
    message(STATUS "Applied macOS build fixes")
endif()