# Testing Infrastructure - Errors and Fixes Reference

## Quick Reference for Common Testing Issues

This document provides a concise reference for all errors encountered during testing infrastructure implementation and their solutions.

## Critical Integration Errors

### 1. Function Not Found: lpzrobots_add_doctest
```
CMake Error: Unknown CMake command "lpzrobots_add_doctest"
```
**Fix**: Add to `cmake/LPZRobotsCore.cmake` line 10:
```cmake
include(${CMAKE_CURRENT_LIST_DIR}/LPZRobotsTesting.cmake)
```

### 2. Duplicate Doctest Target
```
CMake Error: add_library cannot create target "doctest" because another target with the same name already exists
```
**Fix**: Remove FetchContent approach from `LPZRobotsTestingIntegration.cmake`. Use only the single-header approach from `LPZRobotsDependencies.cmake`.

### 3. Missing Test Files
```
Cannot find source file: /path/to/tests/doctest_main.cpp
```
**Fix**: Files actually exist. Verify with:
```bash
ls tests/doctest_main*.cpp
```

### 4. CMake Version Warning
```
setup_tests.sh: CMake version 4.0.3 - recommend 3.20+
```
**Fix**: Update version check in `setup_tests.sh`:
```bash
if [[ "$CMAKE_MAJOR" -gt 3 ]] || ([[ "$CMAKE_MAJOR" -eq 3 ]] && [[ "$CMAKE_MINOR" -ge 20 ]]); then
    echo "✅ CMake version $CMAKE_VERSION"
fi
```

## Build and Runtime Errors

### 5. Test Executable Not Found
```
Could not find executable /path/to/build/tests/unit_tests
```
**Fix**: Build the target first:
```bash
cmake --build build --target unit_tests
```

### 6. Controller Assertion Failures
```
Assertion failed: (conf.model != nullptr), function InvertMotorBigModel
```
**Status**: These are real bugs in the controllers, not test infrastructure issues!
- InvertMotorBigModel - Null model pointer
- InvertMotorSpace - NaN calculations
- SoxExpand - Context coupling failure
- InvertMotorNStep - Model adaptation issue

### 7. Warmup Required for Homeokinetic Controllers
```
Segmentation fault at AbstractController::buffersize
```
**Fix**: Add warmup phase before testing:
```cpp
// Warmup for buffer initialization
for (int i = 0; i < 15; ++i) {
    controller->step(sensors, num_sensors, motors, num_motors);
}
```

## Path and Configuration Issues

### 8. Script Not Found
```
./lpz-test: command not found
```
**Fix**: Use full path or add to PATH:
```bash
./scripts/lpz-test all
# OR
export PATH="$PATH:$(pwd)/scripts"
```

### 9. Doctest Header Not Found
```
fatal error: 'doctest/doctest.h' file not found
```
**Fix**: Ensure doctest is downloaded:
```bash
cmake -B build  # This downloads doctest
ls build/_deps/doctest-src/doctest/doctest.h
```

## Quick Fixes Checklist

1. **Module Loading**: Check `LPZRobotsCore.cmake` includes testing module
2. **Doctest Setup**: Use single-header approach only
3. **Build Order**: Build targets before running tests
4. **Warmup Controllers**: Add 15-step warmup for homeokinetic controllers
5. **Path Issues**: Use CMake variables, not hardcoded paths
6. **Script Location**: Scripts are in `scripts/` directory

## Validation Commands

```bash
# Check setup
./setup_tests.sh

# Build tests
cmake -B build -DBUILD_TESTS=ON
cmake --build build

# Run tests
ctest --output-on-failure
make test_unit
./tests/unit_tests --list-test-cases
```

## Success Indicators

✅ All CMake modules load without errors  
✅ Test executables build successfully  
✅ CTest detects all test executables  
✅ Tests run (even if some fail due to bugs)  
✅ Custom test reporter shows LPZRobots banner  

---

*Reference updated: January 5, 2025*