# AutoTestDiscovery.cmake
# Automatic test discovery and generation based on code patterns

include_guard(GLOBAL)

# Function to analyze source files and discover testable patterns
function(lpzrobots_auto_discover_tests)
    set(options VERBOSE)
    set(oneValueArgs TARGET SOURCE_DIR OUTPUT_DIR)
    set(multiValueArgs PATTERNS EXCLUDE_PATTERNS)
    cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
    
    if(NOT ARG_SOURCE_DIR)
        set(ARG_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
    endif()
    
    if(NOT ARG_OUTPUT_DIR)
        set(ARG_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/auto_tests")
    endif()
    
    file(MAKE_DIRECTORY "${ARG_OUTPUT_DIR}")
    
    # Default patterns to look for
    if(NOT ARG_PATTERNS)
        set(ARG_PATTERNS
            "class.*:.*public.*Controller"     # Controller classes
            "class.*:.*public.*Robot"          # Robot classes
            "class.*:.*public.*Wiring"         # Wiring classes
            "static.*g\\("                     # Static activation functions
            "Matrix.*operator"                 # Matrix operators
            "void.*step\\("                    # Step functions
            "void.*init\\("                    # Init functions
        )
    endif()
    
    # Find all source files
    file(GLOB_RECURSE SOURCE_FILES 
        "${ARG_SOURCE_DIR}/*.cpp"
        "${ARG_SOURCE_DIR}/*.h"
        "${ARG_SOURCE_DIR}/*.hpp"
    )
    
    # Exclude patterns
    if(ARG_EXCLUDE_PATTERNS)
        foreach(pattern IN LISTS ARG_EXCLUDE_PATTERNS)
            list(FILTER SOURCE_FILES EXCLUDE REGEX "${pattern}")
        endforeach()
    endif()
    
    # Analyze each file
    set(DISCOVERED_TESTS "")
    foreach(source_file IN LISTS SOURCE_FILES)
        lpzrobots_analyze_source_file("${source_file}" discovered_items)
        list(APPEND DISCOVERED_TESTS ${discovered_items})
    endforeach()
    
    # Generate test files for discovered items
    foreach(test_item IN LISTS DISCOVERED_TESTS)
        lpzrobots_generate_auto_test("${test_item}" "${ARG_OUTPUT_DIR}")
    endforeach()
    
    if(ARG_VERBOSE)
        message(STATUS "Auto-discovered ${list(LENGTH DISCOVERED_TESTS)} test candidates")
    endif()
endfunction()

# Function to analyze a single source file
function(lpzrobots_analyze_source_file source_file output_var)
    file(READ "${source_file}" file_content)
    get_filename_component(file_name "${source_file}" NAME_WE)
    
    set(discovered_items "")
    
    # Pattern 1: Find controller classes
    string(REGEX MATCHALL "class[ \t]+([A-Za-z0-9_]+)[ \t]*:[ \t]*public[ \t]+[A-Za-z]*Controller" 
           controller_matches "${file_content}")
    foreach(match IN LISTS controller_matches)
        string(REGEX REPLACE "class[ \t]+([A-Za-z0-9_]+).*" "\\1" class_name "${match}")
        list(APPEND discovered_items "CONTROLLER:${class_name}:${source_file}")
    endforeach()
    
    # Pattern 2: Find matrix operations
    string(REGEX MATCHALL "Matrix[ \t]+operator[+\\-*/][ \t]*\\(" 
           matrix_op_matches "${file_content}")
    if(matrix_op_matches)
        list(APPEND discovered_items "MATRIX_OPS:${file_name}:${source_file}")
    endif()
    
    # Pattern 3: Find numerical functions
    string(REGEX MATCHALL "(double|float)[ \t]+[a-zA-Z_][a-zA-Z0-9_]*[ \t]*\\([^)]*\\)[ \t]*{[^}]*return[^}]*}" 
           numeric_func_matches "${file_content}")
    foreach(match IN LISTS numeric_func_matches)
        if(match MATCHES "([a-zA-Z_][a-zA-Z0-9_]*)[ \t]*\\(")
            set(func_name "${CMAKE_MATCH_1}")
            list(APPEND discovered_items "NUMERIC_FUNC:${func_name}:${source_file}")
        endif()
    endforeach()
    
    # Pattern 4: Find parameter structures
    string(REGEX MATCHALL "struct[ \t]+[A-Za-z0-9_]*Conf[ \t]*{" 
           conf_matches "${file_content}")
    if(conf_matches)
        list(APPEND discovered_items "CONFIG_STRUCT:${file_name}:${source_file}")
    endif()
    
    # Pattern 5: Find factory methods
    string(REGEX MATCHALL "static[ \t]+[A-Za-z0-9_*]+[ \t]+create[A-Za-z0-9_]*[ \t]*\\(" 
           factory_matches "${file_content}")
    if(factory_matches)
        list(APPEND discovered_items "FACTORY_METHOD:${file_name}:${source_file}")
    endif()
    
    set(${output_var} "${discovered_items}" PARENT_SCOPE)
endfunction()

# Function to generate test based on discovered pattern
function(lpzrobots_generate_auto_test test_item output_dir)
    # Parse test item: TYPE:NAME:SOURCE_FILE
    string(REPLACE ":" ";" item_parts "${test_item}")
    list(GET item_parts 0 test_type)
    list(GET item_parts 1 item_name)
    list(GET item_parts 2 source_file)
    
    # Generate appropriate test based on type
    if(test_type STREQUAL "CONTROLLER")
        lpzrobots_generate_controller_auto_test("${item_name}" "${source_file}" "${output_dir}")
    elseif(test_type STREQUAL "MATRIX_OPS")
        lpzrobots_generate_matrix_ops_auto_test("${item_name}" "${source_file}" "${output_dir}")
    elseif(test_type STREQUAL "NUMERIC_FUNC")
        lpzrobots_generate_numeric_func_auto_test("${item_name}" "${source_file}" "${output_dir}")
    elseif(test_type STREQUAL "CONFIG_STRUCT")
        lpzrobots_generate_config_auto_test("${item_name}" "${source_file}" "${output_dir}")
    elseif(test_type STREQUAL "FACTORY_METHOD")
        lpzrobots_generate_factory_auto_test("${item_name}" "${source_file}" "${output_dir}")
    endif()
endfunction()

# Generate controller test automatically
function(lpzrobots_generate_controller_auto_test class_name source_file output_dir)
    set(test_file "${output_dir}/auto_test_${class_name}.cpp")
    
    # Extract header file path
    get_filename_component(source_dir "${source_file}" DIRECTORY)
    get_filename_component(source_name "${source_file}" NAME_WE)
    
    # Determine include path
    string(FIND "${source_file}" "selforg" selforg_pos)
    if(selforg_pos GREATER -1)
        string(SUBSTRING "${source_file}" ${selforg_pos} -1 include_path)
        string(REGEX REPLACE "\\.cpp$" ".h" include_path "${include_path}")
    else()
        set(include_path "${source_name}.h")
    endif()
    
    file(WRITE "${test_file}"
"// Auto-generated test for ${class_name}
#include <doctest/doctest.h>
#include <${include_path}>
#include <memory>
#include <vector>

TEST_SUITE(\"Auto-generated ${class_name} Tests\") {
    TEST_CASE(\"${class_name} - Basic Construction\") {
        // Test default construction
        SUBCASE(\"Default configuration\") {
            auto conf = ${class_name}::getDefaultConf();
            ${class_name} controller(conf);
            
            CHECK(controller.getName() != nullptr);
        }
    }
    
    TEST_CASE(\"${class_name} - Initialization\") {
        auto conf = ${class_name}::getDefaultConf();
        ${class_name} controller(conf);
        
        SUBCASE(\"Valid sensor/motor numbers\") {
            controller.init(5, 3);
            CHECK(controller.getSensorNumber() == 5);
            CHECK(controller.getMotorNumber() == 3);
        }
        
        SUBCASE(\"Edge cases\") {
            // Test with minimal sensors/motors
            controller.init(1, 1);
            CHECK(controller.getSensorNumber() == 1);
            CHECK(controller.getMotorNumber() == 1);
            
            // Test with many sensors/motors
            controller.init(20, 20);
            CHECK(controller.getSensorNumber() == 20);
            CHECK(controller.getMotorNumber() == 20);
        }
    }
    
    TEST_CASE(\"${class_name} - Step Function\") {
        auto conf = ${class_name}::getDefaultConf();
        ${class_name} controller(conf);
        controller.init(3, 2);
        
        SUBCASE(\"Normal operation\") {
            double sensors[3] = {0.5, -0.3, 0.8};
            double motors[2] = {0.0, 0.0};
            
            controller.step(sensors, 3, motors, 2);
            
            // Check motors are bounded
            for(int i = 0; i < 2; i++) {
                CHECK(motors[i] >= -1.0);
                CHECK(motors[i] <= 1.0);
                CHECK(std::isfinite(motors[i]));
            }
        }
        
        SUBCASE(\"Extreme inputs\") {
            double sensors[3] = {1.0, -1.0, 0.0};
            double motors[2] = {0.0, 0.0};
            
            controller.step(sensors, 3, motors, 2);
            
            // Should handle extreme inputs gracefully
            for(int i = 0; i < 2; i++) {
                CHECK(std::isfinite(motors[i]));
            }
        }
    }
    
    TEST_CASE(\"${class_name} - Parameter Handling\") {
        auto conf = ${class_name}::getDefaultConf();
        ${class_name} controller(conf);
        controller.init(2, 2);
        
        SUBCASE(\"Get parameter list\") {
            auto params = controller.getParamList();
            CHECK(!params.empty());
            
            // Verify all parameters have valid values
            for(const auto& [name, value] : params) {
                CHECK(value != nullptr);
                CHECK(std::isfinite(*value));
            }
        }
        
        SUBCASE(\"Parameter modification\") {
            auto params = controller.getParamList();
            if(!params.empty()) {
                auto& [name, value] = *params.begin();
                double original = *value;
                
                // Modify parameter
                *value = original * 1.5;
                
                // Verify change persists
                auto params2 = controller.getParamList();
                auto it = std::find_if(params2.begin(), params2.end(),
                    [&name](const auto& p) { return p.first == name; });
                
                if(it != params2.end()) {
                    CHECK(*it->second == original * 1.5);
                }
            }
        }
    }
}
")
    
    # Add to list of generated tests
    set(LPZROBOTS_AUTO_GENERATED_TESTS 
        ${LPZROBOTS_AUTO_GENERATED_TESTS} 
        "${test_file}" 
        PARENT_SCOPE
    )
endfunction()

# Generate matrix operations test
function(lpzrobots_generate_matrix_ops_auto_test item_name source_file output_dir)
    set(test_file "${output_dir}/auto_test_matrix_ops_${item_name}.cpp")
    
    file(WRITE "${test_file}"
"// Auto-generated matrix operations test
#include <doctest/doctest.h>
#include <selforg/matrix.h>
#include <random>

TEST_SUITE(\"Auto-generated Matrix Operations Tests\") {
    using namespace matrix;
    
    TEST_CASE(\"Matrix operations - Arithmetic\") {
        std::mt19937 rng(42);
        std::uniform_real_distribution<> dist(-1.0, 1.0);
        
        SUBCASE(\"Addition commutativity\") {
            Matrix A(4, 4);
            Matrix B(4, 4);
            
            A.mapP(&rng, [&dist](void* r, double) { return dist(*static_cast<std::mt19937*>(r)); });
            B.mapP(&rng, [&dist](void* r, double) { return dist(*static_cast<std::mt19937*>(r)); });
            
            Matrix AB = A + B;
            Matrix BA = B + A;
            
            CHECK((AB - BA).norm_sqr() < 1e-10);
        }
        
        SUBCASE(\"Multiplication associativity\") {
            Matrix A(3, 4);
            Matrix B(4, 5);
            Matrix C(5, 3);
            
            A.mapP(&rng, [&dist](void* r, double) { return dist(*static_cast<std::mt19937*>(r)); });
            B.mapP(&rng, [&dist](void* r, double) { return dist(*static_cast<std::mt19937*>(r)); });
            C.mapP(&rng, [&dist](void* r, double) { return dist(*static_cast<std::mt19937*>(r)); });
            
            Matrix AB_C = (A * B) * C;
            Matrix A_BC = A * (B * C);
            
            CHECK(AB_C.getM() == 3);
            CHECK(AB_C.getN() == 3);
            CHECK((AB_C - A_BC).norm_sqr() < 1e-10);
        }
        
        SUBCASE(\"Identity multiplication\") {
            Matrix A(5, 5);
            A.mapP(&rng, [&dist](void* r, double) { return dist(*static_cast<std::mt19937*>(r)); });
            
            Matrix I = Matrix::eye(5);
            Matrix AI = A * I;
            Matrix IA = I * A;
            
            CHECK((A - AI).norm_sqr() < 1e-10);
            CHECK((A - IA).norm_sqr() < 1e-10);
        }
    }
    
    TEST_CASE(\"Matrix operations - Edge cases\") {
        SUBCASE(\"Empty matrix operations\") {
            Matrix A(0, 0);
            Matrix B(0, 0);
            
            Matrix C = A + B;
            CHECK(C.getM() == 0);
            CHECK(C.getN() == 0);
        }
        
        SUBCASE(\"Single element operations\") {
            Matrix A(1, 1);
            A.val(0, 0) = 5.0;
            
            Matrix B(1, 1);
            B.val(0, 0) = 3.0;
            
            Matrix C = A * B;
            CHECK(C.val(0, 0) == 15.0);
        }
    }
}
")
    
    set(LPZROBOTS_AUTO_GENERATED_TESTS 
        ${LPZROBOTS_AUTO_GENERATED_TESTS} 
        "${test_file}" 
        PARENT_SCOPE
    )
endfunction()

# Generate numeric function test
function(lpzrobots_generate_numeric_func_auto_test func_name source_file output_dir)
    set(test_file "${output_dir}/auto_test_numeric_${func_name}.cpp")
    
    file(WRITE "${test_file}"
"// Auto-generated test for numeric function ${func_name}
#include <doctest/doctest.h>
#include <cmath>
#include <limits>

// Forward declaration - actual function should be included
extern double ${func_name}(double x);

TEST_SUITE(\"Auto-generated Numeric Function Tests\") {
    TEST_CASE(\"${func_name} - Properties\") {
        SUBCASE(\"Domain tests\") {
            // Test common values
            CHECK(std::isfinite(${func_name}(0.0)));
            CHECK(std::isfinite(${func_name}(1.0)));
            CHECK(std::isfinite(${func_name}(-1.0)));
            
            // Test extremes
            CHECK(std::isfinite(${func_name}(1e-10)));
            CHECK(std::isfinite(${func_name}(1e10)));
        }
        
        SUBCASE(\"Special values\") {
            // Test how function handles special values
            auto nan_result = ${func_name}(std::numeric_limits<double>::quiet_NaN());
            auto inf_result = ${func_name}(std::numeric_limits<double>::infinity());
            auto neg_inf_result = ${func_name}(-std::numeric_limits<double>::infinity());
            
            // Function should handle these gracefully (either return finite or propagate)
            MESSAGE(\"NaN handling: \" << nan_result);
            MESSAGE(\"Inf handling: \" << inf_result);
            MESSAGE(\"Neg Inf handling: \" << neg_inf_result);
        }
        
        SUBCASE(\"Monotonicity check\") {
            // Check if function is monotonic over a range
            bool increasing = true;
            bool decreasing = true;
            
            double prev = ${func_name}(-10.0);
            for(double x = -9.9; x <= 10.0; x += 0.1) {
                double curr = ${func_name}(x);
                if(curr < prev) increasing = false;
                if(curr > prev) decreasing = false;
                prev = curr;
            }
            
            MESSAGE(\"Function is \" << (increasing ? \"increasing\" : 
                                      decreasing ? \"decreasing\" : \"non-monotonic\"));
        }
    }
}
")
    
    set(LPZROBOTS_AUTO_GENERATED_TESTS 
        ${LPZROBOTS_AUTO_GENERATED_TESTS} 
        "${test_file}" 
        PARENT_SCOPE
    )
endfunction()

# Function to create test suites from discovered tests
function(lpzrobots_create_auto_test_suites)
    if(NOT LPZROBOTS_AUTO_GENERATED_TESTS)
        return()
    endif()
    
    # Group tests by type
    set(controller_tests "")
    set(matrix_tests "")
    set(numeric_tests "")
    set(other_tests "")
    
    foreach(test_file IN LISTS LPZROBOTS_AUTO_GENERATED_TESTS)
        get_filename_component(test_name "${test_file}" NAME_WE)
        
        if(test_name MATCHES "controller")
            list(APPEND controller_tests "${test_file}")
        elseif(test_name MATCHES "matrix")
            list(APPEND matrix_tests "${test_file}")
        elseif(test_name MATCHES "numeric")
            list(APPEND numeric_tests "${test_file}")
        else()
            list(APPEND other_tests "${test_file}")
        endif()
    endforeach()
    
    # Create test executables
    if(controller_tests)
        lpzrobots_add_test_executable(auto_controller_tests
            UNIT
            SOURCES ${controller_tests}
            DEPS lpzrobots::selforg
            LABELS auto-generated controller
        )
    endif()
    
    if(matrix_tests)
        lpzrobots_add_test_executable(auto_matrix_tests
            UNIT
            SOURCES ${matrix_tests}
            DEPS lpzrobots::selforg
            LABELS auto-generated matrix
        )
    endif()
    
    if(numeric_tests)
        lpzrobots_add_test_executable(auto_numeric_tests
            UNIT
            SOURCES ${numeric_tests}
            DEPS lpzrobots::selforg
            LABELS auto-generated numeric
        )
    endif()
endfunction()

# Macro to enable automatic test discovery for a component
macro(lpzrobots_enable_auto_test_discovery component)
    lpzrobots_auto_discover_tests(
        TARGET ${component}
        SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}"
        OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/auto_tests"
        VERBOSE
    )
    
    lpzrobots_create_auto_test_suites()
endmacro()