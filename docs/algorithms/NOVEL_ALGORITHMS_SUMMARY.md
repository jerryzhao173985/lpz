# Novel Homeokinetic Algorithms Summary

## Overview
Three novel homeokinetic controllers have been implemented, extending the Sox algorithm with different approaches to self-organization and adaptation.

## 1. AH-Sox (Adaptive Horizon Sox)
**Status**: ✅ Fully Functional

### Concept
Dynamically adjusts prediction horizons based on uncertainty, allowing the robot to adapt its temporal focus.

### Key Features
- Multiple forward models with different time horizons (1-5 steps)
- Uncertainty tracking for each model
- Weighted combination based on prediction accuracy
- Automatic horizon selection

### Implementation Details
- Files: `ahsox.h`, `ahsox.cpp`
- Supports both square and non-square sensor-motor configurations
- Tested and working correctly

### Usage
```cpp
AHSox::AHSoxConf conf = AHSox::getDefaultConf();
conf.numModels = 5;
conf.horizonRange = 5;
auto controller = std::make_unique<AHSox>(conf);
```

## 2. MSHC (Multi-Scale Homeokinetic Controller)
**Status**: ✅ Fully Functional

### Concept
Hierarchical time scales for different behavioral levels - from fast reflexes to slow patterns.

### Key Features
- Three time scales: Fast (1-2 steps), Medium (5-10 steps), Slow (20-50 steps)
- Scale-specific models and controllers
- Dynamic scale weighting based on activity
- Emergent behavioral hierarchy

### Implementation Details
- Files: `mshc.h`, `mshc.cpp`
- Supports both square and non-square configurations
- Tested and working correctly

### Usage
```cpp
MSHC::MSHCConf conf = MSHC::getDefaultConf();
conf.numScales = 3;
conf.scaleInteraction = 0.1;
auto controller = std::make_unique<MSHC>(conf);
```

## 3. CASox (Context-Aware Sox)
**Status**: ⚠️ Partially Functional

### Concept
Automatic context detection with specialized models for different situations.

### Key Features
- Dynamic context creation and merging
- Context-specific forward models
- Prototype-based context detection
- Adaptive exploration levels per context

### Implementation Details
- Files: `casox.h`, `casox.cpp`
- ✅ Works with square configurations (equal sensors/motors)
- ❌ Has issues with non-square configurations due to Jacobian calculation

### Known Issues
1. **Non-square Configuration Bug**: Matrix dimension mismatch in homeokinetic learning
   - Problem: Jacobian L calculation assumes square matrices
   - Location: `learnController()` function
   - Fix needed: Proper handling of sensor-motor loop Jacobian

2. **Dimension Handling**: Already fixed for context prototype adaptation

### Usage
```cpp
CASox::CASoxConf conf = CASox::getDefaultConf();
conf.maxContexts = 5;
conf.contextRadius = 0.5;
auto controller = std::make_unique<CASox>(conf);
```

## Test Results

### Unit Tests
All algorithms pass basic initialization and step tests:
```bash
cd /Users/jerry/lpzrobot_mac/tests/unit
./novel_homeokinetic_test
```

### Integration Tests
- AH-Sox: ✅ Fully functional in simulations
- MSHC: ✅ Fully functional in simulations  
- CASox: ⚠️ Works only with square configurations

### Demo Simulation
```bash
cd /Users/jerry/lpzrobot_mac/ode_robots/simulations/sphererobot_novel_controllers
make && ./start -g
```
- Shows 4 sphere robots using different controllers
- Press 'v' to cycle through robot views
- Press 'x' to create perturbations

## Performance Characteristics

### AH-Sox
- Memory: O(H × M × N) where H=horizons, M=sensors, N=motors
- Computation: O(H) forward model updates per step
- Best for: Environments with varying predictability

### MSHC
- Memory: O(S × M × N) where S=scales
- Computation: O(S) scale updates per step
- Best for: Complex behaviors requiring multiple time scales

### CASox
- Memory: O(C × M × N) where C=contexts
- Computation: O(C) context evaluations per step
- Best for: Environments with distinct behavioral contexts

## Future Improvements

### CASox Non-Square Fix
To fix the non-square configuration issue:
1. Implement proper Jacobian calculation for sensor-motor loop
2. Consider using reduced-dimension projections
3. Add explicit handling for over/under-actuated systems

### General Enhancements
1. Add save/restore functionality for learned models
2. Implement visualization for:
   - Horizon selection (AH-Sox)
   - Scale activations (MSHC)
   - Context maps (CASox)
3. Add performance benchmarks
4. Create parameter tuning guidelines

## Integration with LPZRobots
All controllers are registered in the ControllerFactory:
```cpp
auto controller = ControllerFactory::createController("AHSox");
auto controller = ControllerFactory::createController("MSHC");
auto controller = ControllerFactory::createController("CASox");
```

Categories:
- All three are registered under "Homeokinetic" category
- Can be used with any robot via the standard Agent-Robot-Controller pattern