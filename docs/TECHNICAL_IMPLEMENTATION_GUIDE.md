# LPZRobots Technical Implementation Guide

---
**Document Type**: Guide  
**Last Updated**: 2025-01-06  
**Status**: Current  
**Version**: 1.0  
**Component(s)**: All  
**Tags**: implementation, technical-guide, best-practices, development  
---

## Introduction

This guide provides detailed technical implementation guidance for developers working with the modernized LPZRobots codebase, covering best practices, patterns, and practical examples.

## 1. Modern C++ Implementation Guidelines

### Smart Pointer Usage Patterns

**Factory Pattern Implementation:**
```cpp
// Controller factory with proper ownership semantics
class ControllerFactory {
public:
    // Factory returns unique ownership
    static std::unique_ptr<AbstractController> create(const std::string& type) {
        if (type == "Sox") {
            return std::make_unique<Sox>();
        } else if (type == "APEXSox") {
            auto controller = std::make_unique<APEXSox>();
            // Configure with default parameters
            controller->setParam("learningRate", 0.1);
            return controller;
        }
        throw std::invalid_argument("Unknown controller type: " + type);
    }
    
    // Alternative: with configuration
    static std::unique_ptr<AbstractController> create(
        const std::string& type, 
        const Configuration& config) {
        auto controller = create(type);
        controller->configure(config);
        return controller;
    }
};
```

**Shared Resource Management:**
```cpp
// Shared configuration data
class SimulationConfig {
    std::shared_ptr<PhysicsSettings> physics;
    std::shared_ptr<RenderSettings> render;
    
public:
    // Safe sharing of configuration
    std::shared_ptr<PhysicsSettings> getPhysics() const { 
        return physics; 
    }
    
    // Modification through copy-on-write
    void updatePhysics(const PhysicsSettings& newSettings) {
        physics = std::make_shared<PhysicsSettings>(newSettings);
    }
};
```

### RAII and Exception Safety

**Resource Management Pattern:**
```cpp
class ScopedSimulation {
    OdeHandle odeHandle;
    OsgHandle osgHandle;
    std::vector<std::unique_ptr<OdeAgent>> agents;
    bool initialized = false;
    
public:
    ScopedSimulation() {
        // Initialize physics
        odeHandle = OdeHandle::create();
        
        // Initialize graphics
        osgHandle = OsgHandle::create();
        
        initialized = true;
    }
    
    ~ScopedSimulation() {
        // Automatic cleanup in reverse order
        agents.clear();  // Agents destroyed first
        // Handles cleaned up automatically
    }
    
    // Strong exception guarantee
    void addAgent(std::unique_ptr<OdeAgent> agent) {
        if (!initialized) {
            throw std::runtime_error("Simulation not initialized");
        }
        agents.push_back(std::move(agent));
    }
};
```

### Modern Type Aliases and Templates

**Type-Safe Matrix Operations:**
```cpp
namespace lpzrobots {

// Modern type aliases
using Sensor = double;
using Motor = double;
using SensorArray = std::vector<Sensor>;
using MotorArray = std::vector<Motor>;

// Template aliases for specific matrix types
template<int Rows, int Cols>
using FixedMatrix = Matrix<double, Rows, Cols>;

using SensorMatrix = FixedMatrix<Dynamic, 1>;
using MotorMatrix = FixedMatrix<Dynamic, 1>;

// Concept for controller requirements (C++20)
template<typename T>
concept Controller = requires(T t, const Sensor* s, Motor* m) {
    { t.init(10, 5) } -> std::same_as<void>;
    { t.step(s, 10, m, 5) } -> std::same_as<void>;
    { t.getParam("eps") } -> std::convertible_to<double>;
};

}
```

## 2. Performance Optimization Techniques

### SIMD Implementation Guide

**Platform-Specific Optimization:**
```cpp
class MatrixOperations {
public:
    // Dispatch to platform-specific implementation
    static void multiply(Matrix& result, const Matrix& a, const Matrix& b) {
        #ifdef USE_NEON
            multiply_neon(result, a, b);
        #elif defined(USE_AVX2)
            multiply_avx2(result, a, b);
        #else
            multiply_scalar(result, a, b);
        #endif
    }
    
private:
    // ARM64 NEON implementation
    static void multiply_neon(Matrix& result, const Matrix& a, const Matrix& b) {
        const int block_size = 4;
        for (int i = 0; i < a.rows(); i += block_size) {
            for (int j = 0; j < b.cols(); j += block_size) {
                float64x2_t c00 = vdupq_n_f64(0.0);
                float64x2_t c01 = vdupq_n_f64(0.0);
                float64x2_t c10 = vdupq_n_f64(0.0);
                float64x2_t c11 = vdupq_n_f64(0.0);
                
                for (int k = 0; k < a.cols(); ++k) {
                    // Load A block
                    float64x2_t a0 = vld1q_f64(&a(i, k));
                    float64x2_t a1 = vld1q_f64(&a(i + 2, k));
                    
                    // Load B block
                    float64x2_t b0 = vld1q_f64(&b(k, j));
                    float64x2_t b1 = vld1q_f64(&b(k, j + 2));
                    
                    // Accumulate
                    c00 = vfmaq_f64(c00, a0, b0);
                    c01 = vfmaq_f64(c01, a0, b1);
                    c10 = vfmaq_f64(c10, a1, b0);
                    c11 = vfmaq_f64(c11, a1, b1);
                }
                
                // Store results
                vst1q_f64(&result(i, j), c00);
                vst1q_f64(&result(i, j + 2), c01);
                vst1q_f64(&result(i + 2, j), c10);
                vst1q_f64(&result(i + 2, j + 2), c11);
            }
        }
    }
};
```

### Memory Pool Implementation

**Thread-Safe Object Pool:**
```cpp
template<typename T>
class ObjectPool {
    struct PoolDeleter {
        ObjectPool* pool;
        void operator()(T* ptr) {
            if (pool) pool->release(ptr);
        }
    };
    
    mutable std::mutex mutex_;
    std::vector<std::unique_ptr<T>> available_;
    std::atomic<size_t> total_created_{0};
    std::atomic<size_t> current_size_{0};
    const size_t max_size_;
    
public:
    using Ptr = std::unique_ptr<T, PoolDeleter>;
    
    explicit ObjectPool(size_t max_size = 1000) 
        : max_size_(max_size) {}
    
    template<typename... Args>
    Ptr acquire(Args&&... args) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        if (!available_.empty()) {
            auto obj = std::move(available_.back());
            available_.pop_back();
            current_size_--;
            
            // Reset object state
            *obj = T(std::forward<Args>(args)...);
            return Ptr(obj.release(), PoolDeleter{this});
        }
        
        // Create new object
        total_created_++;
        return Ptr(new T(std::forward<Args>(args)...), 
                  PoolDeleter{this});
    }
    
private:
    void release(T* ptr) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        if (current_size_ < max_size_) {
            available_.push_back(std::unique_ptr<T>(ptr));
            current_size_++;
        } else {
            delete ptr;
        }
    }
};

// Usage example
ObjectPool<Matrix> matrixPool(100);
auto matrix = matrixPool.acquire(10, 10);
// Matrix automatically returned to pool when out of scope
```

## 3. Testing Best Practices

### Comprehensive Test Structure

**Test Organization Pattern:**
```cpp
// test_advanced_controller.cpp
#include <doctest/doctest.h>
#include <memory>
#include <random>

TEST_SUITE("Advanced Controller Tests") {
    // Shared test fixture
    struct ControllerFixture {
        std::unique_ptr<APEXSox> controller;
        std::vector<sensor> sensors;
        std::vector<motor> motors;
        std::mt19937 rng{42};  // Fixed seed for reproducibility
        
        ControllerFixture() {
            controller = std::make_unique<APEXSox>();
            controller->init(10, 5);
            sensors.resize(10);
            motors.resize(5);
        }
        
        void randomSensorInput() {
            std::uniform_real_distribution<> dist(-1.0, 1.0);
            for (auto& s : sensors) {
                s = dist(rng);
            }
        }
        
        void step() {
            controller->step(sensors.data(), sensors.size(),
                           motors.data(), motors.size());
        }
    };
    
    TEST_CASE_FIXTURE(ControllerFixture, "Initialization") {
        CHECK(controller->getSensorNumber() == 10);
        CHECK(controller->getMotorNumber() == 5);
        CHECK(controller->getParam("epsA") > 0);
    }
    
    TEST_CASE_FIXTURE(ControllerFixture, "Motor bounds") {
        for (int i = 0; i < 1000; ++i) {
            randomSensorInput();
            step();
            
            for (const auto& m : motors) {
                CHECK(m >= -1.0);
                CHECK(m <= 1.0);
            }
        }
    }
    
    TEST_CASE_FIXTURE(ControllerFixture, "Learning convergence") {
        std::vector<double> errors;
        
        // Warm-up phase
        for (int i = 0; i < 100; ++i) {
            randomSensorInput();
            step();
        }
        
        // Measure convergence
        for (int i = 0; i < 1000; ++i) {
            randomSensorInput();
            step();
            
            if (i % 10 == 0) {
                errors.push_back(controller->getPredictionError());
            }
        }
        
        // Check that error decreases
        double early_avg = std::accumulate(errors.begin(), 
                                          errors.begin() + 10, 0.0) / 10;
        double late_avg = std::accumulate(errors.end() - 10, 
                                         errors.end(), 0.0) / 10;
        CHECK(late_avg < early_avg * 0.9);
    }
}
```

### Performance Testing

**Benchmark Implementation:**
```cpp
TEST_SUITE("Performance Benchmarks") {
    TEST_CASE("Matrix multiplication performance") {
        const std::vector<int> sizes = {10, 50, 100, 200, 500};
        
        for (int size : sizes) {
            SUBCASE(("Size " + std::to_string(size)).c_str()) {
                Matrix a(size, size);
                Matrix b(size, size);
                Matrix c(size, size);
                
                // Initialize with random values
                a.randomize(-1.0, 1.0);
                b.randomize(-1.0, 1.0);
                
                auto start = std::chrono::high_resolution_clock::now();
                
                // Perform multiplication
                for (int i = 0; i < 100; ++i) {
                    c = a * b;
                }
                
                auto end = std::chrono::high_resolution_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
                
                double us_per_mult = duration.count() / 100.0;
                double gflops = (2.0 * size * size * size) / (us_per_mult * 1000);
                
                MESSAGE("Size ", size, ": ", us_per_mult, " µs, ", gflops, " GFLOPS");
                
                // Performance thresholds
                if (size <= 100) {
                    CHECK(us_per_mult < 1000);  // < 1ms for small matrices
                }
            }
        }
    }
}
```

## 4. Build System Integration

### CMake Component Creation

**Adding New Components:**
```cmake
# components/my_component/CMakeLists.txt
lpzrobots_add_component_library(
    NAME my_component
    TYPE SHARED STATIC
    VERSION ${PROJECT_VERSION}
    HEADERS
        include/my_component/feature.h
        include/my_component/algorithm.h
    SOURCES
        src/feature.cpp
        src/algorithm.cpp
    PUBLIC_DEPS
        lpzrobots::selforg
        Threads::Threads
    PRIVATE_DEPS
        $<$<BOOL:${USE_OPENMP}>:OpenMP::OpenMP_CXX>
    COMPILE_FEATURES
        cxx_std_17
    COMPILE_OPTIONS
        $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall -Wextra>
        $<$<CONFIG:Release>:-O3>
)

# Add tests
if(BUILD_TESTING)
    lpzrobots_add_doctest(
        NAME test_my_component
        SOURCES
            tests/test_feature.cpp
            tests/test_algorithm.cpp
        LIBRARIES
            lpzrobots::my_component
        LABELS unit my_component
    )
endif()

# Add examples
add_subdirectory(examples)

# Install headers
install(
    DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    COMPONENT development
)
```

### Platform-Specific Configuration

**Detecting and Enabling Features:**
```cmake
# Platform-specific optimizations
if(CMAKE_SYSTEM_PROCESSOR MATCHES "arm64|aarch64")
    message(STATUS "ARM64 detected - enabling NEON")
    target_compile_definitions(selforg PRIVATE USE_NEON)
    target_sources(selforg PRIVATE 
        matrix/matrix_neon.cpp
        matrix/vector_neon.cpp
    )
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|AMD64")
    include(CheckCXXCompilerFlag)
    
    # Check for AVX2 support
    check_cxx_compiler_flag("-mavx2" COMPILER_SUPPORTS_AVX2)
    if(COMPILER_SUPPORTS_AVX2)
        message(STATUS "AVX2 support detected")
        target_compile_options(selforg PRIVATE -mavx2)
        target_compile_definitions(selforg PRIVATE USE_AVX2)
        target_sources(selforg PRIVATE 
            matrix/matrix_avx2.cpp
            matrix/vector_avx2.cpp
        )
    endif()
endif()

# Sanitizer support
if(ENABLE_SANITIZERS)
    lpzrobots_add_sanitizer_flags(selforg)
endif()
```

## 5. Debugging and Profiling

### Advanced Debugging Techniques

**Debug Helper Macros:**
```cpp
// debug_helpers.h
#ifdef DEBUG_LPZROBOTS
    #define LPZROBOTS_ASSERT(cond, msg) \
        do { \
            if (!(cond)) { \
                std::cerr << "Assertion failed: " << #cond << "\n" \
                         << "Message: " << msg << "\n" \
                         << "File: " << __FILE__ << "\n" \
                         << "Line: " << __LINE__ << "\n"; \
                std::abort(); \
            } \
        } while(0)
    
    #define LPZROBOTS_DEBUG_PRINT(var) \
        std::cout << "[DEBUG] " << #var << " = " << var << std::endl
#else
    #define LPZROBOTS_ASSERT(cond, msg) ((void)0)
    #define LPZROBOTS_DEBUG_PRINT(var) ((void)0)
#endif

// Matrix debugging
class MatrixDebug {
public:
    static void checkNaN(const Matrix& m, const std::string& context) {
        for (int i = 0; i < m.rows(); ++i) {
            for (int j = 0; j < m.cols(); ++j) {
                if (std::isnan(m(i, j))) {
                    throw std::runtime_error(
                        "NaN detected in " + context + 
                        " at (" + std::to_string(i) + "," + 
                        std::to_string(j) + ")");
                }
            }
        }
    }
    
    static void printStats(const Matrix& m, const std::string& name) {
        double min = m.min();
        double max = m.max();
        double mean = m.mean();
        double stddev = m.stddev();
        
        std::cout << "Matrix " << name << " stats:\n"
                  << "  Size: " << m.rows() << "x" << m.cols() << "\n"
                  << "  Range: [" << min << ", " << max << "]\n"
                  << "  Mean: " << mean << ", StdDev: " << stddev << "\n";
    }
};
```

### Profiling Integration

**Performance Profiling Helpers:**
```cpp
class ScopedTimer {
    std::string name_;
    std::chrono::high_resolution_clock::time_point start_;
    
public:
    explicit ScopedTimer(const std::string& name) 
        : name_(name), start_(std::chrono::high_resolution_clock::now()) {}
    
    ~ScopedTimer() {
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start_);
        std::cout << "[PROFILE] " << name_ << ": " 
                  << duration.count() << " µs\n";
    }
};

// Usage
void processStep() {
    ScopedTimer timer("Controller::step");
    // ... processing ...
}
```

## Best Practices Summary

1. **Use RAII everywhere** - No manual resource management
2. **Prefer value semantics** - Use move semantics for efficiency
3. **Write testable code** - Dependency injection, interfaces
4. **Profile before optimizing** - Measure, don't guess
5. **Document intentions** - Not just what, but why
6. **Follow naming conventions** - Consistency is key
7. **Use modern C++ features** - They exist for good reasons

This implementation guide provides the foundation for maintaining and extending the LPZRobots codebase with professional software engineering practices.