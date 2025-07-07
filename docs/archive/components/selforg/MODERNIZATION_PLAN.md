# selforg C++ Modernization Plan

## Critical Issues Found

### 1. Memory Management Issues (HIGH PRIORITY)
The codebase extensively uses raw memory allocation with malloc/free and new/delete. This is error-prone and not exception-safe.

#### Current Issues:
- **agent.cpp**: Uses malloc/free for sensor/motor arrays
- **wiredcontroller.cpp**: Uses malloc/free for sensor/motor arrays
- **85 files** use raw memory allocation patterns
- No RAII compliance in core components
- Mixing C-style (malloc/free) with C++ (new/delete)

#### Recommended Fix:
```cpp
// Current (agent.cpp:71-72)
rsensors = static_cast<sensor*>(malloc(sizeof(sensor) * rsensornumber));
rmotors = static_cast<motor*>(malloc(sizeof(motor) * rmotornumber));

// Modernized Option 1: std::vector (preferred for flexibility)
std::vector<sensor> rsensors;
std::vector<motor> rmotors;
// In init:
rsensors.resize(rsensornumber);
rmotors.resize(rmotornumber);

// Modernized Option 2: std::unique_ptr (for fixed size)
std::unique_ptr<sensor[]> rsensors;
std::unique_ptr<motor[]> rmotors;
// In init:
rsensors = std::make_unique<sensor[]>(rsensornumber);
rmotors = std::make_unique<motor[]>(rmotornumber);
```

### 2. Raw Pointer Ownership (HIGH PRIORITY)
Classes store raw pointers without clear ownership semantics.

#### Current Issues:
- **Agent**: Owns robot, controller (deletes in destructor)
- **WiredController**: Owns controller, wiring, motorBabbler
- Ownership not expressed in type system
- Manual delete required

#### Recommended Fix:
```cpp
// Current (agent.h)
AbstractRobot* robot;

// Modernized
std::unique_ptr<AbstractRobot> robot;

// Current (agent.cpp)
if(robot) delete robot;

// Modernized - automatic cleanup
// No explicit delete needed
```

### 3. API Design Issues (MEDIUM PRIORITY)

#### a. C-style Arrays in Interfaces
```cpp
// Current
virtual int getSensors(sensor* sensors, int sensornumber) = 0;

// Modernized Option 1: span (C++20)
virtual void getSensors(std::span<sensor> sensors) = 0;

// Modernized Option 2: vector reference
virtual void getSensors(std::vector<sensor>& sensors) = 0;

// Modernized Option 3: Return by value with RVO
virtual std::vector<sensor> getSensors() = 0;
```

#### b. Error Handling
Current code uses assertions and fprintf for errors. Should use exceptions or std::expected (C++23).

### 4. Type Safety Issues (MEDIUM PRIORITY)
- Using double* for sensor/motor types loses semantic information
- Could use strong types:
```cpp
// Strong typing
template<typename Tag>
class StrongType {
    double value;
public:
    explicit StrongType(double v) : value(v) {}
    double get() const { return value; }
};

using Sensor = StrongType<struct SensorTag>;
using Motor = StrongType<struct MotorTag>;
```

### 5. Const-Correctness (LOW PRIORITY)
Many methods that should be const are not marked as such.

## Modernization Strategy

### Phase 1: Memory Safety (Critical)
1. Replace all malloc/free with std::vector or std::unique_ptr
2. Convert raw owning pointers to smart pointers
3. Add #include <memory> and #include <vector> where needed

### Phase 2: API Modernization
1. Create new API alongside old one for compatibility
2. Mark old API as [[deprecated]]
3. Implement span-based or vector-based interfaces

### Phase 3: Error Handling
1. Replace fprintf with proper logging
2. Add exception specifications
3. Consider std::expected for fallible operations

### Phase 4: Performance & Safety
1. Add move constructors/assignment operators
2. Mark appropriate methods noexcept
3. Use constexpr where possible

## Implementation Priority

### Immediate Actions (Do Now):
1. Fix memory management in core classes:
   - agent.cpp/h
   - wiredcontroller.cpp/h
   - abstractrobot.h (API design)

2. Add missing includes:
```cpp
#include <memory>   // for smart pointers
#include <vector>   // for std::vector
#include <algorithm> // for algorithms
#include <span>     // for std::span (C++20)
```

### Next Steps:
1. Propagate changes to derived classes
2. Update simulation examples
3. Create migration guide for users

## Backward Compatibility

To maintain compatibility:
1. Keep old interfaces but mark [[deprecated]]
2. Provide adapter functions
3. Document migration path

Example:
```cpp
class AbstractRobot {
public:
    // New API
    virtual std::vector<sensor> getSensors() = 0;
    
    // Old API (deprecated)
    [[deprecated("Use getSensors() returning vector")]]
    virtual int getSensors(sensor* sensors, int sensornumber) {
        auto values = getSensors();
        std::copy_n(values.begin(), std::min(values.size(), size_t(sensornumber)), sensors);
        return values.size();
    }
};
```

## Testing Strategy
1. Compile with AddressSanitizer
2. Run existing simulations
3. Check for memory leaks with Valgrind
4. Verify performance hasn't regressed

## Benefits
- Memory safety (no leaks, no double-free)
- Exception safety
- Clear ownership semantics
- Modern C++ idioms
- Better compiler optimizations
- Easier to maintain and extend