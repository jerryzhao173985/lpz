# LPZRobots Developer Guide

This guide covers essential development tools and practices for the LPZRobots project.

## Development Environment Setup

### Essential Files Created

1. **`.editorconfig`** - Ensures consistent coding styles across all editors
2. **`.gitattributes`** - Handles line endings and file types correctly
3. **`.vscode/launch.json`** - VSCode debugging configurations
4. **`.vscode/tasks.json`** - Build and analysis tasks
5. **`.gdbinit`** - GDB debugger configuration
6. **`.lldbinit`** - LLDB debugger configuration (macOS)

## Debugging

### VSCode Debugging

Multiple launch configurations are available:

1. **Debug Simulation (lldb)** - Standard debugging
2. **Debug Simulation with Sanitizers** - Memory and undefined behavior detection
3. **Debug CMake Build** - For CMake-based builds
4. **Debug Unit Tests** - Test debugging
5. **Profile with Instruments** - macOS performance profiling
6. **Debug with Memory Leak Detection** - macOS memory debugging

### Command-Line Debugging

#### GDB (Linux)
```bash
# Start debugging
gdb ./start
(gdb) lpz_catch_errors  # Enable error catching
(gdb) lpz_break_sim     # Set simulation breakpoints
(gdb) run -noshadow -g

# Custom commands
(gdb) pmatrix A         # Print matrix contents
(gdb) psensors robot    # Print sensor values
(gdb) pmotors robot     # Print motor values
(gdb) pagent agent      # Print agent info
```

#### LLDB (macOS)
```bash
# Start debugging
lldb ./start
(lldb) lpz_catch_errors  # Enable error catching
(lldb) lpz_break_sim     # Set simulation breakpoints
(lldb) run -noshadow -g

# Custom commands
(lldb) pmatrix A         # Print matrix contents
(lldb) psensors robot    # Print sensor values
(lldb) pmotors robot     # Print motor values
(lldb) pagent agent      # Print agent info
```

## Code Quality Tools

### Formatting
```bash
# Format single file
clang-format -i file.cpp

# Format all project files
find . -name "*.cpp" -o -name "*.h" | xargs clang-format -i

# Check formatting
clang-format --dry-run --Werror file.cpp
```

### Static Analysis
```bash
# Run clang-tidy
clang-tidy -p build/ file.cpp

# Run with auto-fix
clang-tidy -fix -p build/ file.cpp

# Run on changed files
git diff --name-only HEAD | grep -E '\.(cpp|h)$' | xargs clang-tidy -p build/
```

## Memory Debugging

### AddressSanitizer (ASAN)
```bash
# Build with ASAN
make asan  # or cmake -DLPZROBOTS_ENABLE_ASAN=ON

# Run with leak detection
ASAN_OPTIONS=detect_leaks=1 ./start_asan -noshadow

# Full options
export ASAN_OPTIONS=detect_leaks=1:print_stats=1:check_initialization_order=1:strict_string_checks=1
./start_asan -noshadow
```

### ThreadSanitizer (TSAN)
```bash
# Build with TSAN
make tsan  # or cmake -DLPZROBOTS_ENABLE_TSAN=ON

# Run
./start_tsan -noshadow
```

### UndefinedBehaviorSanitizer (UBSAN)
```bash
# Build with UBSAN
make ubsan  # or cmake -DLPZROBOTS_ENABLE_UBSAN=ON

# Run with stack traces
UBSAN_OPTIONS=print_stacktrace=1 ./start_ubsan -noshadow
```

### macOS Memory Debugging
```bash
# Enable malloc debugging
export MallocStackLogging=1
export MallocScribble=1
export MallocGuardEdges=1
./start -noshadow

# Use leaks tool
leaks --atExit -- ./start -noshadow

# Use Instruments
instruments -t "Leaks" ./start -noshadow
```

## Performance Profiling

### Linux (perf)
```bash
# Record profile
perf record -g ./start_opt -noshadow

# Analyze
perf report

# Real-time analysis
perf top -p $(pgrep start_opt)
```

### macOS (Instruments)
```bash
# Time profiler
instruments -t "Time Profiler" ./start_opt -noshadow

# Allocations
instruments -t "Allocations" ./start_opt -noshadow

# System Trace
instruments -t "System Trace" ./start_opt -noshadow
```

### Valgrind (Linux)
```bash
# Memory profiling
valgrind --tool=memcheck --leak-check=full ./start -noshadow

# Cache profiling
valgrind --tool=cachegrind ./start_opt -noshadow

# Call graph profiling
valgrind --tool=callgrind ./start_opt -noshadow
kcachegrind callgrind.out.*
```

## Build Configurations

### Make System
```bash
# Debug build (default)
make clean && make

# Optimized build
make clean && make opt

# With sanitizers (if supported)
make asan
make tsan
make ubsan

# Parallel build
make -j$(nproc)  # Linux
make -j$(sysctl -n hw.ncpu)  # macOS
```

### CMake System
```bash
# Debug build
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build

# Release build
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build

# With sanitizers
cmake -B build -DLPZROBOTS_ENABLE_ASAN=ON
cmake --build build

# With coverage
cmake -B build -DLPZROBOTS_ENABLE_COVERAGE=ON
cmake --build build
cmake --build build --target coverage
```

## Testing

### Running Tests
```bash
# Make system
cd selforg/tests && make && ./test_matrix

# CMake system
cmake --build build --target test
# or
ctest --test-dir build -V

# With coverage
cmake --build build --target coverage
open build/coverage/index.html  # View results
```

### Writing Tests
```cpp
// Example test structure
#include <gtest/gtest.h>
#include "matrix.h"

TEST(MatrixTest, Multiplication) {
    Matrix A(2, 2);
    Matrix B(2, 2);
    // ... setup matrices ...
    Matrix C = A * B;
    EXPECT_EQ(C.getM(), 2);
    EXPECT_EQ(C.getN(), 2);
    // ... verify results ...
}
```

## Continuous Integration

The project uses GitHub Actions for CI/CD:

1. **Code Quality** - Runs on every push
2. **Performance Tracking** - Monitors performance metrics
3. **Claude Code Review** - AI-powered code review

### Local CI Testing
```bash
# Act (run GitHub Actions locally)
act push

# Pre-commit hooks
pre-commit install
pre-commit run --all-files
```

## Best Practices

1. **Before Committing**
   - Run clang-format on modified files
   - Run clang-tidy to check for issues
   - Build and test your changes
   - Run with sanitizers if applicable

2. **Debugging Tips**
   - Use the custom debugger commands for matrices
   - Enable error catching breakpoints
   - Use sanitizers during development
   - Check memory leaks before committing

3. **Performance**
   - Profile before optimizing
   - Use the optimized build for benchmarking
   - Monitor memory allocations
   - Check cache efficiency with cachegrind

4. **Code Review**
   - Follow the style guide (.clang-format)
   - Address clang-tidy warnings
   - Write meaningful commit messages
   - Update documentation as needed

## Troubleshooting

### Common Issues

1. **Debugger not loading .gdbinit/.lldbinit**
   ```bash
   # GDB
   echo "set auto-load safe-path /" >> ~/.gdbinit
   
   # LLDB
   echo "settings set target.load-cwd-lldbinit true" >> ~/.lldbinit
   ```

2. **Sanitizer conflicts**
   - Don't mix ASAN with TSAN
   - MSAN requires entire toolchain rebuild
   - Use one sanitizer at a time

3. **Performance profiling noise**
   - Use optimized builds for profiling
   - Disable CPU frequency scaling
   - Close unnecessary applications
   - Run multiple iterations

4. **VSCode IntelliSense issues**
   - Generate compile_commands.json
   - Restart C++ extension
   - Check include paths in c_cpp_properties.json