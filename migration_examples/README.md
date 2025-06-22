# lpzrobots Migration Examples

This directory contains production-ready code examples for migrating lpzrobots to modern C++ and build systems.

## Contents

### 1. `1_selforg_CMakeLists.txt`
A complete CMake configuration for the selforg library featuring:
- Multi-configuration support (Debug, Release, Optimized)
- Platform detection with ARM64 support
- Modern CMake best practices
- Package configuration for downstream projects
- Parallel build support

### 2. `2_guilogger_qt5_migration.cpp`
Demonstrates the exact changes needed to remove Qt3Support:
- `Q3ScrollView` → `QScrollArea/QGraphicsView`
- `Q3Canvas` → `QGraphicsScene + QGraphicsView`
- `Q3ListView` → `QTreeWidget`
- `Q3ValueList` → `QList`
- `Q3Dict` → `QHash/std::unordered_map`
- Modern signal/slot syntax with lambdas

### 3. `3_controller_cpp17_modernization.cpp`
Shows C++17 modernization for controller classes:
- `typedef` → `using` declarations
- Raw pointers → `std::vector` and smart pointers
- Manual memory management → RAII
- Old-style loops → STL algorithms
- `[[nodiscard]]` attributes for getters
- `std::optional` for optional parameters
- Structured bindings
- `if constexpr` for compile-time branching
- `std::variant` for flexible parameter types

### 4. `4_platform_detection.cmake`
A robust CMake module for platform detection:
- Detects macOS ARM64 (Apple Silicon) vs Intel
- Handles universal binaries on macOS
- Linux architecture detection (x86_64, ARM64, ARM)
- Windows 32/64-bit detection
- Sets appropriate compiler flags per platform
- Rosetta 2 detection on macOS

### 5. `5_complete_example_usage.cpp`
Integrates all components in a working example:
- Uses the modernized controller interface
- Demonstrates platform-specific compilation
- Shows modern timing with `std::chrono`
- Structured bindings for return values
- Exception handling with RAII

## Usage

To use these examples in your migration:

1. **For CMake migration:**
   ```bash
   cp migration_examples/1_selforg_CMakeLists.txt selforg/CMakeLists.txt
   # Adjust paths and add your specific requirements
   ```

2. **For Qt migration:**
   - Use `2_guilogger_qt5_migration.cpp` as a reference
   - Apply similar transformations to your Qt3Support code

3. **For C++17 modernization:**
   - Follow patterns in `3_controller_cpp17_modernization.cpp`
   - Use tools like `clang-tidy` with modernize checks

4. **For platform detection:**
   ```cmake
   include(migration_examples/4_platform_detection.cmake)
   print_platform_info()
   add_library(mylib ...)
   set_platform_flags(mylib)
   ```

5. **Build the complete example:**
   ```bash
   mkdir build && cd build
   cmake .. -DCMAKE_BUILD_TYPE=Release
   cmake --build . --parallel
   ./simulation_example
   ```

## Key Migration Points

1. **Build System**: Makefile → CMake provides better cross-platform support
2. **Qt Version**: Qt3 → Qt5/Qt6 removes deprecated APIs
3. **C++ Standard**: C++98 → C++17 enables modern features
4. **Memory Management**: Manual → RAII and smart pointers
5. **Platform Support**: Enhanced detection for modern architectures

## Testing

Each example includes compilation instructions. Test on:
- macOS ARM64 (M1/M2/M3)
- macOS x86_64 (Intel)
- Linux x86_64
- Linux ARM64
- Windows (if applicable)

## Notes

- These examples are designed to be drop-in replacements
- They maintain backward compatibility where possible
- Performance improvements are included where applicable
- All code follows modern C++ best practices