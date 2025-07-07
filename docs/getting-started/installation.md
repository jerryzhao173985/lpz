# Installation Guide

This guide provides detailed installation instructions for LPZRobots on different platforms.

## System Requirements

### Minimum Requirements
- **C++ Compiler**: C++17 support required (clang++ 14+ or g++ 11+)
- **Memory**: 4GB RAM minimum, 8GB recommended
- **Disk Space**: 2GB for full installation

### Platform Support
- **macOS**: ARM64 (Apple Silicon) and x86_64
- **Linux**: x86_64 (ARM64 experimental)
- **Windows**: Experimental via WSL2

## Dependencies

### Core Dependencies
- **Qt6**: GUI framework (6.2+)
- **OpenSceneGraph**: 3D visualization (3.6+)
- **GSL**: GNU Scientific Library
- **OpenGL**: 3D graphics
- **readline**: Console interaction

### Optional Dependencies
- **gnuplot**: Additional plotting capabilities
- **Java**: For javacontroller (optional)

## Platform-Specific Installation

### macOS (Homebrew)

```bash
# Install Xcode Command Line Tools
xcode-select --install

# Install Homebrew if not already installed
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# Install dependencies
brew install qt@6 open-scene-graph gsl readline gnuplot
brew install autoconf automake libtool cmake ninja

# For development tools
brew install llvm  # For clang-tidy and clang-format
```

### Linux (Ubuntu/Debian)

```bash
# Update package manager
sudo apt update

# Install dependencies
sudo apt install build-essential cmake ninja-build
sudo apt install qt6-base-dev qt6-tools-dev libqt6opengl6-dev
sudo apt install libopenscenegraph-dev libgsl-dev
sudo apt install libreadline-dev libncurses5-dev
sudo apt install libglu1-mesa-dev freeglut3-dev

# Optional tools
sudo apt install gnuplot clang-tidy clang-format
```

### Linux (Arch)

```bash
# Install dependencies
sudo pacman -S base-devel cmake ninja
sudo pacman -S qt6-base openscenegraph gsl
sudo pacman -S readline ncurses glu freeglut
sudo pacman -S gnuplot clang
```

## Building LPZRobots

### Option 1: CMake Build (Recommended)

```bash
# Clone the repository
git clone https://github.com/georgmartius/lpzrobots.git
cd lpzrobots

# Configure with CMake
cmake --preset macos-arm64  # For Apple Silicon
# OR
cmake --preset linux        # For Linux
# OR
cmake --preset macos-x64    # For Intel Mac

# Build
cmake --build --preset default

# Install (optional)
cmake --install --preset default --prefix ~/lpzrobots
```

### Option 2: Traditional Make Build

```bash
# Clone the repository
git clone https://github.com/georgmartius/lpzrobots.git
cd lpzrobots

# Configure installation
make conf
# When prompted:
# - Choose installation directory (e.g., ~/lpzrobots)
# - Choose 'u' for user installation

# Build everything
make all -j$(nproc)  # Linux
make all -j$(sysctl -n hw.ncpu)  # macOS

# Or build components individually
make selforg        # Core controller library
make ode           # Physics engine
make ode_robots    # Robot simulation framework
make utils         # GUI tools
```

## Verifying Installation

### Test Basic Functionality

```bash
# Add to PATH (adjust path as needed)
export PATH=$HOME/lpzrobots/bin:$PATH

# Test GUI tools
guilogger --version
matrixviz --version

# Run example simulation
cd ode_robots/simulations/template_sphererobot
make
./start -g  # Run with guilogger
```

### Common Issues

#### macOS: Shadow Rendering Crash
```bash
# Use -noshadow flag
./start -noshadow -g
```

#### Missing Dependencies
```bash
# Check which dependencies are missing
ldd ./start  # Linux
otool -L ./start  # macOS
```

#### Build Errors
```bash
# Clean and rebuild
make clean
make all -j4
```

## Environment Setup

### Shell Configuration

Add to your `~/.bashrc` or `~/.zshrc`:

```bash
# LPZRobots environment
export LPZROBOTS_HOME=$HOME/lpzrobots
export PATH=$LPZROBOTS_HOME/bin:$PATH
export LD_LIBRARY_PATH=$LPZROBOTS_HOME/lib:$LD_LIBRARY_PATH  # Linux
export DYLD_LIBRARY_PATH=$LPZROBOTS_HOME/lib:$DYLD_LIBRARY_PATH  # macOS
```

### IDE Setup

#### VS Code
```json
// .vscode/c_cpp_properties.json
{
    "configurations": [{
        "name": "LPZRobots",
        "includePath": [
            "${workspaceFolder}/selforg/include",
            "${workspaceFolder}/ode_robots/include"
        ],
        "compilerPath": "/usr/bin/clang++",
        "cStandard": "c11",
        "cppStandard": "c++17"
    }]
}
```

## Next Steps

- Follow the [Quick Start Tutorial](quick-start.md) to create your first simulation
- Explore [Example Simulations](examples.md) to see what's possible
- Read about [Available Controllers](../user-guide/controllers.md)

## Troubleshooting

For common issues and solutions, see:
- [Build System Documentation](../developer-guide/build-system.md)
- [Known Issues](../maintenance/known-issues.md)
- [GitHub Issues](https://github.com/georgmartius/lpzrobots/issues)