# MatrixViz Ctrl+M Fix Instructions

## Problem Summary
MatrixViz is not launching when Ctrl+M is pressed in the simulation, although it works with the `-m` command line flag.

## To properly diagnose and fix:

1. **Run the simulation with debug output**:
```bash
export LPZROBOTS_HOME=/Users/jerry/lpzrobots_mac
export DYLD_LIBRARY_PATH=/Users/jerry/lpzrobots_mac/ode_robots:/Users/jerry/lpzrobots_mac/selforg:$DYLD_LIBRARY_PATH
cd /Users/jerry/lpzrobots_mac/ode_robots/simulations/template_sphererobot
./start
```

2. **When simulation is running**:
- Click on the simulation window to focus it
- Press Ctrl+M
- Check console for these debug messages:
  - "Debug: Ctrl key detected, code: 13"
  - "Ctrl+M detected - attempting to launch MatrixViz"
  - "PlotOption: Attempting to launch MatrixViz..."

3. **If you see the debug messages but no MatrixViz window**:
The issue is likely with the popen() and wrapper script interaction.

## Recommended Fix

Create a direct launcher script that bypasses the wrapper chain:

```bash
#!/bin/bash
# Save as: /Users/jerry/lpzrobots_mac/matrixviz/matrixviz-direct-launcher
export QT_PLUGIN_PATH=/opt/homebrew/opt/qt@5/plugins
export QT_ENABLE_HIGHDPI_SCALING=1
export QT_AUTO_SCREEN_SCALE_FACTOR=1
export QT_MAC_WANTS_LAYER=1

# Launch in background to avoid blocking popen()
/Users/jerry/lpzrobots_mac/matrixviz/bin/matrixviz.app/Contents/MacOS/matrixviz "$@" &
```

Then update PlotOption to use this launcher when running from simulation.

## Alternative Solutions

1. **Check if it's a focus issue**: The simulation window might not be properly receiving keyboard events
2. **Check if it's a threading issue**: The popen() call might be blocked by the simulation's event loop
3. **Use system() instead of popen()**: This might work better with the wrapper scripts

## Testing Steps

1. Test direct matrixviz launch: `echo "test" | matrixviz -noCtrlC -novideo`
2. Test from simulation with -m flag: `./start -m`
3. Test from simulation with Ctrl+M after applying fixes

The key is to ensure the matrixviz process can launch independently without blocking the parent process.