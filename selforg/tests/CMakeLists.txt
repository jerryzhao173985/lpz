# Selforg component tests

if(NOT BUILD_TESTS)
    return()
endif()

# Find or fetch GoogleTest (already done in dependencies)
if(NOT LPZROBOTS_HAS_GOOGLETEST)
    message(STATUS "GoogleTest not available, skipping selforg tests")
    return()
endif()

# Matrix tests
lpzrobots_add_test(matrix_test
    GTEST
    COMPONENT selforg
    SOURCES matrix_test.cpp
    DEPS lpzrobots::selforg
    LABELS "unit;matrix;core"
)

# Matrix pool tests
lpzrobots_add_test(matrixpool_test
    GTEST
    COMPONENT selforg
    SOURCES matrixpool_test.cpp
    DEPS lpzrobots::selforg
    LABELS "unit;memory;performance"
)

# Controller base tests (to be implemented)
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/controller_base_test.cpp")
    lpzrobots_add_test(controller_base_test
        GTEST
        COMPONENT selforg
        SOURCES controller_base_test.cpp
        DEPS lpzrobots::selforg
        LABELS "unit;controller"
    )
endif()

# Learning strategy tests (to be implemented)
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/learning_strategy_test.cpp")
    lpzrobots_add_test(learning_strategy_test
        GTEST
        COMPONENT selforg
        SOURCES learning_strategy_test.cpp
        DEPS lpzrobots::selforg
        LABELS "unit;learning"
    )
endif()

# Controller factory tests (to be implemented)
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/controller_factory_test.cpp")
    lpzrobots_add_test(controller_factory_test
        GTEST
        COMPONENT selforg
        SOURCES controller_factory_test.cpp
        DEPS lpzrobots::selforg
        LABELS "unit;factory"
    )
endif()

# Add test suite target for running all selforg tests
add_custom_target(test_selforg
    COMMAND ${CMAKE_CTEST_COMMAND} -L selforg --output-on-failure
    COMMENT "Running selforg component tests"
)

# Add coverage target if enabled
if(LPZROBOTS_ENABLE_COVERAGE)
    include(CodeCoverage)
    
    # Get all test executables for this component
    get_property(SELFORG_TESTS GLOBAL PROPERTY LPZROBOTS_selforg_TESTS)
    
    setup_target_for_coverage_lcov(
        NAME selforg_coverage
        EXECUTABLE ctest -L selforg
        DEPENDENCIES ${SELFORG_TESTS}
        EXCLUDE "/usr/*" "*/tests/*" "*/examples/*"
    )
endif()

# Add benchmark tests if Google Benchmark is available
find_package(benchmark QUIET)
if(benchmark_FOUND)
    # Matrix benchmark
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/matrix_benchmark.cpp")
        lpzrobots_add_test(matrix_benchmark
            BENCHMARK
            COMPONENT selforg
            SOURCES matrix_benchmark.cpp
            DEPS lpzrobots::selforg benchmark::benchmark
            LABELS "benchmark;matrix;performance"
        )
    endif()
endif()

# Integration tests that need multiple components
if(TARGET ode_robots)
    # Controller integration tests
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/controller_integration_test.cpp")
        lpzrobots_add_test(controller_integration_test
            GTEST
            COMPONENT selforg
            SOURCES controller_integration_test.cpp
            DEPS lpzrobots::selforg lpzrobots::ode_robots
            LABELS "integration;controller"
        )
    endif()
endif()