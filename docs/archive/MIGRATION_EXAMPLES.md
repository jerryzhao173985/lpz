# LPZRobots Migration Examples

## Overview
This document provides concrete examples of how to migrate existing code to use the new refactored architecture.

## 1. Migrating a Controller to Use CircularBuffer

### Before (Sox Controller with C arrays)
```cpp
class Sox : public AbstractController {
    static const unsigned short buffersize = 10;
    matrix::Matrix x_buffer[buffersize];
    matrix::Matrix y_buffer[buffersize];
    
    void step(...) {
        // Store current values
        x_buffer[t % buffersize] = x_smooth;
        y_buffer[t % buffersize] = y;
        
        // Access previous values
        const Matrix& x_tm1 = x_buffer[(t - 1 + buffersize) % buffersize];
        const Matrix& x_tm2 = x_buffer[(t - 2 + buffersize) % buffersize];
        
        // Error-prone index calculations everywhere
        for(int i = 0; i < buffersize; i++) {
            const Matrix& x_old = x_buffer[(t - i + buffersize) % buffersize];
            // ...
        }
    }
};
```

### After (Sox Controller with CircularBuffer)
```cpp
class Sox : public AbstractController {
    static constexpr unsigned short buffersize = 10;
    lpzrobots::MatrixBuffer<buffersize> x_buffer;
    lpzrobots::MatrixBuffer<buffersize> y_buffer;
    
    void step(...) {
        // Store current values
        x_buffer.push(x_smooth);
        y_buffer.push(y);
        
        // Access previous values with clear semantics
        const Matrix& x_tm1 = x_buffer.get(-1);  // 1 step back
        const Matrix& x_tm2 = x_buffer.get(-2);  // 2 steps back
        
        // Clean iteration
        for(int i = 0; i < buffersize; i++) {
            const Matrix& x_old = x_buffer.get(-i);
            // ...
        }
    }
};
```

### Migration Steps
1. Replace array declarations with `MatrixBuffer<size>`
2. Replace `buffer[index] = value` with `buffer.push(value)`
3. Replace `buffer[(t - n + buffersize) % buffersize]` with `buffer.get(-n)`
4. Update any manual buffer iteration to use `get()` method

## 2. Migrating a Controller to Use ControllerBase

### Before (DEP Controller with manual initialization)
```cpp
class DEP : public AbstractController {
    unsigned short number_sensors;
    unsigned short number_motors;
    matrix::Matrix A, C, S, h, b, L, R;
    RingBuffer<matrix::Matrix> x_buffer;
    RingBuffer<matrix::Matrix> y_buffer;
    matrix::Matrix x_smooth;
    int t;
    
    void init(int sensornumber, int motornumber, RandGen* randGen) {
        number_sensors = sensornumber;
        number_motors = motornumber;
        t = 0;
        
        // Manual matrix initialization
        A.set(number_sensors, number_motors);
        C.set(number_motors, number_sensors);
        S.set(number_sensors, number_sensors);
        h.set(number_motors, 1);
        b.set(number_sensors, 1);
        L.set(number_sensors, number_sensors);
        R.set(number_sensors, number_sensors);
        
        // Set to identity
        A.toId();
        C.toId();
        S.toId();
        S *= 0.05;
        
        // Initialize buffers
        x_smooth.set(number_sensors, 1);
        x_buffer.init(buffersize, Matrix(number_sensors, 1));
        y_buffer.init(buffersize, Matrix(number_motors, 1));
        
        // ... more initialization
    }
};
```

### After (DEP Controller using BufferedControllerBase)
```cpp
class DEP : public lpzrobots::BufferedControllerBase<150> {
    // Inherited: number_sensors, number_motors, t
    // Inherited: A, C, S, h, b, L, R
    // Inherited: x_buffer, y_buffer, x_smooth
    
    // Only DEP-specific members
    DEPConf conf;
    matrix::Matrix C_update;
    
    void init(int sensornumber, int motornumber, RandGen* randGen) override {
        // Base class handles all common initialization
        BufferedControllerBase<150>::init(sensornumber, motornumber, randGen);
        
        // Only DEP-specific initialization
        C_update.set(number_motors, number_sensors);
        
        if (conf.useExtendedModel) {
            initExtendedModel(conf.factorS);  // Helper from base class
        }
        
        // Custom matrix initialization if needed
        if (conf.initModel && number_sensors >= 2 * number_motors) {
            Matrix A1(number_sensors / 2, number_motors);
            A1.toId();
            A = A1.above(A1);
        }
    }
};
```

### Benefits
- Eliminated 40+ lines of boilerplate initialization
- Consistent initialization across all controllers
- Access to helper methods like `initModelMatrices()`, `initBiasVectors()`
- Automatic buffer management

## 3. Creating a New Controller with Base Classes

### Step-by-Step Guide

```cpp
// Step 1: Choose appropriate base class
class MyController : public lpzrobots::BufferedControllerBase<30> {
    // BufferSize of 30 timesteps
    
    // Step 2: Add controller-specific parameters
    double learning_rate = 0.1;
    double adaptation_rate = 0.01;
    
public:
    // Step 3: Constructor
    MyController() : BufferedControllerBase<30>("MyController", "1.0") {
        // Add parameters for runtime configuration
        addParameterDef("learning", &learning_rate, 0.1, 0, 1, 
                       "learning rate");
        addParameterDef("adaptation", &adaptation_rate, 0.01, 0, 0.1,
                       "adaptation rate");
    }
    
    // Step 4: Override init
    void init(int sensornumber, int motornumber, RandGen* randGen) override {
        // Always call base class first
        BufferedControllerBase<30>::init(sensornumber, motornumber, randGen);
        
        // Your custom initialization
        // Common matrices A, C, h, b are already initialized
        // Buffers are ready to use
        
        // Example: Initialize with specific values
        C *= 0.5;  // Scale controller matrix
        
        // Use helper methods
        if (conf.useNoise) {
            initNoiseGenerators(randGen);
        }
    }
    
    // Step 5: Implement step function
    void step(const sensor* x_, int number_sensors,
              motor* y_, int number_motors) override {
        // Convert to matrix
        Matrix x(number_sensors, 1, x_);
        
        // Store in buffer (automatic circular behavior)
        x_buffer.push(x);
        
        // Your control algorithm
        Matrix y = (C * x + h).map(g);  // g() from base class
        
        // Learning
        if (t > buffersize) {
            learn();
        }
        
        // Store motor values
        y_buffer.push(y);
        y.convertToBuffer(y_, number_motors);
        
        ++t;  // Don't forget to increment time
    }
    
private:
    void learn() {
        // Access previous values easily
        const Matrix& x = x_buffer.get(0);      // current
        const Matrix& x_old = x_buffer.get(-1); // previous
        const Matrix& y_old = y_buffer.get(-1);
        
        // Your learning algorithm
        Matrix error = x - predictNextState(x_old, y_old);
        
        // Update weights
        C += (error * (x_old^T)) * learning_rate;
        C = C.mapP(0.1, clip);  // clip() from base class
    }
};
```

## 4. Using Factory Pattern for Object Creation

### Before (Direct instantiation in simulation)
```cpp
void MySimulation::start(const OdeHandle& odeHandle, 
                        const OsgHandle& osgHandle,
                        GlobalData& global) {
    // Hard-coded robot creation
    Sphererobot3MassesConf conf = Sphererobot3Masses::getDefaultConf();
    conf.diameter = 0.5;
    conf.pendularMass = 0.2;
    
    OdeRobot* robot = new Sphererobot3Masses(odeHandle, osgHandle, 
                                             conf, "MySphere");
    robot->place(Pos(0, 0, 0.5));
    
    // Hard-coded controller creation
    AbstractController* controller = new Sox();
    controller->setParam("epsC", 0.1);
    controller->setParam("epsA", 0.05);
    
    // Manual wiring
    One2OneWiring* wiring = new One2OneWiring(new ColorUniformNoise(0.1));
    
    // Create agent
    OdeAgent* agent = new OdeAgent(global);
    agent->init(controller, robot, wiring);
    global.agents.push_back(agent);
}
```

### After (Using factories)
```cpp
void MySimulation::start(const OdeHandle& odeHandle, 
                        const OsgHandle& osgHandle,
                        GlobalData& global) {
    // Configuration-driven creation
    std::string robotType = global.configs.getValue("robot", "Sphererobot3Masses");
    std::string controllerType = global.configs.getValue("controller", "Sox");
    
    // Create robot from factory
    auto robot = RobotFactory::createRobot(robotType, odeHandle, osgHandle, "MyRobot");
    if (!robot) {
        std::cerr << "Unknown robot type: " << robotType << std::endl;
        return;
    }
    robot->place(Pos(0, 0, 0.5));
    
    // Create controller from factory
    auto controller = ControllerFactory::createController(controllerType);
    if (!controller) {
        std::cerr << "Unknown controller type: " << controllerType << std::endl;
        return;
    }
    
    // Configure from file or command line
    controller->setParam("epsC", global.configs.getValue("epsC", 0.1));
    controller->setParam("epsA", global.configs.getValue("epsA", 0.05));
    
    // Create agent with smart pointers
    auto agent = std::make_unique<OdeAgent>(global);
    agent->init(controller.release(), robot.release(), 
                new One2OneWiring(new ColorUniformNoise(0.1)));
    global.agents.push_back(agent.release());
}
```

### Benefits of Factory Approach
1. **Flexibility**: Change robot/controller via configuration
2. **Discovery**: List available types at runtime
3. **Validation**: Factory returns nullptr for unknown types
4. **Extensibility**: Register new types without modifying factory

## 5. Using Strategy Pattern for Learning

### Before (Hard-coded learning in controller)
```cpp
class MyController : public AbstractController {
    void learn() {
        // Learning algorithm hard-coded in controller
        Matrix error = calculateError();
        
        // Only one learning algorithm possible
        C += (error * (x^T)) * epsC;
        h += error * epsC * 0.1;
    }
};
```

### After (Strategy pattern)
```cpp
class MyController : public AbstractController {
    std::unique_ptr<LearningStrategy> learningStrategy;
    
    void setLearningStrategy(std::unique_ptr<LearningStrategy> strategy) {
        learningStrategy = std::move(strategy);
    }
    
    void learn() {
        if (!learningStrategy) return;
        
        // Delegate to strategy
        auto [C_update, h_update] = learningStrategy->calculateControllerUpdate(
            C, h, xsi, x_buffer, y_buffer, t);
        
        C += C_update;
        h += h_update;
    }
};

// Usage: Switch learning algorithms at runtime
auto controller = std::make_unique<MyController>();

// Use standard homeokinetic learning
controller->setLearningStrategy(
    std::make_unique<HomeokineticsLearning>(0.1, 0.05));

// Later: Switch to teachable learning
controller->setLearningStrategy(
    std::make_unique<TeachableHomeokineticsLearning>(0.1, 0.05, 0.001));
```

## 6. Composite Management Strategy

### Creating Complex Management Behavior
```cpp
// Create composite management
auto management = std::make_unique<CompositeManagement>();

// Add damping every 10 steps
management->addStrategy(
    std::make_unique<DampingManagement>(0.00001, 10));

// Add k-winner-take-all every 100 steps
management->addStrategy(
    std::make_unique<KWTAManagement>(5, 0.001, 100));

// Add receptive field limitation every 50 steps
auto rfManagement = std::make_unique<ReceptiveFieldManagement>(10, 50);
management->addStrategy(std::move(rfManagement));

// Apply to controller
controller->setManagementStrategy(std::move(management));
```

## Common Migration Pitfalls and Solutions

### 1. Namespace Issues
**Problem**: "error: 'BufferedControllerBase' was not declared"
**Solution**: Use fully qualified name: `lpzrobots::BufferedControllerBase<N>`

### 2. Buffer Size Mismatch
**Problem**: Buffer operations fail at runtime
**Solution**: Ensure buffer size template parameter matches usage

### 3. Initialization Order
**Problem**: Accessing uninitialized matrices
**Solution**: Always call base class init() first

### 4. Missing Override
**Problem**: Virtual function not being called
**Solution**: Add `override` keyword to catch signature mismatches

### 5. Factory Registration
**Problem**: "Unknown controller type"
**Solution**: Ensure controller is registered in ControllerFactory::registerBuiltinControllers()

## Testing Your Migration

```cpp
// Test 1: Verify initialization
TEST(ControllerMigration, Initialization) {
    MyController controller;
    controller.init(4, 2);  // 4 sensors, 2 motors
    
    // Verify matrices are initialized
    EXPECT_EQ(controller.getA().getM(), 4);
    EXPECT_EQ(controller.getA().getN(), 2);
}

// Test 2: Verify buffer behavior
TEST(ControllerMigration, BufferAccess) {
    MatrixBuffer<10> buffer;
    Matrix m1(2, 1); m1.val(0, 0) = 1.0;
    Matrix m2(2, 1); m2.val(0, 0) = 2.0;
    
    buffer.push(m1);
    buffer.push(m2);
    
    EXPECT_EQ(buffer.get(0).val(0, 0), 2.0);  // Current
    EXPECT_EQ(buffer.get(-1).val(0, 0), 1.0); // Previous
}
```

## Conclusion

The migration to the new architecture provides:
- **Type Safety**: No more array index errors
- **Code Reuse**: Common code in base classes
- **Flexibility**: Runtime configuration via factories and strategies
- **Maintainability**: Clear separation of concerns
- **Performance**: Better cache usage, move semantics

Start with small controllers and gradually migrate larger ones. The compiler will guide you through most issues.