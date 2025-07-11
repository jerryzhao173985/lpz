# macOS Build Fixes for LPZRobots

This document describes the comprehensive fixes applied to resolve macOS build issues, particularly for macOS 15 (Sequoia), Xcode 15+, Qt6, and OpenSceneGraph compatibility.

## Issues Addressed

### 1. macOS 15 + Xcode 15+ Compatibility
**Problem**: Newer Xcode versions and macOS SDK cause compilation failures
**Solutions**:
- Proper SDK detection and configuration
- Compatibility flags for newer clang versions (`-Wno-enum-constexpr-conversion`)
- Correct deployment target settings (`CMAKE_OSX_DEPLOYMENT_TARGET`)

### 2. Qt6 macOS Integration Issues
**Problem**: Qt6 frameworks causing linking errors and AGL framework deprecation warnings
**Solutions**:
- Automatic Qt6 path detection and configuration
- AGL framework filtering (existing QtMacOSConfig.cmake)
- OpenGL deprecation warning suppression (`-DGL_SILENCE_DEPRECATION`)
- Proper Qt6 library path configuration

### 3. OpenSceneGraph Homebrew Compatibility
**Problem**: OSG from Homebrew has threading and framework conflicts on macOS
**Solutions**:
- Detection of Homebrew OSG installation
- Single-threaded OSG viewer mode for macOS stability
- OpenGL framework explicit linking
- OSG-specific deprecation warning suppression

### 4. Homebrew vs System Library Conflicts
**Problem**: Mixed Homebrew and system libraries causing linking failures
**Solutions**:
- Proper Homebrew prefix detection (ARM64: `/opt/homebrew`, Intel: `/usr/local`)
- Correct library path prioritization
- PKG_CONFIG_PATH configuration for dependency discovery

### 5. C++ Standard Library Issues
**Problem**: Mixed libc++/libstdc++ usage and Boost compatibility
**Solutions**:
- Force libc++ usage across all compilation and linking
- Boost compatibility flags for newer clang
- C++17 deprecation warning suppression

### 6. Xcode 15+ Linking Issues  
**Problem**: Newer Xcode linker being stricter about duplicate libraries and weak symbols
**Solutions**:
- Duplicate library warning suppression (`-Wl,-no_warn_duplicate_libraries`)
- Weak import handling (`-Wl,-no_weak_imports`) 
- Dynamic symbol lookup for compatibility (`-Wl,-undefined,dynamic_lookup`)

## Files Modified

### New Files
- `cmake/LPZRobotsMacOSFixes.cmake` - Comprehensive macOS build fixes
- `MACOS_BUILD_FIXES.md` - This documentation

### Modified Files
- `CMakeLists.txt` - Include macOS fixes module
- `.github/workflows/simple-ci.yml` - Updated macOS CI workflow
- `cmake/LPZRobotsDependencies.cmake` - Fixed ODE compatibility setup

## Key Features

### Automatic Platform Detection
The fixes automatically detect:
- macOS version (`sw_vers -productVersion`)
- Xcode version (`xcodebuild -version`)
- Architecture (ARM64 vs Intel)
- Homebrew installation paths
- Qt6 installation paths

### Preset-Based Configuration
Updated CI workflow to use macOS-specific presets:
- `macos-arm64` for Apple Silicon runners
- `macos-x64` for Intel runners  
- Proper CMAKE_PREFIX_PATH configuration

### Improved Dependency Installation
Enhanced Homebrew dependency installation with:
- Update before installation
- Retry mechanism for transient network issues
- Proper environment variable setup
- Debug information output

### Target-Specific Configuration
Function `lpzrobots_configure_target_macos()` for applying macOS-specific settings to individual targets:
- RPATH configuration
- Framework linking
- Compile definitions

## Testing

The fixes are designed to be:
- **Non-intrusive**: Only apply on macOS systems
- **Backward compatible**: Work with older macOS/Xcode versions
- **Comprehensive**: Address multiple related issues systematically
- **Debuggable**: Extensive status messaging

## Usage

### For Developers
The fixes are applied automatically when building on macOS. No additional configuration required.

### For CI/CD
The updated workflow:
1. Uses platform-specific CMake presets
2. Installs dependencies with retry logic
3. Sets up proper environment variables
4. Uses correct build directories for each platform

### Manual Build
For manual builds, use the macOS presets:
```bash
# Apple Silicon
cmake --preset=macos-arm64 -DCMAKE_BUILD_TYPE=Release

# Intel
cmake --preset=macos-x64 -DCMAKE_BUILD_TYPE=Release
```

## Future Considerations

### Qt Version Migration
- These fixes support Qt6 primarily
- Qt5 support maintained through existing AGL filtering
- Future Qt versions may require additional compatibility updates

### Dependency Updates  
- OpenSceneGraph: Monitor Homebrew formula updates
- Boost: Future versions may resolve compatibility issues
- ODE: System vs bundled ODE selection strategies

### Apple Platform Evolution
- New Xcode versions may require additional linker flags
- macOS SDK changes may need new compatibility measures
- Apple Silicon optimizations may be enhanced

## Known Limitations

1. **OpenSceneGraph Threading**: Single-threaded mode may impact performance in multi-threaded scenarios
2. **Dynamic Linking**: Some undefined symbol handling may mask real linking issues
3. **Homebrew Dependency**: Strong reliance on Homebrew package manager

## Troubleshooting

### Common Issues
1. **"Qt not found"**: Ensure Qt6 is installed via Homebrew and paths are set
2. **"OpenSceneGraph errors"**: Check Homebrew OSG installation and OpenGL framework
3. **"Linking errors"**: Verify CMAKE_PREFIX_PATH includes Homebrew prefix
4. **"SDK errors"**: Ensure Xcode Command Line Tools are updated

### Debug Information  
The build system outputs extensive debug information including:
- Detected macOS and Xcode versions
- Library paths and prefixes  
- Qt6 installation details
- Applied compiler and linker flags

### Getting Help
Include the following in bug reports:
- macOS version (`sw_vers -productVersion`)
- Xcode version (`xcodebuild -version`)
- Architecture (`uname -m`)  
- Homebrew prefix (`brew --prefix`)
- Qt6 path (`brew --prefix qt@6`)
- Full build log with CMake configure output