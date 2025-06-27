# LPZRobots Installation Guide

## Quick Start

### macOS (Homebrew)
```bash
brew tap lpzrobots/tap
brew install lpzrobots
```

### Ubuntu/Debian
```bash
sudo add-apt-repository ppa:lpzrobots/stable
sudo apt update
sudo apt install lpzrobots
```

### From Source (All Platforms)
```bash
git clone https://github.com/georgmartius/lpzrobots.git
cd lpzrobots
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
sudo make install
```

## Detailed Installation Instructions

### Prerequisites

#### Required Dependencies
- C++17 compatible compiler (GCC 8+, Clang 7+, MSVC 2019+)
- CMake 3.20 or higher
- POSIX Threads

#### Optional Dependencies
- Qt6 (for GUI tools)
- OpenSceneGraph 3.6+ (for 3D visualization)
- GSL (GNU Scientific Library)
- OpenMP (for parallelization)
- gnuplot (for plotting)

### Platform-Specific Instructions

#### macOS (Apple Silicon & Intel)

1. **Install Xcode Command Line Tools**
   ```bash
   xcode-select --install
   ```

2. **Install Homebrew** (if not already installed)
   ```bash
   /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
   ```

3. **Install Dependencies**
   ```bash
   brew install cmake qt@6 open-scene-graph gsl readline gnuplot
   ```

4. **Build from Source**
   ```bash
   git clone https://github.com/georgmartius/lpzrobots.git
   cd lpzrobots
   mkdir build && cd build
   cmake .. -DCMAKE_BUILD_TYPE=Release -DENABLE_SIMD=ON
   make -j$(sysctl -n hw.ncpu)
   sudo make install
   ```

#### Linux (Ubuntu/Debian)

1. **Install Build Tools**
   ```bash
   sudo apt update
   sudo apt install build-essential cmake git
   ```

2. **Install Dependencies**
   ```bash
   sudo apt install qtbase6-dev qt6-base-dev-tools \
                    libopenscenegraph-dev libgsl-dev \
                    libreadline-dev gnuplot
   ```

3. **Build from Source**
   ```bash
   git clone https://github.com/georgmartius/lpzrobots.git
   cd lpzrobots
   mkdir build && cd build
   cmake .. -DCMAKE_BUILD_TYPE=Release
   make -j$(nproc)
   sudo make install
   ```

#### Windows (vcpkg)

1. **Install vcpkg**
   ```powershell
   git clone https://github.com/Microsoft/vcpkg.git
   .\vcpkg\bootstrap-vcpkg.bat
   ```

2. **Install LPZRobots**
   ```powershell
   .\vcpkg\vcpkg install lpzrobots
   ```

### Build Options

| Option | Default | Description |
|--------|---------|-------------|
| `BUILD_SHARED_LIBS` | ON | Build shared libraries |
| `BUILD_EXAMPLES` | ON | Build example simulations |
| `BUILD_TESTS` | OFF | Build unit tests |
| `BUILD_GUI_TOOLS` | ON | Build GUI tools (requires Qt6) |
| `ENABLE_SIMD` | ON | Enable SIMD optimizations |
| `ENABLE_OPENMP` | ON | Enable OpenMP parallelization |

### Custom Build Example

```bash
cmake .. \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX=/opt/lpzrobots \
  -DBUILD_SHARED_LIBS=ON \
  -DBUILD_EXAMPLES=ON \
  -DBUILD_TESTS=ON \
  -DBUILD_GUI_TOOLS=ON \
  -DENABLE_SIMD=ON \
  -DENABLE_OPENMP=ON
```

### Verification

After installation, verify the installation:

```bash
# Check installed libraries
ls -la /usr/local/lib/liblpzrobots*

# Check headers
ls -la /usr/local/include/selforg/

# Run a simple example (if examples were built)
cd lpzrobots/ode_robots/simulations/template_sphererobot
./start
```

### Using LPZRobots in Your Project

#### CMake Project

```cmake
find_package(lpzrobots REQUIRED)

add_executable(my_robot_sim main.cpp)
target_link_libraries(my_robot_sim 
    lpzrobots::selforg
    lpzrobots::ode_robots
)
```

#### Makefile Project

```makefile
CXXFLAGS += $(shell pkg-config --cflags lpzrobots)
LDFLAGS += $(shell pkg-config --libs lpzrobots)
```

### Troubleshooting

#### macOS: "Library not loaded" Error
```bash
export DYLD_LIBRARY_PATH=/usr/local/lib:$DYLD_LIBRARY_PATH
```

#### Linux: Missing Dependencies
```bash
# Check missing libraries
ldd /usr/local/lib/liblpzrobots.so

# Install missing packages
sudo apt install --fix-missing
```

#### Build Errors
```bash
# Clean build
rm -rf build
mkdir build && cd build

# Verbose build for debugging
cmake .. -DCMAKE_VERBOSE_MAKEFILE=ON
make VERBOSE=1
```

### Docker Installation

```dockerfile
FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
    build-essential cmake git \
    qtbase6-dev libopenscenegraph-dev \
    libgsl-dev libreadline-dev

RUN git clone https://github.com/georgmartius/lpzrobots.git && \
    cd lpzrobots && \
    mkdir build && cd build && \
    cmake .. && make -j$(nproc) && \
    make install

ENV LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH
```

### Support

- GitHub Issues: https://github.com/georgmartius/lpzrobots/issues
- Documentation: https://robot.informatik.uni-leipzig.de/software/
- Mailing List: lpzrobots@informatik.uni-leipzig.de