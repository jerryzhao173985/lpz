# LPZRobots Troubleshooting Guide

---
**Document Type**: Guide  
**Last Updated**: 2025-01-06  
**Status**: Current  
**Version**: 1.0  
**Component(s)**: All  
**Tags**: troubleshooting, debugging, errors, solutions  
---

## Introduction

This guide helps diagnose and resolve common issues when working with the LPZRobots framework, covering build errors, runtime issues, and performance problems.

## 1. Build Issues

### CMake Configuration Errors

#### Problem: CMake cannot find dependencies
```
CMake Error: Could NOT find GSL (missing: GSL_LIBRARY GSL_INCLUDE_DIR)
```

**Solution:**
```bash
# macOS
brew install gsl

# Ubuntu/Debian
sudo apt-get install libgsl-dev

# Specify path manually
cmake -B build -DGSL_ROOT=/opt/homebrew
```

#### Problem: Qt6 not found
```
CMake Error: Could not find a package configuration file provided by "Qt6"
```

**Solution:**
```bash
# macOS
brew install qt@6
export Qt6_DIR=/opt/homebrew/opt/qt@6/lib/cmake/Qt6

# Linux
sudo apt-get install qt6-base-dev
```

### Compiler Errors

#### Problem: Missing override specifier
```
error: 'virtual' function 'step' overrides a member function but is not marked 'override'
```

**Solution:**
Add `override` to all virtual function implementations:
```cpp
void step(const sensor* sensors, int number_sensors,
          motor* motors, int number_motors) override;
```

#### Problem: Deprecated throw() specification
```
error: dynamic exception specifications are deprecated in C++11
```

**Solution:**
Replace `throw()` with `noexcept`:
```cpp
// Old
void cleanup() throw();

// New
void cleanup() noexcept;
```

#### Problem: Ambiguous overload with nullptr
```
error: call to 'setParam' is ambiguous
controller->setParam("eps", nullptr);
```

**Solution:**
Use proper type or default value:
```cpp
controller->setParam("eps", 0.0);  // Use appropriate default
```

### Linker Errors

#### Problem: Undefined symbols
```
Undefined symbols for architecture arm64:
  "Matrix::Matrix(int, int)", referenced from:
```

**Solution:**
1. Ensure all required libraries are linked:
```cmake
target_link_libraries(myapp PRIVATE 
    lpzrobots::selforg
    lpzrobots::ode_robots
)
```

2. Check library build type matches:
```bash
# Use consistent build type
cmake -B build -DCMAKE_BUILD_TYPE=Debug
```

#### Problem: Duplicate symbols
```
duplicate symbol 'globalData' in:
```

**Solution:**
Use `extern` in headers, define in one source file:
```cpp
// header.h
extern GlobalData globalData;

// source.cpp
GlobalData globalData;
```

## 2. Runtime Issues

### Segmentation Faults

#### Problem: Crash on startup
```
Segmentation fault: 11
```

**Debugging Steps:**
1. Run with debugger:
```bash
lldb ./myrobot
(lldb) run
(lldb) bt  # Show backtrace when crashes
```

2. Enable sanitizers:
```bash
cmake -B build-debug -DENABLE_SANITIZERS=ON
./build-debug/myrobot
```

3. Common causes:
- Null pointer dereference
- Accessing deleted objects
- Buffer overflows

#### Problem: Matrix dimension mismatch
```
terminate called after throwing an instance of 'Matrix::MatrixDimensionMismatchException'
```

**Solution:**
Check matrix dimensions before operations:
```cpp
void safeMultiply(const Matrix& A, const Matrix& B, Matrix& C) {
    assert(A.getN() == B.getM());
    C = A * B;
}
```

### Memory Issues

#### Problem: Memory leaks detected
```
==12345== LEAK SUMMARY:
==12345==    definitely lost: 1,024 bytes in 16 blocks
```

**Solution:**
1. Use smart pointers:
```cpp
// Instead of
AbstractController* ctrl = new Sox();

// Use
auto ctrl = std::make_unique<Sox>();
```

2. Check for circular references:
```cpp
// Use weak_ptr to break cycles
std::weak_ptr<Node> parent;
std::shared_ptr<Node> child;
```

### Performance Issues

#### Problem: Simulation runs slowly
**Diagnosis:**
```cpp
// Add timing code
auto start = std::chrono::high_resolution_clock::now();
controller->step(sensors, nsensors, motors, nmotors);
auto end = std::chrono::high_resolution_clock::now();
auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
std::cout << "Step time: " << duration.count() << " µs\n";
```

**Solutions:**
1. Enable optimizations:
```bash
cmake -B build-release -DCMAKE_BUILD_TYPE=Release
```

2. Use SIMD optimizations:
```cmake
set(USE_NEON ON)  # ARM64
set(USE_AVX2 ON)  # x86_64
```

3. Profile the code:
```bash
# macOS
instruments -t "Time Profiler" ./myrobot

# Linux
perf record ./myrobot
perf report
```

## 3. GUI Issues

### Qt Problems

#### Problem: Qt application crashes on macOS
```
qt.qpa.plugin: Could not find the Qt platform plugin "cocoa"
```

**Solution:**
```bash
export QT_PLUGIN_PATH=/opt/homebrew/opt/qt@6/share/qt/plugins
```

#### Problem: HiDPI scaling issues
**Solution:**
```cpp
// In main()
QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
```

### OpenGL Issues

#### Problem: Shadow rendering crashes
```
Assertion failed: (shadowTexture), function bindShadowTexture
```

**Solution:**
Run without shadows on macOS:
```bash
./start -noshadow
```

## 4. Physics Simulation Issues

### ODE Problems

#### Problem: Unstable simulation
**Symptoms:** Objects exploding, jittering, or passing through each other

**Solutions:**
1. Adjust simulation parameters:
```cpp
dWorldSetCFM(world, 1e-5);  // Constraint force mixing
dWorldSetERP(world, 0.2);   // Error reduction parameter
```

2. Reduce timestep:
```cpp
simulation.setSimStepSize(0.005);  // Default is 0.01
```

3. Check collision parameters:
```cpp
// Increase contact points
dWorldSetMaxContacts(world, 40);
```

### Robot Configuration Issues

#### Problem: Robot falls apart
**Solution:**
Check joint parameters:
```cpp
// Ensure proper joint limits
dJointSetHingeParam(joint, dParamLoStop, -M_PI/2);
dJointSetHingeParam(joint, dParamHiStop, M_PI/2);
```

## 5. Controller Issues

### Learning Problems

#### Problem: Controller doesn't learn
**Diagnosis:**
```cpp
// Monitor learning progress
std::cout << "Error: " << controller->getLastError() << "\n";
std::cout << "EpsA: " << controller->getParam("epsA") << "\n";
```

**Solutions:**
1. Adjust learning rates:
```cpp
controller->setParam("epsA", 0.1);  // Default might be too low
controller->setParam("epsC", 0.05);
```

2. Check sensor scaling:
```cpp
// Ensure sensors are in [-1, 1] range
for (int i = 0; i < nsensors; i++) {
    sensors[i] = clip(sensors[i], -1.0, 1.0);
}
```

### Numerical Instability

#### Problem: NaN or Inf values
```
Motor value is nan at step 1000
```

**Solution:**
Add numerical checks:
```cpp
void checkNumericalStability(const Matrix& m) {
    for (int i = 0; i < m.getM(); i++) {
        for (int j = 0; j < m.getN(); j++) {
            if (!std::isfinite(m.val(i, j))) {
                throw std::runtime_error("Non-finite value detected");
            }
        }
    }
}
```

## 6. Platform-Specific Issues

### macOS ARM64

#### Problem: Rosetta translation warning
```
Warning: Running under Rosetta translation
```

**Solution:**
Build native ARM64 binary:
```bash
cmake -B build -DCMAKE_OSX_ARCHITECTURES=arm64
```

#### Problem: Code signing issues
**Solution:**
```bash
codesign --force --sign - ./myrobot
```

### Linux

#### Problem: Missing shared libraries
```
error while loading shared libraries: libselforg.so: cannot open shared object file
```

**Solution:**
```bash
# Add to .bashrc
export LD_LIBRARY_PATH=$HOME/lpzrobots/lib:$LD_LIBRARY_PATH

# Or use rpath
cmake -B build -DCMAKE_INSTALL_RPATH_USE_LINK_PATH=ON
```

## 7. Development Environment

### IDE Integration

#### Problem: VSCode IntelliSense not working
**Solution:**
Generate compile_commands.json:
```bash
cmake -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
ln -s build/compile_commands.json .
```

#### Problem: Debugging doesn't work
**Solution:**
Create `.vscode/launch.json`:
```json
{
    "version": "0.2.0",
    "configurations": [{
        "name": "Debug Robot",
        "type": "lldb",
        "request": "launch",
        "program": "${workspaceFolder}/build/myrobot",
        "args": ["-noshadow"],
        "cwd": "${workspaceFolder}"
    }]
}
```

## Common Error Messages Reference

| Error | Cause | Solution |
|-------|-------|----------|
| `Matrix dimension mismatch` | Incompatible matrix sizes | Check dimensions before operations |
| `Segmentation fault` | Null pointer or memory corruption | Use debugger and sanitizers |
| `Pure virtual method called` | Calling virtual function in constructor | Move initialization to init() |
| `std::bad_alloc` | Out of memory | Reduce buffer sizes or use pools |
| `Symbol not found` | Missing library | Check library paths and linking |

## Getting Help

If issues persist:

1. **Check logs**: Enable verbose output
```cpp
Configurable::setVerboseMode(true);
```

2. **Minimal example**: Create smallest code that reproduces issue

3. **Debug build**: Always test with debug build first
```bash
cmake -B build -DCMAKE_BUILD_TYPE=Debug
```

4. **Community**: Post detailed error messages and system info

This troubleshooting guide covers the most common issues encountered when working with LPZRobots.