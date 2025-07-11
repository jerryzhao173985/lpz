# Tests CMakeLists.txt
cmake_minimum_required(VERSION 3.20)

# Enable testing
enable_testing()

# Add include directory for test utilities
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Add test data directory definition
add_compile_definitions(LPZROBOTS_TEST_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data")

# Include test discovery module if available
if(COMMAND lpzrobots_discover_tests)
    # TestDiscovery module is already loaded from parent
else()
    # Add module path if needed
    list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
    include(TestDiscovery OPTIONAL)
endif()

# Create main test executable with all test files
set(TEST_SOURCES
    src/doctest_main.cpp
    src/test_selforg_basic.cpp
    src/test_ga_tools_basic.cpp
)

# Integration test sources
set(INTEGRATION_TEST_SOURCES
    src/doctest_main_simple.cpp
    src/test_selforg_basic.cpp
)

# Main unit test executable
lpzrobots_add_doctest(unit_tests
    SOURCES ${TEST_SOURCES}
    DEPS 
        lpzrobots::selforg
        lpzrobots::ga_tools
    COMPONENT core
    LABELS unit
)

# Alternative unit test executable with standard doctest output
set(TEST_SOURCES_STANDARD
    src/doctest_main_standard.cpp
    src/test_selforg_basic.cpp
    src/test_ga_tools_basic.cpp
)

lpzrobots_add_doctest(unit_tests_verbose
    SOURCES ${TEST_SOURCES_STANDARD}
    DEPS 
        lpzrobots::selforg
        lpzrobots::ga_tools
    COMPONENT core
    LABELS unit
)

# Create integration test executable
lpzrobots_add_doctest(integration_tests
    SOURCES ${INTEGRATION_TEST_SOURCES}
    DEPS 
        lpzrobots::selforg
    COMPONENT integration
    LABELS integration
)

# Performance test sources
set(PERFORMANCE_TEST_SOURCES
    src/doctest_main_simple.cpp
    src/test_selforg_basic.cpp
)

# Create performance test executable
lpzrobots_add_doctest(performance_tests
    SOURCES ${PERFORMANCE_TEST_SOURCES}
    DEPS 
        lpzrobots::selforg
    COMPONENT performance
    LABELS performance
)

# BDD-style test sources
set(BDD_TEST_SOURCES
    src/doctest_main_simple.cpp
    src/test_selforg_basic.cpp
)

# Create BDD test executable
lpzrobots_add_doctest(bdd_tests
    SOURCES ${BDD_TEST_SOURCES}
    DEPS 
        lpzrobots::selforg
    COMPONENT bdd
    LABELS bdd scenarios
)

# Create component-specific test executables
if(TARGET selforg)
    lpzrobots_add_doctest(selforg_tests
        SOURCES 
            src/doctest_main.cpp
            src/test_selforg_basic.cpp
        DEPS lpzrobots::selforg
        COMPONENT selforg
        LABELS unit selforg
    )
endif()

if(TARGET ode_robots)
    lpzrobots_add_doctest(ode_robots_tests
        SOURCES 
            src/doctest_main.cpp
            src/test_selforg_basic.cpp
        DEPS lpzrobots::ode_robots
        COMPONENT ode_robots
        LABELS unit ode_robots physics
    )
endif()

# Add custom targets for running specific test suites
# Note: test_unit, test_integration, test_performance are created by TestDiscovery.cmake
# Only add test_bdd here as it's not in the main setup
if(NOT TARGET test_bdd)
    add_custom_target(test_bdd
        COMMAND ${CMAKE_CTEST_COMMAND} -L bdd --output-on-failure
        COMMENT "Running BDD-style tests"
        DEPENDS bdd_tests
    )
endif()

# Create test data directory
file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/data)

# Copy test configuration files if they exist
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/data/test_config.xml)
    configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/data/test_config.xml
        ${CMAKE_CURRENT_BINARY_DIR}/test_config.xml
        COPYONLY
    )
endif()

# Add valgrind target for memory testing
find_program(VALGRIND_EXECUTABLE valgrind)
if(VALGRIND_EXECUTABLE)
    add_custom_target(test_memcheck
        COMMAND ${VALGRIND_EXECUTABLE} 
            --leak-check=full 
            --show-leak-kinds=all 
            --track-origins=yes 
            --verbose 
            --log-file=memcheck.log
            $<TARGET_FILE:unit_tests>
        COMMENT "Running memory check with valgrind"
        DEPENDS unit_tests
    )
endif()

# Add coverage target if enabled
if(CMAKE_BUILD_TYPE STREQUAL "Debug" AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    find_program(GCOV_EXECUTABLE gcov)
    find_program(LCOV_EXECUTABLE lcov)
    find_program(GENHTML_EXECUTABLE genhtml)
    
    if(LCOV_EXECUTABLE AND GENHTML_EXECUTABLE)
        add_custom_target(test_coverage
            # Clear previous coverage data
            COMMAND ${LCOV_EXECUTABLE} --directory . --zerocounters
            
            # Run tests
            COMMAND ${CMAKE_CTEST_COMMAND} -L unit
            
            # Capture coverage data
            COMMAND ${LCOV_EXECUTABLE} --directory . --capture --output-file coverage.info
            
            # Remove system headers and test files from coverage
            COMMAND ${LCOV_EXECUTABLE} --remove coverage.info 
                '/usr/*' '*/test/*' '*/doctest/*' 
                --output-file coverage.filtered.info
            
            # Generate HTML report
            COMMAND ${GENHTML_EXECUTABLE} coverage.filtered.info 
                --output-directory coverage_report
            
            COMMENT "Generating code coverage report"
            DEPENDS unit_tests
        )
        
        message(STATUS "Coverage target available: make test_coverage")
    endif()
endif()

# Summary of available test targets
message(STATUS "")
message(STATUS "Available test targets:")
message(STATUS "  make test              - Run all tests")
message(STATUS "  make test_unit         - Run unit tests only")
message(STATUS "  make test_integration  - Run integration tests")
message(STATUS "  make test_performance  - Run performance benchmarks")
message(STATUS "  make test_bdd          - Run BDD-style scenario tests")
message(STATUS "  make test_selforg      - Run selforg component tests")
message(STATUS "  make test_ode_robots   - Run ode_robots component tests")
if(VALGRIND_EXECUTABLE)
    message(STATUS "  make test_memcheck     - Run memory leak detection")
endif()
if(TARGET test_coverage)
    message(STATUS "  make test_coverage     - Generate code coverage report")
endif()
message(STATUS "")

# Create a convenience script for running tests
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/run_tests.sh
"#!/bin/bash
# Convenience script for running LPZRobots tests

if [ \"\$1\" = \"--help\" ] || [ \"\$1\" = \"-h\" ]; then
    echo \"Usage: \$0 [unit|integration|performance|all] [doctest-args]\"
    echo \"\"
    echo \"Test types:\"
    echo \"  unit         - Run unit tests\"
    echo \"  integration  - Run integration tests\"
    echo \"  performance  - Run performance benchmarks\"
    echo \"  all          - Run all tests (default)\"
    echo \"\"
    echo \"Additional doctest arguments can be passed after the test type\"
    echo \"Examples:\"
    echo \"  \$0 unit --reporter=xml\"
    echo \"  \$0 unit --test-suite=Matrix*\"
    echo \"  \$0 performance --no-skip\"
    exit 0
fi

TEST_TYPE=\${1:-all}
shift

case \$TEST_TYPE in
    unit)
        ./unit_tests --unit \$@
        ;;
    integration)
        ./unit_tests --integration \$@
        ;;
    performance)
        ./unit_tests --performance \$@
        ;;
    all)
        ./unit_tests \$@
        ;;
    *)
        echo \"Unknown test type: \$TEST_TYPE\"
        echo \"Use --help for usage information\"
        exit 1
        ;;
esac
")

file(CHMOD ${CMAKE_CURRENT_BINARY_DIR}/run_tests.sh
    PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                GROUP_READ GROUP_EXECUTE
                WORLD_READ WORLD_EXECUTE
)

# Install test executables (optional)
if(LPZROBOTS_INSTALL_TESTS)
    install(TARGETS unit_tests
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}/tests
    )
    
    install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/run_tests.sh
        DESTINATION ${CMAKE_INSTALL_BINDIR}/tests
    )
endif()