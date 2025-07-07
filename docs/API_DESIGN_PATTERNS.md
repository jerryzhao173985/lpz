# LPZRobots API Design Patterns

---
**Document Type**: Reference  
**Last Updated**: 2025-01-06  
**Status**: Current  
**Version**: 1.0  
**Component(s)**: All  
**Tags**: api, design-patterns, architecture, best-practices  
---

## Introduction

This document describes the key design patterns and architectural principles used throughout the LPZRobots codebase, providing guidance for maintaining consistency and quality.

## 1. Core Design Principles

### SOLID Principles Application

#### Single Responsibility Principle (SRP)
```cpp
// Good: Each class has one clear purpose
class MatrixOperations {
    static Matrix multiply(const Matrix& a, const Matrix& b);
    static Matrix inverse(const Matrix& m);
};

class MatrixIO {
    static void save(const Matrix& m, const std::string& filename);
    static Matrix load(const std::string& filename);
};

// Bad: Mixed responsibilities
class Matrix {
    void multiply(const Matrix& other);
    void saveToFile(const std::string& filename);  // IO mixed with math
    void drawOnScreen();  // Graphics mixed with data
};
```

#### Open/Closed Principle (OCP)
```cpp
// Base class closed for modification, open for extension
class AbstractController {
public:
    virtual ~AbstractController() = default;
    virtual void step(const sensor* sensors, int nsensors,
                     motor* motors, int nmotors) = 0;
protected:
    // Template method pattern
    virtual void updateWeights() = 0;
    virtual void calculateError() = 0;
};

// Extend through inheritance
class HomeokineticsController : public AbstractController {
    void updateWeights() override {
        // Specific implementation
    }
};
```

#### Liskov Substitution Principle (LSP)
```cpp
// All controllers can be used interchangeably
void runSimulation(AbstractController* controller) {
    controller->init(10, 5);
    // Works with any controller implementation
    controller->step(sensors, 10, motors, 5);
}
```

#### Interface Segregation Principle (ISP)
```cpp
// Focused interfaces
class Steppable {
    virtual void step(double time) = 0;
};

class Configurable {
    virtual void setParam(const std::string& key, double value) = 0;
    virtual double getParam(const std::string& key) const = 0;
};

class Storable {
    virtual void store(FILE* f) const = 0;
    virtual void restore(FILE* f) = 0;
};

// Classes implement only what they need
class MyController : public AbstractController, 
                    public Configurable,
                    public Storable {
    // Implements all three interfaces
};
```

#### Dependency Inversion Principle (DIP)
```cpp
// Depend on abstractions, not concrete classes
class OdeAgent {
    std::unique_ptr<AbstractController> controller_;
    std::unique_ptr<AbstractRobot> robot_;
    std::unique_ptr<AbstractWiring> wiring_;
    
public:
    // Constructor accepts interfaces
    OdeAgent(std::unique_ptr<AbstractController> ctrl,
             std::unique_ptr<AbstractRobot> robot,
             std::unique_ptr<AbstractWiring> wiring)
        : controller_(std::move(ctrl)),
          robot_(std::move(robot)),
          wiring_(std::move(wiring)) {}
};
```

## 2. Creational Patterns

### Factory Pattern

**Implementation:**
```cpp
class ControllerFactory {
    using CreatorFunc = std::function<std::unique_ptr<AbstractController>()>;
    using ConfigFunc = std::function<void(AbstractController*)>;
    
    struct ControllerInfo {
        CreatorFunc creator;
        ConfigFunc configurator;
        std::string description;
        ControllerCategory category;
    };
    
    static std::unordered_map<std::string, ControllerInfo>& getRegistry() {
        static std::unordered_map<std::string, ControllerInfo> registry;
        return registry;
    }
    
public:
    template<typename T>
    static void registerController(const std::string& name,
                                  const std::string& description,
                                  ControllerCategory category) {
        getRegistry()[name] = {
            []() { return std::make_unique<T>(); },
            [](AbstractController* c) { /* default config */ },
            description,
            category
        };
    }
    
    static std::unique_ptr<AbstractController> create(const std::string& name) {
        auto& registry = getRegistry();
        auto it = registry.find(name);
        if (it == registry.end()) {
            throw std::invalid_argument("Unknown controller: " + name);
        }
        
        auto controller = it->second.creator();
        it->second.configurator(controller.get());
        return controller;
    }
};
```

### Builder Pattern

**Complex Object Construction:**
```cpp
class SimulationBuilder {
    struct SimulationConfig {
        double timestep = 0.01;
        int osgResolutionX = 800;
        int osgResolutionY = 600;
        bool shadows = true;
        bool realtime = true;
    };
    
    SimulationConfig config_;
    std::vector<std::unique_ptr<OdeAgent>> agents_;
    
public:
    SimulationBuilder& withTimestep(double dt) {
        config_.timestep = dt;
        return *this;
    }
    
    SimulationBuilder& withResolution(int x, int y) {
        config_.osgResolutionX = x;
        config_.osgResolutionY = y;
        return *this;
    }
    
    SimulationBuilder& withoutShadows() {
        config_.shadows = false;
        return *this;
    }
    
    SimulationBuilder& addRobot(const std::string& type,
                               const std::string& controller) {
        auto robot = RobotFactory::create(type);
        auto ctrl = ControllerFactory::create(controller);
        auto wiring = std::make_unique<One2OneWiring>();
        
        agents_.push_back(std::make_unique<OdeAgent>(
            std::move(ctrl), std::move(robot), std::move(wiring)
        ));
        return *this;
    }
    
    std::unique_ptr<Simulation> build() {
        auto sim = std::make_unique<Simulation>(config_);
        for (auto& agent : agents_) {
            sim->addAgent(std::move(agent));
        }
        return sim;
    }
};

// Usage
auto simulation = SimulationBuilder()
    .withTimestep(0.005)
    .withResolution(1920, 1080)
    .withoutShadows()
    .addRobot("Sphererobot3Masses", "Sox")
    .addRobot("Hexapod", "DEP")
    .build();
```

### Singleton Pattern (Thread-Safe)

```cpp
class GlobalConfig {
    GlobalConfig() = default;
    
public:
    static GlobalConfig& instance() {
        static GlobalConfig instance;  // Thread-safe in C++11+
        return instance;
    }
    
    // Delete copy/move
    GlobalConfig(const GlobalConfig&) = delete;
    GlobalConfig& operator=(const GlobalConfig&) = delete;
    GlobalConfig(GlobalConfig&&) = delete;
    GlobalConfig& operator=(GlobalConfig&&) = delete;
    
    // Configuration methods
    void setParam(const std::string& key, double value);
    double getParam(const std::string& key) const;
    
private:
    mutable std::mutex mutex_;
    std::unordered_map<std::string, double> params_;
};
```

## 3. Structural Patterns

### Adapter Pattern

**Adapting External Libraries:**
```cpp
// Adapt GSL matrix to our Matrix interface
class GSLMatrixAdapter : public IMatrix {
    gsl_matrix* matrix_;
    
public:
    explicit GSLMatrixAdapter(int rows, int cols) 
        : matrix_(gsl_matrix_alloc(rows, cols)) {}
    
    ~GSLMatrixAdapter() {
        gsl_matrix_free(matrix_);
    }
    
    double get(int i, int j) const override {
        return gsl_matrix_get(matrix_, i, j);
    }
    
    void set(int i, int j, double value) override {
        gsl_matrix_set(matrix_, i, j, value);
    }
    
    int rows() const override { return matrix_->size1; }
    int cols() const override { return matrix_->size2; }
};
```

### Decorator Pattern

**Adding Functionality:**
```cpp
// Base wiring
class AbstractWiring {
public:
    virtual ~AbstractWiring() = default;
    virtual void wireSensors(const sensor* rsensors, sensor* csensors) = 0;
    virtual void wireMotors(const motor* cmotors, motor* rmotors) = 0;
};

// Decorator base
class WiringDecorator : public AbstractWiring {
protected:
    std::unique_ptr<AbstractWiring> wiring_;
    
public:
    explicit WiringDecorator(std::unique_ptr<AbstractWiring> wiring)
        : wiring_(std::move(wiring)) {}
};

// Concrete decorator - adds noise
class NoisyWiring : public WiringDecorator {
    double noiseLevel_;
    
public:
    NoisyWiring(std::unique_ptr<AbstractWiring> wiring, double noise)
        : WiringDecorator(std::move(wiring)), noiseLevel_(noise) {}
    
    void wireSensors(const sensor* rsensors, sensor* csensors) override {
        wiring_->wireSensors(rsensors, csensors);
        // Add noise
        for (int i = 0; i < getSensorNumber(); ++i) {
            csensors[i] += random(-noiseLevel_, noiseLevel_);
        }
    }
};

// Usage - compose decorators
auto wiring = std::make_unique<One2OneWiring>();
wiring = std::make_unique<NoisyWiring>(std::move(wiring), 0.1);
wiring = std::make_unique<DelayedWiring>(std::move(wiring), 5);
```

### Facade Pattern

**Simplifying Complex Interfaces:**
```cpp
class SimulationFacade {
    OdeHandle odeHandle_;
    OsgHandle osgHandle_;
    GlobalData global_;
    std::vector<std::unique_ptr<OdeAgent>> agents_;
    
public:
    void initialize() {
        // Complex initialization sequence
        initPhysics();
        initGraphics();
        initAgents();
    }
    
    void addRobot(const std::string& robotType, 
                  const std::string& controllerType) {
        // Hide complexity of creating and wiring components
        auto robot = RobotFactory::create(robotType, odeHandle_, osgHandle_);
        auto controller = ControllerFactory::create(controllerType);
        auto wiring = std::make_unique<One2OneWiring>();
        
        auto agent = std::make_unique<OdeAgent>(global_);
        agent->init(std::move(controller), std::move(robot), std::move(wiring));
        agents_.push_back(std::move(agent));
    }
    
    void run(int steps) {
        for (int i = 0; i < steps; ++i) {
            step();
        }
    }
    
private:
    void initPhysics();
    void initGraphics();
    void initAgents();
    void step();
};
```

## 4. Behavioral Patterns

### Strategy Pattern

**Interchangeable Algorithms:**
```cpp
// Learning strategy interface
class LearningStrategy {
public:
    virtual ~LearningStrategy() = default;
    virtual void updateWeights(Matrix& A, Matrix& C, 
                              const Matrix& error,
                              double epsilon) = 0;
    virtual std::string getName() const = 0;
};

// Concrete strategies
class HebbianLearning : public LearningStrategy {
public:
    void updateWeights(Matrix& A, Matrix& C, 
                      const Matrix& error, double epsilon) override {
        // Hebbian learning rule implementation
    }
    std::string getName() const override { return "Hebbian"; }
};

class HomeokineticsLearning : public LearningStrategy {
public:
    void updateWeights(Matrix& A, Matrix& C,
                      const Matrix& error, double epsilon) override {
        // Homeokinetic learning implementation
    }
    std::string getName() const override { return "Homeokinetic"; }
};

// Context class using strategy
class AdaptiveController : public AbstractController {
    std::unique_ptr<LearningStrategy> strategy_;
    
public:
    void setLearningStrategy(std::unique_ptr<LearningStrategy> strategy) {
        strategy_ = std::move(strategy);
    }
    
    void learn() override {
        if (strategy_) {
            strategy_->updateWeights(A_, C_, error_, epsilon_);
        }
    }
};
```

### Observer Pattern

**Event Notification:**
```cpp
// Observer interface
class SimulationObserver {
public:
    virtual ~SimulationObserver() = default;
    virtual void onStep(double time) = 0;
    virtual void onReset() = 0;
    virtual void onPause(bool paused) = 0;
};

// Subject
class Simulation {
    std::vector<std::weak_ptr<SimulationObserver>> observers_;
    
public:
    void attach(std::shared_ptr<SimulationObserver> observer) {
        observers_.push_back(observer);
    }
    
    void notify(auto memberFunc, auto... args) {
        // Remove expired observers
        observers_.erase(
            std::remove_if(observers_.begin(), observers_.end(),
                [](const auto& weak) { return weak.expired(); }),
            observers_.end()
        );
        
        // Notify active observers
        for (auto& weakObs : observers_) {
            if (auto obs = weakObs.lock()) {
                ((*obs).*memberFunc)(args...);
            }
        }
    }
    
    void step(double time) {
        // ... simulation step ...
        notify(&SimulationObserver::onStep, time);
    }
};

// Concrete observer
class DataLogger : public SimulationObserver {
public:
    void onStep(double time) override {
        // Log data at each step
    }
    
    void onReset() override {
        // Clear logs
    }
    
    void onPause(bool paused) override {
        // Flush logs if paused
    }
};
```

### Template Method Pattern

**Algorithm Structure:**
```cpp
class AbstractController {
public:
    // Template method defining algorithm structure
    void step(const sensor* sensors, int nsensors,
              motor* motors, int nmotors) final {
        // 1. Process sensors
        processSensors(sensors, nsensors);
        
        // 2. Calculate control
        calculateMotorValues();
        
        // 3. Learn from experience
        if (isLearning()) {
            updateModel();
        }
        
        // 4. Output motors
        outputMotors(motors, nmotors);
    }
    
protected:
    // Steps to be implemented by subclasses
    virtual void processSensors(const sensor* sensors, int n) = 0;
    virtual void calculateMotorValues() = 0;
    virtual void updateModel() = 0;
    virtual void outputMotors(motor* motors, int n) = 0;
    virtual bool isLearning() const { return true; }
};
```

### Command Pattern

**Encapsulating Requests:**
```cpp
// Command interface
class Command {
public:
    virtual ~Command() = default;
    virtual void execute() = 0;
    virtual void undo() = 0;
    virtual std::string getName() const = 0;
};

// Concrete commands
class SetParameterCommand : public Command {
    Configurable* target_;
    std::string param_;
    double oldValue_;
    double newValue_;
    
public:
    SetParameterCommand(Configurable* target, const std::string& param,
                       double newValue)
        : target_(target), param_(param), newValue_(newValue) {
        oldValue_ = target_->getParam(param_);
    }
    
    void execute() override {
        target_->setParam(param_, newValue_);
    }
    
    void undo() override {
        target_->setParam(param_, oldValue_);
    }
    
    std::string getName() const override {
        return "Set " + param_ + " to " + std::to_string(newValue_);
    }
};

// Command manager with undo/redo
class CommandManager {
    std::vector<std::unique_ptr<Command>> history_;
    size_t currentIndex_ = 0;
    
public:
    void execute(std::unique_ptr<Command> cmd) {
        // Remove any commands after current position
        history_.erase(history_.begin() + currentIndex_, history_.end());
        
        // Execute and add to history
        cmd->execute();
        history_.push_back(std::move(cmd));
        currentIndex_ = history_.size();
    }
    
    void undo() {
        if (currentIndex_ > 0) {
            --currentIndex_;
            history_[currentIndex_]->undo();
        }
    }
    
    void redo() {
        if (currentIndex_ < history_.size()) {
            history_[currentIndex_]->execute();
            ++currentIndex_;
        }
    }
};
```

## 5. Modern C++ Patterns

### CRTP (Curiously Recurring Template Pattern)

```cpp
template<typename Derived>
class MatrixBase {
public:
    Derived& operator+=(const Derived& other) {
        auto& self = static_cast<Derived&>(*this);
        // Implementation using derived class
        return self;
    }
    
    Derived operator+(const Derived& other) const {
        Derived result = static_cast<const Derived&>(*this);
        result += other;
        return result;
    }
};

class Matrix : public MatrixBase<Matrix> {
    // Inherits optimized operators
};
```

### Expression Templates

```cpp
template<typename LHS, typename RHS, typename Op>
class MatrixExpression {
    const LHS& lhs_;
    const RHS& rhs_;
    
public:
    MatrixExpression(const LHS& lhs, const RHS& rhs) 
        : lhs_(lhs), rhs_(rhs) {}
    
    double operator()(int i, int j) const {
        return Op::apply(lhs_(i, j), rhs_(i, j));
    }
};

// Operators return expressions, not results
template<typename LHS, typename RHS>
auto operator+(const LHS& lhs, const RHS& rhs) {
    return MatrixExpression<LHS, RHS, AddOp>(lhs, rhs);
}

// Evaluation happens only on assignment
Matrix C = A + B * 2.0;  // No temporaries created
```

### Type Erasure

```cpp
class Any {
    struct Concept {
        virtual ~Concept() = default;
        virtual std::unique_ptr<Concept> clone() const = 0;
        virtual std::type_info const& type() const = 0;
    };
    
    template<typename T>
    struct Model : Concept {
        T value;
        
        explicit Model(T v) : value(std::move(v)) {}
        
        std::unique_ptr<Concept> clone() const override {
            return std::make_unique<Model>(*this);
        }
        
        std::type_info const& type() const override {
            return typeid(T);
        }
    };
    
    std::unique_ptr<Concept> ptr_;
    
public:
    template<typename T>
    Any(T value) : ptr_(std::make_unique<Model<T>>(std::move(value))) {}
    
    template<typename T>
    T& get() {
        if (typeid(T) != ptr_->type()) {
            throw std::bad_cast();
        }
        return static_cast<Model<T>*>(ptr_.get())->value;
    }
};
```

## Best Practices Summary

1. **Prefer Composition over Inheritance** where possible
2. **Use RAII** for all resource management
3. **Make interfaces minimal and focused**
4. **Favor immutability** where performance allows
5. **Use smart pointers** for ownership semantics
6. **Apply const-correctness** throughout
7. **Provide strong exception guarantees**
8. **Document ownership and lifetime** clearly
9. **Use modern C++ features** appropriately
10. **Test all public interfaces** thoroughly

These patterns form the foundation of the LPZRobots architecture, ensuring maintainability, extensibility, and robustness.