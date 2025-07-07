# TestGeneration.cmake
# Automated test generation for LPZRobots components

include_guard(GLOBAL)

# Function to generate controller tests automatically
function(lpzrobots_generate_controller_tests controller_class)
    set(template_file "${CMAKE_SOURCE_DIR}/cmake/templates/controller_test.cpp.in")
    set(output_file "${CMAKE_CURRENT_BINARY_DIR}/tests/generated/${controller_class}_generated_test.cpp")
    
    # Create generated tests directory
    file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/tests/generated")
    
    # Configure the test template
    configure_file(${template_file} ${output_file} @ONLY)
    
    # Add to test sources
    set(LPZROBOTS_GENERATED_TEST_SOURCES 
        ${LPZROBOTS_GENERATED_TEST_SOURCES} 
        ${output_file} 
        PARENT_SCOPE
    )
    
    message(STATUS "Generated test for ${controller_class}")
endfunction()

# Function to generate matrix operation tests
function(lpzrobots_generate_matrix_tests)
    set(operations "add;subtract;multiply;transpose;invert")
    set(sizes "2x2;3x3;4x4;10x10;50x50")
    
    foreach(op IN LISTS operations)
        foreach(size IN LISTS sizes)
            string(REPLACE "x" ";" dims ${size})
            list(GET dims 0 rows)
            list(GET dims 1 cols)
            
            set(test_name "matrix_${op}_${size}_test")
            set(output_file "${CMAKE_CURRENT_BINARY_DIR}/tests/generated/${test_name}.cpp")
            
            # Generate test file
            file(WRITE ${output_file} 
"#include <doctest/doctest.h>
#include <selforg/matrix.h>
#include <lpzrobots_test_utils.h>

TEST_CASE(\"Matrix ${op} operation ${size}\") {
    using namespace matrix;
    const int ROWS = ${rows};
    const int COLS = ${cols};
    
    Matrix A(ROWS, COLS);
    Matrix B(ROWS, COLS);
    
    // Initialize with random values
    A.mapP(0, random_double);
    B.mapP(0, random_double);
    
    SUBCASE(\"${op} produces valid results\") {
        Matrix result;
        
")
            
            # Add operation-specific code
            if(op STREQUAL "add")
                file(APPEND ${output_file} 
"        result = A + B;
        CHECK(result.getM() == ROWS);
        CHECK(result.getN() == COLS);
        for(int i = 0; i < ROWS; i++) {
            for(int j = 0; j < COLS; j++) {
                CHECK_MESSAGE(std::abs(result.val(i,j) - (A.val(i,j) + B.val(i,j))) < 1e-10,
                    \"Addition error at (\" << i << \",\" << j << \")\");
            }
        }
")
            elseif(op STREQUAL "multiply")
                file(APPEND ${output_file} 
"        Matrix C(COLS, ROWS);  // For valid multiplication
        C.mapP(0, random_double);
        result = A * C;
        CHECK(result.getM() == ROWS);
        CHECK(result.getN() == ROWS);
        // Verify multiplication correctness
        for(int i = 0; i < ROWS; i++) {
            for(int j = 0; j < ROWS; j++) {
                double expected = 0.0;
                for(int k = 0; k < COLS; k++) {
                    expected += A.val(i,k) * C.val(k,j);
                }
                CHECK_MESSAGE(std::abs(result.val(i,j) - expected) < 1e-10,
                    \"Multiplication error at (\" << i << \",\" << j << \")\");
            }
        }
")
            endif()
            
            file(APPEND ${output_file} 
"    }
}
")
            
            # Add to generated sources
            set(LPZROBOTS_GENERATED_TEST_SOURCES 
                ${LPZROBOTS_GENERATED_TEST_SOURCES} 
                ${output_file} 
                PARENT_SCOPE
            )
        endforeach()
    endforeach()
endfunction()

# Function to generate performance benchmarks
function(lpzrobots_generate_benchmarks component)
    set(benchmark_file "${CMAKE_CURRENT_BINARY_DIR}/benchmarks/${component}_benchmark.cpp")
    file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/benchmarks")
    
    # Determine component type and generate appropriate benchmarks
    if(component MATCHES "controller$")
        set(benchmark_type "controller")
    elseif(component STREQUAL "matrix")
        set(benchmark_type "matrix")
    else()
        set(benchmark_type "generic")
    endif()
    
    file(WRITE ${benchmark_file}
"#include <doctest/doctest.h>
#include <chrono>
#include <vector>
#include <fstream>
#include <random>
#include <numeric>
#include <algorithm>
#include <lpzrobots_test_utils.h>
")
    
    # Add component-specific includes
    if(benchmark_type STREQUAL "controller")
        file(APPEND ${benchmark_file}
"#include <selforg/${component}.h>
#include <selforg/matrix.h>

")
    elseif(benchmark_type STREQUAL "matrix")
        file(APPEND ${benchmark_file}
"#include <selforg/matrix.h>

")
    endif()
    
    file(APPEND ${benchmark_file}
"struct BenchmarkResult {
    std::string name;
    double mean_time_us;
    double std_dev_us;
    double ops_per_second;
    double min_time_us;
    double max_time_us;
};

template<typename Func>
BenchmarkResult measure_performance(const std::string& name, int iterations, Func&& func) {
    using namespace std::chrono;
    
    std::vector<double> timings;
    timings.reserve(iterations);
    
    // Warmup
    for(int i = 0; i < std::min(10, iterations/10); i++) {
        func();
    }
    
    // Actual measurements
    for(int i = 0; i < iterations; i++) {
        auto start = high_resolution_clock::now();
        func();
        auto end = high_resolution_clock::now();
        timings.push_back(duration_cast<nanoseconds>(end - start).count() / 1000.0);
    }
    
    // Calculate statistics
    double mean = std::accumulate(timings.begin(), timings.end(), 0.0) / timings.size();
    double sq_sum = std::inner_product(timings.begin(), timings.end(), timings.begin(), 0.0);
    double std_dev = std::sqrt(sq_sum / timings.size() - mean * mean);
    
    auto [min_it, max_it] = std::minmax_element(timings.begin(), timings.end());
    
    return {
        name,
        mean,
        std_dev,
        1000000.0 / mean,  // ops per second
        *min_it,
        *max_it
    };
}

TEST_CASE(\"${component} Performance Benchmarks\" * doctest::skip(false)) {
    std::vector<BenchmarkResult> results;
    std::mt19937 rng(42);
")
    
    # Generate component-specific benchmarks
    if(benchmark_type STREQUAL "controller")
        file(APPEND ${benchmark_file}
"    
    SUBCASE(\"Controller Step Performance\") {
        const int NUM_SENSORS = 10;
        const int NUM_MOTORS = 8;
        const int ITERATIONS = 10000;
        
        auto conf = ${component}::getDefaultConf();
        ${component} controller(conf);
        controller.init(NUM_SENSORS, NUM_MOTORS);
        
        std::vector<double> sensors(NUM_SENSORS);
        std::vector<double> motors(NUM_MOTORS);
        
        // Random sensor input
        std::uniform_real_distribution<> dist(-1.0, 1.0);
        for(auto& s : sensors) {
            s = dist(rng);
        }
        
        auto result = measure_performance(
            \"${component}_step\",
            ITERATIONS,
            [&]() {
                controller.step(sensors.data(), NUM_SENSORS, motors.data(), NUM_MOTORS);
            }
        );
        
        results.push_back(result);
    }
    
    SUBCASE(\"Learning Performance\") {
        const int NUM_SENSORS = 10;
        const int NUM_MOTORS = 8;
        const int ITERATIONS = 1000;
        
        auto conf = ${component}::getDefaultConf();
        ${component} controller(conf);
        controller.init(NUM_SENSORS, NUM_MOTORS);
        
        // Run some steps to build up history
        std::vector<double> sensors(NUM_SENSORS);
        std::vector<double> motors(NUM_MOTORS);
        
        for(int i = 0; i < 100; i++) {
            std::uniform_real_distribution<> dist(-1.0, 1.0);
            for(auto& s : sensors) {
                s = dist(rng);
            }
            controller.step(sensors.data(), NUM_SENSORS, motors.data(), NUM_MOTORS);
        }
        
        auto result = measure_performance(
            \"${component}_learning\",
            ITERATIONS,
            [&]() {
                controller.stepNoLearning(sensors.data(), NUM_SENSORS, motors.data(), NUM_MOTORS);
                controller.step(sensors.data(), NUM_SENSORS, motors.data(), NUM_MOTORS);
            }
        );
        
        results.push_back(result);
    }
")
    elseif(benchmark_type STREQUAL "matrix")
        file(APPEND ${benchmark_file}
"    
    SUBCASE(\"Matrix Operations\") {
        using namespace matrix;
        
        const std::vector<int> sizes = {2, 4, 8, 16, 32, 64, 128};
        
        for(int size : sizes) {
            Matrix A(size, size);
            Matrix B(size, size);
            A.mapP(0, [](void*, double) { return drand48() * 2.0 - 1.0; });
            B.mapP(0, [](void*, double) { return drand48() * 2.0 - 1.0; });
            
            // Matrix multiplication
            auto mult_result = measure_performance(
                \"matrix_multiply_\" + std::to_string(size) + \"x\" + std::to_string(size),
                1000,
                [&]() {
                    Matrix C = A * B;
                }
            );
            results.push_back(mult_result);
            
            // Matrix addition
            auto add_result = measure_performance(
                \"matrix_add_\" + std::to_string(size) + \"x\" + std::to_string(size),
                10000,
                [&]() {
                    Matrix C = A + B;
                }
            );
            results.push_back(add_result);
            
            // Matrix transpose
            auto trans_result = measure_performance(
                \"matrix_transpose_\" + std::to_string(size) + \"x\" + std::to_string(size),
                10000,
                [&]() {
                    Matrix C = A.transposed();
                }
            );
            results.push_back(trans_result);
            
            // Pseudoinverse (for smaller sizes)
            if(size <= 32) {
                auto pinv_result = measure_performance(
                    \"matrix_pseudoinverse_\" + std::to_string(size) + \"x\" + std::to_string(size),
                    100,
                    [&]() {
                        Matrix C = A.pseudoInverse();
                    }
                );
                results.push_back(pinv_result);
            }
        }
    }
")
    endif()
    
    # Write results to JSON
    file(APPEND ${benchmark_file}
"    
    // Write results to JSON for CI tracking
    std::ofstream out(\"${CMAKE_BINARY_DIR}/benchmarks/${component}_results.json\");
    out << \"{\\n\";
    out << \"  \\\"component\\\": \\\"${component}\\\",\\n\";
    out << \"  \\\"timestamp\\\": \" << std::chrono::system_clock::now().time_since_epoch().count() << \",\\n\";
    out << \"  \\\"system\\\": {\\n\";
    out << \"    \\\"cpu_count\\\": \" << std::thread::hardware_concurrency() << \",\\n\";
    out << \"    \\\"compiler\\\": \\\"\" << CMAKE_CXX_COMPILER_ID << \" \" << CMAKE_CXX_COMPILER_VERSION << \"\\\"\\n\";
    out << \"  },\\n\";
    out << \"  \\\"results\\\": [\\n\";
    
    for(size_t i = 0; i < results.size(); i++) {
        const auto& r = results[i];
        out << \"    {\\n\";
        out << \"      \\\"name\\\": \\\"\" << r.name << \"\\\",\\n\";
        out << \"      \\\"mean_time_us\\\": \" << r.mean_time_us << \",\\n\";
        out << \"      \\\"std_dev_us\\\": \" << r.std_dev_us << \",\\n\";
        out << \"      \\\"ops_per_second\\\": \" << r.ops_per_second << \",\\n\";
        out << \"      \\\"min_time_us\\\": \" << r.min_time_us << \",\\n\";
        out << \"      \\\"max_time_us\\\": \" << r.max_time_us << \"\\n\";
        out << \"    }\" << (i < results.size() - 1 ? \",\" : \"\") << \"\\n\";
    }
    
    out << \"  ]\\n\";
    out << \"}\\n\";
    out.close();
    
    // Print summary
    MESSAGE(\"${component} Benchmark Results:\");
    for(const auto& r : results) {
        MESSAGE(\"  \" << r.name << \": \" 
                << r.mean_time_us << \" ± \" << r.std_dev_us << \" µs, \"
                << r.ops_per_second << \" ops/sec\");
    }
}
")
    
    set(LPZROBOTS_BENCHMARK_SOURCES 
        ${LPZROBOTS_BENCHMARK_SOURCES} 
        ${benchmark_file} 
        PARENT_SCOPE
    )
endfunction()

# Function to generate regression tests from recorded data
function(lpzrobots_generate_regression_test name data_file)
    set(output_file "${CMAKE_CURRENT_BINARY_DIR}/tests/regression/${name}_regression_test.cpp")
    file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/tests/regression")
    
    # Parse data file to extract test cases
    file(READ ${data_file} test_data)
    
    file(WRITE ${output_file}
"#include <doctest/doctest.h>
#include <fstream>
#include <vector>
#include <lpzrobots_test_utils.h>

TEST_CASE(\"${name} Regression Test\") {
    // Load baseline data
    std::ifstream baseline(\"${data_file}\");
    REQUIRE(baseline.is_open());
    
    // TODO: Implement regression test based on data
    
    baseline.close();
}
")
    
    set(LPZROBOTS_REGRESSION_TEST_SOURCES 
        ${LPZROBOTS_REGRESSION_TEST_SOURCES} 
        ${output_file} 
        PARENT_SCOPE
    )
endfunction()

# Function to generate property-based tests
function(lpzrobots_generate_property_tests component)
    set(property_test_file "${CMAKE_CURRENT_BINARY_DIR}/tests/property/${component}_property_test.cpp")
    file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/tests/property")
    
    file(WRITE ${property_test_file}
"#include <doctest/doctest.h>
#include <random>
#include <functional>
#include <lpzrobots_test_utils.h>

// Simple property-based testing framework
template<typename T>
class PropertyTest {
private:
    std::mt19937 rng{std::random_device{}()};
    
public:
    using Generator = std::function<T()>;
    using Property = std::function<bool(const T&)>;
    
    bool check(Generator gen, Property prop, int num_tests = 100) {
        for(int i = 0; i < num_tests; i++) {
            T value = gen();
            if(!prop(value)) {
                INFO(\"Property failed at iteration \" << i);
                return false;
            }
        }
        return true;
    }
    
    // Generators for common types
    auto gen_double(double min = -1.0, double max = 1.0) {
        return [this, min, max]() {
            std::uniform_real_distribution<> dist(min, max);
            return dist(rng);
        };
    }
    
    auto gen_int(int min = 0, int max = 100) {
        return [this, min, max]() {
            std::uniform_int_distribution<> dist(min, max);
            return dist(rng);
        };
    }
    
    auto gen_vector(int size, double min = -1.0, double max = 1.0) {
        return [this, size, min, max]() {
            std::vector<double> vec(size);
            std::uniform_real_distribution<> dist(min, max);
            for(auto& v : vec) {
                v = dist(rng);
            }
            return vec;
        };
    }
};

")
    
    # Add component-specific property tests
    if(component MATCHES "controller$")
        file(APPEND ${property_test_file}
"#include <selforg/${component}.h>

TEST_CASE(\"${component} Property-Based Tests\") {
    PropertyTest<std::vector<double>> prop_test;
    
    SUBCASE(\"Motor outputs bounded\") {
        auto conf = ${component}::getDefaultConf();
        ${component} controller(conf);
        
        auto gen_sensors = prop_test.gen_vector(10, -10.0, 10.0);
        
        auto property = [&controller](const std::vector<double>& sensors) {
            std::vector<double> motors(8);
            controller.step(sensors.data(), sensors.size(), motors.data(), motors.size());
            
            // Check all motors are bounded
            for(double m : motors) {
                if(std::abs(m) > 1.0 || !std::isfinite(m)) {
                    return false;
                }
            }
            return true;
        };
        
        CHECK(prop_test.check(gen_sensors, property, 1000));
    }
    
    SUBCASE(\"Stability under perturbation\") {
        auto conf = ${component}::getDefaultConf();
        ${component} controller(conf);
        controller.init(5, 3);
        
        PropertyTest<double> epsilon_test;
        auto gen_epsilon = epsilon_test.gen_double(0.0, 0.1);
        
        auto property = [&controller, &epsilon_test](double epsilon) {
            std::vector<double> sensors1 = {0.5, -0.3, 0.7, 0.1, -0.9};
            std::vector<double> sensors2 = sensors1;
            
            // Add small perturbation
            for(auto& s : sensors2) {
                s += epsilon * (epsilon_test.gen_double(-1.0, 1.0)());
            }
            
            std::vector<double> motors1(3), motors2(3);
            controller.step(sensors1.data(), 5, motors1.data(), 3);
            controller.step(sensors2.data(), 5, motors2.data(), 3);
            
            // Check outputs are close
            double diff = 0.0;
            for(int i = 0; i < 3; i++) {
                diff += std::abs(motors1[i] - motors2[i]);
            }
            
            // Small input change should produce small output change
            return diff < epsilon * 10.0;  // Allow 10x amplification
        };
        
        CHECK(prop_test.check(gen_epsilon, property, 100));
    }
}
")
    elseif(component STREQUAL "matrix")
        file(APPEND ${property_test_file}
"#include <selforg/matrix.h>

TEST_CASE(\"Matrix Property-Based Tests\") {
    using namespace matrix;
    PropertyTest<int> size_test;
    
    SUBCASE(\"Matrix multiplication associativity\") {
        auto gen_size = size_test.gen_int(2, 10);
        
        auto property = [](int n) {
            Matrix A(n, n), B(n, n), C(n, n);
            A.mapP(0, [](void*, double) { return drand48() * 2.0 - 1.0; });
            B.mapP(0, [](void*, double) { return drand48() * 2.0 - 1.0; });
            C.mapP(0, [](void*, double) { return drand48() * 2.0 - 1.0; });
            
            // (A*B)*C should equal A*(B*C)
            Matrix left = (A * B) * C;
            Matrix right = A * (B * C);
            
            return (left - right).norm_sqr() < 1e-10;
        };
        
        CHECK(size_test.check(gen_size, property, 50));
    }
    
    SUBCASE(\"Transpose involution\") {
        auto property = [](int n) {
            Matrix A(n, n+1);
            A.mapP(0, [](void*, double) { return drand48() * 2.0 - 1.0; });
            
            // (A^T)^T should equal A
            Matrix ATT = A.transposed().transposed();
            return (A - ATT).norm_sqr() < 1e-10;
        };
        
        CHECK(size_test.check(size_test.gen_int(1, 20), property, 100));
    }
}
")
    endif()
    
    file(APPEND ${property_test_file} "\n")
    
    set(LPZROBOTS_PROPERTY_TEST_SOURCES 
        ${LPZROBOTS_PROPERTY_TEST_SOURCES} 
        ${property_test_file} 
        PARENT_SCOPE
    )
endfunction()

# Function to generate fuzz tests
function(lpzrobots_generate_fuzz_tests component)
    set(fuzz_test_file "${CMAKE_CURRENT_BINARY_DIR}/tests/fuzz/${component}_fuzz_test.cpp")
    file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/tests/fuzz")
    
    file(WRITE ${fuzz_test_file}
"#include <cstdint>
#include <cstddef>
#include <vector>
#include <algorithm>

// Fuzzing entry point
extern \"C\" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
")
    
    if(component MATCHES "controller$")
        file(APPEND ${fuzz_test_file}
"    
    #include <selforg/${component}.h>
    
    // Minimum size for meaningful input
    if(size < 32) return 0;
    
    try {
        // Extract configuration from fuzz data
        size_t offset = 0;
        int num_sensors = std::min(100, static_cast<int>(data[offset++] % 20 + 1));
        int num_motors = std::min(100, static_cast<int>(data[offset++] % 20 + 1));
        
        // Create controller with fuzzed config
        auto conf = ${component}::getDefaultConf();
        
        // Fuzz some parameters
        if(offset + sizeof(double) <= size) {
            conf.eps = *reinterpret_cast<const double*>(data + offset);
            conf.eps = std::max(0.0, std::min(1.0, conf.eps));
            offset += sizeof(double);
        }
        
        ${component} controller(conf);
        controller.init(num_sensors, num_motors);
        
        // Run with fuzzed sensor data
        std::vector<double> sensors(num_sensors);
        std::vector<double> motors(num_motors);
        
        // Extract sensor values from fuzz data
        for(int i = 0; i < num_sensors && offset < size; i++) {
            sensors[i] = static_cast<double>(static_cast<int8_t>(data[offset++])) / 127.0;
        }
        
        // Run controller for multiple steps
        for(int step = 0; step < 10 && offset < size; step++) {
            controller.step(sensors.data(), num_sensors, motors.data(), num_motors);
            
            // Mutate sensors based on fuzz data
            if(offset < size) {
                int idx = data[offset++] % num_sensors;
                sensors[idx] = static_cast<double>(static_cast<int8_t>(data[offset++])) / 127.0;
            }
        }
        
    } catch(...) {
        // Catch any exceptions - we're looking for crashes
    }
")
    elseif(component STREQUAL "matrix")
        file(APPEND ${fuzz_test_file}
"    
    #include <selforg/matrix.h>
    using namespace matrix;
    
    if(size < 4) return 0;
    
    try {
        size_t offset = 0;
        int rows = (data[offset++] % 50) + 1;
        int cols = (data[offset++] % 50) + 1;
        
        Matrix A(rows, cols);
        
        // Fill matrix with fuzzed data
        for(int i = 0; i < rows && offset < size; i++) {
            for(int j = 0; j < cols && offset < size; j++) {
                A.val(i, j) = static_cast<double>(static_cast<int8_t>(data[offset++])) / 10.0;
            }
        }
        
        // Perform operations that might crash
        Matrix B = A.transposed();
        Matrix C = A + A;
        
        if(rows == cols && offset < size) {
            // Square matrix operations
            Matrix D = A * A;
            if(data[offset++] % 2 == 0) {
                Matrix E = A.pseudoInverse();
            }
        }
        
        // Test boundary conditions
        A.val(0, 0) = 1e100;
        A.val(rows-1, cols-1) = -1e100;
        
        Matrix F = A.multMT();  // A * A^T
        
    } catch(...) {
        // Expected for some inputs
    }
")
    endif()
    
    file(APPEND ${fuzz_test_file}
"    
    return 0;
}

// Standalone harness for testing without libFuzzer
#ifndef FUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION
#include <iostream>
#include <fstream>

int main(int argc, char** argv) {
    if(argc != 2) {
        std::cerr << \"Usage: \" << argv[0] << \" <input_file>\\n\";
        return 1;
    }
    
    std::ifstream file(argv[1], std::ios::binary);
    if(!file) {
        std::cerr << \"Failed to open input file\\n\";
        return 1;
    }
    
    file.seekg(0, std::ios::end);
    size_t size = file.tellg();
    file.seekg(0, std::ios::beg);
    
    std::vector<uint8_t> data(size);
    file.read(reinterpret_cast<char*>(data.data()), size);
    
    return LLVMFuzzerTestOneInput(data.data(), size);
}
#endif
")
    
    set(LPZROBOTS_FUZZ_TEST_SOURCES 
        ${LPZROBOTS_FUZZ_TEST_SOURCES} 
        ${fuzz_test_file} 
        PARENT_SCOPE
    )
endfunction()

# Function to generate mutation tests (test quality verification)
function(lpzrobots_generate_mutation_tests component)
    set(mutation_config "${CMAKE_CURRENT_BINARY_DIR}/tests/mutation/${component}_mutations.yaml")
    file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/tests/mutation")
    
    # Generate mutation configuration
    file(WRITE ${mutation_config}
"# Mutation testing configuration for ${component}
mutations:
  - type: arithmetic_operator
    operators: ['+', '-', '*', '/']
    target_files: 
      - 'selforg/controller/${component}.cpp'
      
  - type: comparison_operator  
    operators: ['<', '>', '<=', '>=', '==', '!=']
    target_files:
      - 'selforg/controller/${component}.cpp'
      
  - type: constant_replacement
    replacements:
      - {from: 0.0, to: [1.0, -1.0, 0.1]}
      - {from: 1.0, to: [0.0, 2.0, 0.5]}
      
  - type: statement_deletion
    exclude_patterns:
      - 'assert.*'
      - 'CHECK.*'
      
test_command: 'ctest -R ${component}_test --output-on-failure'
timeout: 30
parallel_runs: 4
")
    
    # Generate mutation test runner
    set(mutation_runner "${CMAKE_CURRENT_BINARY_DIR}/tests/mutation/run_mutation_${component}.sh")
    file(WRITE ${mutation_runner}
"#!/bin/bash
# Mutation testing runner for ${component}

COMPONENT=${component}
BUILD_DIR=${CMAKE_BINARY_DIR}
SOURCE_DIR=${CMAKE_SOURCE_DIR}

echo \"Running mutation tests for $COMPONENT...\"

# Check if mutate++ is available
if ! command -v mutate++ &> /dev/null; then
    echo \"mutate++ not found. Install from: https://github.com/nlohmann/mutate_cpp\"
    exit 1
fi

# Run mutation testing
mutate++ \\
    --config ${mutation_config} \\
    --build-dir \"$BUILD_DIR\" \\
    --source-dir \"$SOURCE_DIR\" \\
    --report mutation_report_${component}.html

echo \"Mutation testing complete. Report: mutation_report_${component}.html\"
")
    
    file(CHMOD ${mutation_runner} 
        PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE 
                    GROUP_READ GROUP_EXECUTE 
                    WORLD_READ WORLD_EXECUTE)
endfunction()

# Macro to generate all tests for a component
macro(lpzrobots_generate_all_tests component)
    # Generate standard tests
    lpzrobots_generate_controller_tests(${component})
    
    # Generate benchmarks
    lpzrobots_generate_benchmarks(${component})
    
    # Generate property-based tests
    lpzrobots_generate_property_tests(${component})
    
    # Generate fuzz tests
    if(LPZROBOTS_ENABLE_FUZZING)
        lpzrobots_generate_fuzz_tests(${component})
    endif()
    
    # Generate mutation tests
    if(LPZROBOTS_ENABLE_MUTATION_TESTING)
        lpzrobots_generate_mutation_tests(${component})
    endif()
    
    # Generate regression tests if data exists
    if(EXISTS "${CMAKE_SOURCE_DIR}/tests/regression_data/${component}.json")
        lpzrobots_generate_regression_test(${component} 
            "${CMAKE_SOURCE_DIR}/tests/regression_data/${component}.json")
    endif()
    
    # Add all generated sources to test executable
    if(LPZROBOTS_GENERATED_TEST_SOURCES OR LPZROBOTS_PROPERTY_TEST_SOURCES)
        lpzrobots_add_test_executable(${component}_generated_tests
            UNIT
            SOURCES 
                ${LPZROBOTS_GENERATED_TEST_SOURCES}
                ${LPZROBOTS_PROPERTY_TEST_SOURCES}
            DEPS lpzrobots::${component}
            LABELS generated ${component} property
        )
    endif()
    
    # Add benchmark executable
    if(LPZROBOTS_BENCHMARK_SOURCES)
        lpzrobots_add_test_executable(${component}_benchmark
            PERFORMANCE
            SOURCES ${LPZROBOTS_BENCHMARK_SOURCES}
            DEPS lpzrobots::${component}
            LABELS benchmark ${component}
        )
    endif()
    
    # Add fuzz test executable
    if(LPZROBOTS_FUZZ_TEST_SOURCES AND LPZROBOTS_ENABLE_FUZZING)
        add_executable(${component}_fuzz ${LPZROBOTS_FUZZ_TEST_SOURCES})
        target_link_libraries(${component}_fuzz PRIVATE lpzrobots::${component})
        
        if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
            target_compile_options(${component}_fuzz PRIVATE -fsanitize=fuzzer,address)
            target_link_options(${component}_fuzz PRIVATE -fsanitize=fuzzer,address)
        endif()
    endif()
endmacro()