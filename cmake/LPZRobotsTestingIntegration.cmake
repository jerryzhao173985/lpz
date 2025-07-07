# LPZRobotsTestingIntegration.cmake
# Complete testing integration module that ensures seamless usage

include_guard(GLOBAL)

# Include all testing modules
include(LPZRobotsTesting)
include(TestDiscovery)
include(TestGeneration)
include(AutoTestDiscovery)

# Function to set up complete testing infrastructure
function(lpzrobots_setup_testing)
    message(STATUS "Setting up LPZRobots testing infrastructure...")
    
    # Enable CTest
    enable_testing()
    
    # Find or fetch testing dependencies
    lpzrobots_setup_test_dependencies()
    
    # Create test directories
    file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/tests")
    file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/test_results")
    file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/benchmarks")
    
    # Configure CTest
    configure_file(
        "${CMAKE_SOURCE_DIR}/cmake/CTestCustom.cmake.in"
        "${CMAKE_BINARY_DIR}/CTestCustom.cmake"
        @ONLY
    )
    
    # Set up test data
    if(EXISTS "${CMAKE_SOURCE_DIR}/tests/data")
        file(COPY "${CMAKE_SOURCE_DIR}/tests/data"
             DESTINATION "${CMAKE_BINARY_DIR}/tests")
    endif()
    
    # Create test main files if they don't exist
    lpzrobots_create_test_main_files()
    
    # Set up test utilities
    lpzrobots_setup_test_utilities()
    
    # Enable different test types based on options
    if(LPZROBOTS_ENABLE_BENCHMARKS)
        message(STATUS "  - Performance benchmarks enabled")
        set(LPZROBOTS_BUILD_BENCHMARKS ON CACHE INTERNAL "")
    endif()
    
    if(LPZROBOTS_ENABLE_PROPERTY_TESTS)
        message(STATUS "  - Property-based tests enabled")
        lpzrobots_find_rapidcheck()
    endif()
    
    if(LPZROBOTS_ENABLE_FUZZING)
        message(STATUS "  - Fuzz testing enabled")
        lpzrobots_setup_fuzzing()
    endif()
    
    if(LPZROBOTS_ENABLE_COVERAGE)
        message(STATUS "  - Code coverage enabled")
        lpzrobots_setup_coverage()
    endif()
    
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
    
    add_custom_target(test_benchmarks
        COMMAND ${CMAKE_CTEST_COMMAND} -L benchmark --output-on-failure
        COMMENT "Running benchmarks"
    )
    
    # Create all_benchmarks target
    add_custom_target(all_benchmarks)
    
    message(STATUS "Testing infrastructure setup complete!")
endfunction()

# Function to handle test dependencies
function(lpzrobots_setup_test_dependencies)
    # Doctest is handled by LPZRobotsDependencies.cmake
    # Just verify it's available
    if(NOT TARGET doctest)
        message(FATAL_ERROR "doctest target not found. Please ensure LPZRobotsDependencies.cmake is included.")
    endif()
    
    # Check for other test dependencies
    if(LPZROBOTS_ENABLE_PROPERTY_TESTS)
        find_package(rapidcheck QUIET)
        if(NOT rapidcheck_FOUND)
            message(STATUS "rapidcheck not found, will fetch if needed")
            set(LPZROBOTS_FETCH_RAPIDCHECK ON CACHE INTERNAL "")
        endif()
    endif()
endfunction()

# Function to create test main files
function(lpzrobots_create_test_main_files)
    # Standard test main
    if(NOT EXISTS "${CMAKE_SOURCE_DIR}/tests/doctest_main_standard.cpp")
        file(WRITE "${CMAKE_SOURCE_DIR}/tests/doctest_main_standard.cpp"
"#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

// Standard doctest main for most tests
")
    endif()
    
    # Performance test main
    if(NOT EXISTS "${CMAKE_SOURCE_DIR}/tests/doctest_main.cpp")
        file(WRITE "${CMAKE_SOURCE_DIR}/tests/doctest_main.cpp"
"#define DOCTEST_CONFIG_IMPLEMENT
#include <doctest/doctest.h>
#include <chrono>
#include <iostream>

int main(int argc, char** argv) {
    doctest::Context context;
    
    // Default configuration for performance tests
    context.setOption(\"no-breaks\", true);
    context.setOption(\"duration\", true);
    
    // Apply command line
    context.applyCommandLine(argc, argv);
    
    // Run tests
    int res = context.run();
    
    return res;
}
")
    endif()
    
    # Simple test main
    if(NOT EXISTS "${CMAKE_SOURCE_DIR}/tests/doctest_main_simple.cpp")
        file(WRITE "${CMAKE_SOURCE_DIR}/tests/doctest_main_simple.cpp"
"#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#define DOCTEST_CONFIG_SUPER_FAST_ASSERTS
#include <doctest/doctest.h>
")
    endif()
endfunction()

# Function to set up test utilities
function(lpzrobots_setup_test_utilities)
    # Ensure test utilities directory exists
    if(NOT EXISTS "${CMAKE_SOURCE_DIR}/tests/include")
        file(MAKE_DIRECTORY "${CMAKE_SOURCE_DIR}/tests/include")
    endif()
    
    # Create basic test utilities header if it doesn't exist
    if(NOT EXISTS "${CMAKE_SOURCE_DIR}/tests/include/lpzrobots_test_utils.h")
        file(WRITE "${CMAKE_SOURCE_DIR}/tests/include/lpzrobots_test_utils.h"
"#ifndef LPZROBOTS_TEST_UTILS_H
#define LPZROBOTS_TEST_UTILS_H

#include <doctest/doctest.h>
#include <cmath>
#include <limits>
#include <random>
#include <vector>
#include <chrono>

namespace lpzrobots {
namespace test {

// Floating point comparison helpers
inline bool approx_equal(double a, double b, double epsilon = 1e-10) {
    return std::abs(a - b) < epsilon;
}

inline bool approx_zero(double a, double epsilon = 1e-10) {
    return std::abs(a) < epsilon;
}

// Random number generation
inline double random_double(double min = -1.0, double max = 1.0) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(min, max);
    return dis(gen);
}

inline std::vector<double> random_vector(size_t size, double min = -1.0, double max = 1.0) {
    std::vector<double> vec(size);
    for(auto& v : vec) {
        v = random_double(min, max);
    }
    return vec;
}

// Timing utilities
class Timer {
    using Clock = std::chrono::high_resolution_clock;
    Clock::time_point start_time;
    
public:
    Timer() : start_time(Clock::now()) {}
    
    double elapsed_ms() const {
        auto end_time = Clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
        return duration.count() / 1000.0;
    }
    
    double elapsed_s() const {
        return elapsed_ms() / 1000.0;
    }
    
    void reset() {
        start_time = Clock::now();
    }
};

// Test data generators
template<typename T>
class TestDataGenerator {
public:
    static std::vector<T> edge_cases();
    static std::vector<T> random_samples(size_t count);
    static std::vector<T> systematic_range(T min, T max, T step);
};

// Common test patterns
#define LPZROBOTS_TEST_EDGE_CASES(func, type) \\
    SUBCASE(\"Edge cases\") { \\
        for(auto val : TestDataGenerator<type>::edge_cases()) { \\
            CHECK_NOTHROW(func(val)); \\
            CHECK(std::isfinite(func(val))); \\
        } \\
    }

#define LPZROBOTS_TEST_RANDOM_INPUTS(func, type, count) \\
    SUBCASE(\"Random inputs\") { \\
        for(auto val : TestDataGenerator<type>::random_samples(count)) { \\
            CHECK_NOTHROW(func(val)); \\
            CHECK(std::isfinite(func(val))); \\
        } \\
    }

// Performance measurement macro
#define LPZROBOTS_BENCHMARK(name, iterations) \\
    SUBCASE(name) { \\
        Timer timer; \\
        for(int i = 0; i < iterations; ++i)

#define LPZROBOTS_BENCHMARK_END(iterations) \\
        double elapsed = timer.elapsed_ms(); \\
        double ops_per_sec = (iterations * 1000.0) / elapsed; \\
        MESSAGE(\"Performance: \" << ops_per_sec << \" ops/sec (\" << elapsed << \" ms total)\"); \\
    }

} // namespace test
} // namespace lpzrobots

#endif // LPZROBOTS_TEST_UTILS_H
")
    endif()
endfunction()

# Function to find rapidcheck for property testing
function(lpzrobots_find_rapidcheck)
    find_package(rapidcheck QUIET)
    if(NOT rapidcheck_FOUND AND LPZROBOTS_FETCH_RAPIDCHECK)
        include(FetchContent)
        FetchContent_Declare(
            rapidcheck
            GIT_REPOSITORY https://github.com/emil-e/rapidcheck.git
            GIT_TAG master
            GIT_SHALLOW TRUE
        )
        FetchContent_MakeAvailable(rapidcheck)
    endif()
endfunction()

# Function to set up fuzzing
function(lpzrobots_setup_fuzzing)
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        set(LPZROBOTS_FUZZER_FLAGS "-fsanitize=fuzzer,address,undefined" CACHE STRING "")
        message(STATUS "  - Using libFuzzer with Clang")
    else()
        message(WARNING "Fuzzing requires Clang compiler. Disabling fuzzing.")
        set(LPZROBOTS_ENABLE_FUZZING OFF CACHE BOOL "" FORCE)
    endif()
endfunction()

# Function to set up coverage
function(lpzrobots_setup_coverage)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        set(LPZROBOTS_COVERAGE_FLAGS "-fprofile-arcs -ftest-coverage" CACHE STRING "")
        
        # Find coverage tools
        find_program(LCOV lcov)
        find_program(GENHTML genhtml)
        
        if(LCOV AND GENHTML)
            # Add coverage target
            add_custom_target(coverage
                COMMAND ${LCOV} --directory . --zerocounters
                COMMAND ${CMAKE_CTEST_COMMAND} -j ${PROCESSOR_COUNT}
                COMMAND ${LCOV} --directory . --capture --output-file coverage.info
                COMMAND ${LCOV} --remove coverage.info '/usr/*' '*/test/*' --output-file coverage_filtered.info
                COMMAND ${GENHTML} coverage_filtered.info --output-directory coverage_report
                WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
                COMMENT "Generating coverage report..."
            )
        else()
            message(WARNING "lcov/genhtml not found. Coverage target will not be available.")
        endif()
    else()
        message(WARNING "Coverage requires GCC or Clang. Disabling coverage.")
        set(LPZROBOTS_ENABLE_COVERAGE OFF CACHE BOOL "" FORCE)
    endif()
endfunction()

# Macro to add tests for a component (enhanced version)
macro(lpzrobots_add_component_tests component)
    # Create test suite
    lpzrobots_create_test_suite(${component})
    
    # Auto-discover tests
    if(LPZROBOTS_ENABLE_AUTO_TEST_DISCOVERY)
        lpzrobots_auto_discover_tests(
            TARGET ${component}
            SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}"
            VERBOSE
        )
    endif()
    
    # Generate standard tests
    if(LPZROBOTS_ENABLE_TEST_GENERATION)
        lpzrobots_generate_all_tests(${component})
    endif()
endmacro()

# Function to create a quick start test
function(lpzrobots_create_quickstart_test)
    set(quickstart_test "${CMAKE_BINARY_DIR}/tests/quickstart_test.cpp")
    file(WRITE ${quickstart_test}
"#include <doctest/doctest.h>
#include <selforg/matrix.h>

TEST_CASE(\"LPZRobots Quick Start Test\") {
    using namespace matrix;
    
    SUBCASE(\"Matrix creation and operations\") {
        Matrix A(2, 2);
        A.val(0, 0) = 1.0; A.val(0, 1) = 2.0;
        A.val(1, 0) = 3.0; A.val(1, 1) = 4.0;
        
        Matrix B = A;
        B.toTranspose();
        
        CHECK(B.val(0, 0) == 1.0);
        CHECK(B.val(0, 1) == 3.0);
        CHECK(B.val(1, 0) == 2.0);
        CHECK(B.val(1, 1) == 4.0);
        
        MESSAGE(\"Basic matrix operations work!\");
    }
}
")
    
    lpzrobots_add_test_executable(quickstart_test
        UNIT
        SOURCES ${quickstart_test}
        DEPS lpzrobots::selforg
        LABELS quickstart
    )
endfunction()

# Export this module
set(LPZROBOTS_TESTING_INTEGRATED TRUE CACHE INTERNAL "")