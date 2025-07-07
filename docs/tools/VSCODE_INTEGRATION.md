# VS Code Integration for LPZRobots

## Overview

Complete VS Code integration has been added to the LPZRobots project, providing:
- Full CMake support with IntelliSense
- Debug configurations for simulations and tools
- Build tasks for all components
- Recommended extensions
- Platform-specific settings (macOS ARM64 & Linux)

## Quick Start

1. **Open VS Code**:
   ```bash
   code /Users/jerry/lpzrobot_mac
   ```

2. **Install Recommended Extensions**:
   - When prompted, install all recommended extensions
   - Or manually: Cmd+Shift+P → "Extensions: Show Recommended Extensions"

3. **Configure CMake**:
   - VS Code will automatically detect CMakeLists.txt
   - Select a kit (e.g., "Clang 14.0.0 arm64-apple-darwin")
   - CMake will configure automatically

4. **Build**:
   - Press Cmd+Shift+B to build all (default task)
   - Or use CMake sidebar to build specific targets

## Key Features

### 1. IntelliSense Configuration
- Automatic header discovery
- Compile commands from CMake
- C++17 standard
- Platform-specific paths

### 2. Build Tasks (Cmd+Shift+B)
- **CMake: build** - Build all targets (default)
- **Build selforg** - Build core library only
- **Build ode_robots** - Build robot framework
- **Build GUI tools** - Build guilogger & matrixviz
- **Legacy: make all** - Use original Make system

### 3. Debug Configurations (F5)
- **Debug Current Target** - Debug selected CMake target
- **Debug Simulation** - Debug template_sphererobot
- **Debug with Guilogger** - Run simulation with GUI
- **Debug Guilogger/Matrixviz** - Debug GUI tools
- **Attach to Process** - Attach to running process

### 4. CMake Integration
Settings automatically configured:
- Build type: Debug (with symbols)
- Install type: DEVEL (in-source libraries)
- All components enabled
- Library variants built (_opt, _dbg)

### 5. Environment Setup
Automatically configured:
- Library paths (DYLD_LIBRARY_PATH/LD_LIBRARY_PATH)
- Homebrew paths for macOS
- Debug symbols and pretty printing

## Development Workflow

### Building a Simulation
1. Open simulation file (e.g., `ode_robots/simulations/template_sphererobot/main.cpp`)
2. Press Cmd+Shift+B → Select "Build template_sphererobot"
3. Press F5 to debug

### Running with GUI Tools
1. Build GUI tools: Cmd+Shift+B → "Build GUI tools"
2. Debug: F5 → "Debug with Guilogger"
3. The simulation will start with guilogger attached

### Creating New Simulations
1. Use the template: `ode_robots/simulations/createNewSimulation.sh`
2. VS Code will automatically detect new CMake targets
3. Refresh CMake: Cmd+Shift+P → "CMake: Refresh"

### Static Analysis
- Run cppcheck: Cmd+Shift+B → "Run cppcheck"
- Clang-tidy integration via extension
- Real-time error squiggles

### Code Formatting
- Auto-format on save enabled
- Manual: Cmd+Shift+B → "Format code"
- Uses project .clang-format settings

## Troubleshooting

### IntelliSense Not Working
1. Ensure CMake configured successfully
2. Check compile_commands.json exists in build/
3. Reload window: Cmd+Shift+P → "Developer: Reload Window"

### Build Errors
1. Check CMake output panel
2. Verify dependencies installed (Qt6, OpenSceneGraph, etc.)
3. Try clean rebuild: "CMake: clean" then "CMake: build"

### Debug Issues
1. Ensure debug build: CMAKE_BUILD_TYPE=Debug
2. Check library paths in launch.json
3. On macOS: May need to allow lldb in Security settings

## Customization

### Adding Build Configurations
Edit `.vscode/settings.json`:
```json
"cmake.configureSettings": {
    "MY_OPTION": "value"
}
```

### Adding Debug Configurations
Edit `.vscode/launch.json`:
```json
{
    "name": "Debug My Simulation",
    "type": "cppdbg",
    "program": "${workspaceFolder}/build/path/to/executable",
    ...
}
```

### Custom Tasks
Edit `.vscode/tasks.json`:
```json
{
    "label": "My Task",
    "type": "shell",
    "command": "my-command",
    ...
}
```

## Tips

1. **CMake Sidebar**: Use for fine-grained target control
2. **Output Panel**: Check "CMake/Build" for detailed logs
3. **Problems Panel**: Shows all compilation errors/warnings
4. **Terminal**: Integrated terminal has environment pre-configured
5. **Git Integration**: GitLens shows inline blame and history

## Summary

The VS Code integration provides a modern, efficient development environment for LPZRobots while maintaining compatibility with the traditional build system. All features of the CMake build system are accessible through the VS Code UI, making development faster and more enjoyable.