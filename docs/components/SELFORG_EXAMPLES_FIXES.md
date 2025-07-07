# Selforg Examples Compilation Fixes

## Issues Fixed

### 1. Misplaced `explicit` Keywords ✅
Fixed function declarations that incorrectly had `explicit` outside of class definitions:
- `controllertest.cpp`: `int explicit test(...)` → `int test(...)`
- `feedforwardtest.cpp`: Fixed 4 functions
- `integration/cmdline.cpp`: `void explicit control_c(...)` → `void control_c(...)`

### 2. Missing Variable Declarations ✅
Fixed malformed sensor array declarations:
- `controllertest.cpp`: Added `double sensors[SNumber];`
- `directconnect.cpp`: Added `double sensors[SNumber];`

### 3. Const Correctness Issues ✅
Fixed functions trying to call non-const methods on const objects:
- `feedforwardtest.cpp`: Changed function parameters from `const MultiLayerFFNN&` to `MultiLayerFFNN&`
  - `void test2x2(MultiLayerFFNN& net)`
  - `void testresponse(MultiLayerFFNN& net)`
  - `void testinvertation(MultiLayerFFNN& net)`
  - `void testprojections(ControllerNet& net)`

### 4. Function Pointer Overload Resolution ✅
Fixed ambiguous function pointer usage with Matrix::map/toMap:
- Added helper function: `double random_helper(double)` that wraps `random_minusone_to_one()`
- Replaced all occurrences of `random_minusone_to_one` in map/toMap calls with `random_helper`

## Root Cause
These errors were caused by:
1. Incorrect automated sed replacements that added `explicit` to function names
2. Malformed comments that corrupted variable declarations
3. Test functions incorrectly declared to take const references when they need to modify the objects
4. Function pointer ambiguity due to overloaded functions

## Verification
The examples should now compile successfully. The fixes maintain the original functionality while conforming to C++17 standards.