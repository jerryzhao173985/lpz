# Build System Troubleshooting and Platform Guide

## Platform-Specific Build Instructions

### macOS (Apple Silicon M1/M2/M3)

#### Prerequisites
```bash
# Install Xcode Command Line Tools
xcode-select --install

# Install Homebrew (if not already installed)
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# Install dependencies
brew install cmake qt@6 open-scene-graph gsl readline gnuplot
brew install autoconf automake libtool  # For OpenDE
```

#### Build Commands
```bash
# Ensure Qt6 is in PATH
export PATH="/opt/homebrew/opt/qt@6/bin:$PATH"

# Option 1: Use preset (recommended)
cmake --preset=macos-arm64
cmake --build build/macos-arm64 -j8

# Option 2: Quick build
./lpz-build

# Option 3: Manual configuration
cmake -B build \
    -DCMAKE_OSX_ARCHITECTURES=arm64 \
    -DCMAKE_BUILD_TYPE=Release \
    -DQt6_DIR=/opt/homebrew/opt/qt@6/lib/cmake/Qt6
cmake --build build -j8
```

#### macOS-Specific Issues

1. **AGL Framework Error**
   - **Problem**: Qt configuration includes deprecated `-framework AGL`
   - **Solution**: Already handled by build system, but if persists:
   ```bash
   ./build_helpers/fix_qt_macos.sh
   ```

2. **OpenGL Deprecation Warnings**
   - **Status**: Normal on macOS, OpenGL is deprecated but functional
   - **Solution**: Warnings are suppressed in the build

3. **Shadow Rendering Crashes**
   - **Problem**: Some shadow techniques crash on macOS
   - **Solution**: Run simulations with `-noshadow` flag:
   ```bash
   ./start -noshadow
   ```

4. **Shared Library Issues**
   - **Problem**: `-soname` not supported on macOS
   - **Solution**: Build system automatically uses `-install_name`

### macOS (Intel x86_64)

```bash
# Same dependencies as ARM64
brew install cmake qt@6 open-scene-graph gsl readline gnuplot

# Build for Intel
cmake --preset=macos-x64
cmake --build build/macos-x64

# Or universal binary
cmake --preset=macos-universal
cmake --build build/macos-universal
```

### Linux (Ubuntu/Debian)

#### Prerequisites
```bash
# Update package list
sudo apt update

# Install build tools
sudo apt install build-essential cmake ninja-build

# Install dependencies
sudo apt install \
    qt6-base-dev qt6-tools-dev \
    libopenscenegraph-dev \
    libgsl-dev \
    libreadline-dev \
    libncurses5-dev \
    freeglut3-dev \
    libglu1-mesa-dev

# For Java components (optional)
sudo apt install default-jdk ant
```

#### Build Commands
```bash
# Standard build
cmake --preset=linux
cmake --build build/linux -j$(nproc)

# Or use the build script
./lpz-build

# Install system-wide
sudo cmake --install build/linux
```

### Linux (Arch/Manjaro)

```bash
# Install dependencies
sudo pacman -S base-devel cmake qt6-base openscenegraph \
    gsl readline ncurses mesa glu freeglut

# Build
./lpz-build
```

### Linux (Fedora/RHEL)

```bash
# Install dependencies
sudo dnf install cmake qt6-qtbase-devel OpenSceneGraph-devel \
    gsl-devel readline-devel ncurses-devel \
    freeglut-devel mesa-libGLU-devel

# Build
./lpz-build
```

### Windows (WSL2)

```bash
# Install WSL2 with Ubuntu
wsl --install -d Ubuntu

# Inside WSL2, follow Linux instructions
# Note: GUI apps require WSLg or X server
```

### Windows (Native) - Experimental

```powershell
# Install Visual Studio 2022 with C++ workload
# Install vcpkg
git clone https://github.com/Microsoft/vcpkg.git
.\vcpkg\bootstrap-vcpkg.bat

# Install dependencies
.\vcpkg\vcpkg install qt6 openscenegraph gsl

# Configure with CMake
cmake -B build -G "Visual Studio 17 2022" \
    -DCMAKE_TOOLCHAIN_FILE=vcpkg/scripts/buildsystems/vcpkg.cmake
```

## Common Build Issues and Solutions

### Dependency Issues

#### 1. Qt Not Found
```bash
# Check Qt installation
qmake --version  # Should show Qt 6.x

# Set Qt path explicitly
export Qt6_DIR=/opt/homebrew/opt/qt@6/lib/cmake/Qt6  # macOS
export Qt6_DIR=/usr/lib/x86_64-linux-gnu/cmake/Qt6   # Linux

# Or in CMake
cmake -B build -DQt6_DIR=/path/to/qt6/lib/cmake/Qt6
```

#### 2. OpenSceneGraph Not Found
```bash
# Install OSG
brew install open-scene-graph  # macOS
sudo apt install libopenscenegraph-dev  # Ubuntu

# Check installation
pkg-config --modversion openscenegraph
```

#### 3. GSL Not Found
```bash
# Install GSL
brew install gsl  # macOS
sudo apt install libgsl-dev  # Ubuntu

# Verify
gsl-config --version
```

### Compilation Errors

#### 1. Missing CMakeLists.txt
```bash
# Run the fix script
./fix_cmake_issues.sh

# Or manually create missing files
touch ga_tools/examples/CMakeLists.txt
touch ga_tools/simulations/CMakeLists.txt
```

#### 2. Duplicate add_subdirectory
```cmake
# In CMakeLists.txt, check before adding:
if(NOT TARGET selforg)
    add_subdirectory(selforg)
endif()
```

#### 3. Missing Source Files
```bash
# Common missing files and their solutions:

# agent.cpp missing
# Solution: It's in selforg/, ensure SOURCE list includes it

# wiredcontroller.cpp missing
# Solution: It's in selforg/, check CMakeLists.txt

# Fix by updating CMakeLists.txt:
set(SELFORG_SOURCES
    # ... other sources ...
    agent.cpp
    wiredcontroller.cpp
)
```

#### 4. Syntax Errors in Examples
```cpp
// Common issue: misplaced explicit keyword
// Wrong:
void doSomething() explicit { }

// Correct:
explicit MyClass() { }  // Only on constructors
```

### Linking Errors

#### 1. Undefined OpenGL Symbols
```bash
# macOS: Add framework
target_link_libraries(target "-framework OpenGL")

# Linux: Add GL library
target_link_libraries(target GL GLU)
```

#### 2. Missing pthread
```bash
# Add to CMakeLists.txt
find_package(Threads REQUIRED)
target_link_libraries(target Threads::Threads)
```

#### 3. Readline Library Issues
```bash
# Install readline
brew install readline  # macOS
sudo apt install libreadline-dev  # Ubuntu

# If still not found, set explicitly
cmake -B build -DREADLINE_ROOT=/opt/homebrew/opt/readline
```

### Runtime Issues

#### 1. Library Not Found
```bash
# macOS: Set DYLD_LIBRARY_PATH
export DYLD_LIBRARY_PATH=$HOME/lpzrobots/lib:$DYLD_LIBRARY_PATH

# Linux: Set LD_LIBRARY_PATH
export LD_LIBRARY_PATH=$HOME/lpzrobots/lib:$LD_LIBRARY_PATH

# Or use rpath (better solution)
cmake -B build -DCMAKE_INSTALL_RPATH_USE_LINK_PATH=ON
```

#### 2. Simulation Crashes
```bash
# Common fixes:
./start -noshadow  # Disable shadows
./start -pause     # Start paused
./start -noground  # Disable ground rendering

# Debug mode
gdb ./start
run -noshadow
bt  # Get backtrace when crashed
```

#### 3. Configuration Files Not Found
```bash
# Ensure config scripts are in PATH
export PATH=$HOME/lpzrobots/bin:$PATH

# Or use full paths
$(~/lpzrobots/bin/selforg-config --libs)
```

## Build System Features

### Debug Builds

```bash
# Debug configuration
cmake -B build-debug -DCMAKE_BUILD_TYPE=Debug
cmake --build build-debug

# With sanitizers
cmake -B build-asan \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_CXX_FLAGS="-fsanitize=address -fno-omit-frame-pointer"
cmake --build build-asan

# Run with sanitizers
ASAN_OPTIONS=detect_leaks=1 ./start
```

### Optimized Builds

```bash
# Release with debug info
cmake -B build-relwithdebinfo -DCMAKE_BUILD_TYPE=RelWithDebInfo
cmake --build build-relwithdebinfo

# Maximum optimization
cmake -B build-native \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_CXX_FLAGS="-O3 -march=native -mtune=native"
cmake --build build-native
```

### Parallel Builds

```bash
# Automatic detection
cmake --build build -j

# Specific number of jobs
cmake --build build -j8

# Using ninja (faster)
cmake -B build -G Ninja
ninja -C build
```

### Incremental Builds

```bash
# Only rebuild changed files
cmake --build build

# Rebuild specific target
cmake --build build --target selforg

# Clean specific target
cmake --build build --target selforg --clean-first
```

## Advanced Troubleshooting

### Using Build Logs

```bash
# Verbose build output
cmake --build build --verbose

# Save to file
cmake --build build --verbose 2>&1 | tee build.log

# CMake debug output
cmake -B build --debug-output
cmake -B build --trace  # Very verbose!
```

### Checking Configuration

```bash
# View all CMake variables
cmake -B build -LA

# View specific variable
cmake -B build -LA | grep -i qt

# Interactive ccmake
ccmake build  # Terminal UI for configuration
```

### Common CMake Cache Issues

```bash
# Clean CMake cache
rm -rf build/CMakeCache.txt build/CMakeFiles

# Full clean
rm -rf build
cmake -B build

# Force regeneration
touch CMakeLists.txt
cmake build
```

### Platform Detection

```cmake
# In CMakeLists.txt or debugging
message(STATUS "System: ${CMAKE_SYSTEM_NAME}")
message(STATUS "Processor: ${CMAKE_SYSTEM_PROCESSOR}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "Compiler Version: ${CMAKE_CXX_COMPILER_VERSION}")
```

## Environment Variables

### Build-Time Variables

```bash
# C++ Compiler
export CXX=clang++  # Use clang instead of g++

# Build type
export CMAKE_BUILD_TYPE=Debug

# Installation prefix
export CMAKE_INSTALL_PREFIX=$HOME/lpzrobots

# Generator
export CMAKE_GENERATOR="Ninja"  # Use Ninja instead of Make
```

### Runtime Variables

```bash
# Library paths
export LD_LIBRARY_PATH=$HOME/lpzrobots/lib:$LD_LIBRARY_PATH  # Linux
export DYLD_LIBRARY_PATH=$HOME/lpzrobots/lib:$DYLD_LIBRARY_PATH  # macOS

# Executable path
export PATH=$HOME/lpzrobots/bin:$PATH

# Qt plugin path (if needed)
export QT_PLUGIN_PATH=/opt/homebrew/opt/qt@6/plugins
```

## Quick Reference Card

### Build Commands
```bash
# Quick build
./lpz-build

# CMake presets
cmake --preset=default
cmake --build --preset=default

# Traditional
cmake -B build && cmake --build build

# Clean
cmake --build build --target clean
rm -rf build  # Full clean
```

### Common Fixes
```bash
# Fix permissions
chmod +x lpz-build
chmod +x configure

# Fix line endings (Windows)
dos2unix lpz-build
dos2unix configure

# Update submodules
git submodule update --init --recursive

# Reset to clean state
git clean -xfd  # Warning: removes all untracked files!
git reset --hard
```

### Debugging
```bash
# Check dependencies
ldd ./start  # Linux
otool -L ./start  # macOS

# Run with debugger
gdb ./start
lldb ./start  # macOS

# Memory debugging
valgrind ./start
ASAN_OPTIONS=detect_leaks=1 ./start  # If built with sanitizers
```

## Getting Help

1. **Check build output** - Read error messages carefully
2. **Use verbose mode** - `./lpz-build -v` or `cmake --build build --verbose`
3. **Check CMake cache** - `cat build/CMakeCache.txt | grep -i error`
4. **Platform guides** - Refer to platform-specific sections above
5. **Clean build** - When in doubt, `rm -rf build && ./lpz-build`

Remember: Most build issues are caused by missing dependencies or outdated build files. A clean build often resolves mysterious errors.