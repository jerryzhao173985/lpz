# VS Code Java Components Setup Guide

This guide explains how to build LPZRobots with Java components in VS Code.

## Prerequisites

1. **Install Java**:
   ```bash
   # macOS
   brew install openjdk
   
   # Ubuntu/Debian
   sudo apt-get install default-jdk
   
   # Arch Linux
   sudo pacman -S jdk-openjdk
   ```

2. **Set JAVA_HOME**:
   ```bash
   # macOS (add to ~/.zshrc)
   export JAVA_HOME="/opt/homebrew/opt/openjdk"
   export PATH="$JAVA_HOME/bin:$PATH"
   
   # Linux (add to ~/.bashrc)
   export JAVA_HOME="/usr/lib/jvm/default"
   export PATH="$JAVA_HOME/bin:$PATH"
   ```

3. **Verify Installation**:
   ```bash
   java -version
   javac -version
   echo $JAVA_HOME
   ```

## VS Code Configuration

### Method 1: Using CMake Presets (Recommended)

1. The project includes CMake presets that handle Java configuration:
   - `default` preset: Java enabled by default
   - `default-java` preset: Explicitly enables Java
   - `default-no-java` preset: Explicitly disables Java

2. VS Code is configured to use the `default` preset automatically.

3. To switch presets:
   - Open Command Palette (Cmd+Shift+P)
   - Run "CMake: Select Configure Preset"
   - Choose your desired preset

### Method 2: Manual Configuration

1. The `.vscode/settings.json` file has been updated with:
   ```json
   "BUILD_JAVA_TOOLS": "ON",
   "java.home": "${env:JAVA_HOME}",
   ```

2. The workspace settings enable Java components by default.

## Building with Java Components

### In VS Code:

1. **Clean Configuration** (if needed):
   - Open Command Palette (Cmd+Shift+P)
   - Run "Tasks: Run Task"
   - Select "Delete Cache and Reconfigure"

2. **Configure Project**:
   - CMake should configure automatically on open
   - Or run "CMake: Configure" from Command Palette

3. **Build All**:
   - Press Cmd+Shift+B (default build task)
   - Or run "CMake: Build" from Command Palette

4. **Build Java Components Only**:
   - Run "Tasks: Run Task"
   - Select "Build Java Components"

### From Terminal:

```bash
# Using CMake directly
cmake --preset default
cmake --build build

# Or build specific Java targets
cmake --build build --target soundman javacontroller
```

## Java Components

### soundman
- **Purpose**: Sound manipulation utility for robot simulations
- **Location**: `soundman/`
- **Output**: `build/soundman/bin/soundMan` (executable)
- **Classes**: `build/soundman/class/*.class`

### javacontroller
- **Purpose**: Java-based robot controller interface
- **Location**: `javacontroller/`
- **Output**: `build/javacontroller/javacontroller.jar`
- **Usage**: Allows writing robot controllers in Java

## Troubleshooting

### "Unknown CMake command 'add_jar'" Error
- **Cause**: CMake's UseJava module not included
- **Fix**: Already fixed in CMakeLists.txt with `include(UseJava)`

### JAVA_HOME Not Found
- **Symptoms**: CMake can't find Java even though it's installed
- **Fix**: 
  1. Set JAVA_HOME in your shell profile
  2. Restart VS Code from a terminal with JAVA_HOME set:
     ```bash
     export JAVA_HOME="/opt/homebrew/opt/openjdk"
     code .
     ```

### Java Components Not Building
- **Check**: Run `./enable_java.sh` to verify Java setup
- **Verify**: BUILD_JAVA_TOOLS is ON in CMake output
- **Clean**: Delete build directory and reconfigure

### VS Code Using Wrong Settings
- **Issue**: VS Code might cache old settings
- **Fix**: 
  1. Delete `build/` directory
  2. Restart VS Code
  3. Let CMake reconfigure with new settings

## Switching Between Java/No-Java Builds

### Using Presets:
```bash
# Build with Java
cmake --preset default-java
cmake --build build

# Build without Java
cmake --preset default-no-java
cmake --build build
```

### Using VS Code:
1. Command Palette → "CMake: Select Configure Preset"
2. Choose between presets with/without Java
3. Build normally

## Integration with ga_tools

The `ga_tools` component is a C++ library and doesn't contain Java code. It provides genetic algorithm functionality that can be used by both C++ and Java controllers through the appropriate interfaces.

## VS Code Extensions

Recommended extensions for Java development:
- **Extension Pack for Java** (vscjava.vscode-java-pack)
- **Language Support for Java** (redhat.java)
- **Debugger for Java** (vscjava.vscode-java-debug)

These are optional and only needed if you plan to develop Java controllers.

## Summary

The VS Code configuration has been updated to:
1. ✅ Enable Java components by default (BUILD_JAVA_TOOLS=ON)
2. ✅ Configure Java home path and environment
3. ✅ Add build tasks for Java components
4. ✅ Fix the CMake UseJava module inclusion
5. ✅ Provide easy switching between Java/no-Java builds
6. ✅ Ensure all paths and libraries work correctly

The workspace is now fully configured for building both C++ and Java components seamlessly in VS Code.