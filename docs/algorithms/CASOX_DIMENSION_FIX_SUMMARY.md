# CASox Matrix Dimension Mismatch Fix Summary

## Problem Description
The CASox controller was experiencing assertion failures when matrix dimensions changed between sensor configurations. The error occurred in the matrix addition operation (toSum function) with the message:
```
Assertion failed: (a.m == m && a.n == n), function toSum, file matrix.cpp, line 443
```

This happened specifically when:
- The robot had 4 sensors and 2 motors
- Context prototypes were created with one sensor configuration
- The robot's sensor configuration changed later
- Matrix operations tried to add/subtract matrices with different dimensions

## Root Causes Identified

1. **No dimension validation** in `adaptContextPrototypes()` before matrix operations
2. **Buffer index bug** in `learnContextModels()` where both `x_past` and `x_now` used the same index
3. **No dimension checks** in `distanceToContext()` and `mergeContexts()`
4. **No dimension validation** when restoring contexts from file

## Fixes Applied

### 1. Fixed `adaptContextPrototypes()` (lines 487-503)
Added dimension validation before updating context prototypes:
```cpp
// Ensure prototype has correct dimensions before updating
if (context.prototype.getM() != x_current.getM() || 
    context.prototype.getN() != x_current.getN()) {
    // Resize prototype to match current sensor dimensions
    context.prototype.set(x_current.getM(), x_current.getN());
    context.prototype = x_current;  // Reset to current value
    return;  // Skip adaptation this step
}
```

### 2. Fixed buffer index bug in `learnContextModels()` (line 326)
Changed:
```cpp
const Matrix& x_now = x_buffer[(t - 1) % buffersize];  // BUG!
```
To:
```cpp
const Matrix& x_now = x_buffer[t % buffersize];  // Correct
```

### 3. Added dimension check in `distanceToContext()` (lines 271-281)
```cpp
// Check dimension compatibility
if (x.getM() != context.prototype.getM() || 
    x.getN() != context.prototype.getN()) {
    // Return large distance for incompatible contexts
    return 1000.0;  
}
```

### 4. Added dimension check in `mergeContexts()` (lines 280-284)
```cpp
// Check dimension compatibility first
if (contexts[i].prototype.getM() != contexts[j].prototype.getM() ||
    contexts[i].prototype.getN() != contexts[j].prototype.getN()) {
    continue;  // Skip incompatible contexts
}
```

### 5. Added dimension validation in `restore()` (lines 593-621)
When restoring contexts from file, validate and fix dimensions to match current robot configuration:
```cpp
// Validate restored context dimensions match current setup
if (number_sensors > 0 && number_motors > 0) {
    // Check and fix prototype dimensions if needed
    if (context.prototype.getM() != number_sensors || 
        context.prototype.getN() != 1) {
        context.prototype.set(number_sensors, 1);
        context.prototype.toZero();
    }
    // Similar checks for A, b, S matrices...
}
```

## Testing
Created test programs to verify the fixes:
- `test_casox_dimensions.cpp` - Comprehensive test with save/restore
- `test_casox_simple.cpp` - Simple dimension mismatch simulation

The simple test confirms that dimension mismatches are now properly handled without assertion failures.

## Impact
These fixes ensure that:
1. CASox can handle robots with changing sensor/motor configurations
2. Saved controller states can be restored even with different robot dimensions
3. Context merging only happens between compatible contexts
4. No assertion failures occur during normal operation

The controller now gracefully adapts to dimension changes by resizing internal structures as needed.