# Simulation Fixes Summary

## Sphererobot Novel Controllers Simulation

### Fixed Issues:
1. **PassiveSphere include**: Added `#include <ode_robots/passivesphere.h>`
2. **Vec3 to Pos conversion**: Changed all `osg::Vec3` to `Pos` for position types
3. **showParams deprecation**: Removed deprecated showParams call
4. **Robot configuration**: Fixed member names:
   - `radius` → `diameter`
   - `pendulumMass` → `pendularmass`
   - `motorPower` → `motorpowerfactor`
5. **inspectables removal**: Removed usage of deprecated global.inspectables
6. **const correctness**: Fixed getController() const issues with dynamic_cast
7. **Method signatures**: Updated controller info methods to use available APIs

### Compilation Status:
The simulation should now compile with the standard build system. The main issues were:
- Type mismatches between osg::Vec3 and Pos
- Deprecated API usage
- Incorrect member names in robot configuration

## Hexapod Novel Controllers Simulation

### Fixed Issues:
1. **PassiveSphere include**: Added missing include

### Status:
Both main.cpp and hexapod_behaviors.cpp have been updated with necessary includes.

## Build Instructions:

### Using Make (Recommended):
```bash
cd ode_robots/simulations/sphererobot_novel_controllers
make clean
make

cd ../hexapod_novel_controllers
make clean
make
```

### Using CMake:
```bash
cd build
cmake ..
make sphererobot_novel_controllers_sim
```

## Known Issues:

1. **Include path structure**: The novel algorithm headers use angle brackets (`<selforg/...>`) which expects selforg to be in the system include path. This works with the standard build system but may need adjustment for standalone compilation.

2. **Warning suppressions**: There will be warnings from external libraries (OpenSceneGraph) which are expected and can be ignored.

## Running the Simulations:

### Sphererobot:
```bash
cd ode_robots/simulations/sphererobot_novel_controllers
./start              # Basic run
./start -g           # With guilogger
./start -noshadow    # Without shadows (faster)
```

### Hexapod:
```bash
cd ode_robots/simulations/hexapod_novel_controllers
./start              # Single hexapod with switchable controllers
./start hexapod_behaviors  # Multiple hexapods demo
```

## Controller Switching Keys:
- `1` - Sox (baseline)
- `2` - AH-Sox (Adaptive Horizon)
- `3` - MSHC (Multi-Scale)
- `4` - CASox (Context-Aware)
- `i` - Show controller info
- `h` - Show help

## What to Expect:

1. **Sphererobot simulation**: 
   - 4 spherical robots, each with a different controller
   - Obstacles for context variation
   - Visual comparison of behaviors

2. **Hexapod simulation**:
   - More complex robot morphology
   - Real-time controller switching
   - Status line shows algorithm-specific metrics

## Next Steps:

If compilation still fails:
1. Ensure selforg library is built with the novel algorithms
2. Check that all dependencies are installed (Qt6, OpenSceneGraph, etc.)
3. Use the make-based build system which handles include paths correctly
4. Check build logs for specific error messages

The simulations demonstrate the unique features of each algorithm:
- AH-Sox adapts prediction horizons
- MSHC coordinates multiple time scales
- CASox detects and switches between contexts