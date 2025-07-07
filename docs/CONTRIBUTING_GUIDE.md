# LPZRobots Contributing Guide

---
**Document Type**: Guide  
**Last Updated**: 2025-01-06  
**Status**: Current  
**Version**: 1.0  
**Component(s)**: All  
**Tags**: contributing, development, standards, workflow  
---

## Introduction

Welcome to the LPZRobots project! This guide outlines how to contribute effectively to the codebase while maintaining our high standards of quality and consistency.

## 1. Getting Started

### Prerequisites

Before contributing, ensure you have:
- C++ compiler with C++17 support (clang++ 14+ or g++ 11+)
- CMake 3.20 or higher
- Git for version control
- Required dependencies installed (Qt6, GSL, OpenSceneGraph)

### Development Setup

1. **Fork and clone the repository:**
```bash
git clone https://github.com/yourusername/lpzrobots.git
cd lpzrobots
git remote add upstream https://github.com/georgmartius/lpzrobots.git
```

2. **Create a development branch:**
```bash
git checkout -b feature/my-new-feature
```

3. **Configure and build:**
```bash
cmake --preset dev-debug
cmake --build build-debug
```

## 2. Code Standards

### C++ Style Guide

#### Naming Conventions

```cpp
// Classes: PascalCase
class MyController : public AbstractController {
    // Member variables: camelCase with trailing underscore
    Matrix weights_;
    int sensorNumber_;
    
    // Methods: camelCase
    void updateWeights();
    
    // Constants: UPPER_SNAKE_CASE
    static constexpr double DEFAULT_EPSILON = 0.1;
};

// Functions: camelCase
void calculateError(const Matrix& predicted, const Matrix& actual);

// Namespaces: lowercase
namespace lpzrobots {
namespace utils {
}
}
```

#### Modern C++ Practices

**Use smart pointers:**
```cpp
// Good
auto controller = std::make_unique<Sox>();
std::shared_ptr<ConfigData> config = std::make_shared<ConfigData>();

// Avoid
AbstractController* controller = new Sox();
```

**Prefer auto for complex types:**
```cpp
// Good
auto result = matrix.multiply(other);
auto it = controllers.find(name);

// Less readable
std::unordered_map<std::string, std::unique_ptr<AbstractController>>::iterator it;
```

**Use range-based for loops:**
```cpp
// Good
for (const auto& agent : agents) {
    agent->step();
}

// Avoid
for (size_t i = 0; i < agents.size(); ++i) {
    agents[i]->step();
}
```

#### Header Organization

```cpp
// mycontroller.h
#pragma once  // Use pragma once instead of include guards

// System headers first
#include <memory>
#include <vector>

// External dependencies
#include <gsl/gsl_matrix.h>

// Project headers
#include "selforg/controller/abstractcontroller.h"
#include "selforg/matrix/matrix.h"

namespace lpzrobots {

class MyController : public AbstractController {
    // Public interface first
public:
    MyController();
    ~MyController() override = default;
    
    // AbstractController interface
    void init(int sensornumber, int motornumber, 
              RandGen* randGen = nullptr) override;
    
    // Public methods
    void reset();
    
    // Protected members
protected:
    void updateInternal();
    
    // Private members last
private:
    struct Impl;  // PIMPL idiom for complex implementations
    std::unique_ptr<Impl> pImpl_;
};

} // namespace lpzrobots
```

### Documentation Standards

#### Class Documentation
```cpp
/**
 * @brief Advanced homeostatic controller with meta-learning
 * 
 * This controller implements the APEX-Sox algorithm which combines
 * standard Sox homeokinetic learning with a meta-learning layer
 * that adapts exploration parameters based on performance.
 * 
 * @note Thread-safe for parameter access
 * @see Sox for the base algorithm
 * 
 * Example usage:
 * @code
 * auto controller = std::make_unique<APEXSox>();
 * controller->setParam("metaRate", 0.01);
 * @endcode
 */
class APEXSox : public Sox {
```

#### Method Documentation
```cpp
/**
 * @brief Updates controller weights based on prediction error
 * 
 * @param sensors Current sensor values in range [-1, 1]
 * @param nsensors Number of sensors
 * @param motors Output motor values (will be set to range [-1, 1])
 * @param nmotors Number of motors
 * 
 * @throws std::invalid_argument if nsensors/nmotors don't match initialization
 * @pre Controller must be initialized via init()
 * @post Motor values are updated and within bounds
 */
void step(const sensor* sensors, int nsensors,
          motor* motors, int nmotors) override;
```

## 3. Development Workflow

### Feature Development

1. **Create an issue** describing the feature
2. **Design discussion** if significant change
3. **Implementation** following standards
4. **Testing** with comprehensive coverage
5. **Documentation** updates
6. **Pull request** with clear description

### Bug Fixes

1. **Reproduce** the bug with minimal example
2. **Add failing test** that demonstrates bug
3. **Fix** the issue
4. **Verify** test now passes
5. **Check** for regressions

### Code Review Checklist

Before submitting PR, ensure:
- [ ] Code compiles without warnings
- [ ] All tests pass
- [ ] New tests added for new functionality
- [ ] Documentation updated
- [ ] Code follows style guide
- [ ] No commented-out code
- [ ] No debug prints
- [ ] Performance implications considered

## 4. Testing Requirements

### Unit Tests

Every new class should have tests:
```cpp
// test_mycontroller.cpp
#include <doctest/doctest.h>
#include "selforg/controller/mycontroller.h"

TEST_SUITE("MyController") {
    TEST_CASE("Initialization") {
        MyController controller;
        controller.init(10, 5);
        
        CHECK(controller.getSensorNumber() == 10);
        CHECK(controller.getMotorNumber() == 5);
    }
    
    TEST_CASE("Motor bounds") {
        MyController controller;
        controller.init(4, 2);
        
        sensor sensors[4] = {10.0, -10.0, 0.5, -0.5};  // Extreme values
        motor motors[2];
        
        controller.step(sensors, 4, motors, 2);
        
        for (int i = 0; i < 2; ++i) {
            CHECK(motors[i] >= -1.0);
            CHECK(motors[i] <= 1.0);
        }
    }
}
```

### Integration Tests

Test component interactions:
```cpp
TEST_CASE("Controller with robot integration") {
    auto robot = RobotFactory::createRobot("Sphererobot3Masses", ...);
    auto controller = std::make_unique<MyController>();
    auto wiring = std::make_unique<One2OneWiring>();
    
    OdeAgent agent;
    agent.init(controller.get(), robot.get(), wiring.get());
    
    // Run simulation steps
    for (int i = 0; i < 100; ++i) {
        agent.step(0.1, 0.01);
    }
    
    CHECK(agent.getController()->getParam("error") < 0.1);
}
```

### Performance Tests

Include benchmarks for critical paths:
```cpp
TEST_CASE("Matrix multiplication performance") {
    Matrix a(100, 100), b(100, 100), c(100, 100);
    a.randomize(-1, 1);
    b.randomize(-1, 1);
    
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 100; ++i) {
        c = a * b;
    }
    auto end = std::chrono::high_resolution_clock::now();
    
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    CHECK(duration.count() < 1000);  // Should complete in < 1 second
}
```

## 5. Build System

### Adding New Components

Create `CMakeLists.txt`:
```cmake
lpzrobots_add_component_library(
    NAME mycomponent
    TYPE SHARED STATIC
    VERSION ${PROJECT_VERSION}
    HEADERS
        include/mycomponent/feature.h
    SOURCES
        src/feature.cpp
    PUBLIC_DEPS
        lpzrobots::selforg
    COMPILE_FEATURES
        cxx_std_17
)

# Add tests
if(BUILD_TESTING)
    lpzrobots_add_doctest(
        NAME test_mycomponent
        SOURCES tests/test_feature.cpp
        LIBRARIES lpzrobots::mycomponent
    )
endif()
```

### Platform-Specific Code

Use CMake for platform detection:
```cpp
// config.h.in
#cmakedefine USE_NEON
#cmakedefine USE_AVX2

// mycode.cpp
#include "config.h"

void processData() {
    #ifdef USE_NEON
        processDataNEON();
    #elif defined(USE_AVX2)
        processDataAVX2();
    #else
        processDataScalar();
    #endif
}
```

## 6. Git Workflow

### Commit Messages

Follow conventional commits:
```
type(scope): subject

body

footer
```

Examples:
```
feat(controller): add APEX-Sox meta-learning controller

Implements meta-learning layer on top of Sox controller that adapts
exploration parameters based on prediction performance.

Closes #123
```

```
fix(matrix): prevent overflow in pseudoinverse calculation

Add epsilon to diagonal before inversion to ensure numerical stability
when matrix is near-singular.
```

Types:
- `feat`: New feature
- `fix`: Bug fix
- `docs`: Documentation only
- `style`: Code style (formatting, semicolons, etc)
- `refactor`: Code change that neither fixes bug nor adds feature
- `perf`: Performance improvement
- `test`: Adding missing tests
- `build`: Changes to build system

### Pull Request Process

1. **Update from upstream:**
```bash
git fetch upstream
git rebase upstream/main
```

2. **Run full test suite:**
```bash
cmake --build build-debug --target test
cmake --build build-release --target test
```

3. **Check code quality:**
```bash
cmake --build build-debug --target format  # Auto-format
cmake --build build-debug --target tidy    # Run clang-tidy
```

4. **Create PR with:**
   - Clear title and description
   - Link to related issue
   - Summary of changes
   - Test results
   - Performance impact (if applicable)

## 7. Performance Guidelines

### Profiling

Always profile before optimizing:
```cpp
#ifdef ENABLE_PROFILING
    PROFILE_SCOPE("MyController::step");
#endif
```

### Memory Management

- Use memory pools for frequent allocations
- Prefer stack allocation for small objects
- Implement move constructors/assignment
- Avoid unnecessary copies

### SIMD Usage

When implementing performance-critical code:
```cpp
// Provide scalar fallback
void multiplyScalar(Matrix& result, const Matrix& a, const Matrix& b);

// Platform-specific optimizations
#ifdef USE_NEON
void multiplyNEON(Matrix& result, const Matrix& a, const Matrix& b);
#endif

// Runtime dispatch
void multiply(Matrix& result, const Matrix& a, const Matrix& b) {
    #ifdef USE_NEON
    if (isNEONAvailable()) {
        multiplyNEON(result, a, b);
        return;
    }
    #endif
    multiplyScalar(result, a, b);
}
```

## 8. Common Patterns

### Factory Registration

```cpp
// In .cpp file
namespace {
    bool registered = []() {
        ControllerFactory::registerController<MyController>("MyController");
        return true;
    }();
}
```

### RAII Guards

```cpp
class SimulationGuard {
    Simulation* sim_;
public:
    explicit SimulationGuard(Simulation* sim) : sim_(sim) {
        sim_->pause();
    }
    ~SimulationGuard() {
        sim_->resume();
    }
};
```

## Review Process

Your PR will be reviewed for:
1. **Correctness**: Does it work as intended?
2. **Testing**: Are tests comprehensive?
3. **Performance**: No regressions?
4. **Style**: Follows guidelines?
5. **Documentation**: Clear and complete?

## Getting Help

- **Discord/Slack**: Real-time discussion
- **GitHub Issues**: Bug reports and features
- **Documentation**: Check docs/ directory
- **Examples**: See simulations/ for patterns

Thank you for contributing to LPZRobots!