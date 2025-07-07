# Understanding the Hexapod Novel Controllers Simulation

## What You're Seeing

### Status Line
```
Status: Scenario: Adaptive Terrain | H:1 W:45 36 17 C:5/4
```

This shows real-time controller metrics:
- **Scenario**: Current test scenario (Adaptive Terrain, Rhythm Coordination, etc.)
- **H:1**: Current horizon (for AH-Sox)
- **W:45 36 17**: Weight values (percentages) for different components or scales
- **C:5/4**: Context information (current context / total contexts for CASox)

### Visual Elements

1. **Hexapod Robots**: The walking robots with 6 legs
2. **Terrain**: Various obstacles and surfaces
3. **Colors**: Different colored robots indicate different controllers

### Controls

- **1-4**: Switch between controllers
  - 1: Sox (Standard homeokinetic controller)
  - 2: AH-Sox (Adaptive Horizon Sox)
  - 3: MSHC (Multi-Scale Homeokinetic Controller)
  - 4: CASox (Context-Aware Sox)
- **h**: Help (shows all keyboard shortcuts)
- **Space**: Pause/unpause simulation
- **f**: Follow mode (camera follows robot)
- **v**: Switch camera views
- **r**: Reset simulation
- **ESC**: Quit

### The Missing Textures

The error messages about missing images are non-critical. The simulation runs fine without them - you just see solid colors instead of textures. I've created scripts to fix this for future runs.

## What Each Controller Does

### Sox (Standard)
- Basic homeokinetic controller
- Maintains a balance between predictability and exploration
- Good general-purpose walking

### AH-Sox (Adaptive Horizon)
- Dynamically adjusts prediction horizon
- Better for changing terrains
- Watch the H: value change in the status line

### MSHC (Multi-Scale)
- Operates at multiple time scales simultaneously
- Better rhythm coordination
- Watch the W: values show scale weights

### CASox (Context-Aware)
- Learns different behaviors for different contexts
- Switches strategies based on situation
- Watch C: show current/total contexts

## Performance Indicators

The robots should:
- Walk smoothly and coordinately
- Adapt to obstacles
- Show different behaviors with different controllers
- Maintain balance even on difficult terrain

## Troubleshooting

If you see:
- **Robots not moving**: Press space to unpause
- **Camera issues**: Press 'v' to cycle views or 'f' for follow mode
- **Too fast/slow**: Adjust with +/- keys
- **Want to see metrics**: They appear in the terminal window

## Next Steps

1. Try switching controllers (keys 1-4) and observe behavior differences
2. Watch how each controller handles obstacles differently
3. Run the multi-robot demo: `./hexapod_behaviors -noshadow`
4. Enable guilogger for plots: `./start -g -noshadow`

The simulation is working correctly despite the texture warnings!