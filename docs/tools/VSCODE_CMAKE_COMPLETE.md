# Complete VS Code CMake Integration for LPZRobots

## ✅ Integration Status

The LPZRobots project now has **complete VS Code integration** with the CMake Tools extension, providing:

1. **Full IntelliSense Support** - All headers discovered automatically
2. **One-Click Build** - Press Cmd+Shift+B to build
3. **Integrated Debugging** - F5 to debug any target
4. **Code Analysis** - Clang-tidy integration with modern C++ checks
5. **Target Discovery** - All CMake targets visible in status bar

## 🎯 Key Features Implemented

### 1. **CMake Configuration**
- Automatic configuration on project open
- DEVEL mode by default (libraries in source tree)
- All components enabled
- compile_commands.json generated for IntelliSense

### 2. **Build System Integration**
```json
// Key settings in .vscode/settings.json
"cmake.configureSettings": {
    "CMAKE_BUILD_TYPE": "Debug",
    "CMAKE_EXPORT_COMPILE_COMMANDS": "ON",
    "LPZROBOTS_INSTALL_TYPE": "DEVEL",
    "BUILD_EXAMPLES": "ON",
    "BUILD_GUI_TOOLS": "ON"
}
```

### 3. **Target Management**
- All targets discoverable in CMake Tools sidebar
- Status bar shows active target
- Quick target switching with click

### 4. **IntelliSense Features**
- C++17 modern features highlighted
- Header files auto-discovered from:
  - selforg/include
  - ode_robots/include
  - ga_tools/include
  - System libraries (Qt6, OpenSceneGraph, GSL)
- Real-time error checking
- Code completion with documentation

### 5. **Build Variants**
Three library variants built automatically:
- Standard (libselforg.a)
- Optimized (libselforg_opt.a)
- Debug (libselforg_dbg.a)

## 📁 File Structure

```
lpzrobots/
├── .vscode/
│   ├── settings.json         # Main VS Code settings
│   ├── launch.json          # Debug configurations
│   ├── tasks.json           # Build tasks
│   ├── c_cpp_properties.json # IntelliSense paths
│   ├── extensions.json      # Recommended extensions
│   └── cmake-kits.json      # Compiler configurations
├── lpzrobots.code-workspace # Workspace file with all settings
├── CMakeLists.txt           # Root CMake configuration
├── selforg/CMakeLists.txt   # Core library
├── ode_robots/CMakeLists.txt # Robot framework
├── ga_tools/CMakeLists.txt  # Genetic algorithms
└── build/                   # CMake build directory
    └── compile_commands.json # IntelliSense database
```

## 🚀 Quick Start Guide

### 1. Open Workspace
```bash
# Open with workspace file (recommended)
code lpzrobots.code-workspace

# Or open folder
code /Users/jerry/lpzrobot_mac
```

### 2. Install Extensions
When prompted, install all recommended extensions:
- C/C++ Extension Pack
- CMake Tools
- Clang-Tidy
- LLDB Debugger

### 3. Configure CMake
- VS Code will auto-detect CMakeLists.txt
- Select kit: "Clang 14.0.0 arm64-apple-darwin"
- Configuration happens automatically

### 4. Build Project
- **Quick Build**: Press `Cmd+Shift+B`
- **Select Target**: Click target name in status bar
- **Build Specific**: CMake sidebar → right-click target → Build

### 5. Debug
- Select target in status bar
- Press `F5` to debug
- Breakpoints work immediately

## 🔧 Common Tasks

### Building Individual Components
```bash
# Via Command Palette (Cmd+Shift+P)
> CMake: Build Target
> Select: selforg

# Via Terminal
cmake --build build --target selforg

# Via Tasks (Cmd+Shift+B)
> Build selforg
```

### Running Simulations
1. Build simulation:
   - Status bar → Select `template_sphererobot_sim`
   - Press `Cmd+Shift+B`

2. Debug simulation:
   - Press `F5`
   - Simulation starts with debugger attached

3. Run with guilogger:
   - Launch configuration: "Debug with Guilogger"
   - Both simulation and GUI start

### Code Analysis
- **Real-time**: Errors appear as you type
- **Full analysis**: Cmd+Shift+B → "Run cppcheck"
- **Clang-tidy**: Automatic with saves

### Adding New Files
1. Create .cpp/.h files
2. CMake detects via GLOB patterns
3. Reconfigure: Cmd+Shift+P → "CMake: Configure"

## 🎨 IntelliSense Features

### Modern C++ Support
- C++17 features highlighted
- Auto completion for:
  - STL containers and algorithms
  - LPZRobots classes (Matrix, Controller, Robot)
  - Qt6 widgets
  - OpenSceneGraph nodes

### Code Navigation
- **Go to Definition**: F12
- **Find References**: Shift+F12
- **Peek Definition**: Alt+F12
- **Symbol Search**: Cmd+T

### Refactoring
- **Rename Symbol**: F2
- **Extract Function**: Available via lightbulb
- **Add Include**: Auto-suggested

## 🐛 Debugging Features

### Breakpoints
- Click line number to set
- Conditional breakpoints supported
- Logpoints for non-intrusive debugging

### Debug Console
- Evaluate expressions
- Call functions
- Inspect matrices/vectors

### Visualizers
- STL containers displayed nicely
- Matrix data viewable
- Qt objects inspectable

## 📊 CMake Integration Details

### Target Discovery
All targets automatically discovered:
- Libraries: selforg, ode_robots, ga_tools
- GUI Tools: guilogger, matrixviz, configurator
- Simulations: template_sphererobot_sim, etc.
- Examples: When BUILD_EXAMPLES=ON

### Configuration Cache
- Stored in build/CMakeCache.txt
- Persistent across VS Code restarts
- Editable via CMake Tools UI

### Presets Support
- CMakePresets.json fully supported
- Quick switching between Debug/Release
- Platform-specific settings

## 🔍 Troubleshooting

### IntelliSense Not Working
1. Check compile_commands.json exists
2. Reload window: Cmd+Shift+P → "Developer: Reload Window"
3. Reset IntelliSense: Cmd+Shift+P → "C/C++: Reset IntelliSense Database"

### CMake Configuration Failed
1. Check CMake output channel
2. Verify dependencies installed
3. Delete build folder and reconfigure

### Debugging Issues
1. Ensure Debug build type
2. Check DYLD_LIBRARY_PATH in launch.json
3. Allow lldb in macOS security settings

## ✨ Advanced Features

### Custom Build Tasks
Edit tasks.json to add custom builds:
```json
{
    "label": "Build All Optimized",
    "type": "shell",
    "command": "cmake",
    "args": ["--build", "build", "--config", "Release"],
    "group": "build"
}
```

### Workspace Settings
The lpzrobots.code-workspace file includes:
- All VS Code settings
- Launch configurations  
- Build tasks
- Extension recommendations

### Multi-Configuration
Support for multiple build types:
- Debug (default)
- Release
- RelWithDebInfo
- MinSizeRel

## 📈 Performance Tips

1. **Faster Builds**: Use Ninja generator
   ```json
   "cmake.generator": "Ninja"
   ```

2. **Parallel Builds**: Automatic with CMake Tools

3. **Incremental Builds**: Only changed files rebuilt

4. **Unity Builds**: Can be enabled in CMakeLists.txt

## 🎉 Summary

The VS Code CMake integration is now **fully operational** with:

✅ All CMake targets discovered and buildable  
✅ Complete IntelliSense with modern C++ support  
✅ Integrated debugging with library path handling  
✅ Code analysis and formatting tools  
✅ Seamless workflow from edit to debug  

The integration preserves all original Makefile functionality while adding modern IDE features. Both build systems coexist perfectly, allowing gradual migration while maintaining full compatibility.