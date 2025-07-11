# LPZRobots Simulation Development Guide

This guide covers the practical workflow for creating and running robot simulations with self-organizing controllers.

## Quick Start: Your First Simulation

### 1. Create a New Simulation
```bash
cd ode_robots/simulations
./createNewSimulation.sh template_sphererobot my_experiment
cd my_experiment
```

### 2. Build and Run
```bash
make
./start -g -noshadow  # -g enables guilogger, -noshadow for macOS stability
```

### 3. Basic Controls
- **Space**: Pause/unpause simulation
- **x**: Reset simulation
- **v**: Toggle video recording
- **c**: Camera mode
- **+/-**: Simulation speed
- **o/p**: Add/remove obstacles
- **f**: Toggle FPS display
- **Ctrl+g**: Configurator GUI

## Typical Development Workflow

### Step 1: Choose Robot and Controller
Common combinations that work well:
- **Sphererobot + Sox**: Good for testing homeokinetic control
- **FourWheeled + InvertMotorSpace**: Vehicle with predictive control
- **Hexapod + DEP**: Walking robot with differential extrinsic plasticity
- **Arm2Segm + OneController**: Reaching behaviors

### Step 2: Configure Your Simulation
Edit `main.cpp`:
```cpp
// In start() method
Sphererobot3Masses* robot = new Sphererobot3Masses(odeHandle, osgHandle, 
                                                   conf, "Sphere1", 0.4);
robot->place(osg::Matrix::translate(0,0,0.5));

// Choose controller
AbstractController* controller = new Sox();
controller->setParam("epsA", 0.05);  // Learning rate
controller->setParam("epsC", 0.05);
controller->setParam("Logarithmic", 1);  // Smoother learning

// Wire robot to controller
One2OneWiring* wiring = new One2OneWiring(new ColorUniformNoise(0.1));
OdeAgent* agent = new OdeAgent(global);
agent->init(controller, robot, wiring);
global.agents.push_back(agent);
```

### Step 3: Monitor with guilogger
```bash
# In another terminal
guilogger

# Select channels to plot:
# - MotorValues: Motor commands
# - SensorValues: Sensor readings  
# - Parameters: Controller parameters
# - Intrinsic: Controller internal state
```

### Step 4: Tune Parameters

**Interactive tuning via console:**
Press Ctrl+c during simulation:
```
> set epsA 0.1    # Increase learning rate
> set epsC 0.05   # Adjust controller matrix learning
> store           # Save controller state
> load            # Load saved state
> info            # Show all parameters
```

**Or use Configurator GUI:**
Press Ctrl+g to open parameter GUI

## Common Tasks

### Recording Simulations
```bash
# Start recording immediately
./start -g -noshadow -v

# Or press 'v' during simulation
# Output: video000000.avi
```

### Logging Data
```bash
# Enable logging
./start -g -noshadow -l

# Logs created:
# - *.log: Sensor/motor values
# - *.sel: Selectable internal values
# - *.ctrl: Controller state snapshots
```

### Running Parameter Sweeps
```bash
# In main.cpp, use loops:
for(double eps = 0.01; eps <= 0.1; eps += 0.01) {
    controller->setParam("epsA", eps);
    // Run for fixed time...
}
```

### Debugging Unstable Controllers

1. **Check for NaN/Inf:**
   ```cpp
   // In controller step():
   if(!C.isNormal()) {
       fprintf(stderr, "C matrix has NaN/Inf!\n");
       C.toId(); // Reset to identity
   }
   ```

2. **Monitor eigenvalues:**
   ```cpp
   Matrix eigenvalues = C.eigenValues();
   double maxEV = eigenvalues.map(fabs).max();
   if(maxEV > 2.0) {
       // Controller becoming unstable
   }
   ```

3. **Add damping:**
   ```cpp
   controller->setParam("damping", 0.01);
   ```

## Tips for Better Simulations

### Performance
- Use `-noshadow` on macOS to prevent crashes
- Reduce graphic quality with `-drawstuff` for faster simulation
- Use optimized build: `make opt`

### Stability
- Start with small learning rates (epsA=0.01)
- Enable damping for noisy environments
- Use logarithmic error for smoother learning
- Check sensor scaling matches controller expectations

### Reproducibility
- Set random seed: `global.odeConfig.setParam("randomSeed", 1);`
- Log all parameters at start
- Save controller state periodically
- Document initial positions

## Common Controller Parameters

### Sox/DEP Controllers
- `epsA`: Model learning rate (0.01-0.1)
- `epsC`: Controller learning rate (0.01-0.1)  
- `damping`: Stability damping (0.0-0.1)
- `Logarithmic`: Use log error (0 or 1)
- `sense`: Sensory delay compensation

### InvertMotorSpace
- `s4avg`: Smoothing steps (1-10)
- `s4del`: Delay buffer size
- `factorB`: Model bias learning rate

### SineController (for testing)
- `amplitude`: Oscillation amplitude
- `frequency`: Oscillation frequency
- `phase`: Phase offset

## Troubleshooting

### "Segmentation fault"
- Usually graphics-related on macOS
- Solution: Use `-noshadow` flag

### "Controller exploding" 
- Learning rate too high
- Solution: Reduce epsA/epsC, add damping

### "Robot not moving"
- Controller not converged
- Solution: Wait longer or check wiring

### "Video recording fails"
- Missing ffmpeg/libav
- Solution: `brew install ffmpeg` (macOS)

## Advanced Topics

### Custom Robots
See `ode_robots/robots/` for examples. Key methods:
- `create()`: Build physical structure
- `getSensorNumber()`: Define sensors
- `getSensors()`: Read sensor values
- `setMotors()`: Apply motor commands

### Custom Controllers
See `selforg/controller/` for examples. Implement:
- `init()`: Initialize matrices
- `step()`: Control loop
- `stepNoLearning()`: Test mode
- `store/restore()`: Persistence

### Batch Experiments
```bash
# Create experiment script
for i in {1..10}; do
    ./start -noshadow -l -f 1000 -p $i &
done
wait
```

## Publishing Results

### Generate Plots
```python
# Use included Python scripts
python plot_log.py robot.log
python plot_matrix.py controller.ctrl
```

### Export High-Quality Videos
```bash
# Record at higher resolution
./start -w 1920 -h 1080 -v -noshadow
```

### Cite LPZRobots
```bibtex
@book{der2011playful,
  title={The Playful Machine},
  author={Der, Ralf and Martius, Georg},
  year={2011},
  publisher={Springer}
}
```