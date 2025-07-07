# TestDiscovery.cmake
# Automatic test discovery and registration for LPZRobots

include_guard(GLOBAL)

# Function to automatically discover and register tests
function(lpzrobots_discover_tests target)
    cmake_parse_arguments(PARSE_ARGV 1 DISCOVER
        "WORKING_DIRECTORY"
        "TEST_PREFIX;TEST_SUFFIX;PROPERTIES;ENVIRONMENT"
        ""
    )
    
    # Default values
    if(NOT DISCOVER_TEST_PREFIX)
        set(DISCOVER_TEST_PREFIX "${target}::")
    endif()
    
    if(NOT DISCOVER_WORKING_DIRECTORY)
        set(DISCOVER_WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
    endif()
    
    # Use doctest's test discovery if available
    if(TARGET doctest::doctest AND CMAKE_VERSION VERSION_GREATER_EQUAL 3.10)
        include(${doctest_SOURCE_DIR}/scripts/cmake/doctest.cmake)
        doctest_discover_tests(${target}
            TEST_PREFIX ${DISCOVER_TEST_PREFIX}
            TEST_SUFFIX ${DISCOVER_TEST_SUFFIX}
            PROPERTIES ${DISCOVER_PROPERTIES}
            WORKING_DIRECTORY ${DISCOVER_WORKING_DIRECTORY}
        )
    else()
        # Fallback: Register single test
        add_test(
            NAME ${DISCOVER_TEST_PREFIX}all
            COMMAND ${target}
            WORKING_DIRECTORY ${DISCOVER_WORKING_DIRECTORY}
        )
        
        if(DISCOVER_PROPERTIES)
            set_tests_properties(${DISCOVER_TEST_PREFIX}all PROPERTIES ${DISCOVER_PROPERTIES})
        endif()
        
        if(DISCOVER_ENVIRONMENT)
            set_tests_properties(${DISCOVER_TEST_PREFIX}all PROPERTIES ENVIRONMENT ${DISCOVER_ENVIRONMENT})
        endif()
    endif()
endfunction()

# Function to create a test suite from multiple source files
function(lpzrobots_create_test_suite name)
    cmake_parse_arguments(PARSE_ARGV 1 SUITE
        "AUTO_DISCOVER"
        "TYPE;MAIN_FILE"
        "SOURCES;DEPENDS;LABELS"
    )
    
    # Auto-discover test files if requested
    if(SUITE_AUTO_DISCOVER)
        file(GLOB_RECURSE discovered_tests
            "${CMAKE_CURRENT_SOURCE_DIR}/*_test.cpp"
            "${CMAKE_CURRENT_SOURCE_DIR}/*_tests.cpp"
            "${CMAKE_CURRENT_SOURCE_DIR}/*_doctest.cpp"
        )
        list(APPEND SUITE_SOURCES ${discovered_tests})
    endif()
    
    # Remove duplicates
    list(REMOVE_DUPLICATES SUITE_SOURCES)
    
    # Determine main file
    if(NOT SUITE_MAIN_FILE)
        if(SUITE_TYPE STREQUAL "performance")
            set(SUITE_MAIN_FILE "${CMAKE_SOURCE_DIR}/tests/doctest_main.cpp")
        else()
            set(SUITE_MAIN_FILE "${CMAKE_SOURCE_DIR}/tests/doctest_main_standard.cpp")
        endif()
    endif()
    
    # Create test executable
    lpzrobots_add_test_executable(${name}
        ${SUITE_TYPE}
        SOURCES ${SUITE_SOURCES}
        MAIN_FILE ${SUITE_MAIN_FILE}
        DEPS ${SUITE_DEPENDS}
        LABELS ${SUITE_LABELS}
    )
    
    # Discover tests
    lpzrobots_discover_tests(${name}
        TEST_PREFIX "${name}::"
        PROPERTIES LABELS "${SUITE_TYPE};${SUITE_LABELS}"
    )
endfunction()

# Function to create component test structure
function(lpzrobots_setup_component_tests component_name component_target)
    set(test_base_dir "${CMAKE_CURRENT_SOURCE_DIR}/tests")
    
    # Create test directories if they don't exist
    file(MAKE_DIRECTORY "${test_base_dir}/unit")
    file(MAKE_DIRECTORY "${test_base_dir}/integration")
    file(MAKE_DIRECTORY "${test_base_dir}/performance")
    
    # Unit tests
    file(GLOB unit_tests "${test_base_dir}/unit/*.cpp")
    if(unit_tests)
        lpzrobots_create_test_suite(${component_name}_unit_tests
            TYPE unit
            SOURCES ${unit_tests}
            DEPENDS ${component_target}
            LABELS ${component_name}
        )
    endif()
    
    # Integration tests
    file(GLOB integration_tests "${test_base_dir}/integration/*.cpp")
    if(integration_tests)
        lpzrobots_create_test_suite(${component_name}_integration_tests
            TYPE integration
            SOURCES ${integration_tests}
            DEPENDS ${component_target}
            LABELS ${component_name}
        )
    endif()
    
    # Performance tests
    file(GLOB perf_tests "${test_base_dir}/performance/*.cpp")
    if(perf_tests)
        lpzrobots_create_test_suite(${component_name}_performance_tests
            TYPE performance
            SOURCES ${perf_tests}
            DEPENDS ${component_target}
            LABELS ${component_name}
            ENVIRONMENT "LPZROBOTS_TEST_PERF=1"
        )
    endif()
    
    # Add convenience target to run all component tests
    add_custom_target(test_${component_name}
        COMMAND ${CMAKE_CTEST_COMMAND} -L ${component_name} --output-on-failure
        DEPENDS 
            $<TARGET_NAME_IF_EXISTS:${component_name}_unit_tests>
            $<TARGET_NAME_IF_EXISTS:${component_name}_integration_tests>
            $<TARGET_NAME_IF_EXISTS:${component_name}_performance_tests>
        COMMENT "Running all ${component_name} tests"
    )
endfunction()

# Macro to easily add tests in component CMakeLists.txt
macro(lpzrobots_enable_component_tests)
    if(BUILD_TESTS)
        lpzrobots_setup_component_tests(${PROJECT_NAME} ${PROJECT_NAME})
    endif()
endmacro()

# Function to generate test coverage report
function(lpzrobots_add_coverage_target)
    if(NOT LPZROBOTS_ENABLE_COVERAGE)
        return()
    endif()
    
    find_program(LCOV lcov)
    find_program(GENHTML genhtml)
    
    if(NOT LCOV OR NOT GENHTML)
        message(WARNING "lcov/genhtml not found, coverage target disabled")
        return()
    endif()
    
    add_custom_target(coverage
        # Clean previous coverage data
        COMMAND ${LCOV} --directory . --zerocounters
        
        # Run tests
        COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
        
        # Capture coverage data
        COMMAND ${LCOV} --directory . --capture --output-file coverage.info
        
        # Remove external and test code
        COMMAND ${LCOV} --remove coverage.info 
            '*/tests/*' 
            '*/build/*' 
            '/usr/*' 
            '/opt/*' 
            --output-file coverage.info.cleaned
        
        # Generate HTML report
        COMMAND ${GENHTML} -o coverage_report coverage.info.cleaned
        
        COMMAND ${CMAKE_COMMAND} -E echo "Coverage report generated in coverage_report/index.html"
        
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Generating code coverage report"
    )
endfunction()

# Function to add memory check target
function(lpzrobots_add_memcheck_target)
    find_program(VALGRIND valgrind)
    
    if(NOT VALGRIND)
        message(STATUS "valgrind not found, memcheck target disabled")
        return()
    endif()
    
    add_custom_target(memcheck
        COMMAND ${CMAKE_CTEST_COMMAND} 
            -T memcheck
            --overwrite MemoryCheckCommand=${VALGRIND}
            --overwrite MemoryCheckCommandOptions="--leak-check=full --track-origins=yes --error-exitcode=1"
            --output-on-failure
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Running memory leak detection"
    )
endfunction()

# Function to setup all test infrastructure
function(lpzrobots_setup_testing)
    # Enable CTest
    enable_testing()
    
    # Add convenience targets
    add_custom_target(test_all
        COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
        COMMENT "Running all tests"
    )
    
    add_custom_target(test_unit
        COMMAND ${CMAKE_CTEST_COMMAND} -L unit --output-on-failure
        COMMENT "Running unit tests"
    )
    
    add_custom_target(test_integration
        COMMAND ${CMAKE_CTEST_COMMAND} -L integration --output-on-failure
        COMMENT "Running integration tests"
    )
    
    add_custom_target(test_performance
        COMMAND ${CMAKE_CTEST_COMMAND} -L performance --output-on-failure
        ENVIRONMENT LPZROBOTS_TEST_PERF=1
        COMMENT "Running performance tests"
    )
    
    add_custom_target(test_bdd
        COMMAND ${CMAKE_CTEST_COMMAND} -L bdd --output-on-failure
        COMMENT "Running BDD-style scenario tests"
    )
    
    # Add coverage if enabled
    lpzrobots_add_coverage_target()
    
    # Add memory check
    lpzrobots_add_memcheck_target()
    
    # Configure CTest
    configure_file(
        ${CMAKE_SOURCE_DIR}/cmake/CTestCustom.cmake.in
        ${CMAKE_BINARY_DIR}/CTestCustom.cmake
        @ONLY
    )
endfunction()