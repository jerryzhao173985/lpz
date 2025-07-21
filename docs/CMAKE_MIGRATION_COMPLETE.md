# LPZRobots CMake Migration - Complete Guide

## Table of Contents
1. [Overview](#overview)
2. [Quick Start](#quick-start)
3. [Migration from Make](#migration-from-make)
4. [Build System Features](#build-system-features)
5. [Common Workflows](#common-workflows)
6. [Troubleshooting](#troubleshooting)
7. [Advanced Topics](#advanced-topics)

## Overview

LPZRobots has migrated from a legacy Make/M4 build system to modern CMake (3.20+). This guide provides everything you need to know about using the new build system.

### Key Benefits
- **Out-of-source builds** - keeps source tree clean
- **Multiple configurations** - debug/release/custom in parallel
- **Better dependency management** - automatic detection and clear errors
- **Cross-platform** - unified build on macOS/Linux
- **Modern tooling** - presets, sanitizers, static analysis
- **Faster builds** - better parallelization and caching

## Quick Start

### First Build (Recommended)
```bash
# Clone the repository
git clone https://github.com/georgmartius/lpzrobots.git
cd lpzrobots

# Build everything with presets (easiest)
cmake --preset release
cmake --build --preset release
cmake --install build/release --prefix ~/lpzrobots

# Add to PATH
echo 'export PATH="$HOME/lpzrobots/bin:$PATH"' >> ~/.bashrc
source ~/.bashrc
```

### Alternative: Manual Build
```bash
# Configure
cmake -B build -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build build -j8

# Install
cmake --install build --prefix ~/lpzrobots
```

## Migration from Make

### Command Mapping

| Old Make Command | New CMake Command | Notes |
|-----------------|-------------------|-------|
| `make conf` | `cmake -B build` | Configuration step |
| `make` | `cmake --build build` | Build all |
| `make opt` | `cmake --build build --config Release` | Optimized build |
| `make dbg` | `cmake --build build --config Debug` | Debug build |
| `make clean` | `cmake --build build --target clean` | Clean artifacts |
| `make install` | `cmake --install build` | Install files |
| `make selforg` | `cmake --build build --target selforg` | Build component |
| `make -j4` | `cmake --build build -j4` | Parallel build |

### Key Differences

1. **Build Location**
   - Make: Builds in source tree
   - CMake: Builds in separate `build/` directory

2. **Configuration**
   - Make: Edit `Makefile.conf` manually
   - CMake: Use `-D` options or CMake GUI

3. **Dependencies**
   - Make: Manual configuration
   - CMake: Automatic detection with helpful errors

4. **Clean Builds**
   - Make: `make clean` (partial)
   - CMake: `rm -rf build/` (complete)

## Build System Features

### Configuration Options

```bash
# Core options
-DCMAKE_BUILD_TYPE=Release|Debug|RelWithDebInfo
-DCMAKE_INSTALL_PREFIX=/path/to/install
-DBUILD_SHARED_LIBS=ON|OFF

# Component selection
-DBUILD_GUI_TOOLS=ON|OFF         # guilogger, matrixviz
-DBUILD_EXAMPLES=ON|OFF          # example simulations
-DBUILD_TESTS=ON|OFF             # unit tests
-DBUILD_ALL_EXAMPLES=ON|OFF      # all simulations

# Features
-DENABLE_SIMD=ON|OFF             # SIMD optimizations
-DENABLE_OPENMP=ON|OFF           # OpenMP parallelization
-DENABLE_JAVA_BRIDGE=ON|OFF      # configurator Java bridge

# Dependencies
-DLPZROBOTS_USE_SYSTEM_ODE=ON|OFF  # Use system ODE
```

### CMake Presets

Presets provide pre-configured build settings:

```bash
# List available presets
cmake --list-presets

# Common presets
cmake --preset release           # Optimized build
cmake --preset debug            # Debug build
cmake --preset macos-arm64      # Apple Silicon
cmake --preset linux            # Linux with OpenMP
cmake --preset minimal          # Core components only
cmake --preset ci              # CI/CD configuration
```

### Build Targets

```bash
# Build everything
cmake --build build

# Build specific component
cmake --build build --target selforg
cmake --build build --target ode_robots
cmake --build build --target guilogger

# Build and run tests
cmake --build build --target test

# Install
cmake --build build --target install

# Clean
cmake --build build --target clean
cmake --build build --target clean-all  # Remove build directory
```

## Common Workflows

### Development Workflow

```bash
# 1. Configure for development
cmake --preset dev

# 2. Build with verbose output
cmake --build build/dev --verbose

# 3. Run tests
ctest --test-dir build/dev --output-on-failure

# 4. Install locally for testing
cmake --install build/dev --prefix ~/.local
```

### Debug Workflow

```bash
# 1. Debug build with sanitizers
cmake -B build/debug \
  -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_CXX_FLAGS="-fsanitize=address -fsanitize=undefined"

# 2. Build
cmake --build build/debug

# 3. Run with debugging
ASAN_OPTIONS=detect_leaks=1 gdb ./build/debug/bin/program
```

### Release Workflow

```bash
# 1. Clean release build
rm -rf build/release
cmake --preset release

# 2. Build with multiple cores
cmake --build build/release -j$(nproc)

# 3. Run tests
ctest --preset release

# 4. Create distribution
cmake --install build/release --prefix dist/lpzrobots-$(date +%Y%m%d)
tar -czf lpzrobots-$(date +%Y%m%d).tar.gz dist/lpzrobots-$(date +%Y%m%d)
```

### Simulation Development

```bash
# 1. Create new simulation from template
cd ode_robots/simulations
cp -r template_sphererobot my_simulation
cd my_simulation

# 2. Build just your simulation
cmake -B build -DCMAKE_PREFIX_PATH=~/lpzrobots
cmake --build build

# 3. Run simulation
./build/start -nohud
```

## Troubleshooting

### Common Issues

1. **Missing Dependencies**
   ```
   CMake Error: Could not find Qt6
   ```
   Solution: Install Qt6 or disable GUI tools:
   ```bash
   cmake -B build -DBUILD_GUI_TOOLS=OFF
   ```

2. **Build Errors After Update**
   ```bash
   # Clean rebuild
   rm -rf build/
   cmake --preset release
   cmake --build --preset release
   ```

3. **Can't Find lpzrobots**
   ```bash
   # Ensure installation directory is in PATH
   export PATH="$HOME/lpzrobots/bin:$PATH"
   
   # Or specify prefix during simulation build
   cmake -B build -DCMAKE_PREFIX_PATH=~/lpzrobots
   ```

4. **Slow Builds**
   ```bash
   # Use Ninja for faster builds
   cmake --preset ninja
   cmake --build build/ninja
   
   # Enable ccache
   export CCACHE_DIR=~/.ccache
   cmake -B build -DCMAKE_CXX_COMPILER_LAUNCHER=ccache
   ```

### Platform-Specific

#### macOS
- Ensure Xcode Command Line Tools installed
- Use Homebrew for dependencies
- For Apple Silicon: use `macos-arm64` preset

#### Linux
- Install development packages (`-dev` or `-devel`)
- Enable OpenMP for better performance
- Check `pkg-config` paths

## Advanced Topics

### Custom Build Configurations

Create custom presets in `CMakeUserPresets.json`:
```json
{
  "version": 3,
  "configurePresets": [
    {
      "name": "my-config",
      "inherits": "base",
      "cacheVariables": {
        "CMAKE_BUILD_TYPE": "Release",
        "ENABLE_SIMD": "ON",
        "BUILD_GUI_TOOLS": "OFF"
      }
    }
  ]
}
```

### Cross-Compilation

```bash
# Example: Build for different architecture
cmake -B build \
  -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/arm-linux.cmake \
  -DCMAKE_BUILD_TYPE=Release
```

### Integration with IDEs

#### VS Code
```bash
# Generate compile_commands.json
cmake -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

# Use CMake Tools extension
```

#### CLion
- Open project root
- CLion auto-detects CMakeLists.txt
- Configure toolchains in settings

### Using Config Scripts

The traditional config scripts still work:
```bash
# After installation
selforg-config --cflags
ode_robots-config --libs

# In makefiles
CXXFLAGS += $(shell selforg-config --cflags)
LDFLAGS += $(shell ode_robots-config --libs)
```

## Summary

The CMake migration provides a modern, maintainable build system while preserving the familiar LPZRobots workflow. Key points:

1. **Use presets** for quick configuration
2. **Build out-of-source** in `build/` directory
3. **Install locally** to `~/lpzrobots` for development
4. **Clean rebuilds** with `rm -rf build/` when needed
5. **Leverage CMake features** like parallel builds and better dependency management

For component-specific details, see:
- [Build System Overview](CMAKE_BUILD_SYSTEM_OVERVIEW.md)
- [Simulation Building Guide](guides/CMAKE_SIMULATION_GUIDE.md)
- [Migration Guide](guides/MIGRATION_GUIDE_CMAKE.md)

For help: create an issue on GitHub or consult the online documentation.