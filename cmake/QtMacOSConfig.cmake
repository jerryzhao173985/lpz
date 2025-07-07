# Qt macOS Configuration Helper
# Handles Qt-specific issues on macOS, particularly the AGL framework problem

# Function to configure Qt target for macOS
function(lpzrobots_configure_qt_macos target)
    if(NOT APPLE)
        return()
    endif()
    
    message(STATUS "Configuring ${target} for macOS Qt compatibility...")
    
    # 1. Prevent Qt from adding deprecated frameworks
    get_target_property(target_type ${target} TYPE)
    if(target_type STREQUAL "EXECUTABLE")
        # Remove AGL framework that Qt might add
        set_property(TARGET ${target} PROPERTY LINK_FLAGS_INIT "")
        
        # Use our filter script for linking
        set_property(TARGET ${target} PROPERTY RULE_LAUNCH_LINK 
            "${CMAKE_SOURCE_DIR}/cmake/filter_agl_link.sh")
        
        # Add a post-build check to ensure no AGL references
        add_custom_command(TARGET ${target} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E echo "Checking ${target} for AGL framework references..."
            COMMAND bash -c "if otool -L $<TARGET_FILE:${target}> | grep -q AGL; then echo 'WARNING: AGL framework reference found in ${target}!'; exit 1; fi"
            VERBATIM
        )
    endif()
    
    # 2. Override Qt's OpenGL configuration
    # This mimics what the .pro files do with CONFIG -= opengl
    get_target_property(LINK_LIBS ${target} LINK_LIBRARIES)
    if(LINK_LIBS)
        # Remove any Qt OpenGL modules that might bring in AGL
        list(REMOVE_ITEM LINK_LIBS Qt::OpenGL Qt5::OpenGL)
        set_property(TARGET ${target} PROPERTY LINK_LIBRARIES ${LINK_LIBS})
    endif()
    
    # 2b. Also check INTERFACE_LINK_LIBRARIES
    get_target_property(INTERFACE_LIBS ${target} INTERFACE_LINK_LIBRARIES)
    if(INTERFACE_LIBS)
        string(REPLACE "-framework AGL" "" INTERFACE_LIBS "${INTERFACE_LIBS}")
        string(REPLACE "AGL" "" INTERFACE_LIBS "${INTERFACE_LIBS}")
        set_property(TARGET ${target} PROPERTY INTERFACE_LINK_LIBRARIES ${INTERFACE_LIBS})
    endif()
    
    # 3. Set proper framework search paths (avoiding deprecated ones)
    target_link_options(${target} PRIVATE
        -Wl,-no_weak_imports  # Prevent weak symbol imports
    )
    
    # 4. Ensure we're using the correct OpenGL framework
    if(target_type STREQUAL "EXECUTABLE")
        # First remove any existing OpenGL/AGL references
        get_target_property(CURRENT_LINK_LIBS ${target} LINK_LIBRARIES)
        if(CURRENT_LINK_LIBS)
            list(REMOVE_ITEM CURRENT_LINK_LIBS "-framework AGL" "AGL")
            set_property(TARGET ${target} PROPERTY LINK_LIBRARIES ${CURRENT_LINK_LIBS})
        endif()
        
        target_link_libraries(${target} PRIVATE
            "-framework OpenGL"
            "-framework Cocoa"
            "-framework AppKit"
        )
    endif()
    
    # 5. Fix link order to ensure our frameworks take precedence
    # This ensures that even if Qt adds AGL, our filter will catch it
    set_property(TARGET ${target} PROPERTY LINK_DEPENDS
        "${CMAKE_SOURCE_DIR}/cmake/filter_agl_link.sh"
    )
    
    message(STATUS "macOS Qt compatibility configured for ${target}")
endfunction()

# Function to verify Qt installation doesn't have AGL hardcoded
function(lpzrobots_check_qt_installation)
    if(NOT APPLE)
        return()
    endif()
    
    # Check if Qt's mkspecs contain AGL references
    if(Qt6_FOUND)
        set(QT_MKSPECS_DIR "${Qt6_DIR}/../../../mkspecs")
    elseif(Qt5_FOUND)
        set(QT_MKSPECS_DIR "${Qt5_DIR}/../../../mkspecs")
    else()
        return()
    endif()
    
    if(EXISTS "${QT_MKSPECS_DIR}/common/mac.conf")
        file(READ "${QT_MKSPECS_DIR}/common/mac.conf" MAC_CONF_CONTENT)
        if(MAC_CONF_CONTENT MATCHES "AGL")
            message(WARNING 
                "Your Qt installation at ${QT_MKSPECS_DIR} contains AGL framework references.\n"
                "This is a known issue with Homebrew Qt on macOS.\n"
                "The build system will automatically filter these out."
            )
        endif()
    endif()
endfunction()

# Global settings for Qt on macOS
if(APPLE)
    # Override CMake's default framework handling for Qt apps
    set(CMAKE_FIND_FRAMEWORK LAST)
    
    # Ensure we don't pick up deprecated frameworks
    set(CMAKE_FRAMEWORK_PATH
        /System/Library/Frameworks
        /Library/Frameworks
        # Explicitly exclude paths that might contain old frameworks
    )
    
    # Check Qt installation once
    lpzrobots_check_qt_installation()
endif()