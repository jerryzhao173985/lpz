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

## Build Commands (Current System)

### Initial Setup and Full Build
```bash
make all            # Configure and build everything (utils, selforg, ode, ode_robots, ga_tools)
```

### Individual Component Builds
```bash
make selforg        # Build self-organizing controllers library
make ode_robots     # Build robot simulation library
make utils          # Build utility tools (guilogger, matrixviz)
make ga_tools       # Build genetic algorithms tools
```

### Building and Running Simulations
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

### Development Commands
```bash
make clean          # Remove object files
make clean-all      # Clean everything including libraries
make doc            # Generate Doxygen documentation
make tags           # Generate TAGS for code navigation
make todo           # Show TODOs in source code
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

## Migration Guidelines

**The original build system is well-constructed and carefully designed for each component. Our migration strategy must:**

1. **Improve the existing system** as much as possible where necessary and fit
2. **Improve compatibility** where necessary for:
   - macOS ARM64 compatibility
   - macOS framework library/ package compatibility
   - Qt5/C++17 requirements
3. **Document every change** with clear justification in directory updates/ or logs/
4. **Test that original functionality** remains intact

### Strategy

- Make the hierarchical system works well
- Make sophisticated configuration robust / flexible / easy on macOS
- Each component's build independence
- Installation paths: PREFIX-based installation system
- Dependency detection: Existing scripts for finding libraries
- Note make those libraries either static or dynamic as per best practice in macOS

#### What Must Have Changed:
1. **Platform Detection**:
   ```bash
   # Add to existing configure scripts
   arm64* | aarch64* )  # New ARM64 detection
     # Keep existing logic, add ARM64 flags
   ```

2. **Library Paths**:
   ```bash
   # Update files to support both:
   MACINCLUDE="-I/opt/local/include -I/opt/homebrew/include"  # Add homebrew
   ```

3. **Qt Detection**:
   - QT5 library and correct efficient usage for performance and utilization

4. **Compiler Flags**:
   - Add `-std=c++17` alongside existing flags
   - Keep optimization levels and warning flags
   - Make sure COmpiler warnings are solved propely

5. **For Platform Code**:
   - Add platform detection
   - Use `#ifdef __APPLE__` for macOS-specific code
   - Add `__arm64__` or `__aarch64__` checks

### When Making Changes

1. **Study the original first**: Understand why it was designed this way as a complete software
2. **Continously revise your plan**: Understand in-depth or new knowledge for improvement
3. **Simplification & Clarity**: Make the robot simulator package clear and simplify redudant code/file structure
4. **Preserve consistent functionality**: Original behavior must remain intact
5. **Document rationale**: Every change needs clear justification
6. **Systerm Enhancement**: Must enhance system in crucial ways

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

## Known Issues and Workarounds

### Graphics Issues
- If crash on startup: use `-noshadow`
- Shadow rendering removed from recent OSG versions
- Some .osg mesh files may need conversion

### Build Issues
- Configure scripts may fail on ARM64: manually set flags
- Qt3Support missing on newer systems: must migrate code
- OpenSceneGraph ARM64 builds may need patches

### Performance Issues
- Thread affinity on ARM64 needs tuning
- Memory bandwidth limitations on unified memory
- Power efficiency vs performance trade-offs

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