# LPZRobots Platform Detection and Settings
# Handles platform-specific configuration

# Function to detect platform and set appropriate variables
function(lpzrobots_detect_platform)
    # Platform detection
    if(APPLE)
        set(LPZROBOTS_PLATFORM "MAC" PARENT_SCOPE)
        
        # Detect Apple Silicon
        if(CMAKE_SYSTEM_PROCESSOR MATCHES "arm64|aarch64")
            set(LPZROBOTS_PLATFORM "MAC_ARM64" PARENT_SCOPE)
            set(LPZROBOTS_ARM64 TRUE PARENT_SCOPE)
        endif()
        
        # Set macOS specific settings
        set(CMAKE_MACOSX_RPATH ON PARENT_SCOPE)
        set(CMAKE_INSTALL_RPATH "@loader_path/../lib" PARENT_SCOPE)
        
        # Set deployment target
        if(NOT CMAKE_OSX_DEPLOYMENT_TARGET)
            set(CMAKE_OSX_DEPLOYMENT_TARGET "13.0" CACHE STRING "Minimum macOS version")
        endif()
        
        # Add Homebrew paths
        if(EXISTS "/opt/homebrew")
            list(APPEND CMAKE_PREFIX_PATH "/opt/homebrew")
            set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} PARENT_SCOPE)
        endif()
        
        # macOS framework settings
        set(CMAKE_FIND_FRAMEWORK LAST PARENT_SCOPE)
    elseif(UNIX)
        set(LPZROBOTS_PLATFORM "LINUX" PARENT_SCOPE)
        
        # Linux specific settings
        set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib" PARENT_SCOPE)
        set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE PARENT_SCOPE)
    elseif(WIN32)
        set(LPZROBOTS_PLATFORM "WINDOWS" PARENT_SCOPE)
        message(WARNING "Windows support is experimental")
    endif()
    
    # Export compile definitions based on platform
    if(APPLE)
        add_compile_definitions(MAC)
        if(LPZROBOTS_ARM64)
            add_compile_definitions(MAC_ARM64)
        endif()
    elseif(UNIX)
        add_compile_definitions(LINUX)
    endif()
endfunction()