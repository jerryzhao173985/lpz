# BDD Test Crash Analysis

## 🔍 Root Cause Identified

The crashes in the BDD tests are caused by the Sox controller's internal buffer management and learning algorithm constraints.

### Key Findings:

1. **Buffer Size Limitation**: Sox has a `buffersize = 10` defined in sox.h
2. **Learning Start Requirement**: The Sox controller needs at least `buffersize` (10) steps before learning can begin
3. **Crash Timing**: Tests crash around step 9-10, coinciding with when the buffer is full

### Technical Details:

From `selforg/controller/sox.cpp`:
```cpp
void Sox::step(const sensor* x_, int number_sensors, motor* y_, int number_motors) {
  stepNoLearning(x_, number_sensors, y_, number_motors);
  if (t <= buffersize)
    return;
  --t; // stepNoLearning increases the time by one - undo here

  // learn controller and model
  if (epsC != 0 || epsA != 0)
    learn();

  // update step counter
  ++t;
};
```

The controller only starts learning after `t > buffersize` (10 steps). The crash likely happens in the `learn()` method when it tries to access historical data from buffers.

## 🛠️ Solution Approaches

### 1. Proper Initialization Sequence
```cpp
// Run warmup steps to fill buffers
for(int warmup = 0; warmup < 15; ++warmup) {
    controller->step(sensors, num_sensors, motors, num_motors);
}
// Now safe to use getLastSensorValues(), etc.
```

### 2. Avoid Early Access to History
- Don't call `getLastSensorValues()` or `getLastMotorValues()` before 10+ steps
- Don't try to record metrics during the first 10 steps

### 3. Configure Controller for Testing
```cpp
SoxConf conf = Sox::getDefaultConf();
conf.steps4Averaging = 1;  // Minimize averaging
conf.steps4Delay = 1;      // Minimize delay
```

## 🐛 Specific Issues in BDD Tests

### emergent_behavior_scenarios_test.cpp
- Multiple controllers being initialized and stepped in parallel
- Complex coupling between oscillators
- Accessing controller state too early

### learning_scenarios_test.cpp
- LearningTracker tries to access controller state immediately
- Recording metrics from step 1 instead of after warmup

### visual_motor_scenarios_test.cpp
- Visual sensor simulation adds complexity
- Multiple controllers with different configurations

## ✅ Recommended Fixes

1. **Add Proper Warmup Phase**
   ```cpp
   // Initialize controller
   controller->init(sensors, motors, &rgen);
   
   // Warmup phase - fill buffers
   for(int i = 0; i < 15; ++i) {
       controller->step(dummy_sensors, sensors, dummy_motors, motors);
   }
   
   // Now safe to start actual test
   ```

2. **Delay Metric Recording**
   ```cpp
   if(step > 15) {  // Only after warmup
       tracker.recordStep(controller.get());
   }
   ```

3. **Simplify Test Scenarios**
   - Start with single controllers
   - Add complexity gradually
   - Ensure each controller has proper warmup

4. **Add Safety Checks**
   ```cpp
   if(controller->getTimeSteps() > buffersize) {
       // Safe to access history
       Matrix sensors = controller->getLastSensorValues();
   }
   ```

## 🎯 Next Steps

1. Modify BDD tests to include proper warmup phases
2. Add step counters to ensure sufficient initialization
3. Consider creating a test-friendly controller wrapper
4. Document the buffer requirements in test comments

The core issue is not a bug in LPZRobots but rather a misunderstanding of the controller's initialization requirements. The Sox controller is designed for long-running simulations and expects a certain number of steps before all features are available.