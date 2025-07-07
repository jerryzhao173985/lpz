# CMake Error Patterns and Solutions Reference

## Quick Lookup Guide for Common CMake Errors

### Pattern 1: Framework Not Found on macOS
```
Error: ld: framework 'XXX' not found
```
**Solutions**:
1. Check if framework is deprecated (like AGL)
2. Create filter script to remove during linking
3. Use RULE_LAUNCH_LINK property
4. Add post-build verification

### Pattern 2: Split Framework Arguments
```
Error: clang++: error: no such file or directory: 'GLUT'
```
**Solution**: Never use `separate_arguments()` on framework flags. Create custom parser:
```cmake
function(parse_libs_with_frameworks input_string output_var)
    # Custom parsing that keeps -framework XXX together
endfunction()
```

### Pattern 3: Qt MOC Processing Issues
```
Error: undefined reference to `ClassName::staticMetaObject'
Error: vtable for ClassName
```
**Solutions**:
1. Include headers in executable target
2. For Qt6: Use qt6_add_executable with headers
3. For Qt5: Use qt5_wrap_cpp explicitly
4. Clean MOC cache: `rm -rf *_autogen CMakeFiles`

### Pattern 4: Missing Qt Modules
```
Error: 'QDomNode' file not found
Error: 'qdom.h' file not found
```
**Solutions**:
1. Add module to find_package: `find_package(Qt6 REQUIRED COMPONENTS Core Widgets Xml)`
2. Update old-style includes to new style
3. Link the module: `target_link_libraries(target Qt6::Xml)`

### Pattern 5: Undefined Symbols at Link Time
```
Error: Undefined symbols for architecture arm64:
```
**Common Causes and Solutions**:
1. **Missing library**: Add to target_link_libraries
2. **Wrong link order**: Dependencies must come after dependents
3. **Static/shared mismatch**: Use consistent BUILD_SHARED_LIBS
4. **Missing compile definition**: Add -DNOCONFIGURATOR or similar

### Pattern 6: Duplicate Library Warnings
```
Warning: ld: warning: ignoring duplicate libraries
```
**Solutions**:
1. Use target-based dependencies instead of manual linking
2. Check for circular dependencies
3. Use PRIVATE/PUBLIC/INTERFACE correctly

### Pattern 7: Header File Not Found
```
Error: 'header.h' file not found
```
**Solutions**:
1. Add to target_include_directories
2. Use find_path for system headers
3. Check relative vs absolute paths
4. Verify header actually exists

### Pattern 8: Override Keyword Errors
```
Error: 'function' marked 'override' but does not override
Warning: 'function' overrides but not marked 'override'
```
**Solutions**:
1. Check inheritance hierarchy
2. Remove override if not actually overriding
3. Add override if missing (warning only)

### Pattern 9: C++17 Compatibility
```
Error: 'explicit' can only be specified inside class definition
Error: no member named 'cerr' in namespace 'std'
```
**Solutions**:
1. Remove misplaced keywords
2. Add missing includes (#include <iostream>)
3. Update deprecated syntax

### Pattern 10: CMake Cache Issues
```
Symptom: Changes not taking effect, old errors persisting
```
**Solutions**:
1. Use `cmake --fresh` (CMake 3.24+)
2. Delete CMakeCache.txt
3. Remove entire build directory
4. Clean autogen directories

## CMake Best Practices Learned

### 1. Always Use Target-Based Commands
```cmake
# Good
target_link_libraries(myapp PRIVATE otherlib)

# Bad
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -lотherlib")
```

### 2. Handle macOS Frameworks Properly
```cmake
# Good
target_link_libraries(myapp PRIVATE "-framework OpenGL")

# Bad
target_link_libraries(myapp PRIVATE -framework OpenGL)  # Can get split
```

### 3. Use Generator Expressions
```cmake
# Good
target_compile_definitions(mylib PRIVATE
    $<$<CONFIG:Debug>:DEBUG_MODE>
)
```

### 4. Proper Qt Integration
```cmake
# Complete Qt setup
find_package(Qt6 REQUIRED COMPONENTS Core Widgets)
qt6_standard_project_setup()
qt6_add_executable(myapp ${SOURCES} ${HEADERS})  # Headers for MOC
```

### 5. Multi-Layer Platform Fixes
```cmake
# Don't rely on single-point fixes
if(APPLE)
    # Layer 1: Compile definitions
    # Layer 2: Link flags
    # Layer 3: Post-build verification
endif()
```

## Debugging CMake Builds

### Useful Commands
```bash
# Verbose output
cmake --build . --verbose

# Single target with full output
cmake --build . --target mytarget -- VERBOSE=1

# Check what CMake sees
cmake -LA . | grep VARNAME

# Force reconfigure
cmake --fresh .  # CMake 3.24+
rm -rf CMakeCache.txt CMakeFiles
```

### Finding Issues
1. Check the first error, not the last
2. Look for "error:" not "warning:"
3. Check exit codes: 0 = success
4. Use `make -k` to see all errors at once

## Platform-Specific Notes

### macOS ARM64
- Homebrew installs to /opt/homebrew
- Frameworks use -framework XXX syntax
- .app bundles need Info.plist
- Use otool -L to check dependencies

### Linux
- Use pkg-config when available
- RPATH handling differs from macOS
- No framework concept

### Cross-Platform
- Always test on all target platforms
- Use CMAKE_SYSTEM_NAME for conditionals
- Avoid hardcoded paths

---
*Remember: Most CMake errors fall into these patterns. Check this reference first!*