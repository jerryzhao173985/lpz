# Developer CI/CD Quick Reference

## Overview
This guide helps developers work with the LPZRobots CI/CD system effectively.

## CI Status Badges

Check the current build status on the [main README](../../README.md):
- 🟢 **Build Status**: Core builds on Ubuntu and macOS
- 🟢 **Code Quality**: Formatting and static analysis
- 🟢 **Performance**: Benchmark tracking

## Before Pushing Code

### 1. Check Code Formatting
```bash
# Check if your code matches the style
clang-format --dry-run --Werror src/myfile.cpp

# Auto-format your changes
clang-format -i src/myfile.cpp

# Format all modified files
git diff --name-only | grep -E '\.(cpp|h)$' | xargs clang-format -i
```

### 2. Run Static Analysis
```bash
# Quick check with clang-tidy
clang-tidy src/myfile.cpp -- -std=c++17 -I../../selforg

# With CMake build
cmake -DLPZROBOTS_ENABLE_CLANG_TIDY=ON -B build
cmake --build build
```

### 3. Test Locally
```bash
# Basic build test
cmake --preset=default
cmake --build build/default

# With coverage
cmake -DLPZROBOTS_ENABLE_COVERAGE=ON -B build-coverage
cmake --build build-coverage
ctest --test-dir build-coverage

# With sanitizers
cmake -DLPZROBOTS_ENABLE_SANITIZERS=ON \
      -DLPZROBOTS_SANITIZER_ADDRESS=ON \
      -B build-asan
cmake --build build-asan
```

## Understanding CI Failures

### Build Failures

#### "selforg/matrix.h: No such file or directory"
- **Cause**: Missing include paths
- **Fix**: Ensure proper CMake configuration or add `-I` flags

#### "undefined reference to..."
- **Cause**: Missing library links
- **Fix**: Check CMakeLists.txt target_link_libraries

### Code Quality Failures

#### "Code formatting check failed"
- **Cause**: Code doesn't match .clang-format style
- **Fix**: Run `clang-format -i` on the files
- **Note**: This is currently expected to fail until we format the entire codebase

#### "Static analysis found issues"
- **Cause**: clang-tidy detected problems
- **Fix**: Address the warnings or add NOLINT comments for false positives

### Test Failures

#### "Test timeout exceeded"
- **Cause**: Simulation running too long
- **Fix**: Add appropriate exit conditions or reduce test duration

#### "Sanitizer: heap-use-after-free"
- **Cause**: Memory safety issue detected
- **Fix**: Fix the memory management bug

## Running CI Locally

### Simulate Full CI Build
```bash
# Ubuntu-like environment
docker run -it ubuntu:24.04 bash
# Then run the commands from simple-ci.yml

# macOS (requires macOS machine)
# Follow the brew install commands from simple-ci.yml
```

### Quick CI Checks
```bash
# Format check only
find selforg ode_robots -name "*.cpp" -o -name "*.h" | \
  xargs clang-format --dry-run --Werror

# Basic build test
cmake --preset=ci && cmake --build build/ci --target selforg

# Run core tests
cd build/ci && ctest --output-on-failure
```

## CI Configuration

### Available CMake Options
```cmake
# Quality Tools
-DLPZROBOTS_ENABLE_CLANG_TIDY=ON    # Enable clang-tidy
-DLPZROBOTS_ENABLE_CPPCHECK=ON      # Enable cppcheck
-DLPZROBOTS_ENABLE_IWYU=ON          # Include-what-you-use

# Testing Options  
-DLPZROBOTS_ENABLE_COVERAGE=ON      # Code coverage
-DLPZROBOTS_ENABLE_SANITIZERS=ON    # Enable sanitizers
-DLPZROBOTS_SANITIZER_ADDRESS=ON    # AddressSanitizer
-DLPZROBOTS_SANITIZER_THREAD=ON     # ThreadSanitizer
-DLPZROBOTS_SANITIZER_UNDEFINED=ON  # UndefinedBehaviorSanitizer

# Performance
-DLPZROBOTS_ENABLE_BENCHMARKS=ON    # Build benchmarks
```

### Workflow Files
- **`.github/workflows/simple-ci.yml`**: Main build and test
- **`.github/workflows/code-quality.yml`**: Style and analysis
- **`.github/workflows/performance.yml`**: Benchmark tracking

## Common Tasks

### Skip CI for Documentation Changes
```bash
git commit -m "docs: Update README [skip ci]"
```

### Trigger Manual CI Run
1. Go to [Actions tab](https://github.com/georgmartius/lpzrobots/actions)
2. Select workflow
3. Click "Run workflow"

### Check CI Logs
1. Click on the failing check in PR
2. Expand the failed step
3. Look for error messages
4. Download artifacts if needed

## Best Practices

### 1. Small, Focused PRs
- Easier to review
- Faster CI runs
- Clearer history

### 2. Fix CI Issues Immediately
- Don't accumulate technical debt
- Keep main branch green
- Help other developers

### 3. Use CI Features
- Let CI catch issues early
- Trust the automation
- Learn from CI feedback

### 4. Update Tests
- Add tests for new features
- Fix broken tests immediately
- Maintain good coverage

## Getting Help

### CI Issues
- Check this guide first
- Look at recent successful builds
- Ask in PR comments

### Tool Issues
- **clang-format**: See `.clang-format` config
- **clang-tidy**: See `.clang-tidy` config
- **CMake**: Check `CMakeLists.txt`

### Performance Regressions
- Compare with baseline in PR comment
- Check for algorithmic changes
- Consider platform differences

## Quick Fixes

### Format All Changed Files
```bash
git diff --name-only HEAD^ | grep -E '\.(cpp|h)$' | xargs clang-format -i
```

### Disable Specific clang-tidy Check
```cpp
// NOLINTNEXTLINE(modernize-use-auto)
std::vector<int>::iterator it = vec.begin();
```

### Skip Sanitizer for Function
```cpp
__attribute__((no_sanitize("address", "undefined")))
void performance_critical_function() {
    // ...
}
```

### Suppress Coverage for Test Code
```cpp
// LCOV_EXCL_START
void test_helper_not_covered() {
    // ...
}
// LCOV_EXCL_STOP
```

## Summary

The CI system helps maintain code quality and catch issues early. Use it as a development aid, not an obstacle. When in doubt, check the workflow files for exact commands and reach out for help if needed.