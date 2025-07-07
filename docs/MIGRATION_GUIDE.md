# LPZRobots Migration Guide

---
**Document Type**: Guide  
**Last Updated**: 2025-01-06  
**Status**: Current  
**Version**: 1.0  
**Component(s)**: All  
**Tags**: migration, upgrade, patterns, refactoring  
---

## Introduction

This guide helps developers migrate existing LPZRobots code from legacy patterns to modern C++17/20 patterns, providing step-by-step instructions and examples.

## 1. Memory Management Migration

### Raw Pointers → Smart Pointers

**Legacy Pattern:**
```cpp
class MySimulation : public Simulation {
    AbstractController* controller;
    OdeRobot* robot;
    
    MySimulation() {
        controller = new Sox();
        robot = new Sphererobot3Masses(...);
    }
    
    ~MySimulation() {
        delete controller;  // Easy to forget!
        delete robot;
    }
};
```

**Modern Pattern:**
```cpp
class MySimulation : public Simulation {
    std::unique_ptr<AbstractController> controller;
    std::unique_ptr<OdeRobot> robot;
    
    MySimulation() {
        controller = std::make_unique<Sox>();
        robot = std::make_unique<Sphererobot3Masses>(...);
    }
    // No destructor needed - automatic cleanup
};
```

### Array Migration

**Legacy Pattern:**
```cpp
matrix::Matrix x_buffer[50];
int buffer_index = 0;

void step() {
    x_buffer[buffer_index % 50] = x;
    const Matrix& old_x = x_buffer[(buffer_index - 1 + 50) % 50];
    buffer_index++;
}
```

**Modern Pattern:**
```cpp
lpzrobots::MatrixBuffer<50> x_buffer;

void step() {
    x_buffer.push(x);
    const Matrix& old_x = x_buffer.get(-1);  // -1 = previous value
}
```

## 2. Type System Migration

### typedef → using

**Legacy:**
```cpp
typedef matrix::Matrix Matrix;
typedef std::vector<AbstractController*> ControllerList;
typedef void (*CallbackFunc)(void*);
```

**Modern:**
```cpp
using Matrix = matrix::Matrix;
using ControllerList = std::vector<std::unique_ptr<AbstractController>>;
using CallbackFunc = std::function<void(void*)>;
```

### NULL → nullptr

**Simple replacement:**
```cpp
// Legacy
if (ptr != NULL) { ... }
ptr = NULL;

// Modern
if (ptr != nullptr) { ... }
ptr = nullptr;
```

## 3. Controller Migration

### Old Controller Pattern

**Legacy Implementation:**
```cpp
class MyController : public AbstractController {
    Matrix A, C, h;
    Matrix* x_buffer;
    Matrix* y_buffer;
    int buffersize;
    int t;
    
    void init(int sensornumber, int motornumber, RandGen* randGen) {
        A.set(motornumber, sensornumber);
        C.set(motornumber, motornumber);
        h.set(motornumber, 1);
        
        // Manual buffer allocation
        buffersize = 50;
        x_buffer = new Matrix[buffersize];
        y_buffer = new Matrix[buffersize];
        for (int i = 0; i < buffersize; i++) {
            x_buffer[i].set(sensornumber, 1);
            y_buffer[i].set(motornumber, 1);
        }
        t = 0;
    }
    
    ~MyController() {
        delete[] x_buffer;
        delete[] y_buffer;
    }
};
```

**Modern Implementation:**
```cpp
class MyController : public lpzrobots::BufferedControllerBase<50> {
    // A, C, h, x_buffer, y_buffer automatically inherited
    
    void init(int sensornumber, int motornumber, RandGen* randGen) override {
        // Use base class initialization
        BufferedControllerBase::init(sensornumber, motornumber, randGen);
        
        // Only initialize specific parameters
        setParam("epsA", 0.1);
        setParam("epsC", 0.05);
    }
    
    // No destructor needed - automatic cleanup
};
```

## 4. Factory Pattern Migration

### Direct Creation → Factory

**Legacy:**
```cpp
void start(const OdeHandle& odeHandle, const OsgHandle& osgHandle, 
           GlobalData& global) {
    // Direct creation scattered throughout code
    OdeRobot* robot = new Sphererobot3Masses(odeHandle, osgHandle, 
                                             conf, "sphere1");
    AbstractController* controller = new Sox();
    controller->setParam("epsC", 0.1);
    controller->setParam("epsA", 0.05);
    
    // Manual wiring
    AbstractWiring* wiring = new One2OneWiring(new ColorUniformNoise(0.1));
}
```

**Modern:**
```cpp
void start(const OdeHandle& odeHandle, const OsgHandle& osgHandle, 
           GlobalData& global) {
    // Centralized factory creation
    auto robot = RobotFactory::createRobot("Sphererobot3Masses", 
                                          odeHandle, osgHandle, "sphere1");
    
    auto controller = ControllerFactory::createController("Sox", {
        {"epsC", 0.1},
        {"epsA", 0.05}
    });
    
    auto wiring = WiringFactory::createWiring("One2OneWiring", 
                                             NoiseType::ColorUniform, 0.1);
}
```

## 5. Signal/Slot Migration (Qt)

### Qt4 → Qt6 Signals

**Legacy Qt4:**
```cpp
connect(slider, SIGNAL(valueChanged(int)), 
        this, SLOT(updateParameter(int)));
        
disconnect(button, SIGNAL(clicked()), 
          this, SLOT(handleClick()));
```

**Modern Qt6:**
```cpp
connect(slider, &QSlider::valueChanged, 
        this, &MyClass::updateParameter);
        
disconnect(button, &QPushButton::clicked, 
          this, &MyClass::handleClick);
```

### Qt Containers

**Legacy:**
```cpp
QList<QString> items;
foreach(QString item, items) {
    processItem(item);
}
```

**Modern:**
```cpp
QVector<QString> items;  // QVector preferred in Qt6
for (const auto& item : items) {
    processItem(item);
}
```

## 6. Build System Migration

### Makefile → CMake

**Legacy Makefile:**
```makefile
include Makefile.conf

CXXFLAGS += -I$(SELFORG)/include
LIBS += -L$(SELFORG)/lib -lselforg

myrobot: main.o
	$(CXX) -o $@ $^ $(LIBS)
```

**Modern CMakeLists.txt:**
```cmake
cmake_minimum_required(VERSION 3.20)
project(myrobot)

find_package(lpzrobots REQUIRED)

add_executable(myrobot main.cpp)
target_link_libraries(myrobot PRIVATE lpzrobots::selforg)
```

## 7. Exception Handling Migration

### throw() → noexcept

**Legacy:**
```cpp
class MyClass {
    void cleanup() throw();  // Deprecated
    void process() throw(std::exception);  // Deprecated
};
```

**Modern:**
```cpp
class MyClass {
    void cleanup() noexcept;  // Cannot throw
    void process();  // Can throw (default)
};
```

## 8. Loop Migration

### Index Loops → Range-Based

**Legacy:**
```cpp
std::vector<OdeAgent*> agents;
for (size_t i = 0; i < agents.size(); i++) {
    agents[i]->step(noise, time);
}
```

**Modern:**
```cpp
std::vector<std::unique_ptr<OdeAgent>> agents;
for (auto& agent : agents) {
    agent->step(noise, time);
}
```

### Iterator Loops → Algorithms

**Legacy:**
```cpp
std::vector<double> values;
double sum = 0;
for (std::vector<double>::iterator it = values.begin(); 
     it != values.end(); ++it) {
    sum += *it;
}
```

**Modern:**
```cpp
std::vector<double> values;
double sum = std::accumulate(values.begin(), values.end(), 0.0);
// Or with ranges (C++20)
// double sum = std::ranges::fold_left(values, 0.0, std::plus{});
```

## 9. Constant Migration

### Macros → constexpr

**Legacy:**
```cpp
#define MAX_SENSORS 50
#define PI 3.14159265359
#define SQUARE(x) ((x)*(x))
```

**Modern:**
```cpp
constexpr int MAX_SENSORS = 50;
constexpr double PI = 3.14159265359;

template<typename T>
constexpr T square(T x) { return x * x; }
```

## 10. Virtual Function Migration

### Missing override

**Legacy:**
```cpp
class MyController : public AbstractController {
    virtual void init(int sensornumber, int motornumber, 
                     RandGen* randGen = 0);
    virtual void step(const sensor* sensors, int number_sensors,
                     motor* motors, int number_motors);
};
```

**Modern:**
```cpp
class MyController : public AbstractController {
    void init(int sensornumber, int motornumber, 
              RandGen* randGen = nullptr) override;
    void step(const sensor* sensors, int number_sensors,
              motor* motors, int number_motors) override;
};
```

## Migration Checklist

When migrating a component:

- [ ] Replace raw pointers with smart pointers
- [ ] Convert arrays to std::vector or custom containers
- [ ] Change typedef to using
- [ ] Replace NULL with nullptr
- [ ] Add override to virtual functions
- [ ] Replace throw() with noexcept
- [ ] Update Qt signals/slots to new syntax
- [ ] Convert Makefile to CMakeLists.txt
- [ ] Use range-based for loops
- [ ] Replace macros with constexpr
- [ ] Update to modern STL algorithms
- [ ] Add explicit to single-parameter constructors
- [ ] Use auto for complex type deduction
- [ ] Apply const-correctness throughout

## Common Pitfalls

1. **Memory Ownership**: Ensure clear ownership with unique_ptr
2. **Circular Dependencies**: Use weak_ptr to break cycles
3. **Move Semantics**: Enable move operations for performance
4. **Exception Safety**: Provide strong exception guarantee
5. **Thread Safety**: Use std::mutex, not custom synchronization

## Testing Migration

Always test migrated code:
```bash
# Build with all warnings
cmake -B build -DCMAKE_CXX_FLAGS="-Wall -Wextra"
cmake --build build

# Run with sanitizers
cmake -B build-asan -DENABLE_SANITIZERS=ON
cmake --build build-asan
./build-asan/myrobot

# Run existing tests
ctest --test-dir build
```

This migration guide provides a comprehensive path from legacy to modern LPZRobots code.