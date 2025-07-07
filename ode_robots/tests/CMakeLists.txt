# Ode_robots component tests

if(NOT BUILD_TESTS)
    return()
endif()

# Find or fetch GoogleTest (already done in dependencies)
if(NOT LPZROBOTS_HAS_GOOGLETEST)
    message(STATUS "GoogleTest not available, skipping ode_robots tests")
    return()
endif()

# Robot factory tests
lpzrobots_add_test(robot_factory_test
    GTEST
    COMPONENT ode_robots
    SOURCES robot_factory_test.cpp
    DEPS lpzrobots::ode_robots lpzrobots::selforg
    LABELS "unit;factory;robot"
)

# Add test suite target for running all ode_robots tests
add_custom_target(test_ode_robots
    COMMAND ${CMAKE_CTEST_COMMAND} -L ode_robots --output-on-failure
    COMMENT "Running ode_robots component tests"
)

# Add coverage target if enabled
if(LPZROBOTS_ENABLE_COVERAGE)
    include(CodeCoverage)
    
    # Get all test executables for this component
    get_property(ODE_ROBOTS_TESTS GLOBAL PROPERTY LPZROBOTS_ode_robots_TESTS)
    
    setup_target_for_coverage_lcov(
        NAME ode_robots_coverage
        EXECUTABLE ctest -L ode_robots
        DEPENDENCIES ${ODE_ROBOTS_TESTS}
        EXCLUDE "/usr/*" "*/tests/*" "*/examples/*"
    )
endif()