# 🚀 LPZRobots VSCode Setup Guide

This guide will help you set up Visual Studio Code for optimal development with the LPZRobots codebase.

## 📋 Prerequisites

Before starting, ensure you have:

1. **VSCode** installed (version 1.85 or later)
2. **CMake** 3.20 or later
3. **C++ Compiler**: Clang 14+ or GCC 11+ (C++17 support required)
4. **Dependencies** installed:
   ```bash
   # macOS
   brew install cmake ninja qt@6 open-scene-graph gsl readline llvm
   
   # Linux
   sudo apt-get install cmake ninja-build qt6-base-dev libopenscenegraph-dev \
                        libgsl-dev libreadline-dev clang-tidy clang-format
   ```

## 🏃 Quick Start

### 1. Open the Workspace

```bash
cd lpzrobots
code lpzrobots.code-workspace
```

Or open VSCode and use `File → Open Workspace from File...`

### 2. Install Recommended Extensions

When you open the workspace, VSCode will prompt you to install recommended extensions. Click "Install All" or install manually:

- **C/C++** - Microsoft C++ IntelliSense and debugging
- **CMake Tools** - CMake integration
- **clangd** - Fast language server (optional, replaces Microsoft IntelliSense)
- **CodeLLDB** - Better debugging on macOS/Linux

### 3. Configure CMake

Press `Cmd+Shift+P` (macOS) or `Ctrl+Shift+P` (Linux) and run:
- `CMake: Select a Kit` → Choose "Use CMake Presets"
- `CMake: Configure` → Select your platform preset (e.g., `macos-arm64`)

### 4. Build the Project

- Press `Cmd+Shift+B` to build all targets
- Or use the CMake sidebar to build specific components

## 🎯 Key Features of This Configuration

### 📦 Component-Focused Development

The workspace is organized by components:
- **selforg** - Core controller library
- **ode_robots** - Robot simulation framework
- **ga_tools** - Genetic algorithms
- **GUI Tools** - Visualization utilities

### ⚡ Productivity Shortcuts

| Shortcut | Action |
|----------|--------|
| **Build & Compile** | |
| `Cmd+Shift+B` | Build all components |
| `Cmd+Alt+B` | Build current component |
| `Cmd+Shift+Alt+B` | Clean build workflow |
| **Simulation** | |
| `Cmd+R` | Run current simulation |
| `Cmd+Shift+R` | Run with GUI tools |
| `Cmd+Alt+R` | Run headless mode |
| **Testing** | |
| `Cmd+T` | Run all tests |
| `Cmd+Shift+T` | Test current component |
| **Code Quality** | |
| `Shift+Alt+F` | Format current file |
| `Cmd+Shift+A` | Run clang-tidy |
| **Navigation** | |
| `F12` | Go to definition |
| `Shift+F12` | Find all references |
| `Alt+F12` | Peek definition |
| **Component Build** | |
| `Cmd+1` | Build selforg |
| `Cmd+2` | Build ode_robots |
| `Cmd+3` | Build ga_tools |
| `Cmd+4` | Build GUI tools |

### 🔨 Task Workflows

Access pre-configured workflows via the Command Palette:

1. **🚀 Workflow: Clean Build** - Clean, configure, and build everything
2. **🚀 Workflow: Build and Test** - Build and run all tests
3. **🚀 Workflow: Simulation Development** - Build and run simulation with GUI
4. **🚀 Workflow: Format and Analyze** - Format code and run static analysis

### 🐛 Debugging Configurations

Pre-configured debug scenarios:

- **🐛 Debug: Current CMake Target** - Debug whatever you're working on
- **🌐 Debug: Simulation + GUI Tools** - Debug simulation with visualization
- **🔍 Debug: AddressSanitizer** - Find memory errors
- **🧪 Component Tests** - Debug specific component tests

## 🛠️ Configuration Details

### IntelliSense Configuration

The project supports multiple configurations in `.vscode/c_cpp_properties.json`:

- Platform-specific (macOS ARM64, Linux, etc.)
- Build type specific (Debug, Release, Sanitizers)
- Component-specific configurations

### Code Quality Tools

Integrated tools for maintaining code quality:

- **clang-format**: Automatic code formatting on save
- **clang-tidy**: Static analysis with project-specific checks
- **cppcheck**: Additional static analysis (optional)

### Build System Integration

Full CMake integration with:
- CMake Presets support
- Component-specific build targets
- Parallel builds
- Multiple build configurations

### Debug Visualizers 🔍

Custom debug visualizers for LPZRobots data structures:
- **Matrix**: Grid display with dimensions and statistics
- **OdeRobot**: Robot state with sensors/motors
- **Controllers**: Internal parameters and matrices
- **Pos/Color**: Compact value display

The visualizers load automatically during debugging and provide:
- Pretty-printed output in debug console
- Expandable views in Variables panel
- Custom commands (`matrix`, `robot` in GDB)

## 📝 Using Code Snippets

Type these prefixes and press `Tab` to expand:

| Prefix | Creates |
|--------|---------|
| `lpzcontroller` | New controller class with factory registration |
| `lpzrobot` | New robot class with factory registration |
| `lpzsimmain` | Complete simulation main file |
| `lpzmatrix` | Matrix operations block |
| `lpztest` | Doctest test case |

## 🔧 Customization

### Custom Keybindings

The project includes predefined keybindings in `.vscode/keybindings.json`. To use them:

1. **Option 1**: Copy to your user keybindings
   ```bash
   # macOS
   cp .vscode/keybindings.json ~/Library/Application\ Support/Code/User/keybindings.json
   
   # Linux
   cp .vscode/keybindings.json ~/.config/Code/User/keybindings.json
   ```

2. **Option 2**: Merge with existing keybindings
   - Open Command Palette: `Cmd+Shift+P`
   - Type "Preferences: Open Keyboard Shortcuts (JSON)"
   - Copy desired bindings from `.vscode/keybindings.json`

3. **Linux/Windows Users**: Replace `cmd` with `ctrl` in all keybindings

### Switching Between IntelliSense Engines

**Microsoft C++ IntelliSense** (default):
- Good for most use cases
- Integrated debugging support

**clangd** (alternative):
- Faster and more accurate
- Better for large codebases

To switch to clangd:
1. Install the clangd extension
2. Add to settings.json:
   ```json
   "C_Cpp.intelliSenseEngine": "disabled",
   "clangd.enabled": true
   ```

### Platform-Specific Settings

The configuration automatically detects your platform. To override:

```json
// In settings.json
"cmake.defaultConfigure.configurePreset": "linux",  // or "macos-arm64", etc.
```

### Performance Tuning

For large rebuilds:
```json
"cmake.parallelJobs": 0,  // Use all CPU cores
"C_Cpp.intelliSenseCacheSize": 10240,  // 10GB cache
```

## 🚨 Troubleshooting

### IntelliSense Not Working

1. Ensure CMake configuration succeeded
2. Check for `compile_commands.json` in build directory
3. Reload window: `Cmd+R` (in command palette)

### Build Errors

1. Check CMake output panel for configuration errors
2. Ensure all dependencies are installed
3. Try a clean build: Task → "🧹 Clean: Build Directory"

### Debugging Issues

1. Check library paths in launch.json
2. On macOS: Ensure you've granted debugging permissions
3. Try with sanitizers disabled first

### Performance Issues

1. Exclude build directories from search
2. Increase IntelliSense cache size
3. Consider using clangd instead of Microsoft IntelliSense

## 📚 Additional Resources

- [LPZRobots Documentation](https://github.com/georgmartius/lpzrobots)
- [VSCode C++ Documentation](https://code.visualstudio.com/docs/languages/cpp)
- [CMake Tools Documentation](https://github.com/microsoft/vscode-cmake-tools/blob/main/docs/README.md)

## 💡 Tips and Tricks

1. **Multi-Cursor Editing**: `Alt+Click` to place multiple cursors
2. **Quick Fix**: `Cmd+.` on errors for automatic fixes
3. **Rename Symbol**: `F2` to rename across entire project
4. **Peek Definition**: `Alt+F12` to view without navigating
5. **Format Selection**: `Cmd+K Cmd+F` to format selected code

## 🎉 Ready to Code!

You're now set up for productive LPZRobots development. Start by:

1. Opening a simulation file in `ode_robots/simulations/`
2. Using `lpzsimmain` snippet to create a new simulation
3. Building and debugging with `F5`

Happy coding! 🤖