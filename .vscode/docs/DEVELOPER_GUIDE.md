# 🚀 LPZRobots VSCode Developer Guide

This guide provides comprehensive information for developing with LPZRobots in Visual Studio Code.

## 📋 Table of Contents

1. [Quick Start](#quick-start)
2. [Development Workflow](#development-workflow)
3. [Component Development](#component-development)
4. [Debugging Techniques](#debugging-techniques)
5. [Performance Optimization](#performance-optimization)
6. [Best Practices](#best-practices)
7. [Troubleshooting](#troubleshooting)

## 🏃 Quick Start

### Initial Setup

1. **Clone and Open Workspace**
   ```bash
   git clone https://github.com/georgmartius/lpzrobots.git
   cd lpzrobots
   code lpzrobots.code-workspace
   ```

2. **Run Setup Script**
   ```bash
   .vscode/scripts/setup-environment.sh
   ```

3. **Configure CMake**
   - Press `Cmd+Shift+P`
   - Select "CMake: Select a Kit" → "Use CMake Presets"
   - Select "CMake: Configure"

4. **Build Everything**
   - Press `Cmd+Shift+B` or
   - Run task "🔨 Build: All"

### Your First Simulation

```bash
# Create new simulation
.vscode/scripts/create-simulation.sh my_first_robot

# Open the main file
code ode_robots/simulations/my_first_robot/main.cpp

# Build (Cmd+Alt+B)
# Run (Cmd+R)
# Debug (F5)
```

## 🔄 Development Workflow

### 1. Component-Based Development

LPZRobots is organized into components:
- **selforg**: Controllers and learning algorithms
- **ode_robots**: Robot models and simulations
- **ga_tools**: Genetic algorithms
- **GUI tools**: Visualization utilities

### 2. Typical Development Cycle

```mermaid
graph LR
    A[Create/Edit Code] --> B[Build Component]
    B --> C[Run Tests]
    C --> D[Debug if Needed]
    D --> E[Run Simulation]
    E --> F[Analyze Results]
    F --> A
```

### 3. Keyboard Shortcuts Workflow

| Action | Shortcut | Description |
|--------|----------|-------------|
| Build All | `Cmd+Shift+B` | Build entire project |
| Build Current | `Cmd+Alt+B` | Build active component |
| Run Simulation | `Cmd+R` | Run current simulation |
| Debug | `F5` | Start debugging |
| Run Tests | `Cmd+T` | Run all tests |
| Format Code | `Shift+Alt+F` | Format current file |

### 4. Using Code Snippets

Type prefix and press `Tab`:

```cpp
// Type: lpzcontroller<Tab>
// Generates complete controller class with factory registration

// Type: lpzrobot<Tab>
// Generates robot class template

// Type: lpzmatrix<Tab>
// Generates matrix operation block
```

## 🔧 Component Development

### Creating a New Controller

1. **Use the snippet**:
   ```cpp
   // In selforg/controller/mycontroller.h
   lpzcontroller<Tab>
   ```

2. **Implement required methods**:
   ```cpp
   class MyController : public AbstractController {
   public:
       void init(int sensornumber, int motornumber, RandGen* randGen) override {
           number_sensors = sensornumber;
           number_motors = motornumber;
           
           // Initialize matrices
           A.set(number_motors, number_sensors);
           C.set(number_motors, number_motors);
           h.set(number_motors, 1);
       }
       
       void step(const sensor* sensors, int number_sensors, 
                 motor* motors, int number_motors) override {
           // Control logic here
           Matrix x(number_sensors, 1, sensors);
           Matrix y = (C * y_buffer + A * x + h).map(tanh);
           y.convertToBuffer(motors, number_motors);
           
           // Store for learning
           x_buffer = x;
           y_buffer = y;
       }
   };
   ```

3. **Register with factory**:
   ```cpp
   // In mycontroller.cpp
   namespace {
       bool registered = ControllerFactory::registerController(
           "MyController",
           [](const std::string& name) { 
               return std::make_unique<MyController>(); 
           },
           "My custom controller description"
       );
   }
   ```

### Creating a New Robot

1. **Use the snippet**:
   ```cpp
   // In ode_robots/robots/myrobot.h
   lpzrobot<Tab>
   ```

2. **Implement construction**:
   ```cpp
   void MyRobot::create(const osg::Matrix& pose) {
       // Create body
       Primitive* body = new Box(0.3, 0.2, 0.1);
       body->init(odeHandle, mass, osgHandle);
       body->setPose(pose);
       objects.push_back(body);
       
       // Create wheels
       for(int i = 0; i < 4; i++) {
           Primitive* wheel = new Sphere(0.05);
           wheel->init(odeHandle, 0.1, osgHandle);
           objects.push_back(wheel);
           
           // Create joint
           Joint* joint = new HingeJoint(body, wheel, 
               body->getPosition() + wheelPos[i],
               Axis(0,0,1));
           joint->init(odeHandle, osgHandle);
           joints.push_back(joint);
           
           // Add motor
           auto motor = std::make_shared<AngularMotor1Axis>(
               odeHandle, joint, motorPower);
           addMotor(motor);
       }
   }
   ```

### Creating a New Simulation

1. **Generate from template**:
   ```bash
   .vscode/scripts/create-simulation.sh advanced_robot
   ```

2. **Edit main.cpp**:
   ```cpp
   class ThisSim : public Simulation {
   public:
       bool robots_created = false;
       
       void start(const OdeHandle& odeHandle, const OsgHandle& osgHandle, 
                  GlobalData& global) override {
           // Set simulation parameters
           setCameraHomePos(Pos(5.0, 5.0, 3.0), Pos(0.0, 0.0, 0.5));
           global.odeConfig.setParam("gravity", -9.81);
           
           // Create ground
           Playground* playground = new Playground(odeHandle, osgHandle,
               osg::Vec3(10, 0.2, 0.5));
           playground->setPosition(osg::Vec3(0,0,0));
           global.obstacles.push_back(playground);
       }
       
       void addCallback(GlobalData& global) override {
           if(!robots_created && global.time > 1.0) {
               robots_created = true;
               
               // Create robot
               auto robot = RobotFactory::createRobot("MyRobot", 
                   odeHandle, osgHandle, "robot1");
               robot->place(Pos(0,0,0.5));
               
               // Create controller
               auto controller = ControllerFactory::createController("Sox");
               
               // Create wiring
               auto wiring = new One2OneWiring(new WhiteUniformNoise(0.1));
               
               // Create agent
               OdeAgent* agent = new OdeAgent(global);
               agent->init(controller, robot, wiring);
               global.agents.push_back(agent);
               global.configs.push_back(agent);
           }
       }
   };
   ```

## 🐛 Debugging Techniques

### 1. Using Debug Visualizers

The project includes custom debug visualizers for:
- **Matrix**: Shows dimensions and values in grid format
- **OdeRobot**: Displays robot state and sensor/motor values
- **Controllers**: Shows internal parameters and matrices

```cpp
// In debug console
p myMatrix
// Output: Matrix[3x3] = [1 0 0; 0 1 0; 0 0 1]

p myRobot
// Output: OdeRobot 'Sphererobot' (initialized) 

// Expand in Variables view to see details
```

### 2. Conditional Breakpoints

Right-click on line number → "Add Conditional Breakpoint"

```cpp
// Break when specific condition met
if (error > threshold) {  // Breakpoint here
    // Condition: error > 0.1
}

// Break after N iterations
step();  // Breakpoint here
// Hit Count: 1000
```

### 3. Debug Configurations

Launch configurations available:
- **🐛 Debug: Current CMake Target** - Debug active target
- **🌐 Debug: Simulation + GUI Tools** - Debug with visualization
- **🔍 Debug: AddressSanitizer** - Find memory errors
- **🔍 Debug: ThreadSanitizer** - Find race conditions

### 4. Debugging Matrix Operations

```cpp
// Add debug prints
void MyController::step(...) {
    Matrix x(number_sensors, 1, sensors);
    
    #ifdef DEBUG
    std::cout << "Input: " << x << std::endl;
    std::cout << "A matrix: " << A << std::endl;
    #endif
    
    Matrix y = A * x;
    
    #ifdef DEBUG
    std::cout << "Output: " << y << std::endl;
    
    // Check for NaN or Inf
    if(y.hasNaN()) {
        std::cerr << "WARNING: NaN detected in output!" << std::endl;
    }
    #endif
}
```

## ⚡ Performance Optimization

### 1. Build Configurations

```bash
# Debug build (default)
cmake --preset dev
cmake --build build/dev

# Release build (optimized)
cmake --preset release
cmake --build build/release

# Profile build
cmake --preset profile
cmake --build build/profile
```

### 2. Profiling Tools

```bash
# CPU Profiling (macOS)
instruments -t "Time Profiler" ./start_opt

# Memory profiling
leaks --atExit -- ./start

# Cache analysis
cmake --build build/release --target clean
cmake --build build/release -- VERBOSE=1 | grep "cache"
```

### 3. Matrix Operation Optimization

```cpp
// Avoid temporary matrices
// Bad:
Matrix result = A * B + C * D;

// Better:
Matrix temp1 = A * B;
Matrix temp2 = C * D;
Matrix result = temp1 + temp2;

// Best: Use in-place operations
Matrix result(A);
result *= B;
Matrix temp(C);
temp *= D;
result += temp;
```

### 4. SIMD Optimization

```cpp
// Use NEON on ARM64
#ifdef __ARM_NEON
#include <arm_neon.h>

void vectorAdd(float* a, float* b, float* c, int n) {
    for(int i = 0; i < n; i += 4) {
        float32x4_t va = vld1q_f32(&a[i]);
        float32x4_t vb = vld1q_f32(&b[i]);
        float32x4_t vc = vaddq_f32(va, vb);
        vst1q_f32(&c[i], vc);
    }
}
#endif
```

## 📚 Best Practices

### 1. Code Style

- Use the provided `.clang-format`
- Format before committing: `Shift+Alt+F`
- Follow naming conventions:
  - Classes: `PascalCase`
  - Functions: `camelCase`
  - Variables: `snake_case`
  - Constants: `UPPER_CASE`

### 2. Memory Management

```cpp
// Use smart pointers
auto robot = std::make_unique<MyRobot>(...);

// RAII for resources
class ScopedTimer {
    std::chrono::time_point<std::chrono::steady_clock> start;
public:
    ScopedTimer() : start(std::chrono::steady_clock::now()) {}
    ~ScopedTimer() {
        auto end = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        std::cout << "Elapsed: " << duration.count() << "ms\n";
    }
};
```

### 3. Error Handling

```cpp
// Use assertions for debug checks
assert(matrix.getM() > 0 && "Matrix must have positive dimensions");

// Use exceptions for runtime errors
if(!robot->init()) {
    throw std::runtime_error("Failed to initialize robot");
}

// Log warnings
if(sensor_value > max_expected) {
    std::cerr << "Warning: Sensor value " << sensor_value 
              << " exceeds expected maximum " << max_expected << std::endl;
}
```

### 4. Documentation

```cpp
/**
 * @brief Controls robot using homeokinetic principles
 * 
 * This controller implements self-organized behavior emergence
 * through the homeokinetic framework.
 * 
 * @param sensors Array of sensor values
 * @param number_sensors Number of sensors
 * @param motors Array to fill with motor values
 * @param number_motors Number of motors
 * 
 * @see Martius, G. (2012). "Guided self-organization for robot behavior"
 */
void step(const sensor* sensors, int number_sensors,
          motor* motors, int number_motors) override;
```

## 🔍 Troubleshooting

### Common Issues

1. **IntelliSense Not Working**
   ```bash
   # Refresh IntelliSense database
   Cmd+Shift+P → "C/C++: Reset IntelliSense Database"
   
   # Check compile_commands.json
   ls build/*/compile_commands.json
   ```

2. **Build Errors**
   ```bash
   # Clean build
   cmake --build build/dev --target clean
   cmake --configure build/dev
   cmake --build build/dev
   ```

3. **Debugging Issues**
   ```bash
   # Check debug symbols
   file ./start
   # Should show: "with debug_info, not stripped"
   
   # For macOS code signing issues
   codesign -s - -f ./start
   ```

4. **Performance Issues**
   ```bash
   # Check optimization flags
   cmake -LA build/release | grep CMAKE_CXX_FLAGS
   
   # Profile hotspots
   perf record ./start_opt
   perf report
   ```

### Getting Help

1. **Check documentation**:
   - `.vscode/SETUP.md` - Setup guide
   - `.vscode/scripts/README.md` - Script documentation
   - `doc/` - Project documentation

2. **Debug output**:
   ```cpp
   // Enable debug mode
   export LPZROBOTS_DEBUG=1
   
   // Add debug prints
   #define DEBUG_PRINT(x) std::cout << #x << " = " << x << std::endl
   ```

3. **Community**:
   - GitHub Issues
   - Project discussions
   - Research papers

## 🎉 Happy Coding!

Remember: LPZRobots is about exploring self-organization and emergence. Don't be afraid to experiment!