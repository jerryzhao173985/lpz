# Quick Start Tutorial

Get your first LPZRobots simulation running in 10 minutes!

## Prerequisites

- LPZRobots installed (see [Installation Guide](installation.md))
- Basic familiarity with command line
- Text editor for code modifications

## Your First Simulation

### Step 1: Navigate to Template Simulation

```bash
cd ~/lpzrobots/ode_robots/simulations/template_sphererobot
```

### Step 2: Build the Simulation

```bash
make clean  # Clean any previous builds
make        # Build the simulation
```

### Step 3: Run the Simulation

```bash
# Basic run
./start

# With GUI logger (recommended)
./start -g

# macOS users: add -noshadow if graphics crash
./start -g -noshadow
```

### Step 4: Control the Simulation

#### Keyboard Controls
- **Space**: Pause/unpause simulation
- **x**: Reset robot position
- **v**: Toggle video recording
- **q**: Quit simulation
- **F1-F12**: Camera views
- **Mouse**: Rotate view (left button), zoom (right button)

#### GUI Logger Controls
- Click channels to show/hide
- Right-click for options
- Drag to rearrange plots

## Understanding the Code

### Main Simulation File

Open `main.cpp` to see the simulation structure:

```cpp
class ThisSim : public Simulation {
  public:
    // Initialize robots and controllers
    bool start(const OdeHandle& odeHandle, 
               const OsgHandle& osgHandle, 
               GlobalData& global) {
        
        // Create robot
        Sphererobot3Masses* robot = 
            new Sphererobot3Masses(odeHandle, osgHandle, conf, "Sphere");
        
        // Create controller
        Sox* controller = new Sox();
        controller->setParam("epsC", 0.2);
        controller->setParam("epsA", 0.1);
        
        // Create wiring
        One2OneWiring* wiring = new One2OneWiring(new ColorUniformNoise(0.1));
        
        // Create agent
        OdeAgent* agent = new OdeAgent(global);
        agent->init(controller, robot, wiring);
        
        global.agents.push_back(agent);
        global.configs.push_back(controller);
        
        return true;
    }
};
```

## Making Changes

### Change Controller Parameters

Edit the controller initialization:

```cpp
// Increase learning rate for more dynamic behavior
controller->setParam("epsC", 0.5);  // was 0.2
controller->setParam("epsA", 0.3);  // was 0.1
```

### Change Robot Configuration

```cpp
// Make robot larger
conf.diameter = 2.0;  // was 1.0
conf.pendularMass = 0.5;  // was 0.2
```

### Add Multiple Robots

```cpp
// In start() method, add loop:
for(int i = 0; i < 3; i++) {
    Sphererobot3Masses* robot = 
        new Sphererobot3Masses(odeHandle, osgHandle, conf, 
                               "Sphere" + std::to_string(i));
    
    // Position robots in a line
    robot->place(osg::Matrix::translate(i*2, 0, 0.5));
    
    // ... rest of agent creation
}
```

## Visualization Options

### Using GuiLogger

```bash
# Start with GUI logger
./start -g

# In the logger window:
# - Click channel names to toggle visibility
# - Use mouse wheel to zoom time axis
# - Press 'p' to pause data collection
# - Press 's' to save data
```

### Using MatrixViz

```bash
# For neural network visualization
./start -m 1

# In MatrixViz window:
# - Ctrl+V: Toggle update (not Ctrl+M as shown)
# - Mouse: Rotate 3D view
# - Observe weight matrix changes in real-time
```

## Common Modifications

### 1. Different Controller

```cpp
// Replace Sox with DEP controller
#include <selforg/dep.h>

DEP* controller = new DEP();
controller->setParam("synboost", 1.5);
```

### 2. Different Robot

```cpp
// Use wheeled robot instead
#include <ode_robots/nimm2.h>

Nimm2* robot = new Nimm2(odeHandle, osgHandle);
```

### 3. Add Obstacles

```cpp
// Add a playground
Playground* playground = 
    new Playground(odeHandle, osgHandle, 
                   osg::Vec3(10, 0.2, 0.5));
playground->setPosition(osg::Vec3(0, 0, 0));
global.obstacles.push_back(playground);
```

## Next Steps

1. **Explore Examples**
   - Browse `ode_robots/simulations/` for more examples
   - Try `sphererobot_switchcontroller` for advanced control
   - Check `multiagent_herding` for collective behavior

2. **Learn Controllers**
   - Read [Controllers Guide](../user-guide/controllers.md)
   - Experiment with parameters in real-time
   - Try novel controllers (APEX, CASox)

3. **Create Custom Simulation**
   ```bash
   cd ~/lpzrobots/ode_robots/simulations
   ../createNewSimulation.sh template_sphererobot my_experiment
   cd my_experiment
   # Edit main.cpp
   make && ./start -g
   ```

## Troubleshooting

### Build Errors
```bash
# Check dependencies
make clean
make 2>&1 | grep error

# Verbose build
make VERBOSE=1
```

### Runtime Issues
```bash
# Debug mode
./start -pause  # Start paused
./start -noshadow  # Disable shadows (macOS)
./start -notex  # Disable textures
```

### Performance
```bash
# Optimize build
make opt
./start_opt -g

# Reduce graphics
./start -noshadow -notex -noshade
```

## Getting Help

- Check [Example Simulations](examples.md) for working code
- Read error messages carefully - they're informative
- Use `-h` flag for command options: `./start -h`
- Join the community discussions on GitHub

---

Congratulations! You've run your first LPZRobots simulation. The self-organizing controller is now learning to control the robot through trial and error, developing its own movement patterns.