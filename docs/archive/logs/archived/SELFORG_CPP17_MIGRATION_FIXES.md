# C++17 Migration Fixes for selforg/simulations

## Summary of Common Issues Fixed

This document logs all the patterns of errors and fixes applied during the C++17 migration of the selforg/simulations directory.

### 1. Misplaced `explicit` Keywords
**Issue**: The `explicit` keyword was incorrectly placed on regular functions instead of constructors/conversion operators.
**Pattern**: `void explicit functionName()` or `double explicit functionName()`
**Fix**: Remove `explicit` keyword from non-constructor functions
**Examples**:
- `double explicit toEnv()` → `double toEnv()`
- `void explicit printRobots()` → `void printRobots()`
- `void explicit reinforce()` → `void reinforce()`
- `int explicit coord()` → `int coord()`

### 2. Missing Class Inheritance
**Issue**: Classes that should inherit from `AbstractRobot` were missing the inheritance declaration.
**Pattern**: `class MyRobot {` when it uses AbstractRobot methods
**Fix**: Add proper inheritance: `class MyRobot : public AbstractRobot {`
**Affected Classes**:
- MyRobot in multiple files
- Pendulum class
- Tactile struct (converted to proper class structure)

### 3. Incorrect `nullptr` Usage in Comparisons
**Issue**: Using `nullptr` in integer/numeric comparisons instead of 0.
**Pattern**: 
- `if(strcmp(str1, str2) == nullptr)`
- `if(contains(...) != nullptr)`
- `if(value % n == nullptr)`
**Fix**: Replace `nullptr` with `0` for numeric comparisons
**Note**: `nullptr` is only for pointer comparisons, not for integer return values

### 4. Missing `override` Keywords
**Issue**: Virtual functions from base classes missing `override` specifier.
**Pattern**: Virtual functions without `override` in derived classes
**Fix**: Add `override` to all virtual function implementations
**Common Functions**:
- `getSensors()` → `getSensors() override`
- `setMotors()` → `setMotors() override`
- `getSensorNumber()` → `getSensorNumber() override`
- `getMotorNumber()` → `getMotorNumber() override`
- `getPosition()` → `getPosition() override`

### 5. Malformed Syntax from Bad Sed Replacements
**Issue**: Automated sed scripts created malformed syntax
**Patterns**:
- `ifstatic_cast<x>(delete)[] x;` → `if(x) delete[] x;`
- `static_cast<double*>(malloc)(sizeof(...))` → `static_cast<double*>(malloc(sizeof(...)))`
- `sizeofstatic_cast<int>(override)` → `sizeof(int)`
- `static_cast<distance>(for)` → proper for loop syntax

### 6. Missing Standard Library Includes
**Issue**: Missing includes for standard C/C++ functions
**Common Missing Includes**:
- `<cstring>` for `strcmp()`, `memset()`, `memcpy()`
- `<cmath>` for `sin()`, `cos()`, `fabs()`, `M_PI`
- `<cstdlib>` for `atoi()`, `exit()`
- `<algorithm>` for `sort()`
- `<ctime>` for `time()`

### 7. Const-correctness Issues
**Issue**: Functions trying to modify const parameters
**Pattern**: `Position toEnv(const Position& pos) { pos.x = ...; }`
**Fix**: Change to pass-by-value: `Position toEnv(Position pos)`

### 8. Member Variable Initialization
**Issue**: Missing member variable declarations or initializations
**Pattern**: Using variables without declaring them as class members
**Fix**: Add proper member declarations and initialize in constructor

### 9. Static Cast Issues
**Issue**: Incorrect static_cast syntax and usage
**Patterns**:
- Wrong parenthesization: `static_cast<Type*>(*ptr)->method()`
- Cast to wrong const-ness: `static_cast<Type*>(constPtr)`
**Fix**: Proper casting with correct const qualifiers

### 10. Duplicate Initialization in C++11
**Issue**: Both in-class initialization and constructor initialization
**Pattern**: 
```cpp
struct Foo {
  int x = 0;  // C++11 in-class init
  Foo() : x(0) {} // Also init in constructor
};
```
**Fix**: Remove in-class initialization when using constructor initialization list

## File-by-File Summary

### interaction_sims/interaction1D/
- **main_vision_2agents.cpp**: Fixed explicit keywords, inheritance, camera/toEnv functions
- **main_vision_4objects.cpp**: Similar fixes plus Tactile class structure
- **main_vision_bg.cpp**: Background array handling, PlotOption constructor
- **main_visual_motor.cpp**: Tactile class, vector handling
- **main.cpp**: Replay functionality, member initialization

### interaction_sims/shadoworreal/
- **main.cpp**: Added time() include, fixed sleep_ parameter
- **main_real_and_shadow_and_static.cpp**: Fixed sleep_ as member variable
- **main_real_and_shadow.cpp**: Major fixes to toEnv const-correctness

### pendulum/
- **main_starttop.cpp**: Fixed Pendulum inheritance, static_cast in System function

### simplesystems/
- **main.cpp**: MyRobot inheritance, realtimefactor comparisons

### spherical_xbee/
- **commtest/main.cpp**: Basic nullptr fixes
- **workstation/main.cpp**: Complex fixes including Communicator inheritance, Xbee struct

### tcpcommunication/
- **main.cpp**: Fixed destructor syntax, MyRobot inheritance

## Best Practices Applied

1. **Always check inheritance**: If a class uses AbstractRobot methods, it must inherit from it
2. **Use override consistently**: All virtual function implementations should have override
3. **Include what you use**: Don't rely on transitive includes
4. **Const-correctness**: Use const references when possible, pass-by-value when modification needed
5. **Smart pointer usage**: Prefer smart pointers over raw pointers (future work)
6. **RAII principles**: Ensure proper resource management in constructors/destructors

## Remaining Work

While these files are now fixed, the broader selforg codebase may have similar issues that need addressing:
1. Complete typedef → using migration
2. Add constexpr where appropriate
3. Use auto for complex type deduction
4. Migrate to smart pointers throughout
5. Enable and fix all compiler warnings