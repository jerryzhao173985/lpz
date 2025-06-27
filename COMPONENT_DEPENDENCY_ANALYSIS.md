# LPZRobots Component Dependency Analysis

## 1. Component Dependency Graph

### Build Order Dependencies

```
opende (ODE Physics)
    ↓
selforg (Controllers & Core)
    ↓
ode_robots (Simulation Framework)
    ↓
┌────────┬─────────┬───────────┬────────────┬───────────┐
│guilogger│matrixviz│configurator│ ga_tools   │simulations│
└────────┴─────────┴───────────┴────────────┴───────────┘
```

### Component Relationships

```
┌─────────────────────────────────────────────────────────────┐
│                      Application Layer                       │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────────┐  │
│  │ Simulations  │  │  GUI Tools   │  │    GA Tools      │  │
│  │              │  │  - guilogger │  │                  │  │
│  │              │  │  - matrixviz │  │                  │  │
│  │              │  │  - config.   │  │                  │  │
│  └──────┬───────┘  └───────┬──────┘  └────────┬─────────┘  │
└─────────┼───────────────────┼─────────────────┼────────────┘
          ↓                   ↓                 ↓
┌─────────┴───────────────────┴─────────────────┴────────────┐
│                     ode_robots Framework                    │
│  ┌─────────────┐  ┌──────────────┐  ┌─────────────────┐   │
│  │   Agents    │  │    Robots    │  │    Obstacles    │   │
│  │  OdeAgent   │  │   OdeRobot   │  │ AbstractGround  │   │
│  └──────┬──────┘  └───────┬──────┘  └────────┬────────┘   │
│         ↓                  ↓                  ↓            │
│  ┌─────────────┐  ┌──────────────┐  ┌─────────────────┐   │
│  │   Motors    │  │   Sensors    │  │   Primitives    │   │
│  │  OneAxis    │  │   IRSensor   │  │   Box/Sphere    │   │
│  │  TwoAxis    │  │   Camera     │  │   Cylinder      │   │
│  └──────┬──────┘  └───────┬──────┘  └────────┬────────┘   │
│         ↓                  ↓                  ↓            │
│  ┌──────────────────────────────────────────────────────┐  │
│  │            OSG/ODE Integration Layer                  │  │
│  │  OdeHandle, OsgHandle, Primitive, Joint              │  │
│  └──────────────────────────────────────────────────────┘  │
└─────────────────────────────┬───────────────────────────────┘
                              ↓
┌─────────────────────────────┴───────────────────────────────┐
│                      selforg Library                        │
│  ┌─────────────┐  ┌──────────────┐  ┌─────────────────┐   │
│  │ Controllers │  │   Wirings    │  │     Utils       │   │
│  │  - Sox      │  │  - One2One   │  │  - Trackable    │   │
│  │  - Sos      │  │  - Feedfwd   │  │  - Configurable │   │
│  │  - DEP      │  │  - Random    │  │  - Inspectable  │   │
│  └──────┬──────┘  └───────┬──────┘  └────────┬────────┘   │
│         ↓                  ↓                  ↓            │
│  ┌──────────────────────────────────────────────────────┐  │
│  │               Core Infrastructure                     │  │
│  │    Matrix, RandGen, Storeable, SensorMotorInfo      │  │
│  └──────────────────────────────────────────────────────┘  │
└─────────────────────────────┬───────────────────────────────┘
                              ↓
┌─────────────────────────────┴───────────────────────────────┐
│                    External Dependencies                     │
│        OpenDE (Physics)    OpenSceneGraph (Graphics)        │
│             Qt6 (GUI)         GSL (Math)                    │
└─────────────────────────────────────────────────────────────┘
```

## 2. Class Hierarchy

### Controller Hierarchy (selforg)

```
Configurable
    ↓
AbstractController
    ├── AbstractControllerAdapter
    │   ├── MeasureAdapter
    │   └── DiscreteControllerAdapter
    ├── AbstractIAFController
    │   └── (IAF neural controllers)
    ├── AbstractModel
    │   └── (Model-based controllers)
    ├── AbstractMultiController
    │   ├── SwitchController
    │   └── OneControllerPerChannel
    ├── ControllerBase (NEW)
    │   ├── BufferedControllerBase<N> (NEW)
    │   │   └── DEP
    │   ├── Sox
    │   ├── Sos
    │   ├── Soml
    │   └── Semox
    ├── InvertController
    │   ├── InvertMotorNStep
    │   ├── InvertMotorSpace
    │   └── InvertNChannelController
    ├── FFNNController
    ├── HomeokinBase
    │   ├── DerLinInvert
    │   └── DerLinUnivers
    ├── ClassicReinforce
    ├── MultiReinforce
    ├── SineController
    ├── BraitenbergController
    └── MotorBabbler
```

### Robot Hierarchy (ode_robots)

```
AbstractRobot (selforg)
    ↓
OdeRobot
    ├── Nimm2 (2-wheeled)
    ├── Nimm4 (4-wheeled)
    ├── FourWheeled
    ├── Sphererobot
    │   └── Sphererobot3Masses
    ├── Schlange (Snake robots)
    │   ├── SchlangeServo
    │   ├── SchlangeVelocity
    │   └── PlattfussSchlange
    ├── Hexapod (6-legged)
    │   ├── Hexabot
    │   └── AshiGaru
    ├── Vierbeiner (4-legged)
    ├── AmosII (6-legged)
    ├── Arm
    │   ├── Arm2Segm
    │   └── MuscledArm
    ├── HurlingSnake
    ├── Caterpillar
    ├── Skeleton (Humanoid)
    ├── Hand
    ├── Kuka (Robot arm)
    └── ReplayRobot
```

### Primitive Hierarchy (ode_robots)

```
Primitive (Abstract base)
    ├── Box
    │   └── OSGBoxTex
    ├── Sphere
    │   └── OSGSphere
    ├── Cylinder
    │   └── OSGCylinder
    ├── Capsule
    │   └── OSGCapsule
    ├── Mesh
    │   └── OSGMesh
    ├── Plane
    │   └── OSGPlane
    ├── HeightField
    │   └── OSGHeightField
    └── Ray
        └── OSGRay
```

### Configurable Hierarchy

```
Configurable
    ├── AbstractController
    ├── AbstractWiring
    ├── OdeRobot
    ├── AbstractGround
    ├── AbstractObstacle
    ├── Primitive
    ├── Joint
    ├── Motor
    ├── Sensor
    └── OdeAgent
```

## 3. Component Interfaces

### selforg → ode_robots Interface

```cpp
// ode_robots includes from selforg:
#include <selforg/abstractrobot.h>      // Base robot interface
#include <selforg/abstractcontroller.h>  // Controller interface
#include <selforg/abstractwiring.h>      // Wiring interface
#include <selforg/configurable.h>        // Configuration system
#include <selforg/inspectable.h>         // Inspection system
#include <selforg/storeable.h>           // Persistence
#include <selforg/matrix.h>              // Matrix operations
#include <selforg/randomgenerator.h>     // Random numbers
```

### ode_robots → opende Interface

```cpp
// Direct ODE physics engine usage:
#include <ode/ode.h>                     // Physics simulation
dWorldCreate()                           // Create physics world
dBodyCreate()                            // Create rigid bodies
dJointCreateHinge()                      // Create joints
dSpaceCollide()                          // Collision detection
```

### GUI Tools → Simulation Interface

```cpp
// Data flow via pipes/sockets:
guilogger: receives sensor/motor data via stdin
matrixviz: receives matrix data via stdin  
configurator: connects via ConfiguratorProxy

// Protocol: Channel-based text format
// Example: "#C channel_name value1 value2 ..."
```

### ga_tools Integration

```cpp
// Genetic Algorithm integration:
#include <selforg/inspectable.h>         // For parameter access
Individual → Configurable mapping        // GA individuals as configurations
SingletonGenEngine                       // GA execution engine
```

## 4. Data Flow

### Sensor Data Flow
```
Physical Simulation (ODE)
    ↓ (forces, positions)
OdeRobot::getSensorsIntern()
    ↓ (raw sensor values)
Sensor::get() [IRSensor, Camera, etc.]
    ↓ (processed values)
OdeRobot::getSensors()
    ↓ (sensor array)
OdeAgent::step()
    ↓
AbstractWiring::wireSensors()
    ↓ (wired sensors)
AbstractController::step()
    ↓ (control algorithm)
Motor commands
```

### Motor Command Flow
```
AbstractController::step()
    ↓ (motor values)
AbstractWiring::wireMotors()
    ↓ (wired motors)
OdeAgent::step()
    ↓
OdeRobot::setMotors()
    ↓ (motor array)
Motor::set() [OneAxisServo, etc.]
    ↓ (torques/velocities)
Physical Simulation (ODE)
```

### Configuration Flow
```
Configurable::getParamList()
    ↓
ConfiguratorProxy
    ↓ (network protocol)
Configurator GUI
    ↓ (user changes)
Configurable::setParam()
    ↓
Runtime parameter update
```

### Visualization Data Flow
```
Simulation
    ↓ (channel data)
PlotOptionEngine
    ↓ (stdout pipe)
guilogger
    ↓ (plotting)
gnuplot

Matrix data
    ↓ (stdout pipe)
matrixviz
    ↓ (visualization)
OpenGL display
```

## 5. Build Dependencies

### Mandatory Build Order
1. **opende** - Physics engine (no lpzrobots dependencies)
2. **selforg** - Core library (no lpzrobots dependencies)
3. **ode_robots** - Requires selforg and opende
4. **GUI tools** - Require ode_robots (for headers)
5. **ga_tools** - Requires selforg
6. **simulations** - Require all above

### Library Dependencies

#### selforg
- External: GSL, pthread
- Internal: None
- Produces: libselforg.a, libselforg_opt.a

#### ode_robots  
- External: OpenDE, OpenSceneGraph, OpenGL
- Internal: selforg
- Produces: libode_robots.a, libode_robots_opt.a

#### guilogger
- External: Qt6Core, Qt6Widgets, Qt6OpenGL
- Internal: None (standalone)
- Produces: guilogger executable

#### matrixviz
- External: Qt6Core, Qt6Widgets, Qt6OpenGL
- Internal: None (standalone)
- Produces: matrixviz executable

#### configurator
- External: Qt6Core, Qt6Widgets
- Internal: selforg (for Configurable)
- Produces: libconfigurator.a

#### ga_tools
- External: None
- Internal: selforg
- Produces: libga_tools.a

### Circular Dependencies
- **None found** - The architecture follows a clean layered design

### Optional Dependencies
- **guilogger**: Optional for headless simulations
- **matrixviz**: Optional visualization tool
- **configurator**: Optional runtime configuration
- **ga_tools**: Optional for genetic algorithms
- **soundman**: Optional sound manager (Java-based)

## 6. Key Design Patterns

### Factory Pattern
- **ControllerFactory**: Creates controllers by name
- **RobotFactory**: Creates robots by name
- **XMLPrimitiveFactory**: Creates primitives from XML

### Strategy Pattern
- **LearningStrategy**: Pluggable learning algorithms
- **ManagementStrategy**: Pluggable parameter management
- **WiringStrategy**: Different sensor-motor mappings

### Observer Pattern
- **Inspectable**: Observable parameters
- **Configurable**: Configuration change notifications
- **PlotOptionEngine**: Channel data observation

### Template Method
- **OdeRobot**: Defines robot creation template
- **AbstractController**: Defines control loop template

### Composite Pattern
- **Primitives**: Composite 3D objects
- **MultiController**: Composite controllers

## 7. Component Coupling Analysis

### Tight Coupling
- ode_robots ← selforg (inheritance, templates)
- simulations ← ode_robots (direct usage)
- Motor/Sensor ← Primitive (attachment points)

### Loose Coupling  
- GUI tools ← simulation (pipe/socket communication)
- ga_tools ← selforg (parameter interface only)
- configurator ← Configurable (interface only)

### Interface Segregation
- AbstractRobot: Minimal robot interface
- AbstractController: Minimal controller interface
- Configurable: Configuration interface
- Inspectable: Inspection interface
- Storeable: Persistence interface

This architecture demonstrates good separation of concerns with clear boundaries between physics, control, and visualization layers.