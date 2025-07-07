# Novel Homeokinetic Controllers

This document describes the novel homeokinetic controllers added to the LPZRobots framework. These controllers extend the core homeokinetic principle in creative ways while maintaining the fundamental self-organization philosophy.

## Overview

All homeokinetic controllers share the core principle of learning through time-loop error (TLE) minimization while maintaining exploration through anti-Hebbian mechanisms. The novel controllers extend this principle in different dimensions:

1. **AH-Sox** - Temporal adaptation through dynamic horizons
2. **MSHC** - Hierarchical organization through multiple time scales
3. **CASox** - Contextual adaptation through multiple models

## Controller Descriptions

### 1. Adaptive Horizon Sox (AH-Sox)

**File**: `ahsox.h`, `ahsox.cpp`

**Core Innovation**: Dynamically adjusts prediction horizons based on uncertainty.

**Key Features**:
- Maintains multiple forward models with different time horizons (1-10 steps)
- Tracks uncertainty for each horizon based on prediction quality
- Automatically selects the best horizon or uses weighted combination
- Adapts to different behavioral regimes (fast reflexes vs. slow movements)

**When to Use**:
- Environments with varying dynamics
- Tasks requiring both quick reactions and deliberate movements
- When optimal prediction horizon is unknown or changes

**Parameters**:
```cpp
AHSox::AHSoxConf conf;
conf.minHorizon = 1;          // Minimum prediction horizon
conf.maxHorizon = 10;         // Maximum prediction horizon
conf.horizonAdaptRate = 0.01; // How fast to adapt horizons
conf.useWeightedCombination = true; // Use all models vs. best only
```

### 2. Multi-Scale Homeokinetic Controller (MSHC)

**File**: `mshc.h`, `mshc.cpp`

**Core Innovation**: Hierarchical time-loop errors at multiple scales.

**Key Features**:
- Separate controllers for different time scales (fast/medium/slow)
- Cross-scale coupling enables emergent hierarchical behaviors
- Scale-specific learning rates and exploration
- Dynamic weight adaptation based on prediction quality

**When to Use**:
- Complex behaviors requiring hierarchical organization
- When both reflexes and long-term patterns are important
- Robots with multiple behavioral modes

**Parameters**:
```cpp
MSHC::MSHCConf conf;
conf.scaleHorizons = {2, 20, 100};    // Time horizons for each scale
conf.scaleWeights = {0.5, 0.3, 0.2};  // Initial importance weights
conf.crossScaleCoupling = 0.1;        // How scales influence each other
conf.dynamicScaleWeights = true;      // Adapt weights online
```

### 3. Context-Aware Sox (CASox)

**File**: `casox.h`, `casox.cpp`

**Core Innovation**: Multiple context-dependent forward models.

**Key Features**:
- Automatically detects different contexts from sensor patterns
- Maintains separate models for each context
- Smooth transitions between contexts using gating network
- Context-specific exploration and learning rates

**When to Use**:
- Environments with distinct regions or modes
- Robots interacting with different types of objects
- Tasks requiring different strategies in different situations

**Parameters**:
```cpp
CASox::CASoxConf conf;
conf.maxContexts = 5;              // Maximum number of contexts
conf.contextRadius = 0.5;          // Detection radius for contexts
conf.dynamicContexts = true;       // Allow creating new contexts
conf.contextSpecificExploration = true; // Different exploration per context
```

## Mathematical Foundation

### Time-Loop Error (Common to All)
The prediction error that drives learning:
```
ξ = x(t) - x̂(t)
```
where `x̂(t)` is the prediction from the forward model.

### Controller Update (Homeokinetic Gradient)
```
ΔC = ε_C * (μ * v^T - ε_rel * y * x^T)
```
where:
- `μ` is the backpropagated error
- `v` is the TLE in motor space
- The second term is the anti-Hebbian component

### Novel Extensions

**AH-Sox**: Multiple models with different horizons
```
x̂_h(t) = A_h * y(t-h) + b_h
```

**MSHC**: Hierarchical combination
```
y(t) = Σ_s w_s * g(C_s * x(t) + h_s)
```

**CASox**: Context gating
```
x̂(t) = Σ_c α_c * (A_c * y(t-1) + b_c)
```

## Usage Examples

### Basic Usage
```cpp
// Create controller with default configuration
AHSox controller;
controller.init(sensorNum, motorNum);

// Or with custom configuration
AHSox::AHSoxConf conf = AHSox::getDefaultConf();
conf.maxHorizon = 20;
AHSox controller(conf);
```

### Integration with Robots
```cpp
// In simulation start function
AbstractController* controller = new MSHC();
OdeAgent* agent = new OdeAgent(global);
agent->init(controller, robot, wiring);
```

### Parameter Tuning
```cpp
// Adjust parameters at runtime
controller->setParam("epsC", 0.1);        // Controller learning rate
controller->setParam("horizonAdaptRate", 0.05); // AH-Sox specific
controller->setParam("crossCoupling", 0.2);      // MSHC specific
```

## Behavioral Characteristics

### AH-Sox Behavior
- Initially uses short horizons for stability
- Gradually extends horizon as model improves
- Automatically shortens horizon in novel situations
- Smooth transitions between behavioral modes

### MSHC Behavior
- Fast scale handles immediate reactions
- Medium scale coordinates basic movements
- Slow scale develops long-term patterns
- Emergent hierarchical decomposition of behavior

### CASox Behavior
- Creates distinct behavioral modes for different contexts
- Smooth transitions when moving between contexts
- Context-specific optimization of movements
- Can develop specialized skills for each context

## Performance Considerations

- **AH-Sox**: Higher memory usage (multiple models), adaptive computation
- **MSHC**: Scales linearly with number of time scales
- **CASox**: Computation scales with number of active contexts

## Future Extensions

1. **Information-Theoretic AH-Sox**: Use information gain for horizon selection
2. **Attention-Based MSHC**: Learn which scale to attend to
3. **Meta-Learning CASox**: Learn how to create better contexts

## References

The homeokinetic principle is based on:
- Der, R. & Martius, G. (2012). The Playful Machine
- Martius, G. et al. (2013). Information driven self-organization

These novel extensions explore new dimensions while maintaining the core philosophy of self-organization through the interplay of prediction and exploration.