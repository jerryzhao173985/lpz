# LPZRobots Camera Controls & Window Management

## Window Size
- **Default:** 400x300 pixels (optimized for resource usage)
- **Command line:** Use `-x WxH` to set custom size (e.g., `-x 800x600`)
- **Fullscreen:** Use `-fs` flag
- **Constraints:** 64x64 (minimum) to 3200x2400 (maximum)

## Camera Modes
Switch between modes using number keys:
- **1** - Static Camera
  - Full manual control
  - Best for overview shots
- **2** - Follow Camera  
  - Automatically follows selected robot
  - Maintains relative position
- **3** - TV Camera (default)
  - Television-style tracking
  - Smooth movements

## Mouse/Trackpad Controls

### Zoom
- **Scroll wheel/Two-finger scroll** - Zoom in/out
- **Shift + Scroll** - Fine zoom control

### Rotation
- **Left-click + Drag** - Rotate camera around focus point
- **Ctrl + Left-click + Drag** - Rotate around camera position

### Pan
- **Right-click + Drag** - Pan camera position
- **Middle-click + Drag** - Alternative pan method

### Reset View
- **Space** - Reset to home position
- **H** - Show help/controls

## Keyboard Shortcuts
- **P** - Pause/unpause simulation
- **F** - Toggle fullscreen
- **S** - Take screenshot
- **V** - Start/stop video recording
- **Q/Esc** - Quit simulation
- **+/-** - Increase/decrease simulation speed
- **Ctrl+M** - Launch matrix visualizer
- **Ctrl+G** - Launch guilogger

## Performance Tips
1. Start with smaller window (400x300) for better performance
2. Zoom in only when needed to examine details
3. Use TV mode (3) for automatic tracking
4. Pause simulation (P) when adjusting camera

## Retina Display Notes
- On macOS Retina displays, the viewport is automatically scaled 2x
- Window size shown is logical pixels (e.g., 400x300)
- Actual framebuffer is 2x (e.g., 800x600)
- This ensures sharp, full-window rendering