# selforg Implementation Guide - Memory Modernization

## Quick Reference: What to Change

### 1. Replace malloc/free with std::vector

#### ❌ OLD (Dangerous):
```cpp
// agent.cpp
sensor* rsensors = static_cast<sensor*>(malloc(sizeof(sensor) * rsensornumber));
// ... use rsensors ...
free(rsensors);  // Must remember to free!
```

#### ✅ NEW (Safe):
```cpp
// agent.h
#include <vector>
class Agent {
    std::vector<sensor> rsensors;
    std::vector<motor> rmotors;
};

// agent.cpp
rsensors.resize(rsensornumber);  // Automatic memory management
// No free needed - automatic cleanup!
```

### 2. Replace Raw Owning Pointers with unique_ptr

#### ❌ OLD (Manual delete required):
```cpp
// agent.h
class Agent {
    AbstractRobot* robot;  // Who owns this?
};

// agent.cpp
Agent::~Agent() {
    if(robot) delete robot;  // Manual cleanup
}
```

#### ✅ NEW (Automatic cleanup):
```cpp
// agent.h
#include <memory>
class Agent {
    std::unique_ptr<AbstractRobot> robot;
};

// agent.cpp
Agent::~Agent() {
    // No manual delete needed!
}
```

### 3. Fix Function Signatures

#### ❌ OLD (C-style arrays):
```cpp
virtual int getSensors(sensor* sensors, int sensornumber) = 0;
```

#### ✅ NEW Option 1 (Keep compatibility):
```cpp
// Keep old signature but use vector internally
int getSensors(sensor* sensors, int sensornumber) override {
    // Copy from internal vector
    std::copy_n(rsensors.begin(), std::min(size_t(sensornumber), rsensors.size()), sensors);
    return rsensors.size();
}
```

#### ✅ NEW Option 2 (Modern API):
```cpp
// Add new method alongside old
virtual std::vector<sensor> getSensors() const {
    return rsensors;  // Return by value (RVO optimized)
}
```

## Step-by-Step: Fixing agent.cpp

### Step 1: Update Headers
```cpp
// agent.h
#include <memory>   // for std::unique_ptr
#include <vector>   // for std::vector
#include <algorithm> // for std::copy_n
```

### Step 2: Change Member Variables
```cpp
// agent.h
class Agent : public WiredController {
private:
    // OLD
    // AbstractRobot* robot;
    // sensor *rsensors;
    // motor  *rmotors;
    
    // NEW
    std::unique_ptr<AbstractRobot> robot;
    std::vector<sensor> rsensors;
    std::vector<motor> rmotors;
};
```

### Step 3: Update Constructor/Destructor
```cpp
// agent.cpp
Agent::Agent(...) : WiredController(...) {
    // No initialization needed - vectors start empty
    // unique_ptr starts as nullptr
}

Agent::~Agent() {
    trackrobot.close();
    // No manual delete needed!
}
```

### Step 4: Update init() Method
```cpp
bool Agent::init(AbstractController* controller, AbstractRobot* robot,
                 AbstractWiring* wiring, long int seed) {
    // Take ownership
    this->robot.reset(robot);
    assert(this->robot);
    
    rsensornumber = this->robot->getSensorNumber();
    rmotornumber = this->robot->getMotorNumber();
    
    // Resize vectors
    rsensors.resize(rsensornumber, 0.0);
    rmotors.resize(rmotornumber, 0.0);
    
    // Rest of init...
}
```

### Step 5: Update step() Method
```cpp
void Agent::step(double noise, double time) {
    assert(robot && !rsensors.empty() && !rmotors.empty());
    
    // Use .data() to get raw pointer
    int len = robot->getSensors(rsensors.data(), rsensornumber);
    
    WiredController::step(rsensors.data(), rsensornumber, 
                          rmotors.data(), rmotornumber, noise, time);
    robot->setMotors(rmotors.data(), rmotornumber);
}
```

## Common Patterns

### Pattern 1: Dynamic Arrays
```cpp
// OLD
double* array = new double[size];
delete[] array;

// NEW
std::vector<double> array(size);
// or
std::unique_ptr<double[]> array = std::make_unique<double[]>(size);
```

### Pattern 2: 2D Arrays
```cpp
// OLD
double** matrix = new double*[rows];
for(int i = 0; i < rows; i++) {
    matrix[i] = new double[cols];
}

// NEW
std::vector<std::vector<double>> matrix(rows, std::vector<double>(cols));
```

### Pattern 3: Conditional Ownership
```cpp
// OLD
if(ptr) delete ptr;
ptr = new_value;

// NEW
ptr.reset(new_value);  // Automatically deletes old value
```

## Testing Your Changes

### 1. Compile Test
```bash
cd selforg
make clean
make CXXFLAGS="-std=c++17 -Wall -Wextra -Wpedantic"
```

### 2. Memory Leak Test
```bash
make CXXFLAGS="-fsanitize=address -g"
./test_program
# Should see: "AddressSanitizer: heap-buffer-overflow" for any issues
```

### 3. Valgrind Test
```bash
valgrind --leak-check=full --show-leak-kinds=all ./test_program
# Should see: "All heap blocks were freed -- no leaks are possible"
```

## Checklist for Each File

- [ ] Add `#include <memory>` if using smart pointers
- [ ] Add `#include <vector>` if using vectors
- [ ] Replace all malloc/calloc with vector or unique_ptr
- [ ] Replace all free with nothing (automatic cleanup)
- [ ] Replace raw owning pointers with unique_ptr
- [ ] Update function signatures to use .data() when needed
- [ ] Remove manual delete statements
- [ ] Test with AddressSanitizer

## Performance Considerations

1. **Reserve Vector Capacity**:
```cpp
std::vector<double> data;
data.reserve(1000);  // Avoid reallocations
```

2. **Move Semantics**:
```cpp
robot = std::make_unique<MyRobot>();  // Direct construction
```

3. **Return Value Optimization**:
```cpp
std::vector<sensor> getSensors() {
    return rsensors;  // RVO - no copy
}
```

## Next Files to Fix

After agent.cpp/h:
1. wiredcontroller.cpp/h (same patterns)
2. abstractrobot.h (API design)
3. All simulation main.cpp files
4. Controller implementations

Remember: Start with core files that affect everything else!