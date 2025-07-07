# Enhanced Camera Controls for LPZRobots on macOS

## Overview
The camera control system has been enhanced to provide a fluid, intuitive, and professional experience that follows industry-standard 3D application practices, specifically optimized for macOS trackpad and mouse usage.

## Mouse & Trackpad Controls

### Standard Controls (No Modifiers)

| Control | Action | Description |
|---------|--------|-------------|
| **Left Mouse/Click** | Orbit | Rotate camera around focal point (2.0x scaling) |
| **Right Mouse/Two-finger click** | Pan | Move camera horizontally (2.0x scaling) |
| **Middle Mouse/Three-finger click** | Vertical Pan + Zoom | Move up/down and forward/back (1.5x vertical, 2.0x forward) |
| **Scroll/Two-finger swipe** | Zoom | Adaptive zoom based on distance (0.5x distance factor) |

### With Modifiers

| Modifier + Control | Action | Description |
|-------------------|--------|-------------|
| **Alt + Left Mouse** | Pan | Same as right mouse (2.0x scaling) |
| **Alt + Right Mouse** | Alternative Zoom | Vertical drag for zoom (2.0x scaling) |
| **Shift + Any Mouse** | Precision Mode | 0.1x speed for fine control |
| **Alt + Any Mouse** | Medium Speed | 0.5x speed for moderate control |

## Movement Speed System

The control system implements a three-tier speed system:

1. **Normal Speed** (1.0x) - Default movement speed
2. **Medium Speed** (0.5x) - Hold Alt/Option key
3. **Precision Mode** (0.1x) - Hold Shift key

This allows for both quick navigation and precise positioning.

## Implementation Details

### Mouse Sensitivity
- Base multiplier: 5.0 (optimized from original 10.0)
- Orbit scaling: 2.0x for smooth rotation
- Pan scaling: 2.0x for consistent movement
- Zoom scaling: Adaptive based on current distance

### Zoom Mechanics
```cpp
// Adaptive zoom calculation
double currentDistance = camHandle.eye.length();
double zoomSpeed = scrollDelta * currentDistance * 0.5;
```

The zoom speed automatically adjusts based on your distance from the target:
- Far away: Larger zoom steps
- Close up: Smaller, more precise steps

### Forward Vector Calculation
The system uses proper spherical coordinate transformation:
```cpp
osg::Vec3 forward(
  -sin(heading) * cos(pitch),  // X component
  -cos(heading) * cos(pitch),  // Y component  
  -sin(pitch)                  // Z component
);
```

## macOS-Specific Optimizations

### Retina Display Support
- Automatic coordinate normalization for high-DPI displays
- Proper scaling for 2x Retina screens
- No manual adjustment needed

### Trackpad Gestures
- **Two-finger scroll**: Smooth zoom with inertia
- **Pinch gesture**: Alternative zoom method
- **Two-finger drag**: Pan with right-click held
- **Three-finger drag**: Camera orbit (if enabled in System Preferences)

### Smooth Scrolling
The implementation handles both discrete (mouse wheel) and continuous (trackpad) scrolling:
```cpp
switch(ea.getScrollingMotion()) {
  case SCROLL_UP: scrollDelta = -0.1f; break;
  case SCROLL_DOWN: scrollDelta = 0.1f; break;
  default: 
    // Trackpad precise scrolling
    scrollDelta = ea.getScrollingDeltaY() * 0.01f;
}
```

## Best Practices

### For Quick Navigation
1. Use scroll/pinch for rapid zoom
2. Left mouse for looking around
3. Right mouse for repositioning

### For Precise Work
1. Hold Shift for all movements
2. Use Alt+Left for controlled panning
3. Combine modifiers for ultra-precise control

### Trackpad Tips
1. Enable three-finger drag in macOS Accessibility settings
2. Use light touches for smoother movement
3. Two-finger scroll is often smoother than pinch for zoom

## Industry Standard Compliance

This implementation follows the control schemes used in:
- **Blender**: Alt+mouse modifiers, middle mouse behavior
- **Maya**: Camera orbit and pan controls
- **Unity**: Scene view navigation
- **Unreal Engine**: Viewport controls
- **Cinema 4D**: Navigation modifiers

## Debug Mode

Enable debug output to see control values:
```bash
export LPZROBOTS_DEBUG_EVENTS=1
./start
```

This will display:
- Mouse coordinates and deltas
- Active modifiers
- Movement multipliers
- Button states

## Comparison with Original

| Feature | Original | Enhanced |
|---------|----------|----------|
| Base sensitivity | 10.0 | 5.0 (smoother) |
| Scroll zoom | Not supported | Adaptive zoom |
| Alt modifiers | Not supported | Full Alt+mouse controls |
| Speed control | Fixed | Three-tier system |
| Trackpad | Basic | Optimized gestures |

## Summary

The enhanced camera control system provides:
- ✅ Industry-standard controls familiar to 3D artists
- ✅ Smooth, fluid movement optimized for macOS
- ✅ Flexible speed control with modifiers
- ✅ Full trackpad gesture support
- ✅ Retina display compatibility
- ✅ Professional-grade precision

The implementation has been carefully tuned based on extensive testing to provide the best possible experience for macOS users while maintaining cross-platform compatibility.