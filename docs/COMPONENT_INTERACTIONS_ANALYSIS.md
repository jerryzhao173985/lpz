# LPZRobots Component Interactions and Developer Workflows

## Overview

LPZRobots is a sophisticated modular robotics simulation framework with clear separation of concerns. The system follows a layered architecture where each component has well-defined responsibilities and interfaces.

## Architecture Layers

### 1. Core Layer Structure

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

### 2. Component Responsibilities

**selforg** - The brain of the system
- Abstract controllers implementing self-organization principles
- Wiring system for sensor-motor mappings
- Noise generators and filters
- Learning algorithms (homeokinetic, predictive, etc.)

**opende (ODE)** - Physics engine
- Rigid body dynamics
- Collision detection
- Joint constraints
- Integration with double precision

**ode_robots** - Simulation framework
- Robot implementations
- Environment objects (playgrounds, obstacles)
- Agent management (combines robot + controller + wiring)
- Visualization integration (OpenSceneGraph)
- Main simulation loop

**GUI Tools** - Visualization and analysis
- guilogger: Real-time plotting and data logging
- matrixviz: Neural network weight visualization
- configurator: Runtime parameter adjustment

## Key Design Patterns

### 1. Agent-Robot-Controller Pattern

The fundamental pattern separates concerns into three components:

```cpp
// 1. Robot - Physical body
OdeRobot* robot = new Sphererobot3Masses(odeHandle, osgHandle, conf, "MyRobot");

// 2. Controller - Brain/behavior
AbstractController* controller = new Sox();
controller->setParam("epsC", 0.3);  // learning rate

// 3. Wiring - Sensor-motor mapping
AbstractWiring* wiring = new One2OneWiring(new ColorUniformNoise());

// 4. Agent - Combines all three
OdeAgent* agent = new OdeAgent(global);
agent->init(controller, robot, wiring);
```

### 2. Wiring System

The wiring layer provides flexible sensor-motor mappings:

```cpp
class AbstractWiring {
    // Transforms robot sensors to controller inputs
    virtual bool wireSensors(const sensor* rsensors, int rsensornumber,
                           sensor* csensors, int csensornumber,
                           double noiseStrength);
    
    // Transforms controller outputs to robot motors
    virtual bool wireMotors(const motor* cmotors, int cmotornumber,
                          motor* rmotors, int rmotornumber);
};
```

Common wirings:
- **One2OneWiring**: Direct 1:1 mapping
- **DerivativeWiring**: Adds velocity information
- **FeedbackWiring**: Includes motor feedback
- **SelectiveWiring**: Filters specific sensors

### 3. Factory Pattern Implementation

Recent refactoring introduced factory patterns for centralized object creation:

```cpp
// Robot Factory
auto robot = RobotFactory::createRobot("Sphererobot3Masses", 
                                      odeHandle, osgHandle, "MyRobot");

// Controller Factory with categories
auto controller = ControllerFactory::createController("Sox");
auto types = ControllerFactory::getControllersByCategory(
    ControllerFactory::HOMEOKINETIC);
```

### 4. Configurable System

All major components inherit from `Configurable`:
- Runtime parameter adjustment
- Automatic GUI integration
- Serialization support
- Hierarchical organization

## Inter-Process Communication

### GUI Tool Integration

The system uses **Unix pipes** for real-time communication with external tools:

```cpp
// In PlotOption::open()
switch (mode) {
    case PlotMode::GuiLogger:
        pipe = popen("guilogger -m pipe", "w");
        break;
    case PlotMode::MatrixViz:
        pipe = popen("matrixviz -noCtrlC -novideo", "w");
        break;
}

// Data streaming
fprintf(pipe, "%f %f %f\n", sensor1, sensor2, motor1);
fflush(pipe);
```

### Data Flow
1. **Simulation → GUI Tools**: Via pipes using simple text protocol
2. **Format**: Timestamped sensor/motor values, parameter updates
3. **Control**: `#QUIT` command for clean shutdown
4. **Channels**: Named channels with filtering support

## Typical Developer Workflows

### 1. Creating a New Simulation

```bash
# Use template to start
cd ode_robots/simulations
./createNewSimulation.sh template_sphererobot my_experiment
cd my_experiment
```

Edit `main.cpp`:
```cpp
class ThisSim : public Simulation {
    void start(const OdeHandle& odeHandle, const OsgHandle& osgHandle, 
               GlobalData& global) {
        // 1. Create environment
        Playground* playground = new Playground(odeHandle, osgHandle, 
                                              osg::Vec3(10, 0.2, 1));
        global.obstacles.push_back(playground);
        
        // 2. Create robot
        Sphererobot3Masses* robot = new Sphererobot3Masses(...);
        robot->place(Pos(0, 0, 0.5));
        
        // 3. Create controller
        Sox* controller = new Sox();
        controller->setParam("epsC", 0.3);
        
        // 4. Create agent
        OdeAgent* agent = new OdeAgent(global);
        agent->init(controller, robot, new One2OneWiring());
        global.agents.push_back(agent);
    }
};
```

### 2. Running with GUI Tools

```bash
# Build
make

# Run with guilogger (plotting)
./start -g 5              # Update every 5 steps

# Run with matrixviz (weight visualization)
./start -m 10             # Update every 10 steps

# Run with configurator (parameter tuning)
./start -conf

# Run with all tools
./start -g 5 -m 10 -conf

# Run headless with file logging
./start -nographics -f 1
```

### 3. Creating Custom Robots

```cpp
class MyRobot : public OdeRobot {
    std::vector<Primitive*> bodies;
    std::vector<Joint*> joints;
    std::vector<Motor*> motors;
    
    void place(const osg::Matrix& pose) override {
        // Create physics bodies
        Primitive* body = new Box(odeHandle, osgHandle, 
                                 osg::Vec3(1, 0.5, 0.3));
        body->setPose(pose);
        bodies.push_back(body);
        
        // Create joints
        HingeJoint* joint = new HingeJoint(body1, body2, 
                                          anchor, axis);
        joints.push_back(joint);
        
        // Create motors
        motors.push_back(joint->getMotor(0));
    }
    
    int getSensorNumber() const override { 
        return sensors.size(); 
    }
    
    int getMotorNumber() const override { 
        return motors.size(); 
    }
};
```

### 4. Creating Custom Controllers

```cpp
class MyController : public AbstractController {
    Matrix A, C, h;  // Model parameters
    
    void init(int sensornumber, int motornumber, 
              RandGen* randGen = nullptr) override {
        A.set(motornumber, sensornumber);
        C.set(motornumber, motornumber);  
        h.set(motornumber, 1);
    }
    
    void step(const sensor* sensors, int number_sensors,
              motor* motors, int number_motors) override {
        // Control algorithm
        Matrix x(number_sensors, 1, sensors);
        Matrix y = (C * y_buffer.get(-1) + A * x + h).map(g);
        y.convertToBuffer(motors, number_motors);
        
        // Learning/adaptation
        learn(x, y);
    }
};
```

### 5. Parameter Exploration Workflow

1. **Initial Run**: Start with default parameters
2. **Live Tuning**: Use configurator GUI to adjust parameters
3. **Observation**: Watch guilogger plots for behavior changes
4. **Weight Analysis**: Use matrixviz to see internal model changes
5. **Save Config**: Export successful parameter sets

### 6. Data Analysis Workflow

```bash
# Log data to file
./start -nographics -f 1 mylog

# Creates timestamped log file
# e.g., mylog_2025-01-28_12-30-45.log

# Analyze with external tools
gnuplot plot_script.gp
python analyze_behavior.py mylog*.log
```

## Simulation Loop Flow

```
main()
 └─> Simulation::run()
      └─> start() [user-defined initialization]
      └─> simulation loop:
           ├─> handle events/keys
           ├─> for each agent:
           │    ├─> robot->sense() 
           │    ├─> wiring->wireSensors()
           │    ├─> controller->step()
           │    ├─> wiring->wireMotors()
           │    └─> robot->act()
           ├─> physics step (ODE)
           ├─> graphics update (OSG)
           └─> logging/plotting
```

## Key Integration Points

### 1. Handle System
- **OdeHandle**: Access to physics world, spaces, substances
- **OsgHandle**: Access to graphics, colors, textures
- **GlobalData**: Shared simulation state, agent lists, configuration

### 2. Sensor/Motor Interface
- Simple double arrays for maximum flexibility
- Normalized ranges (typically -1 to 1)
- Wiring provides transformations and filtering

### 3. Time Management
- Fixed timestep (typically 0.01s)
- Control can run at different intervals
- Synchronization with GUI tools via timestamps

### 4. Configuration Management
- Hierarchical parameter system
- Runtime modification support
- Automatic GUI integration
- Persistence via store/restore

## Best Practices

1. **Modularity**: Keep robots, controllers, and wirings independent
2. **Configuration**: Use Configurable interface for all parameters
3. **Factories**: Use factory patterns for object creation
4. **Smart Pointers**: Prefer unique_ptr/shared_ptr over raw pointers
5. **Logging**: Use built-in plotting system for data collection
6. **Testing**: Test components individually before integration

## Common Pitfalls

1. **Memory Management**: Agent manages controller/robot lifetime
2. **Coordinate Systems**: Robot uses local, simulation uses global
3. **Update Order**: Sensors read before motors write
4. **Thread Safety**: GUI tools run in separate processes
5. **Performance**: Matrix operations are bottleneck - reuse when possible

This architecture enables rapid prototyping of robot behaviors while maintaining clean separation between physics, control, and visualization components.