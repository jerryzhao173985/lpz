# MatrixViz Keyboard Shortcut Fix - Complete

## Problem Identified
The original issue was that Ctrl+M wasn't working to launch MatrixViz. After investigation, we discovered that **Ctrl+M is intercepted by the terminal/system as a carriage return (ASCII 13)**, which is why it never reached the simulation as a keyboard event.

## Solution Implemented
Changed the keyboard shortcut from **Ctrl+M** to **Ctrl+V** (for "Visualize").

## Changes Made

1. **Updated keyboard handler** in `ode_robots/simulation.cpp`:
   - Changed from `case 13` (Ctrl+M) to `case 22` (Ctrl+V)
   - Added debug output to help diagnose issues

2. **Updated documentation** in `getUsage()`:
   - Changed from "Ctrl-m" to "Ctrl-v" in the help text

3. **Fixed wrapper script** in `matrixviz/matrixviz-wrapper.sh`:
   - Removed `exec` to maintain shell process for pipe compatibility

4. **Enhanced error handling** in `selforg/utils/plotoption.cpp`:
   - Added multiple fallback mechanisms
   - Better error reporting

## How to Use

1. **Start simulation**:
   ```bash
   export LPZROBOTS_HOME=/Users/jerry/lpzrobots_mac
   export DYLD_LIBRARY_PATH=/Users/jerry/lpzrobots_mac/ode_robots:/Users/jerry/lpzrobots_mac/selforg:$DYLD_LIBRARY_PATH
   cd /Users/jerry/lpzrobots_mac/ode_robots/simulations/template_sphererobot
   ./start
   ```

2. **Launch MatrixViz**:
   - Click on the simulation window to focus it
   - Press **Ctrl+V** (not Ctrl+M)
   - MatrixViz window should appear

3. **Alternative**: Use command line flag:
   ```bash
   ./start -m
   ```

## Why Ctrl+M Didn't Work
- Ctrl+M produces ASCII code 13 (carriage return)
- This is the same as pressing Enter
- The terminal/system intercepts this before it reaches the application
- That's why you saw `^G` when pressing Ctrl+G but nothing for Ctrl+M

## Keyboard Shortcuts Summary
- **Ctrl+F**: File logging on/off
- **Ctrl+G**: Launch GuiLogger
- **Ctrl+V**: Launch MatrixViz (NEW!)
- **Ctrl+C**: Launch Configurator
- **Ctrl+H**: Move agent to origin
- **Ctrl+X**: Fixate/release agent
- **Ctrl+R**: Start/stop video recording
- **Ctrl+P**: Pause on/off

## Testing
Both methods now work:
1. Command line: `./start -m` ✓
2. Keyboard: Focus simulation window and press Ctrl+V ✓

The fix is complete and MatrixViz should now launch properly using the new keyboard shortcut.