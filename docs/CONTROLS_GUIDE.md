# LPZRobots Controls & Navigation Guide

## Table of Contents
1. [Quick Reference](#quick-reference)
2. [Keyboard Controls](#keyboard-controls)
3. [Mouse Controls](#mouse-controls)
4. [Camera Modes](#camera-modes)
5. [Advanced Features](#advanced-features)
6. [Platform-Specific Notes](#platform-specific-notes)
7. [Tips & Best Practices](#tips--best-practices)
8. [Troubleshooting](#troubleshooting)

## Quick Reference

### Essential Controls
| Key | Action | Notes |
|-----|--------|-------|
| **c** | Cycle camera modes | Static → Follow → TV → Race |
| **Space** | Reset camera to home | Returns to default view |
| **+**/**-** | Adjust simulation speed | Use Shift for larger increments |
| **p** | Pause/unpause simulation | Toggle physics simulation |
| **x** | Release/drop selected robot | Useful for testing physics |
| **o** | Spawn random obstacles | Add dynamic objects |
| **Ctrl+C** | Configure parameters | Opens parameter dialog |
| **Ctrl+G** | Launch guilogger | Real-time plotting tool |
| **Ctrl+V** | Launch matrixviz | Neural network visualization |

### WASD Movement (FPS-Style)
| Key | Action | Modifiers |
|-----|--------|-----------|
| **w** | Move forward | Hold Shift for slow |
| **s** | Move backward | Hold Alt for medium |
| **a** | Strafe left | |
| **d** | Strafe right | |
| **q** | Move up | |
| **e** | Move down | |

### Mouse Controls
| Action | Control | Effect | Scaling |
|--------|---------|--------|---------|
| **Orbit** | Left drag | Rotate around focus point | 2.0x |
| **Pan** | Right drag | Move camera laterally | 2.0x |
| **Vertical Pan + Zoom** | Middle drag | Move up/down and forward/back | 1.5x/2.0x |
| **Zoom** | Scroll wheel | Adjust distance to focus | Adaptive |
| **Alt+Left** | Pan alternative | Same as right drag | 2.0x |
| **Alt+Right** | Alternative zoom | Vertical drag for zoom | 2.0x |

### Speed Modifiers
- **Normal**: 1.0x speed (no modifier)
- **Alt/Option**: 0.5x speed (medium precision)  
- **Shift**: 0.1x speed (high precision)

## Keyboard Controls

### Camera & View Controls
- **c** - Cycle through camera modes (Static, Follow, TV, Race)
- **Space** - Reset camera to home position
- **h** - Return to home view
- **1-4** - Direct camera mode selection:
  - 1: Static mode
  - 2: Follow mode
  - 3: TV mode
  - 4: Race mode

### WASD Movement System
The WASD movement system provides FPS-style camera control:
- **w/s** - Move forward/backward along view direction
- **a/d** - Strafe left/right perpendicular to view
- **q/e** - Move up/down in world space

Movement speed modifiers:
- **Normal**: 1.0x speed (no modifier)
- **Alt/Option**: 0.5x speed (medium precision)
- **Shift**: 0.1x speed (high precision)

### Simulation Control
- **p** - Pause/unpause simulation
- **+** - Increase simulation speed
- **-** - Decrease simulation speed
- **Shift++** - Large speed increase
- **Shift+-** - Large speed decrease
- **Ctrl+Q** - Quit simulation

### Robot/Agent Controls
- **x** - Release/drop selected robot
- **X** (Shift+x) - Remove selected robot
- **F1-F12** - Select agent by number
- **b** - Toggle draw bounds
- **Tab** - Select next agent
- **Shift+Tab** - Select previous agent

### Environment Controls
- **o** - Spawn random obstacles
- **O** (Shift+o) - Remove all obstacles
- **l** - Toggle light source
- **Shift+L** - Cycle light positions
- **s** (lowercase) - Toggle shadows
- **w** (lowercase) - Toggle wireframe mode

### Recording & Output
- **Ctrl+G** - Launch guilogger (plotting tool)
- **Ctrl+V** - Launch matrixviz (matrix visualization)
- **Ctrl+F** - Toggle video recording
- **Ctrl+S** - Save current configuration
- **Ctrl+L** - Load configuration

### Debug & Information
- **i** - Print current robot info
- **I** (Shift+i) - Detailed system info
- **Ctrl+I** - Display camera info
- **?** or **F1** - Show help

### Configuration
- **Ctrl+C** - Open configuration dialog
- **u** - Update all drawables
- **j** - Joint axis visibility toggle
- **Ctrl+X** - Store current controller

## Mouse Controls

### Basic Camera Manipulation

#### Left Mouse Button (Orbit)
- **Drag** - Rotate camera around focus point (2.0x scaling)
- **Shift+Drag** - Precision rotation (0.1x speed)
- **Alt+Drag** - Medium rotation (0.5x speed)

#### Right Mouse Button (Pan)
- **Drag** - Pan camera horizontally (2.0x scaling)
- **Shift+Drag** - Precision pan (0.1x speed)
- **Alt+Drag** - Medium pan (0.5x speed)

#### Middle Mouse Button
- **Drag** - Vertical pan (1.5x) and forward/back zoom (2.0x)
- **Click** - Center on clicked point

#### Scroll Wheel
- **Scroll Up** - Zoom in (adaptive speed based on distance)
- **Scroll Down** - Zoom out (adaptive speed based on distance)
- **Shift+Scroll** - Precision zoom (0.1x)
- **Alt+Scroll** - Medium zoom (0.5x)

### Advanced Mouse Controls

#### macOS/Trackpad Gestures
- **Two-finger drag** - Pan view
- **Pinch** - Zoom in/out
- **Three-finger swipe** - Quick camera rotation

#### Combination Controls
- **Alt+Left Mouse** - Alternative pan (same as right mouse, 2.0x scaling)
- **Alt+Right Mouse** - Alternative zoom (vertical drag, 2.0x scaling)
- **Ctrl+Left Mouse** - Manipulate selected agent
- **Ctrl+Right Mouse** - Rotate selected agent

### Mouse Sensitivity
The mouse sensitivity has been optimized for smooth control:
- Base sensitivity: 5.0 (reduced from 10.0 for smoother movement)
- Orbit/pan scaling: 2.0x for responsive control
- Adaptive zoom: 0.5x current distance factor
- Retina display: Automatic coordinate normalization

## Camera Modes

### 1. Static Mode (Default)
- Fixed camera position
- Manual control only
- Best for overview and scene setup
- Use WASD keys for positioning

### 2. Follow Mode
- Camera follows selected agent
- Maintains relative position
- Automatic tracking
- Manual orbit still available

### 3. TV Mode (Television)
- Side-following perspective
- Fixed orientation relative to agent
- Simulates broadcast camera
- Good for observing locomotion

### 4. Race Mode
- Behind-agent perspective
- Follows agent orientation
- First-person adjacent view
- Ideal for navigation tasks

### Switching Between Modes
- **c** key - Cycle through modes
- **1-4** keys - Direct selection
- Visual feedback in console
- Smooth transitions

## Advanced Features

### Speed Control System
The simulation speed can be adjusted in real-time:
- **Normal range**: 0.1x to 10x
- **Shift modifier**: ±1.0 increments
- **No modifier**: ±0.1 increments
- **Current speed** displayed in window title

### Agent Manipulation
When an agent is selected:
- **x** - Drop/release for physics testing
- **Mouse drag** - Apply forces (when enabled)
- **Shift+drag** - Precise force application

### Multi-Agent Control
- **Tab/Shift+Tab** - Cycle through agents
- **F1-F12** - Quick selection
- Selected agent highlighted
- Camera follows selected agent in Follow/TV/Race modes

### Recording Features
- **Video recording**: Ctrl+F starts/stops
- **Frame export**: Automatic numbering
- **Configuration save**: Preserves all parameters
- **Trajectory logging**: Via guilogger

## Platform-Specific Notes

### macOS (Apple Silicon/Intel)

#### Special Key Mappings
On macOS, certain keys are mapped differently:
- **'+' key**: Use '=' with Shift (standard keyboard)
- **Numpad +/-**: Mapped to regular +/- keys
- **Cmd key**: Functions as Ctrl in most cases

#### Retina Display Support
- Automatic 2x scaling detection
- Proper mouse coordinate normalization
- No manual adjustment needed

#### Known Limitations
- OpenGL deprecated (use -nohud flag if crashes occur)
- Some Ctrl+ combinations may conflict with system shortcuts
- Shadow rendering may cause issues (use -noshadow)

### Linux

#### Window Manager Considerations
- Alt+Mouse may conflict with window movement
- Configure your WM to ignore Alt in application windows
- Alternatively, use Super/Win key for WM operations

#### Performance Tips
- Enable compositing for smooth rendering
- Use native Wayland if available
- Disable VSync for lower latency

### Windows

#### High-DPI Support
- Enable DPI awareness in display settings
- May need to adjust mouse sensitivity
- Check OpenGL driver settings

## Tips & Best Practices

### Efficient Navigation

1. **Learn Camera Modes**
   - Use Static for scene overview
   - Switch to Follow for agent tracking
   - TV mode for side analysis
   - Race mode for forward perspective

2. **Master Speed Modifiers**
   - Shift for precise adjustments
   - Alt for medium movements
   - Combine with mouse/WASD

3. **Quick Positioning**
   - Space to reset view
   - WASD for direct movement
   - Save favorite positions

### Workflow Optimization

1. **Multi-Tool Setup**
   ```
   Ctrl+G → Launch guilogger
   Ctrl+V → Launch matrixviz
   Arrange windows for monitoring
   ```

2. **Parameter Tuning**
   - Pause simulation (p)
   - Open config (Ctrl+C)
   - Adjust parameters
   - Unpause and observe

3. **Recording Sessions**
   - Set up camera view
   - Start recording (Ctrl+F)
   - Run experiment
   - Stop recording

### Performance Optimization

1. **Graphics Settings**
   - Disable shadows if slow: -noshadow
   - Reduce FPS if needed: -fps 30
   - Turn off HUD: -nohud

2. **Large Simulations**
   - Use -odethread for physics threading
   - Disable unused visualizations
   - Reduce obstacle count

## Troubleshooting

### Common Issues

#### Keys Not Working
- Check if simulation window has focus
- Verify Caps Lock is off
- Try alternative key combinations
- Check for system shortcut conflicts

#### Mouse Control Issues
- Ensure window is active
- Check mouse sensitivity settings
- Verify no exclusive fullscreen
- Test with different mouse buttons

#### Camera Stuck
- Press Space to reset
- Check if agent still exists
- Verify camera mode is appropriate
- Restart simulation if needed

### Debug Mode
Enable debug output:
```bash
export LPZROBOTS_DEBUG_EVENTS=1
./start
```

This will show:
- Key press events
- Mouse coordinates
- Camera state changes
- Mode transitions

### Performance Issues

1. **Slow Rendering**
   - Add -noshadow flag
   - Reduce window size
   - Lower FPS target
   - Disable unused features

2. **Stuttering**
   - Check CPU usage
   - Verify GPU drivers
   - Disable VSync
   - Use optimized build

3. **High Memory Usage**
   - Reduce simulation time
   - Clear obstacle list (O)
   - Restart periodically
   - Check for memory leaks

## Quick Command Reference Card

```
╔══════════════════════════════════════════════════════════════╗
║                    LPZRobots Quick Controls                   ║
╠═══════════════════╤══════════════════════════════════════════╣
║ Camera            │ c=cycle  Space=reset  1-4=mode  h=home  ║
║ Movement          │ WASD=move  q/e=up/down  Shift=slow       ║
║ Mouse             │ Left=orbit  Right=pan  Scroll=zoom       ║
║ Simulation        │ p=pause  +/-=speed  Ctrl+Q=quit          ║
║ Agents            │ x=drop  Tab=select  F1-F12=quick         ║
║ Tools             │ Ctrl+G=guilogger  Ctrl+V=matrixviz      ║
║ Recording         │ Ctrl+F=video  Ctrl+S=save config         ║
╚═══════════════════╧══════════════════════════════════════════╝
```

---

*Last updated: 2025-01-07*
*LPZRobots version: 0.8.0*
*Platform: macOS ARM64, Linux x86_64*