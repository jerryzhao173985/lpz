# LPZRobots Target Creation
# Provides simplified functions for creating LPZRobots targets

# Function to create a LPZRobots library with all standard settings
function(lpzrobots_add_library name)
    cmake_parse_arguments(PARSE_ARGV 1 LIB
        "SHARED;MODULE;INTERFACE;NO_EXPORT;NO_INSTALL;NO_VARIANTS"
        "VERSION;ALIAS"
        "SOURCES;PUBLIC_HEADERS;PRIVATE_HEADERS;PUBLIC_DEPS;PRIVATE_DEPS;COMPILE_DEFS"
    )
    
    # Determine library type
    if(LIB_INTERFACE)
        add_library(${name} INTERFACE)
    elseif(LIB_MODULE)
        add_library(${name} MODULE ${LIB_SOURCES})
    elseif(LIB_SHARED OR BUILD_SHARED_LIBS)
        add_library(${name} SHARED ${LIB_SOURCES})
    else()
        add_library(${name} STATIC ${LIB_SOURCES})
    endif()
    
    # Create alias if requested
    if(LIB_ALIAS)
        add_library(${LIB_ALIAS} ALIAS ${name})
    else()
        add_library(lpzrobots::${name} ALIAS ${name})
    endif()
    
    # Apply compiler settings
    if(NOT LIB_INTERFACE)
        lpzrobots_apply_compiler_settings(${name})
    endif()
    
    # Set include directories
    if(LIB_PUBLIC_HEADERS)
        target_include_directories(${name} PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/${name}>
            $<INSTALL_INTERFACE:include>
            $<INSTALL_INTERFACE:include/${name}>
        )
    endif()
    
    # Always add source directory as private include
    if(NOT LIB_INTERFACE)
        target_include_directories(${name} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    endif()
    
    # Add dependencies
    if(LIB_PUBLIC_DEPS)
        target_link_libraries(${name} PUBLIC ${LIB_PUBLIC_DEPS})
    endif()
    
    if(LIB_PRIVATE_DEPS)
        target_link_libraries(${name} PRIVATE ${LIB_PRIVATE_DEPS})
    endif()
    
    # Add compile definitions
    if(LIB_COMPILE_DEFS)
        target_compile_definitions(${name} PRIVATE ${LIB_COMPILE_DEFS})
    endif()
    
    # Set library properties
    if(NOT LIB_INTERFACE)
        set_target_properties(${name} PROPERTIES
            VERSION ${LIB_VERSION}
            SOVERSION ${PROJECT_VERSION_MAJOR}
            CXX_VISIBILITY_PRESET hidden
            VISIBILITY_INLINES_HIDDEN ON
            POSITION_INDEPENDENT_CODE ON
        )
        
        # Create variants unless disabled
        if(NOT LIB_NO_VARIANTS)
            lpzrobots_create_variants(${name})
        endif()
    endif()
    
    # Installation
    if(NOT LIB_NO_INSTALL)
        if(NOT LIB_NO_EXPORT)
            install(TARGETS ${name}
                EXPORT LPZRobotsTargets
                LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
                ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
                RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
            )
        else()
            install(TARGETS ${name}
                LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
                ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
                RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
            )
        endif()
        
        # Install headers
        if(LIB_PUBLIC_HEADERS)
            install(DIRECTORY include/${name}
                DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
                FILES_MATCHING PATTERN "*.h"
            )
        endif()
    endif()
endfunction()

# Function to create a LPZRobots executable
function(lpzrobots_add_executable name)
    cmake_parse_arguments(PARSE_ARGV 1 EXE
        "WIN32;MACOSX_BUNDLE;NO_INSTALL"
        ""
        "SOURCES;DEPS"
    )
    
    # Create executable
    if(EXE_WIN32)
        add_executable(${name} WIN32 ${EXE_SOURCES})
    elseif(EXE_MACOSX_BUNDLE)
        add_executable(${name} MACOSX_BUNDLE ${EXE_SOURCES})
    else()
        add_executable(${name} ${EXE_SOURCES})
    endif()
    
    # Apply compiler settings
    lpzrobots_apply_compiler_settings(${name})
    
    # Add dependencies
    if(EXE_DEPS)
        target_link_libraries(${name} PRIVATE ${EXE_DEPS})
    endif()
    
    # Installation
    if(NOT EXE_NO_INSTALL)
        install(TARGETS ${name}
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
            BUNDLE DESTINATION ${CMAKE_INSTALL_BINDIR}
        )
    endif()
endfunction()

# Function to create a Qt-based GUI tool
function(lpzrobots_add_qt_executable name)
    cmake_parse_arguments(PARSE_ARGV 1 QT
        "NO_INSTALL"
        ""
        "SOURCES;UI_FILES;QRC_FILES;DEPS"
    )
    
    # Process Qt files
    if(LPZROBOTS_QT_VERSION EQUAL 6)
        qt6_standard_project_setup()
        if(QT_UI_FILES)
            qt6_wrap_ui(UI_HEADERS ${QT_UI_FILES})
        endif()
        if(QT_QRC_FILES)
            qt6_add_resources(QRC_SOURCES ${QT_QRC_FILES})
        endif()
    else()
        set(CMAKE_AUTOMOC ON)
        set(CMAKE_AUTOUIC ON)
        set(CMAKE_AUTORCC ON)
        if(QT_UI_FILES)
            qt5_wrap_ui(UI_HEADERS ${QT_UI_FILES})
        endif()
        if(QT_QRC_FILES)
            qt5_add_resources(QRC_SOURCES ${QT_QRC_FILES})
        endif()
    endif()
    
    # Create executable  
    if(APPLE)
        add_executable(${name} MACOSX_BUNDLE 
            ${QT_SOURCES} 
            ${UI_HEADERS} 
            ${QRC_SOURCES}
        )
    else()
        add_executable(${name} 
            ${QT_SOURCES} 
            ${UI_HEADERS} 
            ${QRC_SOURCES}
        )
    endif()
    
    # Apply compiler settings
    lpzrobots_apply_compiler_settings(${name})
    
    # Link Qt libraries
    lpzrobots_apply_dependencies(${name} QT)
    
    # Add other dependencies
    if(QT_DEPS)
        target_link_libraries(${name} PRIVATE ${QT_DEPS})
    endif()
    
    # macOS bundle settings
    if(APPLE)
        set_target_properties(${name} PROPERTIES
            MACOSX_BUNDLE_GUI_IDENTIFIER "org.lpzrobots.${name}"
            MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_VERSION}"
            MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION}"
        )
    endif()
    
    # Installation
    if(NOT QT_NO_INSTALL)
        install(TARGETS ${name}
            BUNDLE DESTINATION ${CMAKE_INSTALL_BINDIR}
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        )
        
        # Deploy Qt libraries on macOS
        if(APPLE AND LPZROBOTS_QT_VERSION EQUAL 6)
            qt_generate_deploy_app_script(
                TARGET ${name}
                OUTPUT_SCRIPT deploy_script
            )
            install(SCRIPT ${deploy_script})
        endif()
    endif()
endfunction()

# Function to create header-only library
function(lpzrobots_add_header_library name)
    cmake_parse_arguments(PARSE_ARGV 1 HDR
        "NO_INSTALL"
        ""
        "HEADERS;DEPS"
    )
    
    # Create interface library
    add_library(${name} INTERFACE)
    add_library(lpzrobots::${name} ALIAS ${name})
    
    # Set include directories
    target_include_directories(${name} INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/${name}>
        $<INSTALL_INTERFACE:include>
        $<INSTALL_INTERFACE:include/${name}>
    )
    
    # Add dependencies
    if(HDR_DEPS)
        target_link_libraries(${name} INTERFACE ${HDR_DEPS})
    endif()
    
    # Installation
    if(NOT HDR_NO_INSTALL)
        install(TARGETS ${name}
            EXPORT LPZRobotsTargets
        )
        
        if(HDR_HEADERS)
            install(FILES ${HDR_HEADERS}
                DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${name}
            )
        else()
            install(DIRECTORY include/${name}
                DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
                FILES_MATCHING PATTERN "*.h"
            )
        endif()
    endif()
endfunction()

# Function to add tests for a component
function(lpzrobots_add_test name)
    cmake_parse_arguments(PARSE_ARGV 1 TEST
        "GTEST;BENCHMARK;COVERAGE"
        "WORKING_DIRECTORY;TIMEOUT;COMPONENT"
        "SOURCES;DEPS;ARGS;LABELS;ENVIRONMENT"
    )
    
    # Set default timeout
    if(NOT TEST_TIMEOUT)
        set(TEST_TIMEOUT 60)
    endif()
    
    # Create test executable
    add_executable(${name} ${TEST_SOURCES})
    
    # Apply compiler settings
    lpzrobots_apply_compiler_settings(${name})
    
    # Add GoogleTest support if requested or if gtest is in dependencies
    set(USES_GTEST FALSE)
    if(TEST_GTEST OR "${TEST_DEPS}" MATCHES "gtest" OR "${TEST_DEPS}" MATCHES "GTest")
        set(USES_GTEST TRUE)
    endif()
    
    # Add dependencies
    if(TEST_DEPS)
        target_link_libraries(${name} PRIVATE ${TEST_DEPS})
    endif()
    
    # Add GoogleTest libraries if needed
    if(USES_GTEST AND LPZROBOTS_HAS_GOOGLETEST)
        if(NOT "${TEST_DEPS}" MATCHES "gtest")
            target_link_libraries(${name} PRIVATE GTest::gtest GTest::gtest_main)
        endif()
        # Add pthread on Linux/macOS
        if(NOT WIN32)
            target_link_libraries(${name} PRIVATE pthread)
        endif()
    endif()
    
    # Add benchmark support if requested
    if(TEST_BENCHMARK)
        find_package(benchmark QUIET)
        if(benchmark_FOUND)
            target_link_libraries(${name} PRIVATE benchmark::benchmark)
            target_compile_definitions(${name} PRIVATE LPZROBOTS_HAS_BENCHMARK)
        endif()
    endif()
    
    # Add coverage flags if requested
    if(TEST_COVERAGE AND CMAKE_BUILD_TYPE STREQUAL "Debug")
        if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
            target_compile_options(${name} PRIVATE --coverage -fprofile-arcs -ftest-coverage)
            target_link_options(${name} PRIVATE --coverage)
        endif()
    endif()
    
    # Add sanitizer support if enabled globally
    if(LPZROBOTS_ENABLE_SANITIZERS)
        lpzrobots_add_sanitizers(${name})
    endif()
    
    # Add test based on type
    if(USES_GTEST AND LPZROBOTS_HAS_GOOGLETEST)
        # Use GoogleTest test discovery for better integration
        include(GoogleTest)
        gtest_discover_tests(${name}
            EXTRA_ARGS ${TEST_ARGS}
            WORKING_DIRECTORY ${TEST_WORKING_DIRECTORY}
            PROPERTIES
                LABELS "${TEST_LABELS};${TEST_COMPONENT}"
                TIMEOUT ${TEST_TIMEOUT}
                ENVIRONMENT "${TEST_ENVIRONMENT}"
        )
    else()
        # Add standard CTest
        add_test(NAME ${name}
            COMMAND ${name} ${TEST_ARGS}
            WORKING_DIRECTORY ${TEST_WORKING_DIRECTORY}
        )
        
        # Set test properties
        set_tests_properties(${name} PROPERTIES
            TIMEOUT ${TEST_TIMEOUT}
            FAIL_REGULAR_EXPRESSION "ERROR;FAIL;Failed"
            LABELS "${TEST_LABELS};${TEST_COMPONENT}"
            ENVIRONMENT "${TEST_ENVIRONMENT}"
        )
    endif()
    
    # Group test executables in IDEs
    set_target_properties(${name} PROPERTIES FOLDER "Tests/${TEST_COMPONENT}")
    
    # Add to component test list for coverage reports
    if(TEST_COMPONENT)
        set_property(GLOBAL APPEND PROPERTY LPZROBOTS_${TEST_COMPONENT}_TESTS ${name})
    endif()
endfunction()

# Function to add sanitizer support to a target
function(lpzrobots_add_sanitizers target)
    if(NOT LPZROBOTS_ENABLE_SANITIZERS)
        return()
    endif()
    
    # Determine which sanitizers to use
    set(SANITIZER_FLAGS "")
    
    if(LPZROBOTS_SANITIZER_ADDRESS)
        list(APPEND SANITIZER_FLAGS -fsanitize=address -fno-omit-frame-pointer)
    endif()
    
    if(LPZROBOTS_SANITIZER_THREAD)
        list(APPEND SANITIZER_FLAGS -fsanitize=thread)
    endif()
    
    if(LPZROBOTS_SANITIZER_UNDEFINED)
        list(APPEND SANITIZER_FLAGS -fsanitize=undefined)
    endif()
    
    if(LPZROBOTS_SANITIZER_MEMORY AND CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        list(APPEND SANITIZER_FLAGS -fsanitize=memory -fno-omit-frame-pointer)
    endif()
    
    # Apply sanitizer flags
    if(SANITIZER_FLAGS)
        target_compile_options(${target} PRIVATE ${SANITIZER_FLAGS})
        target_link_options(${target} PRIVATE ${SANITIZER_FLAGS})
    endif()
endfunction()

# Function to create a test suite for a component
function(lpzrobots_add_test_suite component)
    cmake_parse_arguments(PARSE_ARGV 1 SUITE
        ""
        "TEST_DIR"
        "EXTRA_SOURCES;EXTRA_DEPS"
    )
    
    # Default test directory
    if(NOT SUITE_TEST_DIR)
        set(SUITE_TEST_DIR "${CMAKE_CURRENT_SOURCE_DIR}/tests")
    endif()
    
    # Find all test source files
    file(GLOB TEST_SOURCES 
        "${SUITE_TEST_DIR}/*_test.cpp"
        "${SUITE_TEST_DIR}/*_test.cc"
        "${SUITE_TEST_DIR}/*_unittest.cpp"
        "${SUITE_TEST_DIR}/*_unittest.cc"
    )
    
    if(NOT TEST_SOURCES)
        message(STATUS "No test sources found in ${SUITE_TEST_DIR}")
        return()
    endif()
    
    # Create test executable
    lpzrobots_add_test(${component}_tests
        GTEST
        COMPONENT ${component}
        SOURCES ${TEST_SOURCES} ${SUITE_EXTRA_SOURCES}
        DEPS lpzrobots::${component} ${SUITE_EXTRA_DEPS}
        LABELS "unit;${component}"
    )
    
    # Add custom target for running just this component's tests
    add_custom_target(test_${component}
        COMMAND ${CMAKE_CTEST_COMMAND} -L ${component} --output-on-failure
        COMMENT "Running ${component} tests"
        DEPENDS ${component}_tests
    )
endfunction()

# Function to create config script from CMake template
function(lpzrobots_generate_config_script component)
    # Check if we should use CMake templates or fall back to M4
    set(config_template "${CMAKE_SOURCE_DIR}/cmake/${component}-config.sh.in")
    set(config_m4 "${CMAKE_CURRENT_SOURCE_DIR}/${component}-config.m4")
    set(config_output "${CMAKE_CURRENT_BINARY_DIR}/${component}-config")
    
    # Prefer CMake template if it exists
    if(EXISTS ${config_template})
        # Use CMake template
        message(STATUS "Generating ${component}-config from CMake template")
        
        # Set up template variables
        set(GENERATION_DATE "${CMAKE_CURRENT_DATE}")
        
        # GSL configuration
        if(GSL_FOUND)
            string(REPLACE ";" " " GSL_CFLAGS "${GSL_CFLAGS_OTHER}")
            string(REPLACE ";" " " GSL_LDFLAGS "${GSL_LDFLAGS}")
            # Convert list of libraries to space-separated string with -l prefix
            set(GSL_LIBS_STRING "")
            foreach(lib ${GSL_LIBRARIES})
                if(lib MATCHES "^-")
                    # Already has a flag prefix
                    set(GSL_LIBS_STRING "${GSL_LIBS_STRING} ${lib}")
                elseif(lib MATCHES "^/")
                    # Full path to library
                    set(GSL_LIBS_STRING "${GSL_LIBS_STRING} ${lib}")
                else()
                    # Just library name, add -l prefix
                    set(GSL_LIBS_STRING "${GSL_LIBS_STRING} -l${lib}")
                endif()
            endforeach()
            set(GSL_LIBRARIES "${GSL_LIBS_STRING}")
            set(GSL_FOUND "TRUE")  # Use TRUE instead of 1 for shell script
        else()
            set(GSL_CFLAGS "")
            set(GSL_LDFLAGS "")
            set(GSL_LIBRARIES "")
            set(GSL_FOUND "FALSE")
        endif()
        
        # Ensure CMAKE_OSX_ARCHITECTURES is set
        if(NOT CMAKE_OSX_ARCHITECTURES AND APPLE)
            if(CMAKE_SYSTEM_PROCESSOR MATCHES "arm64")
                set(CMAKE_OSX_ARCHITECTURES "arm64")
            elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64")
                set(CMAKE_OSX_ARCHITECTURES "x86_64")
            endif()
        endif()
        
        # Configure the script
        configure_file(
            ${config_template}
            ${config_output}
            @ONLY
        )
        
        # Make it executable
        file(CHMOD ${config_output}
            PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                       GROUP_READ GROUP_EXECUTE
                       WORLD_READ WORLD_EXECUTE
        )
        
        # Create custom target for dependency tracking
        add_custom_target(${component}_config ALL DEPENDS ${config_output})
        
    elseif(EXISTS ${config_m4})
        # Fall back to M4 processing (deprecated)
        message(STATUS "Generating ${component}-config from M4 template (deprecated)")
        
        find_program(M4_EXECUTABLE m4)
        if(NOT M4_EXECUTABLE)
            message(WARNING "m4 not found - cannot generate ${component}-config script")
            return()
        endif()
        
        # Determine source prefix
        if(LPZROBOTS_INSTALL_TYPE STREQUAL "DEVEL")
            set(SRCPREFIX "${CMAKE_CURRENT_SOURCE_DIR}")
        else()
            set(SRCPREFIX "${CMAKE_INSTALL_PREFIX}")
        endif()
        
        # Prepare M4 flags
        set(M4_FLAGS
            -DPREFIX=${CMAKE_INSTALL_PREFIX}
            -DSRCPREFIX=${SRCPREFIX}
            -D${LPZROBOTS_PLATFORM}
            -D${LPZROBOTS_INSTALL_TYPE}
            -DVERSION=${PROJECT_VERSION}
        )
        
        if(NOT LPZROBOTS_HAS_GSL)
            list(APPEND M4_FLAGS -DNOGSL)
        endif()
        
        # Generate config script
        add_custom_command(
            OUTPUT ${config_output}
            COMMAND ${M4_EXECUTABLE} ${M4_FLAGS} ${config_m4} > ${config_output}.tmp
            COMMAND ${CMAKE_COMMAND} -E rename ${config_output}.tmp ${config_output}
            COMMAND chmod +x ${config_output}
            DEPENDS ${config_m4}
            COMMENT "Generating ${component}-config script"
            VERBATIM
        )
        
        add_custom_target(${component}_config ALL DEPENDS ${config_output})
    else()
        message(WARNING "No template found for ${component}-config")
        return()
    endif()
    
    # Install config script
    install(PROGRAMS ${config_output}
        DESTINATION ${CMAKE_INSTALL_BINDIR}
        COMPONENT ${component}
    )
    
    # Create symlink in build directory for development
    if(LPZROBOTS_INSTALL_TYPE STREQUAL "DEVEL")
        add_custom_command(TARGET ${component}_config POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E create_symlink
                ${config_output}
                ${CMAKE_BINARY_DIR}/${component}-config
            COMMENT "Creating ${component}-config symlink"
        )
    endif()
    
    # Also generate pkg-config file if requested
    lpzrobots_generate_pkgconfig(${component})
endfunction()

# Function to generate pkg-config (.pc) files
function(lpzrobots_generate_pkgconfig component)
    set(pc_template "${CMAKE_SOURCE_DIR}/cmake/lpzrobots.pc.in")
    set(pc_output "${CMAKE_CURRENT_BINARY_DIR}/${component}.pc")
    
    if(NOT EXISTS ${pc_template})
        return()
    endif()
    
    # Set up variables for pkg-config
    set(COMPONENT_NAME ${component})
    
    # Build requirements list
    set(PC_REQUIRES "")
    set(PC_REQUIRES_PRIVATE "")
    
    # Component-specific requirements
    if(component STREQUAL "selforg")
        if(GSL_FOUND)
            set(PC_REQUIRES_PRIVATE "gsl")
        endif()
    elseif(component STREQUAL "ode_robots")
        set(PC_REQUIRES "selforg")
    elseif(component STREQUAL "ga_tools")
        set(PC_REQUIRES "selforg")
    elseif(component STREQUAL "configurator")
        set(PC_REQUIRES "selforg")
        if(LPZROBOTS_QT_VERSION EQUAL 6)
            set(PC_REQUIRES "${PC_REQUIRES} Qt6Core Qt6Widgets")
        else()
            set(PC_REQUIRES "${PC_REQUIRES} Qt5Core Qt5Widgets")
        endif()
    endif()
    
    # Build libs and cflags
    set(PC_LIBS "")
    set(PC_LIBS_PRIVATE "-lpthread -lm")
    set(PC_CFLAGS "-std=c++17")
    
    # Component-specific flags
    if(component STREQUAL "ode_robots")
        set(PC_LIBS_PRIVATE "${PC_LIBS_PRIVATE} ${LPZROBOTS_ODE_LIBRARIES}")
        if(OPENSCENEGRAPH_FOUND)
            # Convert OSG libraries to -l format
            foreach(lib ${OPENSCENEGRAPH_LIBRARIES})
                get_filename_component(libname ${lib} NAME_WE)
                string(REGEX REPLACE "^lib" "" libname ${libname})
                set(PC_LIBS_PRIVATE "${PC_LIBS_PRIVATE} -l${libname}")
            endforeach()
        endif()
    endif()
    
    # Generate the pc file
    configure_file(
        ${pc_template}
        ${pc_output}
        @ONLY
    )
    
    # Install pkg-config file
    install(FILES ${pc_output}
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
        COMPONENT ${component}
    )
endfunction()

# Function to add doctest-based tests
function(lpzrobots_add_doctest name)
    cmake_parse_arguments(PARSE_ARGV 1 TEST
        "COVERAGE;BENCHMARK"
        "WORKING_DIRECTORY;TIMEOUT;COMPONENT"
        "SOURCES;DEPS;ARGS;LABELS;ENVIRONMENT;COMPILE_DEFS"
    )
    
    if(NOT LPZROBOTS_HAS_DOCTEST)
        message(WARNING "doctest not available, skipping test ${name}")
        return()
    endif()
    
    # Set default timeout
    if(NOT TEST_TIMEOUT)
        set(TEST_TIMEOUT 60)
    endif()
    
    # Create test executable
    add_executable(${name} ${TEST_SOURCES})
    
    # Apply compiler settings
    lpzrobots_apply_compiler_settings(${name})
    
    # Link with doctest
    target_link_libraries(${name} PRIVATE doctest)
    
    # Add dependencies
    if(TEST_DEPS)
        target_link_libraries(${name} PRIVATE ${TEST_DEPS})
    endif()
    
    # Add compile definitions
    if(TEST_COMPILE_DEFS)
        target_compile_definitions(${name} PRIVATE ${TEST_COMPILE_DEFS})
    endif()
    
    # Add benchmark support if requested
    if(TEST_BENCHMARK)
        target_compile_definitions(${name} PRIVATE LPZROBOTS_ENABLE_BENCHMARKS)
    endif()
    
    # Add coverage flags if requested
    if(TEST_COVERAGE AND CMAKE_BUILD_TYPE STREQUAL "Debug")
        if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
            target_compile_options(${name} PRIVATE --coverage -fprofile-arcs -ftest-coverage)
            target_link_options(${name} PRIVATE --coverage)
        endif()
    endif()
    
    # Add sanitizer support if enabled globally
    if(LPZROBOTS_ENABLE_SANITIZERS)
        lpzrobots_add_sanitizers(${name})
    endif()
    
    # Register with CTest
    add_test(NAME ${name}
        COMMAND ${name} ${TEST_ARGS}
        WORKING_DIRECTORY ${TEST_WORKING_DIRECTORY}
    )
    
    # Set test properties
    set_tests_properties(${name} PROPERTIES
        TIMEOUT ${TEST_TIMEOUT}
        LABELS "${TEST_LABELS};${TEST_COMPONENT};doctest"
        ENVIRONMENT "${TEST_ENVIRONMENT}"
    )
    
    # Group test executables in IDEs
    set_target_properties(${name} PROPERTIES FOLDER "Tests/${TEST_COMPONENT}")
    
    # Add to component test list for coverage reports
    if(TEST_COMPONENT)
        set_property(GLOBAL APPEND PROPERTY LPZROBOTS_${TEST_COMPONENT}_TESTS ${name})
    endif()
    
    # Note: doctest discovery not available with single-header approach
    # Tests will be discovered at runtime
endfunction()

# Function to create a comprehensive doctest suite for a component
function(lpzrobots_add_doctest_suite component)
    cmake_parse_arguments(PARSE_ARGV 1 SUITE
        ""
        "TEST_DIR;MAIN_SOURCE"
        "EXTRA_SOURCES;EXTRA_DEPS"
    )
    
    # Default test directory
    if(NOT SUITE_TEST_DIR)
        set(SUITE_TEST_DIR "${CMAKE_CURRENT_SOURCE_DIR}/tests")
    endif()
    
    # Find all test source files
    file(GLOB TEST_SOURCES 
        "${SUITE_TEST_DIR}/*_test.cpp"
        "${SUITE_TEST_DIR}/*_tests.cpp"
        "${SUITE_TEST_DIR}/*_doctest.cpp"
        "${SUITE_TEST_DIR}/unit/*.cpp"
        "${SUITE_TEST_DIR}/integration/*.cpp"
    )
    
    if(NOT TEST_SOURCES AND NOT SUITE_EXTRA_SOURCES)
        message(STATUS "No doctest sources found in ${SUITE_TEST_DIR}")
        return()
    endif()
    
    # Use provided main or default
    if(SUITE_MAIN_SOURCE)
        list(APPEND TEST_SOURCES ${SUITE_MAIN_SOURCE})
    elseif(EXISTS "${CMAKE_SOURCE_DIR}/tests/doctest_main.cpp")
        list(APPEND TEST_SOURCES "${CMAKE_SOURCE_DIR}/tests/doctest_main.cpp")
    endif()
    
    # Create test executable
    lpzrobots_add_doctest(${component}_doctest
        COMPONENT ${component}
        SOURCES ${TEST_SOURCES} ${SUITE_EXTRA_SOURCES}
        DEPS lpzrobots::${component} ${SUITE_EXTRA_DEPS}
        LABELS "unit;${component}"
    )
    
    # Add custom target for running just this component's tests
    add_custom_target(doctest_${component}
        COMMAND ${CMAKE_CTEST_COMMAND} -R ${component}_doctest --output-on-failure
        COMMENT "Running ${component} doctest suite"
        DEPENDS ${component}_doctest
    )
endfunction()