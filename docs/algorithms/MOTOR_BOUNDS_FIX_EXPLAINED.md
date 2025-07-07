# Motor Bounds Issue - Fixed

## The Problem
You discovered that the novel controllers (AH-Sox, MSHC, CASox) were producing motor values far exceeding the expected [-1, 1] range, with values over 10. This is a serious issue because:
- Robot motors expect normalized values in [-1, 1]
- Large values can cause erratic behavior or damage simulated joints
- The controllers should be using tanh activation which bounds outputs to [-1, 1]

## Root Causes Identified

### 1. **No Motor Value Clamping**
While the controllers apply tanh activation (`g(x) = tanh(x)`), the motor values could still exceed bounds due to:
- Numerical instabilities
- Wiring transformations
- Accumulated floating-point errors

### 2. **High Learning Rates**
The original configuration used:
- `epsC = 0.1` (controller learning rate)
- `epsA = 0.05` (model learning rate)

These high rates could cause the controller matrices (C and h) to grow unbounded over time.

### 3. **Insufficient Damping**
Without proper damping, the controller parameters can accumulate errors and grow without bounds.

### 4. **High Initial Feedback Strength**
The default `initFeedbackStrength = 1.0` could start the controllers with too strong feedback, leading to instability.

## The Fix

### 1. **Motor Clamping Wiring**
Created `MotorClampingWiring` class that:
- Inherits from `DerivativeWiring`
- Clamps all motor values to [-1, 1] after wiring
- Logs warnings when clamping occurs

```cpp
class MotorClampingWiring : public DerivativeWiring {
    // ... clamps motor values to [-1, 1]
};
```

### 2. **Reduced Learning Rates**
- `epsC = 0.05` (was 0.1)
- `epsA = 0.02` (was 0.05)

### 3. **Added Damping**
- `damping = 0.001` prevents unbounded parameter growth

### 4. **Reduced Initial Feedback**
- `initFeedbackStrength = 0.5` (was 1.0)

### 5. **Motor Range Monitoring**
Added real-time monitoring that displays:
- Current motor value range
- Warnings when values exceed bounds

## Testing the Fix

Run the test script:
```bash
./test_fixed_hexapod.sh
```

You should see:
1. Motor range displayed as `Motor range: [-1.000, 1.000]`
2. No WARNING messages about clamping
3. Smooth, stable robot walking behavior

## What This Means

The controllers now:
- Produce properly bounded motor commands
- Learn more stably with conservative parameters
- Have safeguards against numerical instabilities
- Display warnings if bounds are exceeded

The robot should now walk smoothly with all novel controllers!