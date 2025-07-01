# LPZRobots CMake Migration Guide

This document describes the migration from the traditional Makefile-based build system to a modern CMake-based build system for LPZRobots.

## Overview

The LPZRobots project has been successfully migrated from a legacy Makefile-based build system to a modern, standard CMake build system. This migration provides:

- **Modern build practices**: Standard CMake with proper dependency management
- **Cross-platform compatibility**: Works seamlessly on Linux, macOS, and Windows
- **Better IDE integration**: Works with CLion, VS Code, Visual Studio, etc.
- **Parallel builds**: Faster compilation with automatic parallel job detection
- **Package management**: Proper find_package() support for integration with other projects
- **Modular builds**: Build only what you need (libraries, utilities, examples)

## Quick Start

### Prerequisites

- CMake 3.16 or higher
- C++17 compatible compiler (GCC 7+, Clang 5+, MSVC 2019+)
- Qt5 or Qt6 (optional, for GUI tools)
- GSL (GNU Scientific Library) - optional
- ODE (Open Dynamics Engine) - can use system version or bundled

### Building with the new system

#### Option 1: Using the build script (Recommended)

```bash
# Basic build (equivalent to old 'make all')
./build.sh --install

# Custom prefix
./build.sh --prefix ~/lpzrobots --install

# Debug build
./build.sh --build-type Debug

# Minimal build (libraries only)
./build.sh --no-utils --no-examples

# Clean build
./build.sh --clean --install
```

#### Option 2: Using CMake directly

```bash
# Configure
mkdir build && cd build
cmake -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_BUILD_TYPE=Release \
      -DBUILD_UTILS=ON \
      -DBUILD_EXAMPLES=ON \
      ..

# Build
cmake --build . --parallel

# Install
sudo cmake --install .
```

## Migration from Old System

### Old vs New Commands

| Old Makefile Command | New CMake Equivalent |
|---------------------|---------------------|
| `make all` | `./build.sh --install` |
| `make clean` | `./build.sh --clean` |
| `make selforg` | `cmake --build build --target selforg` |
| `make ode_robots` | `cmake --build build --target ode_robots` |
| `make guillogger` | `cmake --build build --target guilogger` |
| `make install` | `cmake --install build` |
| `make uninstall` | _(Not directly supported, manual removal required)_ |

### Configuration Options

| Old Makefile.conf | New CMake Option | Description |
|-------------------|------------------|-------------|
| `PREFIX=...` | `-DCMAKE_INSTALL_PREFIX=...` | Installation directory |
| `TYPE=USER/DEVEL` | `-DBUILD_EXAMPLES=ON/OFF` | Build examples/simulations |
| _(autodetected)_ | `-DBUILD_UTILS=ON/OFF` | Build GUI utilities |
| _(hardcoded)_ | `-DUSE_SYSTEM_ODE=ON/OFF` | Use system vs bundled ODE |
| _(not available)_ | `-DENABLE_SIMD=ON/OFF` | Enable SIMD optimizations |

## Build Options

### Core Options

- `BUILD_SHARED_LIBS` (ON): Build shared libraries instead of static
- `CMAKE_BUILD_TYPE` (Release): Debug, Release, RelWithDebInfo, MinSizeRel
- `CMAKE_INSTALL_PREFIX` (/usr/local): Where to install the software

### Component Options

- `BUILD_UTILS` (ON): Build utility tools (guilogger, matrixviz, configurator)
- `BUILD_EXAMPLES` (ON): Build and install example simulations
- `BUILD_TESTS` (OFF): Build unit tests
- `USE_SYSTEM_ODE` (ON): Use system ODE library instead of bundled version
- `ENABLE_SIMD` (ON): Enable SIMD optimizations when available

### Examples

```bash
# Minimal build (libraries only)
cmake -DBUILD_UTILS=OFF -DBUILD_EXAMPLES=OFF ..

# Development build with tests
cmake -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTS=ON ..

# Build with bundled ODE (if system ODE not available)
cmake -DUSE_SYSTEM_ODE=OFF ..

# Custom installation prefix
cmake -DCMAKE_INSTALL_PREFIX=~/lpzrobots ..
```

## Integration with Your Projects

### Using find_package()

If LPZRobots is installed system-wide:

```cmake
find_package(LPZRobots REQUIRED)

target_link_libraries(your_simulation
    lpzrobots::selforg
    lpzrobots::ode_robots
    lpzrobots::ga_tools  # if using genetic algorithms
)
```

### Using pkg-config (backward compatibility)

The traditional pkg-config files are still generated:

```bash
pkg-config --cflags selforg
pkg-config --libs ode_robots
```

### Manual Integration

If building against the build tree:

```cmake
list(APPEND CMAKE_PREFIX_PATH "/path/to/lpzrobots/build")
find_package(LPZRobots REQUIRED)
```

## Component Details

### Core Libraries

1. **selforg**: Self-organization controller framework
   - Target: `lpzrobots::selforg`
   - Headers: `#include <selforg/...>`

2. **ode_robots**: Physics simulation framework
   - Target: `lpzrobots::ode_robots`
   - Headers: `#include <ode_robots/...>`
   - Depends: selforg, ODE

3. **ga_tools**: Genetic algorithm framework
   - Target: `lpzrobots::ga_tools`
   - Headers: `#include <ga_tools/...>`
   - Depends: selforg

### Utility Tools (optional)

1. **configurator**: Configuration library
   - Target: `lpzrobots::configurator`
   - Requires: Qt5/Qt6, selforg

2. **guilogger**: GUI logging tool
   - Executable: `guilogger`
   - Requires: Qt5/Qt6

3. **matrixviz**: Matrix visualization tool
   - Executable: `matrixviz`
   - Requires: Qt5/Qt6, OpenGL

## Platform-Specific Notes

### macOS

- Automatically detects Homebrew installations (ODE, Qt, etc.)
- Handles Apple Silicon (ARM64) optimizations
- Resolves AGL framework conflicts with Qt
- Uses proper framework linking

### Linux

- Uses pkg-config for dependency detection
- Supports both Qt5 and Qt6
- Proper OpenGL/GLU linking

### Windows (if supported)

- Uses vcpkg or system packages
- Proper DLL handling for shared builds

## Troubleshooting

### Common Issues

1. **CMake too old**: Update to CMake 3.16+
2. **Qt not found**: Install Qt development packages
3. **ODE not found**: Install libode-dev or use `-DUSE_SYSTEM_ODE=OFF`
4. **GSL not found**: Install libgsl-dev or it will be disabled automatically

### Debug Build Issues

```bash
# Clean and rebuild
rm -rf build
./build.sh --build-type Debug --clean

# Verbose output
./build.sh --verbose
```

### Missing Dependencies

```bash
# Check what CMake found
cmake -LH build/

# Check specific package
find_package(Qt6 REQUIRED COMPONENTS Core Widgets)
```

## Performance Improvements

The new build system provides several performance improvements:

1. **Parallel builds**: Automatic detection of CPU cores
2. **Incremental builds**: Only rebuild changed files
3. **Better optimization**: Proper C++17 optimizations
4. **SIMD support**: Automatic SIMD detection and usage
5. **Dependency caching**: Faster subsequent builds

## Backwards Compatibility

### For Users

- Installation paths remain the same (`/usr/local` by default)
- Generated libraries have the same names and APIs
- Utility tools work identically
- Configuration files use the same format

### For Developers

- Header include paths remain the same
- Library names and symbols unchanged
- pkg-config files still generated
- Old configure scripts still work (generate compatibility warnings)

## Advanced Usage

### Cross-compilation

```bash
# For ARM64
cmake -DCMAKE_TOOLCHAIN_FILE=arm64-toolchain.cmake ..

# For specific target
cmake -DCMAKE_SYSTEM_NAME=Linux -DCMAKE_SYSTEM_PROCESSOR=aarch64 ..
```

### Custom Compiler Flags

```bash
cmake -DCMAKE_CXX_FLAGS="-march=native -O3" ..
```

### Integration with Package Managers

Works with modern package managers:
- vcpkg (Windows/Linux/macOS)
- Conan
- Hunter
- CPM

## Maintenance

The new build system is designed to be maintainable:

- Each component has its own CMakeLists.txt
- Clear separation of concerns
- Standard CMake patterns
- Automatic dependency detection
- Proper target export/import

## Support

For issues with the new build system:

1. Check this migration guide
2. Look at CMake output for specific errors
3. Use `--verbose` flag for detailed information
4. Check that all dependencies are properly installed

The old Makefile system remains available as a fallback, but is deprecated and will be removed in future versions.