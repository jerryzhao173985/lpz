# LPZRobots macOS ARM64 Distribution

Version: 0.8.0
Architecture: arm64
Build Date: Mon Jun 23 11:27:16 BST 2025

## Installation

1. Extract this archive to your desired location
2. Add the bin directory to your PATH:
   export PATH=$PATH:/Users/jerry/lpzrobot_mac/dist/lpzrobots-macos-arm64-0.8.0/bin

3. Set library paths:
   export DYLD_LIBRARY_PATH=$DYLD_LIBRARY_PATH:/Users/jerry/lpzrobot_mac/dist/lpzrobots-macos-arm64-0.8.0/lib

4. For development, you may want to set:
   export LPZROBOTS_HOME=/Users/jerry/lpzrobot_mac/dist/lpzrobots-macos-arm64-0.8.0

## Usage

### Creating a new simulation:
```bash
cd examples
cp -r template_sphererobot my_simulation
cd my_simulation
make
./start
```

### Dependencies

Make sure you have the following installed via Homebrew:
- OpenSceneGraph: brew install open-scene-graph
- ODE: brew install ode
- Qt5: brew install qt@5
- GSL: brew install gsl
- readline: brew install readline

## Components Included

- selforg: Self-organizing control library
- ode_robots: Robot simulation framework
- configurator: Qt5-based configuration GUI
- guilogger: Real-time parameter visualization
- matrixviz: Matrix visualization tool

## Documentation

See https://github.com/georgmartius/lpzrobots for full documentation.

