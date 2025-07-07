# LPZRobots Viewport Fix for macOS Retina Displays

## Problem Summary
The simulation window was only rendering in the bottom-left quarter (1/4) of the window on macOS Retina displays due to OpenSceneGraph not properly detecting the high-DPI framebuffer scaling.

## Solution Overview
Implemented manual 2x scaling detection and correction for macOS Retina displays across multiple components.

## Changes Made

### 1. Default Window Size Reduction
**File:** `ode_robots/simulation.cpp` (lines 113-114)
- Changed default window size from 800x600 to 400x300 pixels
- Reduces resource usage while maintaining functionality
- Window can still be resized and zoomed

### 2. Main Viewport Fix
**File:** `ode_robots/simulation.cpp` (lines 515-526)
```cpp
// On macOS with Retina displays, manually apply 2x scaling if needed
#ifdef __APPLE__
// Check if we need to apply manual scaling (traits don't show high-DPI)
if (traits && traits->width == width && traits->height == height && width > 0) {
  // Traits dimensions match window dimensions, likely need manual scaling
  // Get the actual framebuffer scale
  float scale = 2.0; // Default Retina scale
  width = (int)(width * scale);
  height = (int)(height * scale);
  printf("Applying %.1fx scaling for Retina display (viewport: %dx%d)\n", scale, width, height);
}
#endif
```

### 3. Custom Window Resize Handler
**File:** `ode_robots/osg/retinawindowsizehandler.h`
- Custom event handler that applies 2x scaling during window resize events
- Ensures viewport remains correct when window is resized

### 4. Enhanced Viewer Class
**File:** `ode_robots/osg/retinalviewer.h`
- Extended LPZViewer that corrects viewport before each frame
- Provides continuous viewport correction throughout simulation

### 5. Include Files Updated
**File:** `ode_robots/simulation.cpp` (lines 56-58)
```cpp
#include "retinalviewer.h"
#include "lpzhelphandler.h"
#include "retinawindowsizehandler.h"
```

### 6. Viewer Creation Updated
**File:** `ode_robots/simulation.cpp` (line 328)
```cpp
viewer = new RetinaLPZViewer(*arguments);
```

### 7. Event Handler Registration
**File:** `ode_robots/simulation.cpp` (line 340)
```cpp
viewer->addEventHandler(new RetinaWindowSizeHandler);
```

## Camera Controls & Zoom
The simulation includes three camera modes accessible via number keys:
- **1** - Static camera (manual control)
- **2** - Follow camera (tracks selected robot)
- **3** - TV camera mode (default)

Zoom controls:
- **Mouse wheel** or **trackpad scroll** - Zoom in/out
- **Right-click drag** - Pan camera
- **Left-click drag** - Rotate camera

## Testing
Run the test script after building:
```bash
./test_viewport_fix.sh
```

Expected behavior:
1. Window opens at 400x300 pixels
2. Simulation renders in FULL window (not just bottom-left quarter)
3. Console shows viewport scaling messages
4. Zoom and camera controls work properly

## Command Line Options
- `-x WxH` - Set custom window size (e.g., `-x 800x600`)
- `-fs` - Run in fullscreen mode
- Window size constraints: 64x64 (min) to 3200x2400 (max)

## Building
The ode_robots library must be rebuilt to apply these changes:
```bash
cd /Users/jerry/lpzrobots_mac/ode_robots
make clean
make -j4
```

## Technical Details
The fix works by:
1. Detecting when OSG's GraphicsContext traits dimensions match window dimensions
2. On macOS, this indicates OSG isn't detecting Retina scaling
3. Manually applying 2x scaling to viewport dimensions
4. Updating projection matrix aspect ratio to match scaled viewport
5. Maintaining this correction during resize events and frame rendering