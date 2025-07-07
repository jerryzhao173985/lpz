# LPZRobots macOS ARM64 Migration Plan

## Executive Summary

This document outlines the comprehensive plan to migrate LPZRobots from its current Linux/Qt4/Make-based implementation to a modern macOS ARM64 (Apple Silicon) compatible version using Qt5, CMake, and C++17.

## Migration Goals

1. **Platform Support**: Full native support for macOS ARM64 (M1/M2/M3/M4)
2. **Build System**: Transition from GNU Make to CMake 3.16+
3. **GUI Framework**: Migrate from Qt4/Qt3Support to Qt5
4. **C++ Standard**: Upgrade from C++11 to C++17
5. **Compiler**: Full compatibility with Apple clang/clang++
6. **Performance**: Maintain or improve performance on ARM64
7. **Compatibility**: Preserve all existing functionality

## Phase 1: Build System Foundation (Week 1-2)

### 1.1 CMake Infrastructure
Create root CMakeLists.txt:
```cmake
cmake_minimum_required(VERSION 3.16)
project(lpzrobots VERSION 2.0.0 LANGUAGES CXX C)

# C++ Standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Platform Detection
if(APPLE)
  if(CMAKE_SYSTEM_PROCESSOR MATCHES "arm64|aarch64")
    set(MACOS_ARM64 TRUE)
    message(STATUS "Building for macOS ARM64")
  endif()
endif()

# Dependencies
find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)
find_package(OpenSceneGraph REQUIRED)
find_package(Qt5 COMPONENTS Core Gui Widgets OpenGL Xml REQUIRED)
find_package(GSL REQUIRED)

# Subdirectories
add_subdirectory(selforg)
add_subdirectory(ode_robots)
add_subdirectory(opende)
add_subdirectory(guilogger)
add_subdirectory(matrixviz)
add_subdirectory(configurator)
```

### 1.2 Component CMake Files
Create CMakeLists.txt for each component with proper target definitions and dependencies.

### 1.3 Homebrew Integration
Create Homebrew formula for easy installation:
```ruby
class Lpzrobots < Formula
  desc "Robot simulation framework with self-organizing controllers"
  homepage "https://github.com/georgmartius/lpzrobots"
  head "https://github.com/georgmartius/lpzrobots.git"
  
  depends_on "cmake" => :build
  depends_on "qt@5"
  depends_on "open-scene-graph"
  depends_on "gsl"
  
  def install
    system "cmake", "-S", ".", "-B", "build", *std_cmake_args
    system "cmake", "--build", "build"
    system "cmake", "--install", "build"
  end
end
```

## Phase 2: Core Dependencies Update (Week 3-4)

### 2.1 ODE ARM64 Support
Update opende/include/ode/odeconfig.h:
```c
#if defined(__aarch64__) || defined(__arm64__)
  #define ARM64_SYSTEM 1
  typedef double dReal;
  #ifdef dSINGLE
    #error ARM64 requires double precision
  #endif
#endif
```

### 2.2 OpenSceneGraph Configuration
Ensure OSG is built with proper ARM64 flags and Qt5 support.

### 2.3 Platform Detection
Update all configure scripts to detect ARM64:
```bash
case "$host_cpu" in
  arm64* | aarch64* )
    cpu64=yes
    arm64=yes
    AC_DEFINE(ARM64_SYSTEM,1,[ARM64 system])
    ;;
esac
```

## Phase 3: Qt5 Migration (Week 5-6)

### 3.1 Remove Qt3Support

#### guilogger/src/inifile.h
```cpp
// Old Qt3
Q3PtrList<IniSection> sections;

// New Qt5
QList<std::unique_ptr<IniSection>> sections;
```

### 3.2 Update Qt Includes
Replace all lowercase Qt includes:
```cpp
// Old
#include <qwidget.h>
#include <qstring.h>

// New
#include <QWidget>
#include <QString>
```

### 3.3 OpenGL Widget Migration
```cpp
// Old
class MyGLWidget : public QGLWidget {

// New
class MyGLWidget : public QOpenGLWidget, protected QOpenGLFunctions {
protected:
    void initializeGL() override {
        initializeOpenGLFunctions();
        // ...
    }
};
```

### 3.4 Signal/Slot Syntax
```cpp
// Old
connect(button, SIGNAL(clicked()), this, SLOT(handleClick()));

// New
connect(button, &QPushButton::clicked, this, &MyClass::handleClick);
```

## Phase 4: C++17 Modernization (Week 7-8)

### 4.1 Language Features
- Use `std::optional` for optional values
- Replace raw pointers with smart pointers
- Use structured bindings
- Adopt `std::filesystem` for file operations

### 4.2 STL Updates
```cpp
// Old
typedef std::map<std::string, double> ParamMap;
ParamMap::iterator it = params.find(key);

// New
using ParamMap = std::map<std::string, double>;
if (auto it = params.find(key); it != params.end()) {
    // use it->second
}
```

### 4.3 Error Handling
Replace error codes with exceptions or `std::expected` (C++23 backport).

## Phase 5: Platform-Specific Optimizations (Week 9-10)

### 5.1 ARM64 SIMD
Implement NEON optimizations for matrix operations:
```cpp
#ifdef __ARM_NEON
#include <arm_neon.h>

void Matrix::multiply_neon(const Matrix& other) {
    // NEON implementation
}
#endif
```

### 5.2 Grand Central Dispatch
Use GCD for parallel operations on macOS:
```cpp
#ifdef __APPLE__
#include <dispatch/dispatch.h>

void parallel_for(size_t start, size_t end, const std::function<void(size_t)>& func) {
    dispatch_apply(end - start, dispatch_get_global_queue(0, 0), ^(size_t i) {
        func(start + i);
    });
}
#endif
```

## Phase 6: Testing and Validation (Week 11)

### 6.1 Unit Tests
Add Google Test framework:
```cmake
include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG release-1.12.1
)
FetchContent_MakeAvailable(googletest)
```

### 6.2 Integration Tests
- Test each robot type
- Verify all controllers
- Validate GUI tools
- Performance benchmarks

### 6.3 CI/CD Pipeline
```yaml
name: macOS ARM64 Build
on: [push, pull_request]
jobs:
  build:
    runs-on: macos-latest
    steps:
      - uses: actions/checkout@v3
      - name: Install Dependencies
        run: brew install qt@5 open-scene-graph gsl
      - name: Build
        run: |
          cmake -B build -DCMAKE_BUILD_TYPE=Release
          cmake --build build
      - name: Test
        run: ctest --test-dir build
```

## Phase 7: Documentation and Deployment (Week 12)

### 7.1 Update Documentation
- Migration guide for users
- Developer documentation
- API changes
- Performance comparisons

### 7.2 Release Package
Create universal binary support:
```cmake
set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64")
```

## Risk Mitigation

### Fallback Options
1. **Qt4 Compatibility Layer**: Maintain Qt4 branch for legacy systems
2. **Rosetta 2**: Document x86_64 build for compatibility
3. **Docker**: Provide Linux container for original behavior

### Performance Regression
- Benchmark critical paths
- Profile ARM64 vs x86_64
- Optimize hot spots with platform-specific code

## Success Criteria

1. All components build without warnings on macOS ARM64
2. All tests pass with 100% functionality preserved
3. GUI tools run natively without Rosetta 2
4. Performance matches or exceeds Intel builds
5. Documentation updated and accurate

## Timeline Summary

- **Weeks 1-2**: Build system migration
- **Weeks 3-4**: Core dependencies
- **Weeks 5-6**: Qt5 migration
- **Weeks 7-8**: C++17 modernization
- **Weeks 9-10**: Platform optimizations
- **Week 11**: Testing and validation
- **Week 12**: Documentation and release

## Next Steps

1. Start with CMake infrastructure
2. Create feature branches for each phase
3. Set up CI/CD pipeline early
4. Begin with selforg library as proof of concept
5. Iterate based on testing results