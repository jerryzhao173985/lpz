# LPZRobots Build System Guide

## Overview

LPZRobots features a modern CMake build system alongside the legacy Make system for backward compatibility. This guide covers everything you need to know about building the project.

## Quick Start

### One-Command Build (Recommended)

```bash
# Build everything with sensible defaults
./lpz-build

# Build and install to ~/.local
./lpz-build && ./lpz-build install
```

### CMake Presets (Modern Approach)

```bash
# For macOS ARM64 (Apple Silicon)
cmake --preset=macos-arm64
cmake --build build/macos-arm64

# For Linux
cmake --preset=linux
cmake --build build/linux

# For development with debug symbols
cmake --preset=dev
cmake --build build/dev
```

### Traditional CMake

```bash
# Configure
cmake -B build -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build build -j8

# Install
cmake --install build --prefix ~/.local
```

### Legacy Make System (Still Supported)

```bash
# Use original interactive configuration
make

# Or use the build wrapper in legacy mode
./lpz-build legacy
```

## Build System Architecture

### Component Structure

```
┌─────────────┐     ┌──────────────┐     ┌───────────────┐
│   selforg   │     │   opende     │     │  ga_tools     │
│ Controllers │     │  Physics     │     │ Genetic Alg   │
└─────┬───────┘     └──────┬───────┘     └───────┬───────┘
      └──────────┬──────────┴─────────────────────┘
                 │
           ┌─────▼──────┐
           │ ode_robots │
           │ Simulation │
           └─────┬──────┘
                 │
    ┌────────────┼────────────┬──────────────┐
┌───▼────┐  ┌───▼─────┐  ┌───▼────┐   ┌────▼─────┐
│guilogger│  │matrixviz│  │config- │   │simulations│
│  (Qt6)  │  │  (Qt6)  │  │urator  │   │ Examples │
└─────────┘  └─────────┘  └────────┘   └──────────┘
```

### Build Dependencies Order

```
selforg → configurator → ode (compatibility) → ode_robots → ga_tools
         ↓
     GUI tools (parallel)
```

### CMake Module Architecture

The CMake system is organized into modular components:

- `cmake/LPZRobotsCore.cmake` - Main configuration and setup
- `cmake/LPZRobotsCompiler.cmake` - Compiler flags and warnings
- `cmake/LPZRobotsDependencies.cmake` - External dependency management
- `cmake/LPZRobotsLibrary.cmake` - Unified library creation functions
- `cmake/LPZRobotsPlatform.cmake` - Platform-specific detection
- `cmake/LPZRobotsTargets.cmake` - Helper functions
- `cmake/SimulationBuild.cmake` - Simulation build support

## Platform-Specific Instructions

### macOS (Apple Silicon & Intel)

```bash
# Install dependencies
brew install cmake qt@6 open-scene-graph gsl readline gnuplot

# Option 1: Use preset
cmake --preset=macos-arm64
cmake --build build/macos-arm64

# Option 2: Use build script
./lpz-build

# Option 3: Traditional
cmake -B build -DCMAKE_OSX_ARCHITECTURES=arm64
cmake --build build
```

### Linux

```bash
# Install dependencies (Ubuntu/Debian)
sudo apt install build-essential cmake qt6-base-dev \
    libopenscenegraph-dev libgsl-dev libreadline-dev

# Build
./lpz-build
```

### Windows (WSL2 Recommended)

```bash
# Use WSL2 with Ubuntu
# Follow Linux instructions above
```

## Build Options

### CMake Options

| Option | Default | Description |
|--------|---------|-------------|
| `BUILD_SHARED_LIBS` | ON | Build shared libraries |
| `BUILD_EXAMPLES` | ON | Build example simulations |
| `BUILD_TESTS` | OFF | Build unit tests |
| `BUILD_GUI_TOOLS` | ON | Build GUI applications |
| `BUILD_JAVA_TOOLS` | ON | Build Java components |
| `ENABLE_SIMD` | ON | Enable SIMD optimizations |
| `ENABLE_OPENMP` | ON | Enable OpenMP parallelization |
| `LPZROBOTS_USE_SYSTEM_ODE` | OFF | Use system ODE instead of compatibility |

### Setting Options

```bash
# Via command line
cmake -B build -DBUILD_GUI_TOOLS=OFF -DBUILD_TESTS=ON

# Via build script
./lpz-build --no-gui --type Debug

# Via environment
export CMAKE_BUILD_TYPE=Debug
./lpz-build
```

## Using the lpz-build Script

The `lpz-build` script provides a unified interface to both build systems:

### Basic Usage

```bash
# Show help
./lpz-build --help

# Build with default settings
./lpz-build

# Build with custom prefix
./lpz-build --prefix /usr/local

# Debug build
./lpz-build --type Debug

# Build without GUI tools
./lpz-build --no-gui

# Clean build
./lpz-build clean

# Run tests
./lpz-build test

# Create package
./lpz-build package
```

### Advanced Options

```bash
# Use specific number of parallel jobs
./lpz-build -j 4

# Verbose output
./lpz-build -v

# Clean before building
./lpz-build -c

# Use legacy Make system
./lpz-build --make legacy

# Force Qt6 (no Qt5 fallback)
./lpz-build --qt6
```

## Development Workflow

### 1. Initial Setup

```bash
# Clone repository
git clone https://github.com/georgmartius/lpzrobots.git
cd lpzrobots

# Install dependencies (macOS)
brew install cmake qt@6 open-scene-graph gsl

# Build for development
./lpz-build --type Debug
```

### 2. Making Changes

```bash
# Edit source files
# ...

# Incremental build (only rebuilds changed files)
./lpz-build

# Or use CMake directly
cmake --build build
```

### 3. Testing

```bash
# Run tests
./lpz-build test

# Run specific simulation
cd ode_robots/simulations/template_sphererobot
make
./start
```

### 4. Creating Packages

```bash
# Create distribution package
./lpz-build package

# Or use CPack directly
cd build
cpack -C Release
```

## Creating New Simulations

### Using CMake (Modern Approach)

Create a `CMakeLists.txt` file:

```cmake
cmake_minimum_required(VERSION 3.16)
project(my_robot_sim)

# Find LPZRobots
find_package(lpzrobots REQUIRED)

# Add simulation executable
lpzrobots_add_simulation(my_robot_sim
    SOURCES main.cpp
    LIBRARIES 
        lpzrobots::ode_robots
        lpzrobots::selforg
)
```

### Using Traditional Makefile

```bash
# Use the creation script
./createNewSimulation.sh template_sphererobot my_experiment

# Or copy a template manually
cp -r ode_robots/simulations/template_sphererobot ode_robots/simulations/my_experiment
cd ode_robots/simulations/my_experiment
# Edit main.cpp and Makefile
make
```

## Backward Compatibility

### Using Legacy Build System

The original Make-based system is fully preserved:

```bash
# Interactive configuration
make

# Answer prompts for prefix and type
# Then build
make all
```

Or use the wrapper:

```bash
./lpz-build --make legacy
```

### Legacy Config Scripts

The component config scripts (selforg-config, ode_robots-config) are still generated and installed for compatibility with existing projects.

## Troubleshooting

### Common Issues and Solutions

1. **Qt Not Found**
   ```bash
   # Ensure Qt is in PATH
   export PATH="/opt/homebrew/opt/qt@6/bin:$PATH"
   ```

2. **ODE Headers Missing**
   ```bash
   # Install ODE (macOS)
   brew install ode
   ```

3. **Build Errors in ga_tools**
   ```bash
   # Clean and rebuild
   ./lpz-build clean
   ./lpz-build
   ```

4. **OpenGL Deprecation Warnings (macOS)**
   ```bash
   # Normal on macOS, OpenGL is deprecated but still works
   # Warnings are suppressed in the build
   ```

5. **Missing CMakeLists.txt files**
   - Run: `./fix_cmake_issues.sh`

6. **AGL Framework Error (macOS)**
   - Already fixed in the build system
   - If persists, check `guilogger/configure` script

7. **Java Components Build Errors**
   ```bash
   # Enable Java support first
   ./enable_java.sh
   cmake --preset default-with-java
   ```

### Debugging Build Issues

```bash
# Use verbose mode
./lpz-build -v

# Check CMake cache
cmake -B build -LA

# Clean build
./lpz-build clean

# Check generated config
cat build/CMakeCache.txt | grep -i error
```

## Advanced Usage

### Custom Build Configurations

Create your own preset in `CMakeUserPresets.json`:

```json
{
    "version": 6,
    "configurePresets": [
        {
            "name": "my-config",
            "inherits": "default",
            "cacheVariables": {
                "CMAKE_CXX_FLAGS": "-O3 -march=native",
                "CMAKE_INSTALL_PREFIX": "/opt/lpzrobots"
            }
        }
    ]
}
```

### Integration with IDEs

The build system generates `compile_commands.json` for IDE integration:

- **VS Code**: Automatically detected
- **CLion**: Import as CMake project
- **Xcode**: `cmake -G Xcode`

### Continuous Integration

Example GitHub Actions workflow:

```yaml
- name: Configure
  run: cmake --preset=ci

- name: Build
  run: cmake --build --preset=ci

- name: Test
  run: ctest --preset=ci
```

## Summary

The modernized build system provides:

1. **Simplicity**: One-command builds with `./lpz-build`
2. **Flexibility**: Full CMake power when needed
3. **Compatibility**: Legacy Make system still works
4. **Performance**: Parallel builds by default
5. **Portability**: Works on macOS, Linux, Windows (WSL)

For most users, simply running `./lpz-build` will handle everything automatically.