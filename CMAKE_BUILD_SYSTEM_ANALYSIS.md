# LPZRobots CMake Build System Analysis

## Overview

The LPZRobots CMake build system is a sophisticated, modular architecture designed to replace the legacy Make/M4 system while maintaining backward compatibility. It provides modern CMake practices with support for component-based development, multi-platform builds, and various installation types.

## Build System Architecture

### 1. Root CMakeLists.txt Structure

The root `CMakeLists.txt` serves as the entry point and orchestrator:

```cmake
cmake_minimum_required(VERSION 3.20)
project(lpzrobots VERSION ${VERSION_CONTENTS} LANGUAGES CXX C)
```

Key features:
- **Version Management**: Reads version from `VERSION` file for single source of truth
- **C++ Standard**: Enforces C++17 as minimum with `CMAKE_CXX_STANDARD_REQUIRED ON`
- **Module Loading**: Custom modules from `cmake/` directory
- **Options**: Configurable build options (shared libs, examples, tests, GUI tools, SIMD, OpenMP)
- **Quality Tools**: Optional sanitizers, coverage, static analysis (clang-tidy, cppcheck, iwyu)

### 2. CMake Module Organization

The build system is organized into specialized modules in the `cmake/` directory:

#### Core Modules

**LPZRobotsCore.cmake**
- Main configuration module that includes all others
- Functions: `lpzrobots_configure()`, `lpzrobots_print_summary()`, `lpzrobots_add_components()`
- Handles component build order and installation types (USER/DEVEL)

**LPZRobotsLibrary.cmake**
- Unified library creation with `lpzrobots_add_component_library()`
- Features:
  - Automatic source collection from subdirectories
  - Include directory management (BUILD_INTERFACE/INSTALL_INTERFACE)
  - Legacy variant creation (_opt, _dbg)
  - Platform-specific settings
  - Header symlink creation

**LPZRobotsDependencies.cmake**
- Centralized dependency management
- Functions: `lpzrobots_find_dependencies()`, `lpzrobots_find_qt()`, `lpzrobots_find_ode()`
- Handles:
  - Required: Threads
  - Optional: OpenMP, GSL, Qt5/6, OpenSceneGraph, readline
  - Testing: doctest/GoogleTest
  - Special handling for bundled vs system ODE

**SimulationBuild.cmake**
- Simulation executable creation with `lpzrobots_add_simulation()`
- Features:
  - Automatic library linking
  - Development mode detection
  - Legacy variant support
  - Backward-compatible Makefile generation
  - macOS AGL filter application

#### Support Modules

**LPZRobotsTargets.cmake**
- Target creation helpers
- Functions for libraries, executables, Qt apps, tests
- Config script generation from templates
- pkg-config file generation

**LPZRobotsPlatform.cmake**
- Platform detection and configuration
- Install type determination (USER/DEVEL)

**LPZRobotsCompiler.cmake**
- Compiler settings and warning flags
- Platform-specific optimizations

**LPZRobotsTesting.cmake**
- Test infrastructure with doctest/GoogleTest support
- Coverage and sanitizer integration

**LPZRobotsMacOSFixes.cmake**
- macOS-specific fixes and workarounds
- AGL framework removal

### 3. Component Build Structure

Each component follows a consistent pattern:

#### selforg (Core Library)
```cmake
file(GLOB CONTROLLER_SOURCES controller/*.cpp)
file(GLOB WIRING_SOURCES wirings/*.cpp)
# ... collect all sources

add_library(selforg ${SELFORG_SOURCES})
add_library(lpzrobots::selforg ALIAS selforg)

target_include_directories(selforg
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        # ... subdirectories
)
```

Key aspects:
- Source collection by subdirectory
- Alias targets with namespace
- Generator expressions for build/install interfaces
- Optional dependency handling (GSL)

#### ode_robots (Robot Framework)
```cmake
lpzrobots_add_component_library(ode_robots
    SOURCES ${ODE_ROBOTS_MAIN_SOURCES}
    SUBDIRS ${ODE_ROBOTS_SUBDIRS}
    PUBLIC_DEPS
        lpzrobots::selforg
        Threads::Threads
        ${OPENSCENEGRAPH_LIBRARIES}
    DEFINES
        NOCONFIGURATOR
        OSG_LIBRARY_STATIC
)
```

Uses the unified library creation function for consistency.

### 4. Simulation Build System

Simulations can be built two ways:

#### Individual Simulation CMakeLists.txt
```cmake
include(${CMAKE_CURRENT_SOURCE_DIR}/../../../cmake/SimulationBuild.cmake)
lpzrobots_add_simulation(start
    SOURCES main.cpp
)
```

#### Batch Processing (simulations/CMakeLists.txt)
```cmake
function(add_simulation_directory dir)
    # Parse Makefile.conf for additional sources
    # Create simulation executable
    # Set output name to 'start'
    # Generate backward-compatible Makefile
endfunction()
```

### 5. Installation and Packaging

#### Installation Types

**USER Installation**
- Complete installation to CMAKE_INSTALL_PREFIX
- Libraries, headers, binaries, data files
- Config scripts and pkg-config files
- CMake package configuration

**DEVEL Installation**
- Minimal installation for development
- Libraries remain in source tree
- Only utilities and config scripts installed
- Relative paths for in-tree development

#### Package Configuration

**CMake Package**
```cmake
write_basic_package_version_file()
install(FILES
    lpzrobotsConfig.cmake
    lpzrobotsConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/lpzrobots
)
```

**Config Scripts**
- Generated from templates (selforg-config.sh.in)
- Support for --opt, --dbg variants
- Include path and library management
- GSL integration

**pkg-config Files**
- Generated .pc files for each component
- Proper dependency chains
- Platform-specific flags

### 6. Dependency Handling

#### Unified Include Directory
```
include/
├── selforg/       # Symlinks to selforg headers
├── ode_robots/    # Symlinks to ode_robots headers
├── ga_tools/      # Symlinks to ga_tools headers
└── ode-dbl/       # ODE compatibility headers
```

#### ODE Handling
- System ODE: Uses FindODE.cmake module
- Bundled ODE: Headers-only for CMake builds
- Compatibility layer for ode-dbl includes

#### External Library Warnings
- SYSTEM includes for external headers
- Suppresses warnings from OSG, Qt, etc.

### 7. Build Variants and Legacy Support

#### Library Variants
- **Normal**: Default build configuration
- **_opt**: Optimized with -O3 -DNDEBUG
- **_dbg**: Debug with -g -O0
- Optional via LPZROBOTS_LEGACY_VARIANTS

#### Backward Compatibility
- Config scripts compatible with M4 versions
- Makefile generation for simulations
- Legacy install locations preserved

### 8. Testing Infrastructure

#### Test Discovery
```cmake
lpzrobots_add_doctest_suite(selforg)
lpzrobots_add_test(matrix_test
    SOURCES tests/matrix_test.cpp
    DEPS lpzrobots::selforg
)
```

#### Test Features
- Automatic test discovery
- Component-based organization
- Coverage support
- Sanitizer integration
- Benchmark support

### 9. Quality and Analysis Tools

#### Static Analysis
- clang-tidy integration
- cppcheck support
- include-what-you-use

#### Runtime Analysis
- AddressSanitizer
- ThreadSanitizer
- UndefinedBehaviorSanitizer
- MemorySanitizer (Clang only)

#### Coverage
- gcov/lcov support
- Per-component coverage reports

### 10. Platform-Specific Handling

#### macOS
- AGL framework removal
- RPATH handling
- Universal binary support
- Framework detection

#### Linux
- Standard paths
- pkg-config integration
- Distribution packaging support

## Build Process Flow

1. **Configuration Phase**
   - Version detection
   - Platform detection
   - Dependency finding
   - Option processing

2. **Component Addition**
   - selforg (always first)
   - opende (if needed)
   - configurator (before ode_robots)
   - ode_robots
   - ga_tools
   - GUI tools
   - Examples/simulations

3. **Target Creation**
   - Libraries with variants
   - Config script generation
   - Header symlinks
   - Installation rules

4. **Package Generation**
   - CMake config files
   - pkg-config files
   - CPack configuration

## Key Design Decisions

1. **Modular Architecture**: Separate concerns into focused modules
2. **Generator Expressions**: Clean build/install interface separation
3. **Modern CMake**: Target-based approach, no global settings
4. **Backward Compatibility**: Preserve legacy workflows
5. **Development Mode**: Support in-tree development
6. **Unified Functions**: Consistent API for component creation

## Usage Examples

### Basic Build
```bash
cmake -B build
cmake --build build -j8
```

### Development Build
```bash
cmake -B build -DLPZROBOTS_INSTALL_TYPE=DEVEL
cmake --build build
```

### With Options
```bash
cmake -B build \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_SHARED_LIBS=ON \
    -DENABLE_SIMD=ON \
    -DLPZROBOTS_ENABLE_SANITIZERS=ON
```

### Component Testing
```bash
cmake --build build --target selforg_tests
ctest --test-dir build -L selforg
```

## Advantages Over Legacy System

1. **Cross-Platform**: Native Windows support possible
2. **IDE Integration**: Full support for modern IDEs
3. **Dependency Management**: Automatic with find_package
4. **Parallel Builds**: Proper dependency tracking
5. **Modern Tooling**: Static analysis, sanitizers, coverage
6. **Package Management**: CPack, CMake packages, pkg-config
7. **Maintainability**: Clear, documented, modular structure

## Future Enhancements

1. **FetchContent**: For automatic dependency download
2. **CMake Presets**: For common configurations
3. **Unity Builds**: For faster compilation
4. **PCH Support**: Precompiled headers
5. **Conan/vcpkg**: Package manager integration
6. **CI/CD Templates**: GitHub Actions, GitLab CI