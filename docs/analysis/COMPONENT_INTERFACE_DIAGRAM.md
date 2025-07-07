# LPZRobots Component Interface and Data Flow Diagram

## Component Communication Patterns

### 1. Simulation Loop Architecture

```
┌─────────────────────────────────────────────────────────────────────┐
│                        Simulation::start()                           │
│  ┌────────────────────────────────────────────────────────────────┐ │
│  │                    Main Simulation Loop                         │ │
│  │                                                                 │ │
│  │  while(running) {                                              │ │
│  │    1. Physics Step (ODE)                                       │ │
│  │    2. Sensor Update                                            │ │
│  │    3. Agent Step (Controller)                                  │ │
│  │    4. Motor Update                                              │ │
│  │    5. Visualization (OSG)                                      │ │
│  │    6. Logging/GUI Update                                       │ │
│  │  }                                                             │ │
│  └────────────────────────────────────────────────────────────────┘ │
└─────────────────────────────────────────────────────────────────────┘
```

### 2. Inter-Component Communication

```
┌─────────────────────────────────────────────────────────────────────┐
│                      Component Interfaces                            │
├─────────────────────────────────────────────────────────────────────┤
│                                                                     │
│  OdeRobot ←→ OdeAgent ←→ AbstractController                       │
│     ↑            ↑              ↑                                  │
│     │            │              │                                  │
│     │            └── AbstractWiring                                │
│     │                                                              │
│     └── Sensors/Motors ←→ Primitives ←→ ODE Physics              │
│                                                                     │
├─────────────────────────────────────────────────────────────────────┤
│                      Data Channels                                  │
├─────────────────────────────────────────────────────────────────────┤
│                                                                     │
│  Sensor Data:  double[] → Wiring → double[] → Controller          │
│  Motor Data:   Controller → double[] → Wiring → double[] → Robot  │
│  Config Data:  Configurable::setParam() → Runtime Update          │
│  Plot Data:    Inspectable → PlotOption → stdout → guilogger      │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

### 3. Detailed Component Dependencies

```yaml
selforg:
  dependencies:
    external:
      - GSL (GNU Scientific Library)
      - pthread
      - configurator (optional)
    produces:
      - libselforg.a
      - libselforg_opt.a
      - headers: controller/, matrix/, utils/, wirings/
  key_interfaces:
    - AbstractController
    - AbstractWiring
    - AbstractRobot
    - Configurable
    - Inspectable
    - Storeable

ode_robots:
  dependencies:
    internal:
      - selforg (headers + lib)
    external:
      - OpenDE (physics)
      - OpenSceneGraph (graphics)
      - OpenGL/GLU
    produces:
      - libode_robots.a
      - libode_robots_opt.a
      - headers: robots/, sensors/, motors/, obstacles/
  key_interfaces:
    - OdeRobot : public AbstractRobot
    - OdeAgent : public Agent
    - Primitive (3D objects)
    - Joint (constraints)

guilogger:
  dependencies:
    external:
      - Qt6 (Core, Widgets, OpenGL)
      - gnuplot (subprocess)
    communication:
      - stdin pipe (text protocol)
      - channel format: "#C name value1 value2..."
  standalone: true

matrixviz:
  dependencies:
    external:
      - Qt6 (Core, Widgets, OpenGL)
    communication:
      - stdin pipe (matrix data)
      - binary/text matrix format
  standalone: true

configurator:
  dependencies:
    internal:
      - selforg (Configurable interface)
    external:
      - Qt6 (Core, Widgets)
    produces:
      - libconfigurator.a
      - ConfiguratorProxy
  communication:
      - TCP/IP socket protocol
      - Parameter get/set messages

ga_tools:
  dependencies:
    internal:
      - selforg (Inspectable, RandGen)
    produces:
      - libga_tools.a
  key_interfaces:
    - Individual : public Inspectable
    - SingletonGenEngine
    - IFitnessStrategy
```

### 4. Wiring System Architecture

```
┌─────────────────────────────────────────────────────────────────────┐
│                        Wiring System                                 │
├─────────────────────────────────────────────────────────────────────┤
│                                                                     │
│  Robot Sensors ──┐                                                 │
│  (n sensors)     │                                                 │
│                  ▼                                                 │
│             AbstractWiring                                         │
│                  │                                                 │
│                  ├── One2OneWiring (direct mapping)               │
│                  ├── FeedbackWiring (motor feedback)              │
│                  ├── DerivativeWiring (velocity info)             │
│                  ├── SelectiveNoiseWiring                         │
│                  └── RandomWiring                                 │
│                  │                                                 │
│                  ▼                                                 │
│  Controller Inputs ──┐                                            │
│  (m inputs)          │                                            │
│                      ▼                                            │
│               AbstractController::step()                          │
│                      │                                            │
│                      ▼                                            │
│  Controller Outputs ─┐                                            │
│  (m outputs)         │                                            │
│                      ▼                                            │
│                 AbstractWiring                                    │
│                      │                                            │
│                      ▼                                            │
│  Robot Motors ───────┘                                            │
│  (k motors)                                                       │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

### 5. Configuration & Inspection Flow

```
┌─────────────────────────────────────────────────────────────────────┐
│                  Configuration System                                │
├─────────────────────────────────────────────────────────────────────┤
│                                                                     │
│  Configurable Interface:                                           │
│    - getParamList() → list of parameters                         │
│    - getParam(key) → value                                        │
│    - setParam(key, value) → update                               │
│    - getParamBounds(key) → min/max                               │
│                                                                     │
│  Hierarchy:                                                        │
│    Simulation                                                      │
│      └── OdeAgent (named: "Robot1")                              │
│            ├── OdeRobot : Configurable                           │
│            ├── AbstractController : Configurable                 │
│            └── AbstractWiring : Configurable                     │
│                                                                     │
│  Access Pattern:                                                   │
│    "Robot1.controller.epsC" → agent→controller→setParam()        │
│                                                                     │
├─────────────────────────────────────────────────────────────────────┤
│                   Inspection System                                 │
├─────────────────────────────────────────────────────────────────────┤
│                                                                     │
│  Inspectable Interface:                                            │
│    - getInternalParams() → matrix list                           │
│    - getInternalParamNames() → name list                         │
│                                                                     │
│  Data Flow:                                                        │
│    Controller matrices → Inspectable → PlotOption →              │
│    → stdout → guilogger → real-time plots                        │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

### 6. Typical Simulation Initialization

```cpp
// 1. Create environment
Playground* playground = new Playground(odeHandle, osgHandle, 
                                      osg::Vec3(10, 0.2, 0.5));
global.obstacles.push_back(playground);

// 2. Create robot
OdeRobot* robot = new Sphererobot3Masses(odeHandle, osgHandle, 
                                         conf, "Sphererobot");
robot->place(Pos(0,0,0.5));

// 3. Create controller  
AbstractController* controller = new Sox();
controller->setParam("epsC", 0.1);
controller->setParam("epsA", 0.05);

// 4. Create wiring
AbstractWiring* wiring = new One2OneWiring(new ColorUniformNoise(0.1));

// 5. Create agent (combines all)
OdeAgent* agent = new OdeAgent(global);
agent->init(controller, robot, wiring);
global.agents.push_back(agent);

// 6. Configure logging
global.odeConfig.setParam("controlinterval", 1);
agent->setTrackOptions(TrackRobot(true, true, false, true, "robot", 50));
```

### 7. Plugin Architecture (ga_tools)

```
┌─────────────────────────────────────────────────────────────────────┐
│                    GA Tools Integration                              │
├─────────────────────────────────────────────────────────────────────┤
│                                                                     │
│  Genetic Algorithm Components:                                      │
│                                                                     │
│    Individual                                                       │
│      ├── Genes (IValue)                                           │
│      ├── Fitness                                                  │
│      └── Configurable mapping                                     │
│                                                                     │
│    Generation                                                       │
│      ├── Population                                               │
│      ├── Selection (ISelectStrategy)                              │
│      ├── Crossover                                                │
│      └── Mutation (IMutationStrategy)                             │
│                                                                     │
│    SingletonGenEngine                                              │
│      ├── Runs generations                                         │
│      ├── Applies fitness (IFitnessStrategy)                      │
│      └── Manages evolution                                        │
│                                                                     │
│  Integration with Controllers:                                     │
│    - GA individuals → Controller parameters                       │
│    - Fitness evaluation → Simulation runs                         │
│    - Best individual → Optimized controller                       │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

### 8. Memory Management Patterns

```cpp
// Modern C++17 patterns in use:

// 1. Smart pointers for ownership
std::unique_ptr<OdeRobot> robot = 
    RobotFactory::createRobot("Sphererobot3Masses", odeHandle, osgHandle);

// 2. Circular buffers for history
lpzrobots::CircularBuffer<Matrix, 50> x_buffer;
x_buffer.push(currentSensorValues);
const Matrix& oldValues = x_buffer.get(-1); // Previous value

// 3. Factory pattern for object creation
auto controller = ControllerFactory::createController("Sox");

// 4. RAII for resource management
class OdeRobot {
    ~OdeRobot() override {
        cleanup(); // Automatic cleanup
    }
};

// 5. Rule of 5 enforcement
class OdeRobot {
    OdeRobot(const OdeRobot&) = delete;
    OdeRobot& operator=(const OdeRobot&) = delete;
    OdeRobot(OdeRobot&&) = delete;
    OdeRobot& operator=(OdeRobot&&) = delete;
};
```

### 9. Thread Safety Considerations

```
┌─────────────────────────────────────────────────────────────────────┐
│                      Thread Safety                                   │
├─────────────────────────────────────────────────────────────────────┤
│                                                                     │
│  Single-threaded components:                                       │
│    - Main simulation loop                                         │
│    - Controller step()                                            │
│    - Robot sensor/motor updates                                   │
│                                                                     │
│  Thread-safe components:                                           │
│    - RandGen (thread-local generators)                           │
│    - Logging (mutex-protected)                                    │
│                                                                     │
│  Parallel physics (optional):                                      │
│    - ODE with threading enabled                                   │
│    - dInitODE2(0)                                                │
│    - dAllocateODEDataForThread()                                 │
│                                                                     │
│  GUI communication:                                                │
│    - Asynchronous via pipes/sockets                              │
│    - No shared memory with simulation                            │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

This architecture demonstrates:
- Clean separation between physics, control, and visualization
- Well-defined interfaces between components
- No circular dependencies
- Extensible plugin architecture
- Modern C++ patterns throughout
- Clear data flow paths
- Proper encapsulation and abstraction