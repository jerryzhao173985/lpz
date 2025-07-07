# CMake Simulation Build System - Complete Guide

## Summary

The LPZRobots build system has been successfully migrated to CMake. All simulations now build using CMake instead of the old Makefile system.

## Quick Start

### Building Any Simulation

```bash
# From lpzrobots root directory:
./build_and_run_simulation.sh <simulation_name>

# Examples:
./build_and_run_simulation.sh template_sphererobot
./build_and_run_simulation.sh sphererobot_novel_controllers
./build_and_run_simulation.sh hexapod_novel_controllers
```

### Available Options

```bash
./build_and_run_simulation.sh --help          # Show help
./build_and_run_simulation.sh -d <sim>        # Debug build
./build_and_run_simulation.sh -c <sim>        # Clean build
./build_and_run_simulation.sh -n <sim>        # Build only, don't run
```

## Novel Controller Simulations

### 1. Sphererobot with Novel Controllers
```bash
./build_and_run_simulation.sh sphererobot_novel_controllers
```
- Demonstrates all three novel algorithms (AH-Sox, MSHC, CASox)
- 4 sphere robots running different controllers
- Visual comparison of behaviors

### 2. Hexapod with Novel Controllers
```bash
./build_and_run_simulation.sh hexapod_novel_controllers
```
- Single hexapod with switchable controllers
- Press 1-4 to switch between controllers
- Real-time algorithm metrics in status line

## Migration from Old Make System

### What Changed:
1. **Build Location**: Executables now in `build/ode_robots/simulations/<name>/`
2. **Build Command**: Use CMake instead of make
3. **Configuration**: CMakeLists.txt instead of Makefile

### For Users Who See "call ./configure" Error:
This means you're trying to use the old build system. Use the new CMake approach:

```bash
# OLD (no longer works):
cd ode_robots/simulations/template_hexapod
make

# NEW (use this instead):
cd <lpzrobots_root>
./build_and_run_simulation.sh template_hexapod
```

## Creating New Simulations

1. Create directory structure:
```bash
mkdir -p ode_robots/simulations/my_simulation
```

2. Add your `main.cpp` file

3. Create `CMakeLists.txt`:
```cmake
include(${CMAKE_CURRENT_SOURCE_DIR}/../../../cmake/SimulationBuild.cmake)
lpzrobots_add_simulation(my_simulation_sim
    SOURCES main.cpp
)
```

4. Add to parent CMakeLists.txt:
```cmake
# In ode_robots/simulations/CMakeLists.txt
set(EXAMPLE_SIMULATIONS
    # ... existing simulations ...
    my_simulation
)
```

5. Build and run:
```bash
./build_and_run_simulation.sh my_simulation
```

## Verified Working Simulations

✅ template_sphererobot
✅ template_schlange
✅ template_onerobot
✅ sphererobot_novel_controllers
✅ hexapod_novel_controllers

## Benefits of CMake System

1. **Faster Builds**: Only rebuilds changed files
2. **Better Dependencies**: Automatic dependency tracking
3. **IDE Support**: Works with VS Code, CLion, etc.
4. **Cross-Platform**: Same build system on macOS, Linux, Windows
5. **Modern Features**: C++17/20 support, sanitizers, etc.

## Troubleshooting

### Issue: Simulation not found
```bash
# List available simulations:
ls ode_robots/simulations/

# Check if included in CMake:
grep -A 20 "EXAMPLE_SIMULATIONS" ode_robots/simulations/CMakeLists.txt
```

### Issue: Build fails
```bash
# Clean build:
rm -rf build/*
./build_and_run_simulation.sh -c <simulation>

# Debug build for more info:
./build_and_run_simulation.sh -d <simulation>
```

### Issue: Cannot find executable
```bash
# Executable location after build:
build/ode_robots/simulations/<simulation_name>/start
```

## VS Code Integration

1. Open VS Code in lpzrobots root
2. CMake extension will auto-configure
3. Select build target from status bar
4. Press F7 or Ctrl+Shift+B to build
5. Press F5 to run with debugger

## Performance Tips

- Use Release build for performance: default setting
- Use Debug build for development: `-d` flag
- Parallel builds: automatically uses `-j4`
- Disable shadows for speed: `./start -noshadow`

## Summary

The CMake migration is complete and provides a modern, efficient build system for all LPZRobots simulations. The old Makefile system is deprecated and should no longer be used. All simulations now build consistently with better performance and IDE integration.