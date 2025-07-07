# Hexapod Robot with Novel Homeokinetic Controllers

This simulation demonstrates three novel homeokinetic controllers (AH-Sox, MSHC, CASox) controlling a hexapod robot in various environments.

## Overview

The simulation includes:
- A 6-legged hexapod robot with 18+ degrees of freedom
- Real-time switching between different controllers
- Multiple environment scenarios showcasing each controller's strengths
- Visual feedback showing algorithm-specific metrics

## Controllers

### 1. AH-Sox (Adaptive Horizon Sox)
- **Key Feature**: Dynamic prediction horizons based on uncertainty
- **Best For**: Environments with varying predictability
- **Visual Indicator**: Red hexapod
- **Metrics**: Current horizon, horizon weights

### 2. MSHC (Multi-Scale Homeokinetic Controller)
- **Key Feature**: Hierarchical control with multiple time scales
- **Best For**: Complex behaviors with multi-frequency patterns
- **Visual Indicator**: Green hexapod
- **Metrics**: Scale weights, scale activities

### 3. CASox (Context-Aware Sox)
- **Key Feature**: Automatic context detection and specialized learning
- **Best For**: Environments with distinct behavioral modes
- **Visual Indicator**: Blue hexapod
- **Metrics**: Number of contexts, current context, exploration level

## Building and Running

### Build
```bash
cd ode_robots/simulations/hexapod_novel_controllers
make
```

### Run Basic Simulation
```bash
./start
```

### Run with GUI Logger
```bash
./start -g
```

### Run Multiple Hexapods Demo
```bash
./start hexapod_behaviors
```

## Controls

### Controller Selection (main.cpp)
- `1` - Switch to Sox (baseline)
- `2` - Switch to AH-Sox
- `3` - Switch to MSHC
- `4` - Switch to CASox
- `i` - Show controller info
- `h` - Show help

### Environment Selection (hexapod_behaviors.cpp)
- `1` - Adaptive Terrain (varying difficulty)
- `2` - Multi-Scale Environment (rhythmic obstacles)
- `3` - Contextual Environment (distinct areas)
- `4` - Free Exploration

### General Controls
- `v` - Toggle camera mode
- `c` - Toggle camera following
- `f` - Toggle fullscreen
- `x` - Write controller parameters
- `C` - Open configurator
- `+/-` - Adjust simulation speed
- `p` - Pause simulation
- `s` - Single step (when paused)

## Environment Scenarios

### 1. Adaptive Terrain
- Smooth to rough terrain transition
- Tests horizon adaptation capabilities
- AH-Sox excels by adjusting prediction horizons

### 2. Multi-Scale Environment
- Small frequent obstacles (fast scale)
- Medium periodic obstacles (medium scale)
- Large sparse obstacles (slow scale)
- MSHC excels by coordinating multiple time scales

### 3. Contextual Environment
- Narrow corridors (precise movement)
- Open areas (exploration)
- Steps (climbing)
- CASox excels by switching between behavioral contexts

### 4. Free Exploration
- Random obstacles
- Tests general adaptive capabilities

## Algorithm-Specific Features

### AH-Sox Behavior
- Short horizons in unpredictable/rough terrain
- Long horizons in smooth/predictable areas
- Weighted combination of multiple models
- Real-time uncertainty estimation

### MSHC Behavior
- Fast scale handles immediate reactions
- Medium scale coordinates leg movements
- Slow scale manages overall locomotion patterns
- Dynamic weight adjustment based on performance

### CASox Behavior
- Creates distinct contexts for different situations
- Higher exploration in new contexts
- Lower exploration in familiar contexts
- Context-specific model parameters

## Visualization

The status line shows:
- Current controller name
- Algorithm-specific metrics:
  - AH-Sox: Current horizon and weights
  - MSHC: Scale weights and activities
  - CASox: Context count and exploration level

## Performance Tips

- Start with default parameters
- Use configurator (`C` key) to fine-tune:
  - `epsC`: Controller learning rate (0.05-0.2)
  - `epsA`: Model learning rate (0.01-0.1)
  - Algorithm-specific parameters

## Research Applications

This simulation is useful for:
- Comparing adaptive control strategies
- Studying emergent locomotion patterns
- Testing robustness to environmental changes
- Exploring self-organization principles

## Troubleshooting

### Robot Not Moving
- Check noise level (should be > 0)
- Increase learning rates slightly
- Verify controller is initialized

### Unstable Behavior
- Reduce learning rates
- Check joint limits in robot configuration
- Reduce noise level

### Performance Issues
- Reduce number of robots in multi-robot scenario
- Disable shadows: `./start -noshadow`
- Reduce simulation accuracy: `./start -simsteps 1`

## Extensions

To add new behaviors:
1. Modify environment creation in `createEnvironment()`
2. Add new scenario type in `ScenarioType` enum
3. Implement environment update logic if needed
4. Create new controller configurations

## References

- Sox: Der, R. & Martius, G. (2011). The Playful Machine
- Homeokinesis: Der, R. & Martius, G. (2012). Novel plasticity rule
- LPZRobots: https://github.com/georgmartius/lpzrobots