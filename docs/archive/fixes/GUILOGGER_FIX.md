# Guilogger Crash Fix

## Problem
Guilogger was crashing when clicking on any channel to display a plot. The error was:
```
Pipe broken: Unknown error: -1
```

## Root Cause
1. **Missing gnuplot**: The primary issue was that gnuplot was not installed on the system.
2. **No error handling**: When guilogger tried to open a pipe to gnuplot using `popen()`, it didn't check if the pipe was successfully created.
3. **SIGPIPE handling**: The signal handler was set to SIG_DFL for SIGPIPE, which causes program termination when writing to a broken pipe.

## Solution

### 1. Install gnuplot
```bash
brew install gnuplot
```

### 2. Add error handling in gnuplot.cpp
- Check if `popen()` returns NULL and display an error message
- Add null checks before writing to the pipe in `command()` and `plot()` methods

### 3. Improve signal handling in main.cpp
- Changed SIGPIPE handler from SIG_DFL to SIG_IGN to prevent crashes

## Files Modified
1. `/Users/jerry/lpzrobots_mac/guilogger/src/gnuplot.cpp`
   - Added error checking in `open()` method
   - Added null pipe checks in `command()` and `plot()` methods

2. `/Users/jerry/lpzrobots_mac/guilogger/src/main.cpp`
   - Changed SIGPIPE signal handling to SIG_IGN

## Testing
After the fix:
- Guilogger no longer crashes when clicking on channels
- Gnuplot windows appear correctly when channels are selected
- Error messages are displayed if gnuplot is not available

## Usage
Run simulations with guilogger as before:
```bash
./start -g
```

Click on any channel checkbox in guilogger to display its plot in a gnuplot window.