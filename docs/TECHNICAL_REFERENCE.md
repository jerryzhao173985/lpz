# LPZRobots Technical Reference

---
**Document Type**: Reference  
**Last Updated**: 2025-01-06  
**Status**: Current  
**Version**: 1.0  
**Component(s)**: All  
**Tags**: architecture, algorithms, api, design-patterns, reference  
---

## Architecture Overview

### System Components

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

### Core Design Pattern: Agent-Robot-Controller

```cpp
// Fundamental separation of concerns
OdeRobot* robot = new Sphererobot3Masses(...);     // Physical body
AbstractController* controller = new Sox();         // Brain/controller  
AbstractWiring* wiring = new One2OneWiring();      // Sensor-motor mapping
OdeAgent* agent = new OdeAgent(global);            // Combines all three
agent->init(controller, robot, wiring);
```

## Homeokinetic Algorithms

### Core Principle
Self-organizing control through the homeokinetic principle: maximizing sensitivity while maintaining stability.

### Implemented Algorithms

#### 1. **Sox** (Self-organizing eXploration)
- Basic homeokinetic controller
- Time-loop error minimization
- Anti-Hebbian regularization

#### 2. **APEX-Sox** (Adaptive Predictive EXploration)
- Meta-learning system
- Predictive diversity bonus
- Stability-guided exploration
- Combines all advanced features

#### 3. **IT-Sox** (Information-Theoretic)
- Direct MI optimization: I(X_{t+1}; Y_t | X_t)
- Kernel density estimation
- Adaptive kernel width

#### 4. **CA-Sox** (Context-Aware)
- Automatic context detection (up to 5 models)
- Prototype-based recognition
- Smooth transitions via gating

#### 5. **MSHC** (Multi-Scale Homeokinetic)
- Hierarchical time scales (1-50 steps)
- Scale-specific models
- Emergent behavioral hierarchy

#### 6. **AH-Sox** (Adaptive Horizon)
- Dynamic prediction horizons (1-10 steps)
- Uncertainty-based adjustment
- Multiple forward models

## Design Patterns

### Factory Pattern
```cpp
// Controller creation
auto controller = ControllerFactory::createController("Sox");

// Robot creation  
auto robot = RobotFactory::createRobot("Sphererobot3Masses", 
                                       odeHandle, osgHandle, "MyRobot");
```

### Strategy Pattern
```cpp
// Pluggable learning
controller->setLearningStrategy(
    std::make_unique<HomeokineticsLearning>());

// Composable management
management->addStrategy(std::make_unique<DampingManagement>());
management->addStrategy(std::make_unique<KWTAManagement>());
```

### Base Class Hierarchy
```cpp
class Sox : public lpzrobots::BufferedControllerBase<2> {
    // Inherits A, C, S, h, b matrices
    // Automatic buffer management
    // Common initialization
};
```

## Performance Optimizations

### Matrix Operations
- **ARM64 NEON**: 2-4x speedup on Apple Silicon
- **Expression templates**: Zero-overhead operations
- **Memory pooling**: 50-80% allocation reduction
- **Cache blocking**: 3-5x improvement for large matrices

### Key APIs

#### Controller Interface
```cpp
class AbstractController {
    virtual void init(int sensornumber, int motornumber) = 0;
    virtual void step(const sensor* s, int sn, motor* m, int mn) = 0;
    virtual void stepNoLearning(...) = 0;
    virtual bool store(FILE* f) const = 0;
    virtual bool restore(FILE* f) = 0;
};
```

#### Robot Interface
```cpp
class OdeRobot {
    virtual int getSensorNumber() const = 0;
    virtual int getMotorNumber() const = 0;
    virtual void update() = 0;
    virtual void place(const osg::Matrix& pose) = 0;
    virtual void doInternalStuff(GlobalData& global) = 0;
};
```

## Memory Management

### Smart Pointers
```cpp
// Factory returns unique_ptr
auto controller = std::make_unique<Sox>();

// Strategies use unique_ptr
strategies.push_back(std::move(strategy));
```

### Circular Buffers
```cpp
lpzrobots::MatrixBuffer<50> x_buffer;
x_buffer.push(x);
const Matrix& old_x = x_buffer.get(-1);  // Previous value
```

## Build System

### CMake Targets
```cmake
# Link against components
target_link_libraries(my_simulation
    PRIVATE 
        lpzrobots::ode_robots
        lpzrobots::selforg
)
```

### Configuration
```cmake
# Platform-specific features
if(USE_NEON)
    target_compile_definitions(selforg PRIVATE USE_NEON)
endif()
```

## Testing

### Test Categories
- **Unit**: Component isolation
- **Integration**: System interaction
- **Performance**: Benchmarks
- **BDD**: Behavior validation

### Running Tests
```bash
make test                    # All tests
make test_selforg           # Component tests
./test_homeokinetic -tc="*Sox*"  # Specific tests
```

---

For detailed implementation examples, see the `ode_robots/simulations/` directory.
For API documentation, see component headers in `selforg/controller/` and `ode_robots/robots/`.