# LPZRobots CMake Troubleshooting Summary

## Quick Reference: Problems and Solutions

### 1. Framework Handling Errors
**Error**: `clang++: error: no such file or directory: 'GLUT'`
**Fix**: Use parse_libs_with_frameworks function in SimulationBuild.cmake

### 2. AGL Framework Not Found
**Error**: `ld: framework 'AGL' not found`
**Fix**: 
- Create filter_agl_link.sh
- Use QtMacOSConfig.cmake
- Apply lpzrobots_configure_qt_macos() to Qt targets

### 3. Missing Qt Modules
**Error**: `'qdom.h' file not found` or `'QDomNode' file not found`
**Fix**: 
- Add Qt Xml to find_package
- Update includes from Qt4 style to Qt5/6 style

### 4. Undefined Symbols
**Error**: ConfiguratorProxy undefined symbols
**Fix**: Add NOCONFIGURATOR definition when configurator not available

### 5. Example Programs Failing
**Error**: Various C++17 compatibility errors in examples
**Fix**: Set BUILD_EXAMPLES=OFF

### 6. Shared Library Linking Errors
**Error**: Matrix symbols not found when linking shared libraries
**Fix**: Set BUILD_SHARED_LIBS=OFF for static builds

## Build Status Interpretation

✅ **SUCCESS Indicators**:
- "build finished with warning(s)"
- Exit code: 0
- [100%] Built target XXX

❌ **FAILURE Indicators**:
- "build finished with error(s)"
- Exit code: 1 or 2
- "make[2]: *** [target] Error 1"

## Essential Files Created

1. `/cmake/filter_agl_link.sh` - Removes AGL framework
2. `/cmake/QtMacOSConfig.cmake` - Qt macOS configuration
3. `/cmake/SimulationBuild.cmake` - Fixed framework parsing

## Key CMake Options

```cmake
-DBUILD_SHARED_LIBS=OFF    # Use static libraries
-DBUILD_EXAMPLES=OFF       # Skip problematic examples
-DCMAKE_BUILD_TYPE=Release # Or Debug
```

## Platform-Specific Notes

### macOS ARM64 (Apple Silicon)
- Qt from Homebrew includes deprecated AGL references
- OpenGL deprecation warnings are expected
- Use -noshadow flag when running simulations
- Framework paths must keep -framework XXX together

### VSCode Integration
- Use CMake Tools extension
- Select "Unix Makefiles" generator
- Build specific targets to avoid example errors

## Common Warning Types (Can Ignore)

1. **Qt API Changes**: enterEvent parameter type
2. **Missing override**: Older code style
3. **OpenGL deprecation**: macOS moving away from OpenGL
4. **Version mismatch**: Qt built for newer macOS

Remember: Warnings are NOT errors. The build succeeds with warnings!