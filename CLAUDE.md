# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## CRITICAL: macOS ARM64 Migration with C++17 and Modern Build System

**PROJECT STATE**: 98% modernized - Core libraries build successfully with C++17. Remaining work focuses on build warnings, dependency management, and testing infrastructure.

**MIGRATION GOALS**:
1. Complete C++17/20 modernization with zero warnings
2. Native macOS ARM64 (Apple Silicon) support
3. Modern dependency management (CMake + package managers)
4. Comprehensive testing and quality tools
5. Memory safety with smart pointers
6. Performance optimization with SIMD

## Current Migration Status (2025-06-25)

### ✅ Completed Tasks
- Fixed 98% of compilation warnings (4100 → ~131)
- C++17 standard fully adopted
- Core selforg library builds successfully
- ODE physics engine builds with ARM64 support
- Fixed all critical security vulnerabilities
- Applied extensive code modernization:
  - 172+ override keywords added
  - 141+ C-style casts replaced
  - 300+ uninitialized members fixed
  - Deprecated features removed (std::unary_function, ptr_fun)
- GUI tools migrated to Qt6
- Created binary distribution package

### 🎯 Immediate Priorities
1. Complete typedef → using migration (67+ files)
2. Replace remaining NULL with nullptr
3. Fix throw() → noexcept transitions
4. Enable sanitizers (ASAN, TSAN, UBSAN)
5. Set up automated code quality tools

## Build Commands

### Initial Setup (macOS ARM64)
```bash
# Install dependencies
brew install qt@6 open-scene-graph gsl readline gnuplot
brew install autoconf automake libtool cmake ninja
brew install llvm  # For clang-tidy

# Configure installation
make conf
# Choose installation directory (e.g., ~/lpzrobots)
# Choose 'u' for user installation

# Build everything
make all -j8

# Or build components individually
make selforg        # Core controller library
make ode           # Physics engine
make ode_robots    # Robot simulation framework
make utils         # GUI tools
```

### Development Commands
```bash
# Debug build
make clean && make

# Optimized build
make clean && make opt

# Run static analysis
cppcheck --enable=all --std=c++17 selforg/
clang-tidy selforg/**/*.cpp -- -std=c++17 -I selforg/include

# Format code
find . -name "*.cpp" -o -name "*.h" | xargs clang-format -i

# Build and run example
cd ode_robots/simulations/template_sphererobot
make
./start -noshadow -g  # Run with guilogger
```

### Testing Commands
```bash
# Run built-in ODE tests
cd opende
make test

# Check for memory leaks
ASAN_OPTIONS=detect_leaks=1 ./start

# Profile performance
instruments -t "Time Profiler" ./start_opt
```

## Architecture Overview

### Component Hierarchy
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

### Key Design Patterns

1. **Agent-Robot-Controller Architecture**
```cpp
// Agent combines robot + controller + wiring
OdeRobot* robot = new Sphererobot3Masses(...);
AbstractController* controller = new Sox();
AbstractWiring* wiring = new One2OneWiring();
OdeAgent* agent = new OdeAgent(global);
agent->init(controller, robot, wiring);
```

2. **Configurable System**
- All components inherit from `Configurable`
- Runtime parameter adjustment via GUI
- Automatic introspection support

3. **Matrix Operations (Performance Critical)**
- Custom matrix library in `selforg/matrix/`
- Currently no SIMD optimizations
- Consider replacing with Eigen or Apple Accelerate

## C++ Modernization Guidelines

### Memory Management (CRITICAL)
```cpp
// OLD: Raw pointers everywhere
AbstractController* controller = new Sox();
delete controller;  // Manual deletion required

// NEW: Use smart pointers
auto controller = std::make_unique<Sox>();
// Automatic cleanup via RAII
```

### Modern C++ Patterns
```cpp
// Use 'using' instead of 'typedef'
using Matrix = matrix::Matrix;  // NOT: typedef matrix::Matrix Matrix;

// Use nullptr instead of NULL
if (ptr != nullptr)  // NOT: if (ptr != NULL)

// Use override for virtual functions
void step() override;  // NOT: virtual void step();

// Use noexcept instead of throw()
void cleanup() noexcept;  // NOT: void cleanup() throw();

// Use range-based loops
for (const auto& agent : agents) {  // NOT: for(int i=0; i<agents.size(); i++)
    // ...
}

// Use auto for complex types
auto result = controller->getLastSensorValues();

// Use std::optional for nullable returns
std::optional<Matrix> getJacobian() const;
```

### Code Quality Tools Configuration

**.clang-tidy** key checks:
- modernize-*
- performance-*
- readability-*
- cppcoreguidelines-*
- bugprone-*

**.clang-format** uses Google style with:
- IndentWidth: 4
- ColumnLimit: 100
- Custom macros for matrix operations

## Common Development Tasks

### Adding a New Robot
1. Create in `ode_robots/robots/`
2. Inherit from `OdeRobot`
3. Implement required virtual methods:
```cpp
class MyRobot : public OdeRobot {
public:
    void update() override;
    int getSensorNumber() const override;
    int getMotorNumber() const override;
    // ...
};
```
4. Add to `Makefile.conf`
5. Create simulation in `ode_robots/simulations/`

### Adding a New Controller
1. Create in `selforg/controller/`
2. Inherit from `AbstractController`
3. Implement learning algorithm:
```cpp
class MyController : public AbstractController {
public:
    void init(int sensornumber, int motornumber, 
              RandGen* randGen = nullptr) override;
    void step(const sensor* sensors, int number_sensors,
              motor* motors, int number_motors) override;
    void stepNoLearning(const sensor*, int, motor*, int) override;
    // ...
};
```

### Creating a Simulation
```bash
cd ode_robots/simulations
./createNewSimulation.sh template_sphererobot my_experiment
cd my_experiment
# Edit main.cpp to configure experiment
make
./start
```

## Known Issues and Solutions

### Build Warnings
- **String literal warnings in ODE**: Non-critical, from C code
- **Initialization order**: Minor issue in complexmeasure.cpp
- **Unused variables**: Being addressed incrementally

### macOS-Specific
- **Shadow rendering crashes**: Use `-noshadow` flag
- **Keyboard shortcuts**: Some Ctrl+ combinations intercepted by terminal
- **Framework paths**: Ensure `/opt/homebrew/bin` in PATH for ARM64

### Performance
- **Matrix operations slow**: No SIMD optimizations yet
- **Single-threaded physics**: Use `-odethread` for parallel physics
- **Memory allocations**: Frequent new/delete impacts performance

## Testing Requirements

### Before Committing
1. **Build without warnings**:
   ```bash
   make clean && make 2>&1 | grep -i warning
   # Should show minimal/no warnings
   ```

2. **Run static analysis**:
   ```bash
   cppcheck --enable=all --std=c++17 <component>/
   ```

3. **Check formatting**:
   ```bash
   clang-format --dry-run --Werror <files>
   ```

4. **Test on example simulation**:
   ```bash
   cd ode_robots/simulations/template_sphererobot
   make clean && make
   ./start -noshadow
   ```

### Platform Testing
- Test on both macOS ARM64 and Linux x64
- Verify native ARM64 binary: `file ./start | grep arm64`
- Check dependencies: `otool -L ./start` (macOS)

## Dependency Management

### Required Dependencies
- **C++ Compiler**: clang++ 14+ or g++ 11+ (C++17 required)
- **Qt6**: Core, Widgets, OpenGL modules
- **OpenSceneGraph**: 3.6+ (for C++17 compatibility)
- **GSL**: Optional for advanced math
- **OpenGL/GLU**: 3D graphics
- **readline/ncurses**: Console interaction

### Future: Package Manager Integration
```bash
# Planned: conan integration
conan install . --build=missing
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

## Performance Optimization Opportunities

### Matrix Operations (HIGH PRIORITY)
- Current: Basic loops with no vectorization
- Target: SIMD optimizations (NEON for ARM64, AVX for x86)
- Alternative: Replace with Eigen or Apple Accelerate

### Memory Management
- Current: Frequent heap allocations
- Target: Object pools for matrices and temporary objects
- Use stack allocation where possible

### Threading
- Current: Optional threading with complex setup
- Target: Modern std::thread with thread pools
- Parallel STL algorithms for data processing

## Migration Checklist

When working on modernization:

- [ ] Run `make clean` before major changes
- [ ] Check `git status` for untracked files
- [ ] Use smart pointers for new code
- [ ] Replace typedef with using
- [ ] Add override to virtual functions
- [ ] Use nullptr instead of NULL
- [ ] Apply clang-format to modified files
- [ ] Run cppcheck on changed components
- [ ] Test with sanitizers enabled
- [ ] Verify no new warnings introduced
- [ ] Test on both Debug and Release builds
- [ ] Update documentation for API changes

## Important Guidelines

1. **Preserve Functionality**: All changes must maintain backward compatibility
2. **Incremental Migration**: Modernize component by component
3. **Test Continuously**: Run simulations after each major change
4. **Document Changes**: Update component docs as you migrate
5. **Performance First**: This is a real-time simulation system

## Resources

- Original documentation: `doc/` directory
- Example simulations: `ode_robots/simulations/`
- Controller examples: `selforg/examples/`
- Build system details: `*.m4` files contain platform logic