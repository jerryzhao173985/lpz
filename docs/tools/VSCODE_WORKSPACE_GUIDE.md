# VS Code Workspace Guide for LPZRobots

## 🚀 Quick Start

1. **Open the workspace file**:
   ```bash
   code lpzrobots.code-workspace
   ```

2. **Install recommended extensions** when prompted (or via Extensions sidebar)

3. **CMake will auto-configure** on opening - watch the output panel

4. **Start developing!**

## 📁 Workspace Structure

The workspace is organized into logical folders:

- **🤖 LPZRobots - Main**: Root directory with CMakeLists.txt
- **🧠 Core Library (selforg)**: Self-organizing controllers
- **🏃 Robot Framework (ode_robots)**: Physics simulation framework
- **🧬 Genetic Algorithms (ga_tools)**: Evolution and optimization
- **📊 GUI Logger**: Real-time data plotting
- **🔲 Matrix Visualizer**: Neural network weight visualization
- **🎮 Simulations**: Example robot simulations

## ⚡ Key Features

### 1. **Smart IntelliSense**
- All project headers auto-discovered
- System libraries included (Qt6, OpenSceneGraph, GSL)
- C++17 modern features supported
- Real-time error checking

### 2. **One-Click Actions**
- **Build**: `Cmd+Shift+B` (builds current target)
- **Debug**: `F5` (debugs current target)
- **Run Task**: `Cmd+Shift+P` → "Tasks: Run Task"
- **Switch Target**: Click target name in status bar

### 3. **Integrated Debugging**
- Breakpoints work immediately
- Library paths pre-configured
- Multiple debug configurations:
  - 🎯 Debug Current CMake Target
  - 🤖 Debug Sphererobot Simulation
  - 📊 Debug with Guilogger
  - 🚀 Simulation + Guilogger (compound)

### 4. **Build Tasks**
Quick access via `Cmd+Shift+P` → "Tasks: Run Task":
- **Component builds**: selforg, ode_robots, ga_tools
- **GUI tools**: guilogger, matrixviz
- **Simulations**: template_sphererobot
- **Code quality**: Static analysis, formatting
- **Testing**: Run unit tests

### 5. **Code Analysis**
- **Clang-tidy**: Runs automatically on save
- **Cppcheck**: Run via task menu
- **Format on save**: Using clang-format

## 🎯 Common Workflows

### Building a Component
1. Select component in CMake sidebar
2. Press `Cmd+Shift+B` or click build button
3. Watch output in terminal

### Debugging a Simulation
1. Open simulation file (e.g., `template_sphererobot/main.cpp`)
2. Set breakpoints by clicking line numbers
3. Press `F5` to start debugging
4. Use debug console for variable inspection

### Running with GUI Tools
1. Select "🚀 Simulation + Guilogger" debug config
2. Press `F5`
3. Both simulation and guilogger start together

### Adding New Files
1. Create `.cpp`/`.h` files in appropriate directory
2. CMake auto-detects via GLOB patterns
3. Reconfigure: `Cmd+Shift+P` → "CMake: Configure"

## 🔧 Customization

### Change Build Type
- Status bar → Click "Debug" → Select "Release"
- Or edit `cmake.configureSettings` in workspace

### Add Custom Task
Edit workspace file, add to `tasks` section:
```json
{
    "label": "My Custom Task",
    "type": "shell",
    "command": "echo",
    "args": ["Hello LPZRobots!"]
}
```

### Environment Variables
Pre-configured for both macOS and Linux:
- Library paths (DYLD_LIBRARY_PATH/LD_LIBRARY_PATH)
- Homebrew paths
- Project root (LPZROBOTS_ROOT)

## 📊 Status Bar Items

- **CMake Kit**: Compiler selection
- **Build Type**: Debug/Release
- **Target**: Current build target
- **Build**: Quick build button
- **Debug**: Quick debug button

## ⚙️ Settings Overview

### CMake Configuration
- **Build Type**: Debug (with symbols)
- **Install Type**: DEVEL (libraries in source)
- **Examples**: Enabled
- **GUI Tools**: Enabled
- **Optimizations**: SIMD enabled

### Editor Settings
- Tab size: 4 spaces
- Format on save: Enabled
- Line length: 100 characters
- C++ standard: C++17

### Code Analysis
- Clang-tidy checks enabled
- Modern C++ recommendations
- Performance analysis
- Bug detection

## 🐛 Troubleshooting

### IntelliSense Issues
1. Check `compile_commands.json` exists in build/
2. Reload window: `Cmd+Shift+P` → "Developer: Reload Window"
3. Reset IntelliSense database if needed

### Build Failures
1. Check CMake output channel
2. Verify dependencies installed
3. Clean and reconfigure

### Debug Problems
1. Ensure Debug build type selected
2. Check library paths in terminal environment
3. Allow lldb in macOS security settings

## 💡 Tips

1. **Multi-folder workspace**: Each component appears as separate folder for focused work
2. **TODO tracking**: TODO-tree extension highlights all TODOs/FIXMEs
3. **Git integration**: GitLens shows inline blame and history
4. **Bookmarks**: Use bookmarks extension to mark important code
5. **Live Share**: Collaborate in real-time with teammates

## 🎉 Summary

This workspace provides a complete, modern development environment for LPZRobots:
- ✅ All components integrated
- ✅ IntelliSense working perfectly
- ✅ One-click build and debug
- ✅ Code quality tools integrated
- ✅ Multi-folder organization

Just open `lpzrobots.code-workspace` and start coding!