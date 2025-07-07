# LPZRobotsTesting.cmake
# Enhanced testing support for LPZRobots

include_guard(GLOBAL)

# Enable testing if not already done
include(CTest)

# Find doctest
find_package(doctest QUIET)

# Set up testing variables
set(LPZROBOTS_TEST_UTILS_DIR "${CMAKE_SOURCE_DIR}/tests/include" CACHE PATH "Path to test utilities")
set(LPZROBOTS_TEST_DATA_DIR "${CMAKE_SOURCE_DIR}/tests/data" CACHE PATH "Path to test data")

# Helper function to set up test environment
function(lpzrobots_setup_test_environment target)
    # Add test utility include directory
    target_include_directories(${target} PRIVATE ${LPZROBOTS_TEST_UTILS_DIR})
    
    # Add test data directory definition
    target_compile_definitions(${target} PRIVATE
        LPZROBOTS_TEST_DATA_DIR="${LPZROBOTS_TEST_DATA_DIR}"
    )
    
    # Add common test compile options
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(${target} PRIVATE
            -Wall -Wextra -Wpedantic
            $<$<CONFIG:Debug>:-g -O0>
            $<$<CONFIG:Release>:-O3>
        )
    endif()
    
    # Enable sanitizers in debug mode if requested
    if(LPZROBOTS_ENABLE_SANITIZERS AND CMAKE_BUILD_TYPE STREQUAL "Debug")
        lpzrobots_add_sanitizers(${target})
    endif()
endfunction()

# Enhanced doctest integration function
function(lpzrobots_add_test_executable name)
    cmake_parse_arguments(TEST
        "PERFORMANCE;INTEGRATION;UNIT;BDD"
        "MAIN_FILE;TIMEOUT;WORKING_DIR"
        "SOURCES;DEPS;LABELS;ENV;ARGS"
        ${ARGN}
    )
    
    # Check if doctest is available
    if(NOT doctest_FOUND)
        message(WARNING "doctest not found, skipping test ${name}")
        return()
    endif()
    
    # Determine main file
    if(NOT TEST_MAIN_FILE)
        if(TEST_PERFORMANCE)
            set(TEST_MAIN_FILE "${CMAKE_SOURCE_DIR}/tests/doctest_main.cpp")
        else()
            set(TEST_MAIN_FILE "${CMAKE_SOURCE_DIR}/tests/doctest_main_standard.cpp")
        endif()
    endif()
    
    # Create executable
    add_executable(${name} ${TEST_MAIN_FILE} ${TEST_SOURCES})
    
    # Set up test environment
    lpzrobots_setup_test_environment(${name})
    
    # Link dependencies
    target_link_libraries(${name} PRIVATE
        doctest::doctest
        ${TEST_DEPS}
    )
    
    # Set properties
    set_target_properties(${name} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/tests"
        FOLDER "Tests"
    )
    
    # Determine test type for labeling
    set(test_type "general")
    if(TEST_UNIT)
        set(test_type "unit")
    elseif(TEST_INTEGRATION)
        set(test_type "integration")
    elseif(TEST_PERFORMANCE)
        set(test_type "performance")
    elseif(TEST_BDD)
        set(test_type "bdd")
    endif()
    
    # Add to CTest
    add_test(
        NAME ${name}
        COMMAND ${name} ${TEST_ARGS}
        WORKING_DIRECTORY ${TEST_WORKING_DIR}
    )
    
    # Set test properties
    set(test_labels "${test_type}")
    if(TEST_LABELS)
        list(APPEND test_labels ${TEST_LABELS})
    endif()
    
    list(JOIN test_labels ";" labels_string)
    
    if(TEST_ENV)
        set_tests_properties(${name} PROPERTIES
            LABELS "${labels_string}"
            TIMEOUT "${TEST_TIMEOUT}"
            ENVIRONMENT "${TEST_ENV}"
        )
    else()
        set_tests_properties(${name} PROPERTIES
            LABELS "${labels_string}"
            TIMEOUT "${TEST_TIMEOUT}"
        )
    endif()
    
    # Add custom target for this specific test
    add_custom_target(run_${name}
        COMMAND ${name} --success --duration
        DEPENDS ${name}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Running ${name} with detailed output"
    )
endfunction()

# Function to create a test suite for a component
function(lpzrobots_create_test_suite component)
    # Unit tests
    file(GLOB unit_sources "${CMAKE_CURRENT_SOURCE_DIR}/tests/unit/*.cpp")
    if(unit_sources)
        lpzrobots_add_test_executable(${component}_unit_tests
            UNIT
            SOURCES ${unit_sources}
            DEPS lpzrobots::${component}
            LABELS ${component}
        )
    endif()
    
    # Integration tests
    file(GLOB integration_sources "${CMAKE_CURRENT_SOURCE_DIR}/tests/integration/*.cpp")
    if(integration_sources)
        lpzrobots_add_test_executable(${component}_integration_tests
            INTEGRATION
            SOURCES ${integration_sources}
            DEPS lpzrobots::${component}
            LABELS ${component}
        )
    endif()
    
    # Performance tests
    file(GLOB perf_sources "${CMAKE_CURRENT_SOURCE_DIR}/tests/performance/*.cpp")
    if(perf_sources)
        lpzrobots_add_test_executable(${component}_performance_tests
            PERFORMANCE
            SOURCES ${perf_sources}
            DEPS lpzrobots::${component}
            LABELS ${component}
            ENV "LPZROBOTS_TEST_PERF=1"
        )
    endif()
endfunction()

# Macro to simplify test case writing
macro(LPZROBOTS_TEST_CASE name)
    TEST_CASE(${name} "[${CMAKE_PROJECT_NAME}]")
endmacro()

# Function to generate test report
function(lpzrobots_generate_test_report)
    add_custom_target(test_report
        COMMAND ${CMAKE_CTEST_COMMAND} -T Test
        COMMAND ${CMAKE_CTEST_COMMAND} -T Coverage
        COMMAND ${CMAKE_CTEST_COMMAND} -T MemCheck
        COMMENT "Generating comprehensive test report"
    )
endfunction()

# Function to add performance regression tests
function(lpzrobots_add_performance_regression_test name baseline_file)
    if(NOT EXISTS ${baseline_file})
        message(WARNING "Baseline file ${baseline_file} not found for ${name}")
        return()
    endif()
    
    add_test(
        NAME ${name}_regression
        COMMAND ${CMAKE_COMMAND}
            -DTEST_EXECUTABLE=$<TARGET_FILE:${name}>
            -DBASELINE_FILE=${baseline_file}
            -DTOLERANCE=0.1
            -P ${CMAKE_SOURCE_DIR}/cmake/RunPerformanceRegression.cmake
    )
    
    set_tests_properties(${name}_regression PROPERTIES
        LABELS "performance;regression"
        DEPENDS ${name}
    )
endfunction()

# Export test utilities for use in other projects
if(LPZROBOTS_EXPORT_TESTING)
    install(
        FILES ${LPZROBOTS_TEST_UTILS_DIR}/lpzrobots_test_utils.h
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/lpzrobots/testing
    )
    
    install(
        FILES ${CMAKE_CURRENT_LIST_FILE}
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/lpzrobots
    )
endif()