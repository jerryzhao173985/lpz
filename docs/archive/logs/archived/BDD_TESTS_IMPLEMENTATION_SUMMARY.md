# BDD-Style Tests Implementation Summary

## 📋 Overview

Phase 7 of the doctest migration involved creating Behavior-Driven Development (BDD) style tests for high-level behavior scenarios in the LPZRobots framework. This phase aimed to test emergent behaviors, learning dynamics, and complex interactions in a more natural, scenario-based format.

## 🚀 Implementation Progress

### Files Created

1. **`/tests/bdd/visual_motor_scenarios_test.cpp`**
   - Visual-motor coordination scenarios
   - Phototaxis behavior testing
   - Hand-eye coordination learning
   - Emergent scanning behavior
   - **Status**: Compiled successfully, crashes during runtime

2. **`/tests/bdd/emergent_behavior_scenarios_test.cpp`** (User modified)
   - Spontaneous pattern formation in coupled systems
   - Self-organized criticality in neural dynamics
   - Swarm behavior from local interactions
   - **Status**: Compiled successfully, crashes during runtime

3. **`/tests/bdd/learning_scenarios_test.cpp`**
   - Controller learning to track moving targets
   - Pattern generalization
   - Multi-controller collaboration
   - Adaptation to environmental changes
   - Learning with sparse rewards
   - **Status**: Compiled successfully, crashes during runtime

### CMake Integration

Successfully updated `/tests/CMakeLists.txt` to include:
```cmake
# BDD-style test sources
set(BDD_TEST_SOURCES
    doctest_main_simple.cpp
    bdd/visual_motor_scenarios_test.cpp
    bdd/emergent_behavior_scenarios_test.cpp
    bdd/learning_scenarios_test.cpp
)

# Create BDD test executable
lpzrobots_add_doctest(bdd_tests
    SOURCES ${BDD_TEST_SOURCES}
    DEPS 
        lpzrobots::selforg
    COMPONENT bdd
    LABELS bdd scenarios
)

add_custom_target(test_bdd
    COMMAND ${CMAKE_CTEST_COMMAND} -L bdd --output-on-failure
    COMMENT "Running BDD-style tests"
    DEPENDS bdd_tests
)
```

## 🔧 Technical Implementation

### BDD Macros
- Used doctest's built-in BDD support: `SCENARIO`, `GIVEN`, `WHEN`, `THEN`
- Added custom `AND` macro for additional context
- Fixed macro redefinition warnings by using doctest's native BDD support

### Key Design Patterns

1. **Scenario-Based Testing**
   ```cpp
   SCENARIO("Robot learns to track a moving light source") {
       GIVEN("a robot with visual sensors and motor control") {
           // Setup
           WHEN("the robot perceives the light on its right") {
               // Action
               THEN("the robot should turn towards the light") {
                   // Verification
               }
           }
       }
   }
   ```

2. **Helper Classes for Complex Behaviors**
   - `VisualMotorAgent` - Encapsulates visual-motor coordination
   - `VisualEnvironment` - Simulates visual stimuli
   - `BehaviorAnalyzer` - Analyzes time series for emergent patterns
   - `LearningEnvironment` - Provides various learning tasks
   - `PatternAnalyzer` - Detects synchronization and periodicity

3. **Realistic Simulations**
   - Multi-agent swarm dynamics
   - Coupled oscillator networks
   - Visual sensor systems with optical flow
   - Energy-aware robots for efficiency testing

## ❌ Current Issues

### Runtime Crashes
All BDD tests crash with segmentation faults during controller initialization:
```
FATAL ERROR: test case CRASHED: SIGSEGV - Segmentation violation signal
```

### Likely Causes
1. **Uninitialized RandGen**: The random generator might need proper seeding
2. **Controller Factory Issues**: Direct instantiation might be problematic
3. **Memory Management**: Smart pointers with controllers might have issues
4. **Missing Dependencies**: Some controllers might require additional setup

### Example Crash Location
```cpp
RandGen rgen;  // Might need initialization
controller->init(sensors, motors, &rgen);  // Crashes here
```

## 📊 Test Coverage (When Working)

### Visual-Motor Scenarios
- Visual feedback guided learning
- Saccadic eye movements
- Smooth pursuit tracking
- Total: 3 major scenarios with multiple sub-tests

### Emergent Behavior Scenarios  
- Coupled oscillator synchronization
- Self-organized criticality
- Swarm flocking behavior
- Total: 3 major scenarios testing collective dynamics

### Learning Scenarios
- Tracking with different learning rates
- Pattern generalization
- Multi-controller collaboration
- Environmental adaptation
- Sparse reward learning
- Total: 5 major scenarios covering various learning paradigms

## 🔨 Next Steps

1. **Debug Controller Initialization**
   ```cpp
   // Try proper RandGen initialization
   RandGen rgen;
   rgen.init(42);  // Fixed seed for reproducibility
   ```

2. **Simplify Test Cases**
   - Start with minimal controller tests
   - Add complexity gradually
   - Use established patterns from working unit tests

3. **Memory Debugging**
   ```bash
   # Run with address sanitizer
   ./bdd_tests --test-case="*simple*"
   
   # Use debugger
   lldb ./bdd_tests
   run --test-case="*Visual*"
   ```

4. **Alternative Approaches**
   - Use controller factory with proper error checking
   - Initialize controllers similar to working unit tests
   - Add null checks and validation

## 💡 Value When Fixed

The BDD tests provide high-level validation of emergent behaviors that are difficult to test with traditional unit tests:

1. **Emergent Properties**: Tests self-organization, synchronization, and collective behavior
2. **Learning Dynamics**: Validates adaptation and generalization capabilities
3. **Realistic Scenarios**: Tests controllers in contexts similar to actual research use
4. **Documentation**: Scenarios serve as executable documentation of expected behaviors

## 📝 Lessons Learned

1. **Controller Lifecycle**: LPZRobots controllers have specific initialization requirements
2. **BDD Complexity**: High-level scenarios require careful setup and teardown
3. **Doctest BDD**: Native BDD support works well but requires string literals
4. **Integration Challenges**: Complex behaviors need proper component integration

## 🎯 Conclusion

While the BDD test implementation is syntactically complete and compiles successfully, runtime issues prevent execution. The framework is in place for comprehensive behavioral testing once the initialization crashes are resolved. The BDD approach shows promise for testing the complex, emergent behaviors that are central to the LPZRobots research focus on self-organization and autonomous behavior development.

**Status**: Implementation 90% complete, debugging required for runtime stability.