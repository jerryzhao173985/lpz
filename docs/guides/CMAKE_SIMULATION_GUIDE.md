# CMake Simulation Build Guide

## Quick Start

Since LPZRobots has migrated to CMake, the old `make` commands in simulation directories no longer work. Here's how to build and run simulations with CMake.

## Building Simulations

### Method 1: Using the Build Script (Recommended)

```bash
# From the lpzrobots root directory:
./build_and_run_simulation.sh hexapod_novel_controllers
./build_and_run_simulation.sh sphererobot_novel_controllers
./build_and_run_simulation.sh template_hexapod

# With options:
./build_and_run_simulation.sh --debug template_hexapod      # Debug build
./build_and_run_simulation.sh -c sphererobot_novel_controllers  # Clean build
./build_and_run_simulation.sh -n template_hexapod           # Build only, don't run
```

### Method 2: Direct CMake Commands

```bash
# From lpzrobots root directory:
mkdir -p build
cd build
cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_EXAMPLES=ON ..
make hexapod_novel_controllers_sim -j4
make sphererobot_novel_controllers_sim -j4
```

### Method 3: Build All Simulations

```bash
cd build
cmake -DBUILD_EXAMPLES=ON ..
make -j4  # Builds everything including simulations
```

## Running Simulations

After building, simulations are located in:
```
build/ode_robots/simulations/<simulation_name>/start
```

### Run from build directory:
```bash
cd build/ode_robots/simulations/hexapod_novel_controllers
./start                      # Basic run
./start -g                   # With guilogger
./start -noshadow            # Without shadows (faster)
./hexapod_behaviors          # Multi-robot demo (if available)
```

### Run with original directory structure:
```bash
cd ode_robots/simulations/hexapod_novel_controllers
../../../build/ode_robots/simulations/hexapod_novel_controllers/start
```

## Available Simulations with Novel Controllers

1. **sphererobot_novel_controllers**
   - Demonstrates all novel algorithms on sphere robots
   - 4 robots with Sox, AH-Sox, MSHC, and CASox

2. **hexapod_novel_controllers**
   - Hexapod robot with switchable controllers
   - Press 1-4 to switch between algorithms
   - Shows algorithm-specific metrics in status line

## Common Issues and Solutions

### Issue: "call ./configure or make all in lpzrobots again!"
**Solution**: This error occurs when using old Makefiles. Use CMake instead:
```bash
cd <lpzrobots_root>
./build_and_run_simulation.sh <simulation_name>
```

### Issue: Cannot find simulation executable
**Solution**: Ensure you built with examples enabled:
```bash
cmake -DBUILD_EXAMPLES=ON ..
```

### Issue: Simulation not in CMake list
**Solution**: Add it to `ode_robots/simulations/CMakeLists.txt`:
```cmake
set(EXAMPLE_SIMULATIONS
    # ... existing simulations ...
    your_new_simulation
)
```

## Creating New Simulations

1. Create directory: `ode_robots/simulations/my_simulation/`
2. Add `main.cpp` with simulation code
3. Create `CMakeLists.txt`:
```cmake
include(${CMAKE_CURRENT_SOURCE_DIR}/../../../cmake/SimulationBuild.cmake)
lpzrobots_add_simulation(my_simulation_sim
    SOURCES main.cpp
)
```
4. Add to parent `CMakeLists.txt` simulation list
5. Build with: `./build_and_run_simulation.sh my_simulation`

## CMake Build Options

```bash
cmake -DCMAKE_BUILD_TYPE=Debug ..           # Debug build
cmake -DCMAKE_BUILD_TYPE=Release ..         # Optimized build
cmake -DBUILD_SHARED_LIBS=ON ..             # Build shared libraries
cmake -DBUILD_EXAMPLES=ON ..                # Include simulations
cmake -DCMAKE_INSTALL_PREFIX=/usr/local ..  # Set install path
```

## Tips

1. **Parallel builds**: Use `-j4` or `-j8` for faster compilation
2. **Incremental builds**: CMake only rebuilds changed files
3. **Clean builds**: `rm -rf build/*` for a fresh start
4. **Debug runs**: Use `lldb` or `gdb` with debug builds
5. **Performance**: Use Release build for better performance

## VS Code Integration

If using VS Code, press `Ctrl+Shift+B` to build, or:
1. Open Command Palette (`Ctrl+Shift+P`)
2. Run "CMake: Build"
3. Select target (e.g., `hexapod_novel_controllers_sim`)

## Summary

The migration to CMake provides:
- Better dependency management
- Faster incremental builds
- Cross-platform compatibility
- Modern IDE integration
- Easier debugging

For any simulation that previously used `make`, now use:
```bash
./build_and_run_simulation.sh <simulation_name>
```