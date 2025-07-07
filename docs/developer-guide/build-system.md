# Build System Guide

This guide covers both the modern CMake build system and the legacy Make/M4 system.

## Build System Overview

LPZRobots supports two build systems:
1. **CMake** (recommended) - Modern, cross-platform build system
2. **Make/M4** (legacy) - Original build system, still maintained

## CMake Build System

### Quick Start

```bash
# Configure
cmake --preset macos-arm64  # or linux, macos-x64

# Build
cmake --build --preset default

# Install
cmake --install --preset default --prefix ~/lpzrobots
```

### CMake Presets

The project uses CMakePresets.json for standardized configurations:

#### Platform Presets
- `macos-arm64` - Apple Silicon Macs
- `macos-x64` - Intel Macs
- `linux` - Linux systems
- `macos-universal` - Universal binary for macOS

#### Build Type Presets
- `debug` - Debug symbols, no optimization
- `release` - Full optimization
- `release-with-debug` - Optimized with debug info
- `dev` - Development mode with all warnings

### CMake Options

```cmake
option(BUILD_SHARED_LIBS "Build shared libraries" ON)
option(BUILD_EXAMPLES "Build example simulations" ON)
option(BUILD_TESTS "Build unit tests" OFF)
option(BUILD_GUI_TOOLS "Build GUI tools" ON)
option(ENABLE_SIMD "Enable SIMD optimizations" ON)
option(ENABLE_OPENMP "Enable OpenMP parallelization" ON)
option(WITH_JAVA "Build Java controller support" OFF)
```

### Custom Build Configuration

```bash
# Manual configuration
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release \
         -DBUILD_TESTS=ON \
         -DCMAKE_INSTALL_PREFIX=/usr/local

# Build specific target
cmake --build . --target selforg

# Run tests
ctest --output-on-failure
```

## Make/M4 Build System (Legacy)

### Initial Configuration

```bash
# Run configuration
make conf

# This creates Makefile.conf with:
# - PREFIX: Installation directory
# - TYPE: DEVEL or USER installation
```

### Build Commands

```bash
# Build everything
make all -j$(nproc)

# Build individual components
make selforg        # Core controller library
make ode           # Physics engine  
make ode_robots    # Robot simulation framework
make utils         # GUI tools (guilogger, matrixviz)
make ga_tools      # Genetic algorithm tools
make configurator  # Configuration GUI

# Optimized build
make opt

# Clean build
make clean
make distclean     # Also removes configuration
```

### Build Variants

The Make system creates multiple library variants:
- `libselforg.a` - Static library (debug)
- `libselforg_opt.a` - Static library (optimized)
- `libselforg.so` - Shared library
- `libselforg_dbg.a` - Debug library with symbols

### Component Dependencies

Build order matters in the Make system:
1. `selforg` - Must be built first
2. `ode` - Can be built in parallel with selforg
3. `ode_robots` - Requires selforg and ode
4. `utils` - Can be built after selforg
5. `ga_tools` - Requires selforg
6. `configurator` - Requires selforg

## Platform-Specific Notes

### macOS

#### Apple Silicon (ARM64)
- Native NEON optimizations enabled automatically
- Use `-noshadow` flag when running simulations
- Ensure Homebrew paths are in environment

#### Framework Issues
- AGL framework removed (deprecated)
- Qt6 requires proper framework paths
- Use provided helper scripts for Qt issues

### Linux

#### RPATH Configuration
- Libraries use RPATH for installed locations
- Development builds use relative paths
- Use `LD_LIBRARY_PATH` for custom locations

### Windows

- Experimental support via CMake
- Recommended: Use WSL2 with Linux build

## Simulation Building

### Using Make

```bash
cd ode_robots/simulations/my_simulation
make
./start -g  # Run with guilogger
```

### Using CMake

```cmake
# In simulation CMakeLists.txt
lpzrobots_add_simulation(my_simulation
    SOURCES main.cpp MyRobot.cpp
    EXTRA_LIBS my_custom_lib
)
```

## Troubleshooting

### Common Build Errors

#### Missing Dependencies
```bash
# Check configuration
make -p | grep "not found"

# Verify paths
echo $PKG_CONFIG_PATH
pkg-config --modversion qt6
```

#### Compiler Errors
```bash
# Use verbose mode
make VERBOSE=1

# Check compiler flags
make -n selforg | head
```

#### Link Errors
```bash
# Check library paths
make print-LDFLAGS

# Verify library creation
ls -la selforg/*.a
```

### Build System Debugging

#### Make System
```bash
# Print all variables
make print-VARIABLE_NAME

# Trace execution
make --trace

# Dry run
make -n target
```

#### CMake System
```bash
# Verbose output
cmake --build . --verbose

# Show all cache variables
cmake -LAH

# Trace mode
cmake --trace-source=CMakeLists.txt
```

## Advanced Topics

### Cross-Compilation

```cmake
# CMake toolchain file
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64)
set(CMAKE_C_COMPILER aarch64-linux-gnu-gcc)
set(CMAKE_CXX_COMPILER aarch64-linux-gnu-g++)
```

### Custom Installation

```bash
# Make system
make install PREFIX=/custom/path

# CMake system  
cmake --install . --prefix /custom/path --component Runtime
```

### Development Mode

```bash
# In-tree development (no installation)
cd lpzrobots
make selforg
export LPZROBOTS_HOME=$(pwd)
```

## Migration from Make to CMake

If migrating existing projects:

1. Create `CMakeLists.txt` in simulation directory
2. Use `lpzrobots_add_simulation()` helper
3. Remove old Makefile (keep for reference)
4. Update include paths to use targets

Example migration:
```cmake
# Old Makefile approach
# CPPFLAGS += -I$(ODEROBOTS)/include

# New CMake approach
target_link_libraries(my_sim PRIVATE lpzrobots::ode_robots)
```

## Best Practices

1. **Use CMake** for new projects
2. **Keep builds out-of-source** 
3. **Use presets** for consistency
4. **Enable warnings** in development
5. **Test multiple configurations**
6. **Document build requirements**

## Further Reading

- [CMake Module Documentation](cmake-modules.md)
- [Creating Simulations](../user-guide/simulations.md)
- [Testing Guide](testing.md)