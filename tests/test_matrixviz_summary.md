# MatrixViz Ctrl+M Fix Summary

## Problem
The matrixviz tool was not launching when triggered via Ctrl+M keyboard command during simulation, while it worked fine with the `-m` command line flag.

## Root Cause
The issue was caused by the wrapper script chain using `exec`, which replaces the shell process. This interfered with `popen()` used by the simulation to launch matrixviz as a pipe-connected subprocess.

## Solution Applied

1. **Modified wrapper script** (`matrixviz-wrapper.sh`):
   - Removed `exec` command to maintain shell process for pipe compatibility
   - Changed from `exec "$MATRIXVIZ_BIN" "$@"` to `"$MATRIXVIZ_BIN" "$@"`

2. **Enhanced PlotOption class** (`selforg/utils/plotoption.cpp`):
   - Added better error handling and diagnostics
   - Added fallback mechanisms to try different launch methods:
     - First tries matrixviz from PATH
     - Falls back to LPZROBOTS_HOME paths if available
     - Tries pipe-friendly wrapper, then regular wrapper, then guilogger directory

3. **Created pipe-friendly wrapper** (`matrixviz-pipe`):
   - Alternative wrapper without exec for better pipe compatibility
   - Sets all required Qt environment variables

## Testing

To verify the fix works:

1. **Test with -m flag**:
   ```bash
   cd /Users/jerry/lpzrobots_mac/ode_robots/simulations/template_sphererobot
   export LPZROBOTS_HOME=/Users/jerry/lpzrobots_mac
   export DYLD_LIBRARY_PATH=/Users/jerry/lpzrobots_mac/ode_robots:/Users/jerry/lpzrobots_mac/selforg:$DYLD_LIBRARY_PATH
   ./start -m
   ```

2. **Test with Ctrl+M**:
   ```bash
   ./start
   # Press Ctrl+M after simulation starts
   # MatrixViz window should appear
   ```

## Key Changes
- No hardcoded paths in the source code
- Respects LPZROBOTS_HOME environment variable
- Multiple fallback mechanisms for robustness
- Better error reporting to diagnose issues