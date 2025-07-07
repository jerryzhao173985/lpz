# LPZRobots Quick Start Guide

Get up and running with LPZRobots in 5 minutes!

## 🚀 Installation

### macOS (Apple Silicon/Intel)

```bash
# Install dependencies
brew install cmake qt@6 open-scene-graph gsl readline

# Clone and build
git clone https://github.com/georgmartius/lpzrobots.git
cd lpzrobots
cmake --preset default
cmake --build --preset default
```

### Ubuntu/Debian

```bash
# Install dependencies
sudo apt-get update
sudo apt-get install -y build-essential cmake qt6-base-dev \
    libopenscenegraph-dev libgsl-dev libreadline-dev libglu1-mesa-dev

# Clone and build
git clone https://github.com/georgmartius/lpzrobots.git
cd lpzrobots
cmake --preset default
cmake --build --preset default
```

## 🎮 Your First Simulation

### 1. Run a Template Simulation

```bash
cd ode_robots/simulations/template_sphererobot
mkdir build && cd build
cmake ..
make
./template_sphererobot
```

### 2. Control the Simulation

**Keyboard Controls:**
- `x` - Show/hide console menu
- `g` - Show/hide GUI logger
- `v` - Toggle video recording
- `Space` - Pause/unpause
- `f` - Fullscreen
- `Esc` - Quit

**Console Commands (after pressing 'x'):**
- `set <param> <value>` - Change controller parameters
- `show` - Display all parameters
- `store` - Save current configuration
- `help` - Show all commands

### 3. Visualize Data

The simulation automatically starts `guilogger` for real-time plotting:
- Motor values
- Sensor readings
- Controller internal states

## 🤖 Create Your Own Robot

### Simple Example: Custom Sphere Robot

Create `my_robot.cpp`:

```cpp
#include <ode_robots/simulation.h>
#include <ode_robots/odeagent.h>
#include <ode_robots/playground.h>
#include <selforg/sox.h>
#include <ode_robots/sphererobot3masses.h>

using namespace lpzrobots;

class MySimulation : public Simulation {
public:
    void start(const OdeHandle& odeHandle, const OsgHandle& osgHandle, 
               GlobalData& global) override {
        
        // Create arena
        Playground* playground = new Playground(odeHandle, osgHandle, 
            osg::Vec3(10, 0.2, 0.5));
        playground->setPosition(osg::Vec3(0,0,0));
        global.obstacles.push_back(playground);
        
        // Create robot
        Sphererobot3MassesConf conf = Sphererobot3Masses::getDefaultConf();
        conf.radius = 0.3;
        conf.pendularMass = 0.2;
        
        OdeRobot* robot = new Sphererobot3Masses(odeHandle, osgHandle, 
            conf, "MySphere");
        robot->place(osg::Matrix::translate(0,0,1));
        
        // Create controller
        Sox* controller = new Sox();
        controller->setParam("epsC", 0.1);
        controller->setParam("epsA", 0.05);
        
        // Create agent and initialize
        OdeAgent* agent = new OdeAgent(global);
        agent->init(controller, robot, new One2OneWiring(new ColorUniformNoise(0.1)));
        global.agents.push_back(agent);
        global.configs.push_back(agent);
    }
};

int main(int argc, char** argv) {
    MySimulation sim;
    return sim.run(argc, argv) ? 0 : 1;
}
```

### Build and Run:

```bash
mkdir build && cd build
cmake ..
make
./my_robot
```

## 📊 Analyze Results

### Live Plotting with guilogger

Data is automatically sent to guilogger. Customize plots:

1. Right-click on plot → "Add Channel"
2. Select sensors/motors to visualize
3. Adjust time window and scaling
4. Save plots as images or data

### Record Data

```bash
./my_robot -g -l  # Start with logging
# Data saved to *.log files
```

### Process with gnuplot

```bash
gnuplot
> plot "robot.log" using 1:2 with lines title "Sensor 0"
> replot "robot.log" using 1:3 with lines title "Motor 0"
```

## 🎨 Common Modifications

### Change Controller Parameters

```cpp
controller->setParam("epsC", 0.2);    // Learning rate
controller->setParam("epsA", 0.1);    // Model adaptation
controller->setParam("s4avg", 10);    // Averaging window
```

### Add Obstacles

```cpp
// Add a box
Box* box = new Box(0.5, 0.5, 0.5);
box->setPosition(osg::Vec3(2, 0, 0));
global.obstacles.push_back(box);

// Add a sphere
Sphere* sphere = new Sphere(0.3);
sphere->setPosition(osg::Vec3(-2, 0, 0.5));
global.obstacles.push_back(sphere);
```

### Multiple Robots

```cpp
for(int i = 0; i < 5; i++) {
    OdeRobot* robot = new Sphererobot3Masses(...);
    robot->place(osg::Matrix::translate(i*2, 0, 1));
    // ... create controller and agent
}
```

## 🔧 Troubleshooting

### macOS Issues

**OpenGL Crashes:**
```bash
./start -noshadow  # Disable shadows
./start -nohud     # Disable HUD (default on macOS)
```

**Performance:**
```bash
./start_opt        # Use optimized build
```

### Linux Issues

**Missing Libraries:**
```bash
ldd ./template_sphererobot  # Check dependencies
sudo ldconfig               # Refresh library cache
```

### Common Problems

1. **Black screen**: Try `-noshadow -nographics`
2. **Slow performance**: Use Release build: `cmake --preset release`
3. **Can't find libraries**: Set `LD_LIBRARY_PATH` (Linux) or check `DYLD_LIBRARY_PATH` (macOS)

## 📚 Next Steps

1. **Explore Examples**: Check `ode_robots/simulations/` for more robots
2. **Read Documentation**: See architecture in [DEEP_UNDERSTANDING_ANALYSIS.md](DEEP_UNDERSTANDING_ANALYSIS.md)
3. **Create Controllers**: Implement your own learning algorithms
4. **Join Community**: Report issues on GitHub

## 🎯 Quick Reference

### Build Commands
```bash
cmake --preset default        # Configure
cmake --build --preset default # Build all
cmake --build build --target selforg # Build specific
```

### Useful Simulations
- `template_sphererobot` - Basic sphere robot
- `template_schlangeforce` - Snake robot
- `humanoid` - Humanoid robot
- `hexapod` - Six-legged walker

Happy robot learning! 🤖