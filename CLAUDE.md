# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## CRITICAL: macOS ARM64 Migration with QT5, standard C++17 and Makefile system

1. **C++ Modernization**
   - Fix deprecated unary_function usage
   - Fix namespace conflicts
   - Resolve C++17 header conflicts

### 📋 Remaining Tasks 
- Complete Qt5 migration for configurator
- Create binary distribution for macOS (ARM64)

**PROJECT GOAL**: Migrate LPZRobots to compile and run correctly on macOS ARM64 (Apple Silicon M4) with the following requirements:
- Advanced and correct usage of Qt5 
- Adapt original Make/m4 build systemto modern standard
- Adopt C++11->C++17 standards progressively
- Ensure compatibility with clang/clang++ on macOS
- Maintain all existing functionality
- Follow modern C++ best practices / styles

**IMPORTANT**: Enhance original build system architecture as much as possible.

## Project Overview

LPZRobots is a comprehensive C++ robotics simulation framework focused on self-organizing control algorithms. It uses ODE (Open Dynamics Engine) for physics simulation and OpenSceneGraph for visualization.

### Current State
- **Build System**: CMake
- **C++ Standard**: C++11 (migrating to C++17)
- **GUI Framework**: Qt5
- **Platform**: Originally Linux, macOS ARM64 support added
- **Dependencies**: ODE, OpenSceneGraph, Qt, GSL, readline
- **Refactoring**: Code cleaning and refactoring needed

## Migration 

### 1. Build System Migration (CMake)
- Enhance and adapt to CMakeLists.txt
- Use modern CMake (4+) features
- Support both Homebrew and manual builds
- Add proper dependency detection for ARM64

### 2. macOS ARM64 Compatibility
- Fix hardcoded paths (/opt/local → /opt/homebrew)
- Fix hardcoded complete paths as per different machines
- Optimize ARM64 system with best proper framework 

### 3. C++17 Modernization
- Replace deprecated features and C++ warnings 
- Check properly on C++ with cppcheck or valgrind
- Use common standard library (std::optional, std::filesystem)
- Update to modern STL containers, STL templates etc
- Use structured bindings where appropriate

## Build System Architecture

### Overview
LPZRobots uses a sophisticated hierarchical build system based on:
- **GNU Make**: Orchestrates the overall build process
- **m4 Macro Processor**: Generates platform-specific configuration scripts
- **Autoconf/Automake**: Used by opende component
- **qmake**: Used by Qt-based GUI tools (guilogger, matrixviz, configurator)

### Configuration System (m4-based)

The build system uses m4 macro files to generate platform-specific configuration scripts:

1. **Main Configuration**: `make conf` triggers:
   ```bash
   # createMakefile.conf.sh asks for:
   - Installation prefix (e.g., /usr/local or $HOME)
   - Installation type (USER or DEVEL)
   # Then calls configure for each component
   ```

2. **Component Configuration**: Each component has:
   - `configure` script: Processes command-line options
   - `*-config.m4` template: m4 template for generating config script
   - `*-config` output: Generated script providing compiler/linker flags

3. **Platform Detection in m4**:
   ```m4
   # Example from selforg-config.m4:
   ifdef(`MAC', `define(`LINUXORMAC', $2)', `define(`LINUXORMAC', $1)')
   # Usage: LINUXORMAC(linux_value, mac_value)
   ```

### Build Commands (Current System)

#### Initial Setup and Full Build
```bash
make conf           # Configure installation (only needed once)
make all            # Configure and build everything (utils, selforg, ode, ode_robots, ga_tools)
                   # Automatically uses sudo if needed for system directories
```

#### Individual Component Builds
```bash
make selforg        # Build self-organizing controllers library
make ode_robots     # Build robot simulation library  
make ode            # Build ODE physics engine (double precision)
make utils          # Build utility tools (guilogger, matrixviz)
make ga_tools       # Build genetic algorithms tools
make configurator   # Build parameter configuration GUI
```

#### Installation Commands
```bash
make install_selforg     # Install selforg library and headers
make install_ode_robots  # Install ode_robots library
make install_ode         # Install customized ODE (libode_dbl)
make install_utils       # Install GUI tools and scripts
make install_ga_tools    # Install GA tools
```

#### Building and Running Simulations
```bash
# Create new simulation from template
cd ode_robots/simulations
./createNewSimulation.sh template_sphererobot my_simulation

# Build simulation
cd ode_robots/simulations/my_simulation
make                # Debug build
make opt            # Optimized build

# Run simulation
./start             # Run debug version
./start -g          # Run with guilogger for parameter visualization
./start_opt         # Run optimized version
```

#### Development Commands
```bash
make clean          # Remove object files
make clean-all      # Clean everything including libraries
make doc            # Generate Doxygen documentation
make tags           # Generate TAGS for code navigation
make todo           # Show TODOs in source code
make uninstall      # Remove all installed files
```

## Architecture Overview

### Core Components

1. **Controllers** (`selforg/controller/`): Learning algorithms
   - Base class: `AbstractController`
   - Key implementations: DEP, Sox, Pimax, various neural network controllers
   - All controllers implement: `init()`, `step()`, `stepNoLearning()`

2. **Robots** (`ode_robots/robots/`): Physical robot models
   - Base class: `OdeRobot` (inherits from `AbstractRobot`)
   - Examples: Sphererobot3Masses, Nimm2/4, Schlange, Hexapod
   - Interface: `getSensors()`, `setMotors()`, physical primitive management

3. **Simulations** (`ode_robots/simulations/`): Experiment setups
   - Inherit from `Simulation` class
   - Implement `start()` to create robots, controllers, and environment
   - Handle keyboard commands via `command()`

### Key Design Patterns

1. **Agent Pattern**: Combines robot + controller + wiring
   ```
   Robot ← Wiring ← Controller
     ↑        ↑         ↑
     └────Agent─────────┘
   ```

2. **Wiring**: Transforms between robot sensors/motors and controller I/O
   - `One2OneWiring`: Direct connection
   - `DerivativeWiring`: Adds velocity information
   - `FeedbackWiring`: Adds motor feedback to sensors

3. **Handle Pattern**:
   - `OdeHandle`: Physics world access
   - `OsgHandle`: Graphics context
   - `GlobalData`: Shared simulation state

4. **Configuration System**:
   - All components inherit from `Configurable`
   - Runtime parameter adjustment
   - Automatic GUI integration

### Typical Simulation Structure

```cpp
class MySimulation : public Simulation {
    void start(const OdeHandle& odeHandle, const OsgHandle& osgHandle, GlobalData& global) {
        // 1. Create playground/environment
        Playground* playground = new Playground(odeHandle, osgHandle, ...);
        
        // 2. Create and configure robot
        Sphererobot3MassesConf conf = Sphererobot3Masses::getDefaultConf();
        OdeRobot* robot = new Sphererobot3Masses(odeHandle, osgHandle, conf, "MyRobot");
        robot->place(Pos(0,0,0.5));
        
        // 3. Create controller
        AbstractController* controller = new Sox();
        controller->setParam("epsC", 0.1);
        
        // 4. Create wiring
        AbstractWiring* wiring = new One2OneWiring(new ColorUniformNoise(0.1));
        
        // 5. Create agent and add to simulation
        OdeAgent* agent = new OdeAgent(global);
        agent->init(controller, robot, wiring);
        global.agents.push_back(agent);
        global.configs.push_back(controller);
    }
};
```

### Important Conventions

- **Matrix Operations**: Controllers use the custom matrix library (`selforg/matrix/`)
- **Sensor/Motor Arrays**: Always use `sensor` and `motor` types (doubles)
- **Timestep**: Default is 0.01s, configurable in simulation
- **Coordinate System**: Z-axis points up
- **Units**: Meters for distances, radians for angles

### Common Development Tasks

1. **Adding a New Robot**: 
   - Inherit from `OdeRobot`
   - Create primitives in constructor
   - Implement sensor/motor interfaces
   - See existing robots in `ode_robots/robots/` for examples

2. **Adding a New Controller**:
   - Inherit from `AbstractController`
   - Implement required virtual methods
   - Add to `selforg/controller/`
   - Include in `Makefile.conf`

3. **Creating a Simulation**:
   - Use `createNewSimulation.sh` script
   - Modify `main.cpp` to set up your experiment
   - Edit `Makefile.conf` to include needed files

### Debugging Tips

- Use `./start -g` to visualize controller parameters in real-time
- Enable logging with `./start -f` for post-analysis
- Use `./start -pause` to start simulation paused
- Check `~/.lpzrobots/` for log files and configurations

## Dependency Management

### Required Dependencies

#### Core Dependencies
- **C++ Compiler**: clang++ (macOS) or g++ (Linux) with C++17 support
- **OpenGL**: For 3D visualization
- **readline**: For console interaction
- **ncurses**: For terminal UI
- **pthread**: Threading support

#### Component-Specific Dependencies

1. **opende (Physics Engine)**:
   - autoconf/automake/libtool
   - No external physics dependencies (self-contained)

2. **ode_robots (Simulation Framework)**:
   - OpenSceneGraph 3.x (OSG) - 3D graphics
   - ODE physics (via opende)
   - GLUT/OpenGL

3. **GUI Tools (guilogger, matrixviz, configurator)**:
   - Qt5 (Core, Gui, Widgets, OpenGL, XML modules)
   - GLU library
   - gnuplot (for guilogger)

4. **selforg (Controllers)**:
   - GSL (GNU Scientific Library) - optional
   - No GUI dependencies

### Dependency Detection

The build system detects dependencies through:

1. **Configure Scripts**: Each component's `configure` script checks for dependencies
2. **m4 Macros**: Platform-specific library paths are handled in *-config.m4 files
3. **pkg-config**: Not used - custom detection scripts instead

Example dependency check (from selforg/configure):
```bash
# Test for GSL
if test -z "$GSL" && ! type gsl-config; then
    GSL=-DNOGSL
    echo "disabled GSL because \"gsl-config\" is not in PATH"
fi
```

### macOS-Specific Paths

The system checks multiple paths for macOS:
```bash
# MacPorts (old Intel Macs)
/opt/local/include
/opt/local/lib

# Homebrew (ARM64 Macs)  
/opt/homebrew/include
/opt/homebrew/lib
```

## Migration Guidelines

### Build System Enhancement Strategy

**The original build system is well-constructed and carefully designed. Our migration must:**

1. **Preserve the hierarchical make structure** - It enables independent component builds
2. **Enhance the m4 configuration system** - Add ARM64/Qt5 detection without rewriting
3. **Maintain backward compatibility** - Linux builds must continue to work
4. **Document all changes** - Clear rationale for every modification

### Platform Detection Updates

#### 1. ARM64 Detection (Already implemented in opende)
```bash
# In configure.in:
arm64* | aarch64* )
    cpu64=yes
    arm64=yes
    AC_DEFINE(ARM64_SYSTEM,1,[ARM64 system])
;;
```

#### 2. Update m4 Macros for ARM64
```m4
# Add to *-config.m4 files:
ifdef(`ARM64',
  `define(`ARCH_FLAGS', '-arch arm64')
   define(`NEON_FLAGS', '-march=armv8-a+simd')',
  `define(`ARCH_FLAGS', '')')
```

#### 3. Library Path Updates
```bash
# In selforg-config.m4 and similar:
CBASEFLAGS="-std=c++17 -pthread LINUXORMAC(-I/usr/include,-I/opt/local/include -I/opt/homebrew/include)"
```

### C++17 Migration Patterns

#### 1. Replace deprecated features
```cpp
// Old: std::unary_function (deprecated)
template<typename T>
struct MyFunc : std::unary_function<T, bool> { ... };

// New: Define types directly
template<typename T>
struct MyFunc {
    using argument_type = T;
    using result_type = bool;
    ...
};
```

#### 2. Use modern STL features
```cpp
// Use std::optional for nullable values
std::optional<Matrix> getJacobian() const;

// Use structured bindings
auto [sensors, motors] = robot->getSensorMotorInfo();

// Use std::filesystem (when available)
#if __cpp_lib_filesystem
    namespace fs = std::filesystem;
#endif
```

### Qt5 Migration Guidelines

#### 1. Update .pro files
```qmake
# Old Qt4
QT += core gui

# New Qt5
QT += core gui widgets
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
```

#### 2. Update includes
```cpp
// Qt4
#include <QtGui>

// Qt5
#include <QtWidgets>
#include <QtGui>
#include <QtCore>
```

#### 3. Replace deprecated Qt classes
```cpp
// QGLWidget → QOpenGLWidget
// toAscii() → toLatin1()
// QRegExp → QRegularExpression (where appropriate)
```

### Component-Specific Migration

#### opende (Physics Engine)
- ✅ ARM64 detection already added
- TODO: Replace deprecated macOS frameworks (Carbon → Cocoa)
- TODO: Update for modern OpenGL context creation

#### selforg (Core Library)
- TODO: Add ARM64 SIMD optimizations for matrix operations
- TODO: Update QuickMP threading to std::thread
- TODO: C++17 modernization of controller interfaces

#### ode_robots (Simulation Framework)
- TODO: Evaluate OpenSceneGraph alternatives (OSG may need updates)
- TODO: Modernize shadow rendering system
- TODO: Update resource loading for .osg mesh files

#### GUI Tools
- ✅ guilogger: Already using Qt5
- ✅ matrixviz: Already using Qt5  
- TODO: configurator: Complete Qt5 migration
- TODO: Fix platform-specific keyboard shortcuts

### Testing Requirements

1. **Build Testing**:
   ```bash
   # After each change:
   make clean
   make all
   # Test on both Linux and macOS ARM64
   ```

2. **Component Testing**:
   - Each component must build independently
   - Run example simulations
   - Verify GUI tools launch correctly

3. **Performance Testing**:
   - Matrix operations benchmark
   - Physics simulation frame rate
   - Memory usage profiling

### Build System Strengths 

#### 1. **Sophisticated Configuration System**

**Approach**: Add new macros for Qt5/ARM64 detection rather than rewriting

#### 2. **Modular Make Structure**
```make
# Original pattern to preserve:
all: selforg ode ode_robots utils ga_tools
selforg:
	$(MAKE) -C selforg
```
This allows independent component builds and is well-organized.

#### 3. **Intelligent Dependency Management**
The `*-config` scripts (e.g., `selforg-config --libs`) provide proper linking flags. Extend these for new dependencies rather than replacing.


### Testing Requirements

- All components must build with: `clang++ -std=c++17 -arch arm64`
- GUI tools must run natively on MacOS Arm64
- Performance should match or exceed 

## Implementation Resources

1. **CMake Templates**:
   - `selforg_CMakeLists.txt`: Modern CMake setup for selforg library
   - `platform_detection.cmake`: Cross-platform detection including ARM64

2. **C++17 Modernization**:
   - `controller_cpp17_modernization.cpp`: Modern AbstractController implementation
   - Demonstrates std::optional, structured bindings, RAII patterns

3. **Platform Integration**:
   - `complete_example_usage.cpp`: Integration example showing all components
   - Platform-specific compilation with proper feature detection

### Migration Workflow

1. **Start with Build System**: Create CMakeLists.txt using provided templates
2. **Update Dependencies**: Use platform_detection.cmake for proper ARM64 support
3. **Modernize Code**: Apply C++17 patterns from examples
4. **Test Incrementally**: Build and test each component before moving to next
5. **Document Changes**: Update component documentation as you migrate

## Component Architecture Details

### Component Overview and Dependencies

```
┌─────────────┐     ┌──────────────┐     ┌───────────────┐
│   selforg   │     │   opende     │     │  ga_tools     │
│ (Qt-free)   │     │  (ode-dbl)   │     │ (optional)    │
└─────┬───────┘     └──────┬───────┘     └───────┬───────┘
      │                     │                     │
      └──────────┬──────────┴─────────────────────┘
                 │
           ┌─────▼──────┐
           │ ode_robots │
           │   (OSG)    │
           └─────┬──────┘
                 │
    ┌────────────┼────────────┬──────────────┐
    │            │            │              │
┌───▼────┐  ┌───▼─────┐  ┌───▼────┐   ┌────▼─────┐
│guilogger│  │matrixviz│  │config- │   │simulations│
│ (Qt4)   │  │ (Qt4)   │  │urator  │   │(50+ demos)│
│         │  │         │  │(Qt4)   │   │          │
└─────────┘  └─────────┘  └────────┘   └──────────┘
```

### Component-Specific Details

#### 1. selforg (Core Library)
- **Status**: Qt-free, relatively easy migration
- **Key Features**: 
  - Matrix library (potential ARM64 NEON optimization)
  - Controller framework (AbstractController base)
  - Agent/Wiring system
  - Configurable/Inspectable interfaces
- **Platform Issues**:
  - Hardcoded /opt/local paths for macOS
  - Uses custom drand48r implementation for macOS
  - QuickMP threading (not Qt)
- **Migration Priority**: HIGH (foundation for everything)

#### 2. ode_robots (Simulation Framework)
- **Status**: Complex due to OSG dependency
- **Key Features**:
  - Heavy OpenSceneGraph integration
  - Custom LPZViewer extends osgViewer
  - Shadow systems (multiple techniques)
  - Threading options (-odethread, -osgthread)
- **Critical Dependencies**:
  - OpenSceneGraph 3.x
  - ODE physics (via opende)
  - OpenGL/GLUT
- **Migration Challenges**:
  - OSG may need replacement or significant updates
  - Shadow rendering system is OSG-specific
  - Resource loading (.osg mesh formats)

#### 3. opende (Physics Engine)
- **Status**: Critical dependency with platform issues
- **Key Issues**:
  - No ARM64 detection in configure.in
  - Uses deprecated Carbon/AGL frameworks on macOS
  - X86_64_SYSTEM macro doesn't recognize ARM64
- **Migration Requirements**:
  ```bash
  # Add to configure.in
  arm64* | aarch64* )
    cpu64=yes
    AC_DEFINE(ARM64_SYSTEM,1,[ARM64 system])
  ```
  - Replace Carbon with Cocoa framework
  - Update odeconfig.h for ARM64

#### 4. GUI Tools
- **guilogger**:
  - Gnuplot integration via pipes
  - Platform-specific gnuplot commands
- **matrixviz**:
  - Uses QGLWidget (deprecated in Qt5)
  - Real-time OpenGL visualization
- **configurator**:
  - Understand well the intentions and necessity
  - Well-designed library
  - Ready for Qt5 migration

#### 5. ga_tools
- **Status**: Low priority, "not well maintained"
- **Features**: Genetic algorithm framework
- **Dependencies**: selforg only
- **Migration**: Simple, just needs build system updates

### Some Components
1. **ARM64 detection** in opende
2. **Deprecated macOS frameworks** in opende
3. **OpenSceneGraph dependency** - May need alternative 3D engine
4. **Shadow rendering system** - Tightly coupled to OSG
5. **Threading model** - Platform-specific considerations
6. **Matrix operations** - Performance critical
7. **Gnuplot integration** - Platform-specific paths
8. **Resource loading** - .osg format files
9. **Color schema system** - File path resolution
10. **ga_tools** - Simple, isolated component
11. **configurator** - Already Qt4 compliant
12. **Controller algorithms** - Pure C++

## Platform-Specific Considerations

### macOS ARM64 Requirements
1. **Compiler Flags**:
   ```cmake
   if(APPLE AND CMAKE_SYSTEM_PROCESSOR MATCHES "arm64")
     set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -arch arm64")
     set(CMAKE_OSX_ARCHITECTURES "arm64")
   endif()
   ```

2. **Framework Updates**:
   - Carbon → Cocoa
   - AGL → Metal/OpenGL via NSOpenGLContext
   - QuickTime → AVFoundation (if used)

3. **Path Updates**:
   - /opt/local → /opt/homebrew
   - Check for Rosetta 2 translation

### Threading Considerations
- ODE thread: Physics runs separately (1-frame sensor delay)
- OSG thread: Graphics runs separately (recommended)
- QuickMP: May need std::thread migration
- Thread affinity: ARM64 big.LITTLE architecture

## Performance Optimization Opportunities

### 1. Matrix Library (selforg/matrix/)
- **Current**: Basic loops, memcpy/memset
- **Optimization**: ARM64 NEON SIMD
- **Alternative**: Apple Accelerate framework
```cpp
#ifdef __APPLE__
  #include <Accelerate/Accelerate.h>
  // Use vDSP for vector operations
  // Use BLAS for matrix multiplication
#endif
```

### 2. Collision Detection
- Spatial hashing for broad phase
- ARM64 cache-friendly data structures
- Parallel collision detection with GCD

### 3. Rendering Pipeline
- Metal backend for macOS (if replacing OSG)
- Instanced rendering for multiple robots
- Compute shaders for particle systems

## Testing Strategy

### Unit Testing
1. **Matrix operations**: Compare NEON vs scalar
2. **Controller algorithms**: Verify numerical stability
3. **Collision detection**: Edge cases

### Integration Testing
1. **Each robot type** with reference controller
2. **GUI tools** with sample data streams
3. **Threading modes** stability tests

### Performance Testing
1. **Benchmark suite**: Matrix operations, collision detection
2. **Frame rate targets**: 60 FPS minimum
3. **Memory usage**: Profile with Instruments

### Platform Testing
1. **Native ARM64**: No Rosetta 2
2. **Universal Binary**: Both architectures
3. **Cross-compilation**: Linux compatibility

## Command Line Options Reference

The simulator supports these runtime options (from README.md):
- `-g [interval]`: Enable guilogger (default interval 1)
- `-f [interval]`: Write logging file (default interval 5)
- `-m [interval]`: Use matrixviz (default interval 10)
- `-noshadow`: Disable shadows (helps with graphics compatibility)
- `-shadow [0-5]`: Shadow types (0=none, 5=ShadowMap default)
- `-odethread`: Run ODE in separate thread
- `-osgthread`: Run OSG in separate thread (recommended)
- `-threads N`: Thread count (defaults to CPU count)

## Incremental Migration Approach

1. **Directory structure** - Keep all paths and organization
2. **Make targets** - Preserve all existing targets
3. **Configuration options** - Keep all existing flags
4. **Installation logic** - PREFIX system works well
5. **Component dependencies** - Build order is carefully designed
6. **Script interfaces** - Keep *-config script APIs
7. **Default behaviors** - All defaults must remain the same

### Change ONLY When Necessary:
1. Extend ARM64 detection for best performance
2. Enhance Qt5 support with best modern usage and redesign
3. Update paths - Add new paths alongside old ones
4. Fix deprecated APIs - when they show warnings compilation for best practices
5. Add C++17 flag - Use modern C++ libraries and packages

### Validation at Each Step:
```bash
# After each change, verify:
make clean
make all
# (Test on Linux to ensure no regression)
# Test on macOS ARM64
```

## Implementation Examples

### Example: Adding ARM64 Support to a Component

1. **Update configure script**:
```bash
# In component/configure
if [ `uname -m` = "arm64" ]; then
    system=MAC_ARM64
    ARCH_FLAGS="-arch arm64"
fi
```

2. **Update m4 configuration**:
```m4
# In component-config.m4
ifdef(`MAC_ARM64',
  `define(`ARCH_FLAGS', '-arch arm64')
   define(`INCLUDES', '-I/opt/homebrew/include')')
```

3. **Update C++ code for platform detection**:
```cpp
#ifdef __APPLE__
  #ifdef __arm64__
    // ARM64-specific code
    #include <Accelerate/Accelerate.h>
  #endif
#endif
```

### Example: Modernizing a Controller Class

```cpp
// Old C++11 style
class MyController : public AbstractController {
    paramval eps;
    Matrix C;
public:
    MyController() : eps(0.1) {}
    
    virtual void init(int sensornumber, int motornumber, 
                     RandGen* randGen = 0) override {
        C.set(motornumber, sensornumber);
    }
};

// Modern C++17 style
class MyController : public AbstractController {
    paramval eps{0.1};  // In-class initialization
    Matrix C;
    
public:
    MyController() = default;  // Defaulted constructor
    
    void init(int sensornumber, int motornumber, 
              RandGen* randGen = nullptr) override {
        C.set(motornumber, sensornumber);
    }
    
    // Use structured bindings
    auto getSensorMotorInfo() const {
        return std::make_tuple(sensornumber, motornumber);
    }
};
```

### Example: Qt5 Widget Migration

```cpp
// Qt4 code
#include <QtGui>
class MyWidget : public QWidget {
    Q_OBJECT
public:
    MyWidget(QWidget* parent = 0) : QWidget(parent) {
        QPushButton* button = new QPushButton("Click", this);
        connect(button, SIGNAL(clicked()), this, SLOT(onClicked()));
    }
};

// Qt5 code
#include <QtWidgets>
#include <QPushButton>
class MyWidget : public QWidget {
    Q_OBJECT
public:
    MyWidget(QWidget* parent = nullptr) : QWidget(parent) {
        QPushButton* button = new QPushButton("Click", this);
        // Modern connect syntax
        connect(button, &QPushButton::clicked, 
                this, &MyWidget::onClicked);
    }
};
```

## Troubleshooting Guide

### Build Errors

#### "Cannot find -lode_dbl"
```bash
# Solution: Install ODE first
make ode
sudo make install_ode
# Then rebuild component
```

#### "Qt5 not found"
```bash
# macOS with Homebrew:
brew install qt@5
export PATH="/opt/homebrew/opt/qt@5/bin:$PATH"
# Re-run configuration
make conf
```

#### "OSG headers not found"
```bash
# Install OpenSceneGraph
brew install open-scene-graph  # macOS
# or
sudo apt-get install libopenscenegraph-dev  # Ubuntu
```

### Runtime Issues

#### Segfault on startup
```bash
# Try disabling shadows
./start -noshadow

# Check for missing libraries
otool -L ./start  # macOS
ldd ./start       # Linux
```

#### GUI tools not launching
```bash
# Check Qt plugin path
export QT_PLUGIN_PATH=/opt/homebrew/opt/qt@5/plugins

# Debug Qt issues
export QT_DEBUG_PLUGINS=1
./guilogger
```

#### Performance issues
```bash
# Use optimized build
make opt
./start_opt

# Disable threading initially
./start -threads 1
```

## Known Issues and Workarounds

### Graphics Issues
- **Shadow crash**: Use `-noshadow` flag (shadow rendering issues with modern OSG)
- **Black screen**: Check OpenGL version compatibility
- **Missing textures**: Verify texture files in ode_robots/textures/

### Build Issues
- **ARM64 detection fails**: Manually set in Makefile.conf:
  ```make
  ARCH_FLAGS = -arch arm64
  ```
- **Qt3Support missing**: Code must be migrated to Qt5 (no workaround)
- **OSG version mismatch**: Use OSG 3.4.x or newer for ARM64

### macOS-Specific Issues
- **Keyboard shortcuts**: Ctrl+M intercepted by terminal, use Ctrl+V for matrixviz
- **App permissions**: Grant terminal/IDE camera/microphone access if needed
- **Rosetta warnings**: Ensure native ARM64 builds with `file` command

### Performance Issues
- **Thread affinity**: Let macOS handle thread scheduling (don't force affinity)
- **Memory pressure**: Reduce simulation complexity or use optimized builds
- **GPU throttling**: Ensure adequate cooling on Apple Silicon

## Build System Analysis Notes

### Why the Original Design Works Well:

1. **Component Independence**: Each component can be built/installed separately
2. **Flexible Configuration**: m4 macros allow complex dependency detection
3. **Cross-Platform**: Already handles Linux/Mac differences elegantly
4. **User/System Install**: Supports both with automatic sudo detection
5. **Dependency Resolution**: *-config scripts handle complex linking

### Integration Points to Preserve:

1. **make all**: Builds everything in correct order
2. **make conf**: Reconfigures without losing settings
3. **Component makefiles**: Call sub-makefiles cleanly
4. **Install detection**: Automatic sudo when needed
5. **Uninstall support**: Clean removal of all components

The enhancement and refactoring for the lpzrobot software package should enhance this system, not replace it. Think of it as adding Enhanced ARM64 Qt5 CMake C++ extensive support to an already excellent build and compiled robotic simulator.

## Quick Start Guide for macOS ARM64

### Prerequisites Installation
```bash
# Install Xcode Command Line Tools
xcode-select --install

# Install Homebrew (if not already installed)
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# Install dependencies
brew install qt@5 open-scene-graph gsl readline gnuplot
brew install autoconf automake libtool  # For opende

# Set up environment
echo 'export PATH="/opt/homebrew/opt/qt@5/bin:$PATH"' >> ~/.zshrc
source ~/.zshrc
```

### Build Steps
```bash
# 1. Configure installation
make conf
# Choose installation directory (e.g., /Users/yourname/lpzrobots)
# Choose 'u' for user installation

# 2. Build everything
make all

# 3. Test with a simulation
cd ode_robots/simulations/template_sphererobot
make
./start -noshadow  # Use -noshadow to avoid graphics issues
```

### Verification
```bash
# Check that binaries are native ARM64
file $(which guilogger)  # Should show "arm64"
file selforg/libselforg.a  # Should show "ar archive"
```

## Important Implementation Notes

### File Organization
- **Configuration files**: Component-level configure scripts and *-config.m4 templates
- **Build outputs**: Libraries in component directories, installed to PREFIX/lib
- **Headers**: Installed to PREFIX/include/component_name
- **Simulations**: User simulations in separate directories with own Makefiles

### Code Style Guidelines
- Use spaces, not tabs (4-space indentation)
- Follow existing naming conventions (camelCase for methods, underscore for members)
- Document public APIs with Doxygen comments
- Keep platform-specific code isolated with #ifdef guards

### Version Control
- Test changes on both Linux and macOS before committing
- Document migration changes in commit messages
- Keep build system changes separate from code modernization

### Performance Considerations
- Matrix operations are performance-critical (consider SIMD optimizations)
- Physics timestep affects both accuracy and performance
- GUI updates can be throttled for better performance

## Migration Progress Tracking

Use git branches for different aspects of migration:
- `feature/arm64-build`: Build system ARM64 support
- `feature/qt5-migration`: Qt5 updates for GUI tools
- `feature/cpp17-modernization`: C++17 language updates
- `feature/osg-updates`: OpenSceneGraph compatibility

Each component can be migrated independently following the patterns documented above.

# important-instruction-reminders
Do what has been asked; nothing more, nothing less.
NEVER create files unless they're absolutely necessary for achieving your goal.
ALWAYS prefer editing an existing file to creating a new one.
NEVER proactively create documentation files (*.md) or README files. Only create documentation files if explicitly requested by the User.

      
      IMPORTANT: this context may or may not be relevant to your tasks. You should not respond to this context or otherwise consider it in your response unless it is highly relevant to your task. Most of the time, it is not relevant.