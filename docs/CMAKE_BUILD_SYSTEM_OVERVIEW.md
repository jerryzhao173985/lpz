# LPZRobots Modern CMake Build System and Testing Infrastructure

---
**Document Type**: Technical Overview  
**Last Updated**: 2025-01-07  
**Status**: Current  
**Version**: 1.0  
**Component(s)**: Build System, Testing  
**Tags**: cmake, testing, ci-cd, developer-workflow  
---

## Executive Summary

LPZRobots has successfully migrated from a legacy Make/M4-based build system to a modern CMake infrastructure while maintaining backward compatibility. The new system features comprehensive testing with doctest, advanced CI/CD pipelines, and developer-friendly presets.

## Build System Architecture

### Core Components

1. **Root CMakeLists.txt**: Orchestrates the entire build
   - Minimum CMake version: 3.20
   - C++17 standard (required)
   - Modular component selection
   - Comprehensive warning flags

2. **CMake Modules** (`cmake/`):
   - `LPZRobotsCore.cmake`: Main configuration module
   - `LPZRobotsCompiler.cmake`: Compiler detection and flags
   - `LPZRobotsPlatform.cmake`: Platform-specific settings
   - `LPZRobotsLibrary.cmake`: Library creation helpers
   - `LPZRobotsDependencies.cmake`: External dependency management
   - `LPZRobotsTesting.cmake`: Testing infrastructure
   - `SimulationBuild.cmake`: Simulation building support

### Build Options

```cmake
option(BUILD_SHARED_LIBS "Build shared libraries" ON)
option(BUILD_EXAMPLES "Build example simulations" ON)
option(BUILD_TESTS "Build unit tests" OFF)
option(BUILD_GUI_TOOLS "Build GUI tools (requires Qt6)" ON)
option(ENABLE_SIMD "Enable SIMD optimizations" ON)
option(ENABLE_OPENMP "Enable OpenMP parallelization" ON)
```

### Component Build Order

The system enforces proper dependency order:
1. selforg (core controller library)
2. opende (physics engine)
3. configurator (if GUI enabled)
4. ode_robots (robot framework)
5. ga_tools (genetic algorithms)
6. GUI tools (guilogger, matrixviz)
7. Tests (if enabled)

## CMake Presets

### Configuration Presets

The `CMakePresets.json` provides ready-to-use configurations:

1. **Platform-Specific**:
   - `macos-arm64`: Apple Silicon optimized
   - `macos-x64`: Intel Mac support
   - `macos-universal`: Universal binary
   - `linux`: Linux with OpenMP

2. **Build Types**:
   - `debug`: Debug symbols, no optimization
   - `release`: Full optimization
   - `release-with-debug`: Optimized with debug info
   - `dev`: Development with all warnings
   - `minimal`: Core components only

3. **Special Purpose**:
   - `ci`: Continuous integration
   - `ninja`: Fast builds with Ninja

### Usage Examples

```bash
# Configure for macOS ARM64
cmake --preset macos-arm64

# Build with preset
cmake --build --preset macos-arm64

# Run tests
ctest --preset default
```

## Testing Infrastructure

### Test Organization

```
tests/
├── CMakeLists.txt         # Test configuration
├── doctest_main.cpp       # Enhanced main with utilities
├── doctest_main_standard.cpp  # Standard doctest main
├── include/               # Test utilities
│   ├── lpzrobots_test_fixtures.h
│   ├── lpzrobots_test_scenarios.h
│   └── lpzrobots_test_utils.h
├── unit/                  # Unit tests
│   ├── selforg/          # Component-specific tests
│   └── ode_robots/
├── integration/          # Integration tests
├── performance/          # Performance benchmarks
└── bdd/                  # Behavior-driven tests
```

### Test Categories

1. **Unit Tests** (`make test_unit`):
   - Fast, isolated component tests
   - No external dependencies
   - Run on every commit

2. **Integration Tests** (`make test_integration`):
   - Test component interactions
   - May require display/resources
   - Run on pull requests

3. **Performance Tests** (`make test_performance`):
   - Benchmark critical paths
   - Track regression
   - Run nightly

4. **BDD Tests** (`make test_bdd`):
   - Scenario-based testing
   - User story validation
   - Run on releases

### Testing Tools

1. **doctest Integration**:
   ```cpp
   #include <doctest/doctest.h>
   
   TEST_CASE("Matrix operations") {
       Matrix m(3, 3);
       CHECK(m.getM() == 3);
       CHECK(m.getN() == 3);
   }
   ```

2. **Custom Test Utilities**:
   - Test fixtures for common setups
   - Scenario builders for complex tests
   - Performance measurement helpers

3. **Coverage and Analysis**:
   ```bash
   # Generate coverage report
   make test_coverage
   
   # Memory leak detection
   make test_memcheck
   
   # Run with sanitizers
   cmake -B build -DLPZROBOTS_ENABLE_SANITIZERS=ON
   ```

## CI/CD Pipeline

### GitHub Actions Workflows

1. **Comprehensive CI** (`ci-comprehensive.yml`):
   - Multi-platform matrix (macOS ARM64/x64, Ubuntu, various compilers)
   - Quick sanity check before expensive tests
   - Parallel test execution
   - Artifact upload on failure

2. **Code Quality** (`code-quality.yml`):
   - Static analysis (clang-tidy, cppcheck)
   - Code formatting checks
   - Documentation generation

3. **Performance Tracking** (`performance-tracking.yml`):
   - Benchmark execution
   - Regression detection
   - Historical tracking

4. **Release Process** (`release.yml`):
   - Automated packaging
   - Multi-platform builds
   - Documentation updates

### CI Matrix

| Platform | Compiler | Architecture | Qt Version | Special Features |
|----------|----------|--------------|------------|------------------|
| macOS 14 | Clang 15 | ARM64 | Qt6 | Native Apple Silicon |
| macOS 13 | Clang 14 | x86_64 | Qt6 | Intel compatibility |
| Ubuntu 22.04 | GCC 11 | x86_64 | Qt6 | Primary Linux |
| Ubuntu 22.04 | Clang 14 | x86_64 | Qt6 | Alternative compiler |
| Ubuntu 22.04 | GCC 11 | x86_64 | Qt6 | Debug + Sanitizers |

## Developer Workflow

### Quick Start

```bash
# Clone and configure
git clone https://github.com/georgmartius/lpzrobots.git
cd lpzrobots
cmake --preset default

# Build
cmake --build build/default -j8

# Test
cd build/default
ctest -j4
```

### Development Mode

The build system supports "DEVEL" mode where libraries remain in the source tree:

```cmake
set(LPZROBOTS_INSTALL_TYPE "DEVEL")
```

Benefits:
- No installation required for development
- Simulations use relative paths
- Faster iteration cycle

### Creating New Simulations

```cmake
# In simulation's CMakeLists.txt
cmake_minimum_required(VERSION 3.20)
project(my_simulation)

find_package(lpzrobots REQUIRED)
lpzrobots_add_simulation(my_simulation)
```

### Backward Compatibility

1. **Legacy Make Support**:
   - `make conf` still works
   - Generates Makefile.conf
   - Wraps CMake commands

2. **Config Scripts**:
   - `selforg-config`, `ode_robots-config` maintained
   - Used by legacy simulations
   - Generated by CMake

3. **Library Variants**:
   - `_opt` suffix for optimized builds
   - `_dbg` suffix for debug builds
   - Controlled by `LPZROBOTS_LEGACY_VARIANTS`

## Advanced Features

### Sanitizer Support

```cmake
# Enable all sanitizers
cmake -B build -DLPZROBOTS_ENABLE_SANITIZERS=ON

# Specific sanitizers
-DLPZROBOTS_SANITIZER_ADDRESS=ON    # Memory errors
-DLPZROBOTS_SANITIZER_THREAD=ON     # Race conditions
-DLPZROBOTS_SANITIZER_UNDEFINED=ON  # Undefined behavior
```

### Static Analysis

```cmake
# Enable clang-tidy
cmake -B build -DLPZROBOTS_ENABLE_CLANG_TIDY=ON

# Enable cppcheck
cmake -B build -DLPZROBOTS_ENABLE_CPPCHECK=ON
```

### Platform-Specific Features

1. **macOS**:
   - AGL framework filtering
   - Universal binary support
   - Homebrew integration

2. **Linux**:
   - OpenMP enabled by default
   - System package integration
   - Virtual display for CI

## Migration from Make to CMake

### For Users

```bash
# Old way
make conf
make all
make install

# New way
cmake -B build
cmake --build build
cmake --install build
```

### For Developers

1. Replace `Makefile.conf` includes with `find_package(lpzrobots)`
2. Use `lpzrobots_add_simulation()` instead of manual rules
3. Link to namespaced targets: `lpzrobots::selforg`

### For Simulations

Old Makefile:
```makefile
include Makefile.conf
LIBS += -lselforg -lode_robots
```

New CMakeLists.txt:
```cmake
find_package(lpzrobots REQUIRED)
lpzrobots_add_simulation(mysim)
```

## Best Practices

1. **Use Presets**: Start with predefined configurations
2. **Enable Testing**: Always build with tests in development
3. **Check Warnings**: Use `-Wall -Wextra` flags
4. **Run Sanitizers**: Catch bugs early
5. **Use CI**: Let automation find issues

## Future Enhancements

1. **Conan/vcpkg Integration**: Modern package management
2. **GPU Acceleration**: CUDA/OpenCL support
3. **Web Assembly**: Browser-based simulations
4. **Better IDE Integration**: Enhanced VS Code/CLion support
5. **Incremental Migration**: Gradual C++20/23 adoption

## Troubleshooting

### Common Issues

1. **Qt6 Not Found**: Install Qt6 development packages
2. **OpenSceneGraph Issues**: Check version compatibility (3.6+)
3. **Linker Errors**: Ensure proper build order
4. **Test Failures**: Check LPZROBOTS_WARMUP_STEPS for controllers

### Debug Commands

```bash
# Verbose build
cmake --build build --verbose

# Test with output
ctest --output-on-failure

# Single test
ctest -R test_name -V

# Clean rebuild
rm -rf build && cmake -B build && cmake --build build
```

This modern build system provides a solid foundation for LPZRobots' continued development while maintaining compatibility with existing code and workflows.