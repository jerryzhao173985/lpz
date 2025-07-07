# LPZRobots Quick Start Guide

---
**Document Type**: Guide  
**Last Updated**: 2025-01-06  
**Status**: Current  
**Version**: 2.0  
**Tags**: quickstart, installation, tutorial, getting-started  
---

## 🚀 Get Started in 5 Minutes

### Prerequisites

- **macOS**: Xcode Command Line Tools
- **Linux**: GCC 11+ or Clang 14+
- **Dependencies**: Qt6, OpenSceneGraph, GSL

### 1. Install Dependencies

#### macOS (Homebrew)
```bash
brew install qt@6 open-scene-graph gsl readline gnuplot
```

#### Ubuntu/Debian
```bash
sudo apt-get install qt6-base-dev libopenscenegraph-dev \
    libgsl-dev libreadline-dev libncurses5-dev
```

### 2. Build LPZRobots

```bash
# Configure (choose user installation)
make conf

# Build everything (parallel)
make all -j8

# Or build specific components
make selforg ode_robots utils
```

### 3. Run Your First Simulation

```bash
cd ode_robots/simulations/template_sphererobot
make
./start -g -noshadow
```

**Controls:**
- `Space`: Pause/unpause
- `Mouse`: Rotate view
- `Scroll`: Zoom
- `x`: Show menu

## 📖 Understanding the Basics

### The Simulation Loop

```cpp
class MySim : public Simulation {
    OdeRobot* robot;
    AbstractController* controller;
    
    bool command(const OdeHandle&, GlobalData&, int key) {
        // Handle keyboard input
    }
    
    void start(const OdeHandle& ode, const OsgHandle& osg) {
        // Create robot
        robot = new Sphererobot3Masses(ode, osg, conf, "Robot1");
        robot->place(Pos(0,0,0.5));
        
        // Create controller
        controller = new Sox();
        
        // Create wiring
        auto* wiring = new One2OneWiring(new ColorUniformNoise(0.1));
        
        // Create agent (combines all)
        auto* agent = new OdeAgent(global);
        agent->init(controller, robot, wiring);
        
        // Add to simulation
        global.agents.push_back(agent);
        global.configs.push_back(controller);
    }
};
```

### Available Controllers

- **Sox**: Basic self-organizing exploration
- **APEX-Sox**: Advanced meta-learning controller
- **Sos**: Self-organizing sensitivity
- **SineController**: Simple sine wave generator

### Available Robots

- **Sphererobot3Masses**: Rolling sphere robot
- **FourWheeled**: Car-like robot
- **Hexapod**: Six-legged walker
- **Snake**: Snake-like chain robot

## 🛠️ Common Tasks

### Change Controller Parameters

```cpp
Sox* sox = new Sox();
sox->setParam("epsC", 0.1);   // Learning rate
sox->setParam("epsA", 0.05);  // Model adaptation rate
```

### Add Obstacles

```cpp
// Add a playground
auto* playground = new Playground(ode, osg, 
    osg::Vec3(10, 0.2, 0.5));  // size
global.obstacles.push_back(playground);

// Add boxes
auto* box = new PassiveBox(ode, osg, 
    osg::Vec3(1,1,1));  // size
box->setPosition(Pos(2,0,0));
global.obstacles.push_back(box);
```

### Use GUI Tools

```bash
# Start with GUI logger (real-time plots)
./start -g

# Start with parameter configurator
./start -c

# Save simulation video
./start -noshadow -f 10  # Record at 10 FPS
```

## 📊 Data Analysis

### Log Data
```cpp
// In your simulation
global.odeConfig.setParam("logfile", "data.log");
```

### Plot with guilogger
```bash
# In separate terminal while simulation runs
guilogger -p  # Starts in pipe mode
```

### Analyze with gnuplot
```bash
gnuplot
> plot "data.log" using 1:2 with lines
```

## 🔧 Troubleshooting

### macOS Issues
- **Shadow crashes**: Use `-noshadow`
- **Slow performance**: Use `-ns` (no graphics)

### Build Issues
- **Qt6 not found**: Check `brew list qt@6`
- **Warnings**: Normal for external libraries

### Runtime Issues
- **Segfault**: Often from uninitialized robots
- **No movement**: Check motor power range [-1,1]

## 📚 Next Steps

1. **Explore Examples**: `ode_robots/simulations/`
2. **Read Architecture**: [TECHNICAL_REFERENCE.md](TECHNICAL_REFERENCE.md)
3. **Create New Robot**: See templates in `ode_robots/robots/`
4. **Implement Controller**: Extend `AbstractController`

## 🎯 Pro Tips

- Use CMake build for better IDE support
- Enable sanitizers for debugging: `make asan`
- Profile performance: `make opt` for optimized build
- Test changes: `make test` runs all tests

---

**Need Help?** 
- Examples: `ode_robots/simulations/`
- Reference: [TECHNICAL_REFERENCE.md](TECHNICAL_REFERENCE.md)
- Full docs: [README.md](README.md)