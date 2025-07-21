# Configurator Test Application

if(BUILD_TESTS)
    add_executable(configurator_test main.cpp)
    target_link_libraries(configurator_test PRIVATE
        lpzrobots::configurator
        lpzrobots::selforg
        Qt6::Core
        Qt6::Widgets
    )
    
    # Apply macOS Qt configuration to avoid AGL framework issues
    if(APPLE AND COMMAND lpzrobots_configure_qt_macos)
        lpzrobots_configure_qt_macos(configurator_test)
    endif()
endif()