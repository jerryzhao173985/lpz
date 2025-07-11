# Controller Template for LPZRobots

This directory contains a template for creating new self-organizing controllers.

## Quick Start

1. **Copy the template files** to your controller directory:
   ```bash
   cp mycontroller.h mycontroller.cpp ../yourcontroller.*
   ```

2. **Rename the class** and update the header guard:
   - Replace `MyController` with `YourController`
   - Update `__MYCONTROLLER_H` to `__YOURCONTROLLER_H`

3. **Modify the controller logic** in `calculateControllerValues()` and `learn()`

4. **Add to your simulation**:
   ```cpp
   #include <selforg/yourcontroller.h>
   
   // In start() method
   AbstractController* controller = new YourController();
   ```

## Template Structure

### Key Methods to Implement

1. **`calculateControllerValues()`** - Core control logic
   ```cpp
   // Example: Simple linear controller
   return C * x + h;
   
   // Example: Nonlinear controller
   Matrix z = C * x + h;
   return z.map(tanh);
   ```

2. **`learn()`** - Learning/adaptation logic
   ```cpp
   // Calculate prediction error
   xi = x - x_pred;
   
   // Update model
   A += (xi * y_tm1.T()) * epsA;
   
   // Update controller (your learning rule here)
   C += learning_signal * epsC;
   ```

3. **`ensureStability()`** - Stability mechanisms
   ```cpp
   // Check matrix norms
   if(C.norm() > threshold) {
     C *= 0.95;  // Scale down
   }
   ```

### Important Features

1. **Parameter Management**
   - Use `addParameter()` in constructor for runtime-adjustable parameters
   - Parameters are automatically exposed to GUI and console

2. **Matrix Operations**
   - Use LPZRobots matrix library (efficient, optimized)
   - Common operations: `*`, `+`, `.T()` (transpose), `.map()`

3. **Stability Helpers**
   - `clip()` - Limit values to [-1, 1]
   - `g()` / `g_s()` - Activation function and derivative
   - Check `.isNormal()` to detect NaN/Inf

4. **Interfaces Implemented**
   - **Configurable** - Runtime parameters
   - **Inspectable** - Internal state access
   - **Storeable** - Save/load controller state
   - **Teachable** - Supervised learning mode

## Common Controller Patterns

### 1. Homeokinetic Controller
```cpp
// In learn():
Matrix v = y_tm1.map(g_s);  // Derivative
Matrix dC = (A.T() * xi) * v.T();
C += dC * epsC;
```

### 2. Predictive Controller
```cpp
// Multi-step prediction
Matrix x_future = x;
for(int i = 0; i < prediction_steps; ++i) {
  Matrix y_future = calculateControllerValues(x_future);
  x_future = A * y_future + b;
}
```

### 3. Adaptive Noise
```cpp
// Adjust noise based on prediction error
double error_magnitude = xi.norm();
noise = noise * 0.99 + error_magnitude * 0.01;
```

### 4. Teaching Integration
```cpp
if(useTeaching && y_teaching) {
  Matrix y_target(number_motors, 1, y_teaching);
  Matrix teaching_error = y_target - y;
  // Blend teaching with exploration
}
```

## Testing Your Controller

1. **Basic Stability Test**
   ```cpp
   // In simulation main.cpp
   controller->setParam("epsA", 0.01);  // Start with low learning rates
   controller->setParam("epsC", 0.01);
   controller->setParam("damping", 0.05);
   ```

2. **Monitor Key Metrics**
   ```cpp
   // Add to simulation loop
   if(t % 100 == 0) {
     Matrix C = controller->getMatrix("C");
     printf("C norm: %.3f\n", C.norm());
   }
   ```

3. **Use Debugging Tools**
   ```bash
   # Check for stability issues
   lpz-debug --check-stability robot.log
   
   # Analyze controller dynamics
   lpz-analyze --eigenvalues controller.ctrl
   ```

## Best Practices

1. **Start Simple** - Get basic control working before adding complexity
2. **Use Small Learning Rates** - Start with epsA=0.01, epsC=0.01
3. **Add Damping** - Helps prevent instability
4. **Check Matrix Norms** - Detect problems early
5. **Test with Different Robots** - Ensure generalization
6. **Log Everything** - Use guilogger to monitor behavior
7. **Save Checkpoints** - Store controller state periodically

## Common Issues and Solutions

### Controller Explodes (NaN/Inf)
- Reduce learning rates
- Add damping
- Check sensor scaling
- Use logarithmic error

### No Learning
- Increase learning rates carefully
- Check if prediction error is meaningful
- Verify matrix dimensions
- Ensure sensors provide information

### Oscillations
- Increase damping
- Use smoothing/filtering
- Reduce controller gain
- Add noise for exploration

## References

- Der & Martius (2011) "The Playful Machine"
- LPZRobots documentation
- Example controllers in `selforg/controller/`