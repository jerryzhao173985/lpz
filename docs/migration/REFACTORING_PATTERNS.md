# Design Patterns Applied in LPZRobots Refactoring

## Overview
This document catalogs all design patterns applied during the LPZRobots modernization, providing implementation details and usage examples.

## 1. Factory Pattern

### Intent
Provide an interface for creating objects without specifying their exact class, enabling runtime object creation based on configuration.

### Implementation

#### RobotFactory
```cpp
class RobotFactory {
private:
    using RobotCreator = std::function<std::unique_ptr<OdeRobot>(
        const OdeHandle&, const OsgHandle&, const std::string&)>;
    
    static std::map<std::string, RobotCreator> creators;
    
public:
    static void registerRobot(const std::string& type, RobotCreator creator);
    static std::unique_ptr<OdeRobot> createRobot(const std::string& type, ...);
    static std::vector<std::string> getAvailableRobots();
};
```

#### ControllerFactory
```cpp
class ControllerFactory {
public:
    enum Category { HOMEOKINETIC, PREDICTIVE, MOTOR_SPACE, NEURAL_NETWORK };
    
    static std::unique_ptr<AbstractController> createController(
        const std::string& type);
    static std::vector<std::string> getControllersByCategory(Category cat);
    
private:
    static void registerBuiltinControllers();
};
```

### Benefits
- Decouples object creation from usage
- Enables plugin architecture
- Simplifies configuration-based creation
- Centralizes available types

### Usage Examples
```cpp
// Creating a robot
auto robot = RobotFactory::createRobot("Sphererobot3Masses", 
                                       odeHandle, osgHandle, "MyRobot");

// Creating a controller
auto controller = ControllerFactory::createController("Sox");
controller->setParam("epsC", 0.1);

// Listing available types
auto robots = RobotFactory::getAvailableRobots();
for (const auto& type : robots) {
    std::cout << "Available robot: " << type << std::endl;
}
```

## 2. Strategy Pattern

### Intent
Define a family of algorithms, encapsulate each one, and make them interchangeable at runtime.

### Implementation

#### Learning Strategy
```cpp
class LearningStrategy {
public:
    virtual ~LearningStrategy() = default;
    
    virtual std::pair<matrix::Matrix, matrix::Matrix> 
    calculateControllerUpdate(const matrix::Matrix& C,
                             const matrix::Matrix& h,
                             const matrix::Matrix& xsi,
                             const matrix::Matrix* x_buffer,
                             const matrix::Matrix* y_buffer,
                             int t) = 0;
                             
    virtual void setLearningRates(double epsC, double epsA) = 0;
};

// Concrete strategies
class HomeokineticsLearning : public LearningStrategy { ... };
class TeachableHomeokineticsLearning : public HomeokineticsLearning { ... };
```

#### Management Strategy
```cpp
class ManagementStrategy {
public:
    virtual ~ManagementStrategy() = default;
    
    virtual void manage(matrix::Matrix& C, matrix::Matrix& A,
                       matrix::Matrix& h, matrix::Matrix& b,
                       int t) = 0;
    
    virtual bool shouldManage(int t) const = 0;
};

// Concrete strategies
class DampingManagement : public ManagementStrategy { ... };
class KWTAManagement : public ManagementStrategy { ... };
class ReceptiveFieldManagement : public ManagementStrategy { ... };
```

### Benefits
- Algorithms can be changed at runtime
- New algorithms can be added without modifying existing code
- Separates algorithm from its usage context
- Enables A/B testing of different approaches

### Usage Examples
```cpp
// Using different learning strategies
auto controller = std::make_unique<MyController>();

// Standard learning
auto learning1 = std::make_unique<HomeokineticsLearning>(0.1, 0.05);
controller->setLearningStrategy(std::move(learning1));

// Switch to teachable learning
auto learning2 = std::make_unique<TeachableHomeokineticsLearning>(0.1, 0.05, 0.001);
controller->setLearningStrategy(std::move(learning2));

// Composite management
auto management = std::make_unique<CompositeManagement>();
management->addStrategy(std::make_unique<DampingManagement>(0.0001, 10));
management->addStrategy(std::make_unique<KWTAManagement>(5, 0.001, 100));
controller->setManagementStrategy(std::move(management));
```

## 3. Template Method Pattern

### Intent
Define the skeleton of an algorithm in a base class, letting subclasses override specific steps.

### Implementation

#### ControllerBase
```cpp
class ControllerBase : public AbstractController {
public:
    // Template method
    virtual void init(int sensornumber, int motornumber, 
                     RandGen* randGen = nullptr) override {
        // Common initialization steps
        number_sensors = sensornumber;
        number_motors = motornumber;
        t = 0;
        
        // Initialize current state
        x.set(number_sensors, 1);
        y.set(number_motors, 1);
        
        // Hook methods for subclasses
        initializeMatrices();
        initializeBuffers();
        initializeParameters();
    }
    
protected:
    // Hook methods (can be overridden)
    virtual void initializeMatrices() {
        initModelMatrices();
        initBiasVectors();
    }
    
    virtual void initializeBuffers() { }
    virtual void initializeParameters() { }
    
    // Helper methods available to all subclasses
    void initModelMatrices(double cInitFactor = 1.0);
    void initBiasVectors();
    void initExtendedModel(double sFactor = 0.05);
};
```

### Benefits
- Eliminates code duplication
- Enforces consistent initialization order
- Provides flexibility through hooks
- Makes common operations reusable

### Usage Example
```cpp
class MyController : public BufferedControllerBase<50> {
    void init(int sensornumber, int motornumber, RandGen* randGen) override {
        // Call base implementation
        BufferedControllerBase<50>::init(sensornumber, motornumber, randGen);
        
        // Custom initialization
        myParam = 0.5;
        myMatrix.set(number_motors, number_sensors);
    }
    
    // Override specific hook if needed
    void initializeParameters() override {
        ControllerBase::initializeParameters();
        // Add custom parameter initialization
    }
};
```

## 4. RAII (Resource Acquisition Is Initialization)

### Intent
Tie resource lifetime to object lifetime, ensuring automatic cleanup.

### Implementation

#### CircularBuffer
```cpp
template<typename T, std::size_t N>
class CircularBuffer {
private:
    std::array<T, N> buffer;  // Stack allocation, no manual memory management
    std::size_t head = 0;
    bool filled = false;
    
public:
    // Resources acquired in constructor
    CircularBuffer() = default;
    
    // Automatic cleanup via destructor
    ~CircularBuffer() = default;
    
    // Safe copy/move operations
    CircularBuffer(const CircularBuffer&) = default;
    CircularBuffer(CircularBuffer&&) noexcept = default;
};
```

#### Smart Pointer Usage
```cpp
class RobotSimulation {
    // Automatic memory management
    std::vector<std::unique_ptr<OdeRobot>> robots;
    std::vector<std::unique_ptr<AbstractController>> controllers;
    
public:
    void addRobot(const std::string& type) {
        // No manual delete needed
        robots.push_back(RobotFactory::createRobot(type, ...));
    }
}; // Automatic cleanup when simulation ends
```

### Benefits
- No memory leaks
- Exception safety
- Clear ownership semantics
- Simplified code (no manual delete)

## 5. Composite Pattern

### Intent
Compose objects into tree structures to represent part-whole hierarchies.

### Implementation

#### CompositeManagement
```cpp
class CompositeManagement : public ManagementStrategy {
private:
    std::vector<std::unique_ptr<ManagementStrategy>> strategies;
    
public:
    void addStrategy(std::unique_ptr<ManagementStrategy> strategy) {
        strategies.push_back(std::move(strategy));
    }
    
    void manage(matrix::Matrix& C, matrix::Matrix& A,
               matrix::Matrix& h, matrix::Matrix& b, int t) override {
        // Delegate to all child strategies
        for (auto& strategy : strategies) {
            if (strategy->shouldManage(t)) {
                strategy->manage(C, A, h, b, t);
            }
        }
    }
    
    bool shouldManage(int t) const override {
        // Manage if any child should manage
        return std::any_of(strategies.begin(), strategies.end(),
            [t](const auto& s) { return s->shouldManage(t); });
    }
};
```

### Benefits
- Treats individual and composite objects uniformly
- Enables building complex behaviors from simple ones
- Easy to add new components
- Flexible configuration

### Usage Example
```cpp
auto composite = std::make_unique<CompositeManagement>();

// Add basic damping
composite->addStrategy(std::make_unique<DampingManagement>(0.0001, 10));

// Add k-winner-take-all every 100 steps
composite->addStrategy(std::make_unique<KWTAManagement>(5, 0.001, 100));

// Add receptive field limitation every 50 steps
composite->addStrategy(std::make_unique<ReceptiveFieldManagement>(10, 50));

// Use composite as a single strategy
controller->setManagementStrategy(std::move(composite));
```

## 6. Iterator Pattern (Implicit)

### Intent
Provide a way to access elements of a container sequentially without exposing its underlying representation.

### Implementation
CircularBuffer provides STL-compatible iteration:

```cpp
template<typename T, std::size_t N>
class CircularBuffer {
public:
    // STL-style iteration
    using const_iterator = typename std::array<T, N>::const_iterator;
    
    const_iterator begin() const { return buffer.begin(); }
    const_iterator end() const { return buffer.begin() + size(); }
    
    // Range-based for loop support
    auto& front() const { return buffer[tail()]; }
    auto& back() const { return buffer[most_recent()]; }
};
```

### Usage
```cpp
MatrixBuffer<50> buffer;
// ... fill buffer ...

// Range-based iteration
for (const auto& matrix : buffer) {
    processMatrix(matrix);
}

// Algorithm usage
auto it = std::find_if(buffer.begin(), buffer.end(),
    [](const Matrix& m) { return m.norm() > 1.0; });
```

## Summary

These design patterns work together to create a flexible, maintainable architecture:

1. **Factories** create objects based on configuration
2. **Strategies** allow algorithm selection at runtime
3. **Template Methods** eliminate duplication while allowing customization
4. **RAII** ensures resource safety
5. **Composite** enables complex behaviors from simple components
6. **Iterators** provide uniform access to containers

The combination results in code that is:
- Easy to extend (add new robots/controllers/strategies)
- Safe (automatic resource management)
- Flexible (runtime configuration)
- Maintainable (clear separation of concerns)
- Testable (components can be tested in isolation)