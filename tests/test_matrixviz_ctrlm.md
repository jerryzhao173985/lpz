# Testing MatrixViz with Ctrl+M

## Steps to test:

1. Start the simulation:
   ```bash
   cd /Users/jerry/lpzrobots_mac/ode_robots/simulations/template_sphererobot
   export DYLD_LIBRARY_PATH=/Users/jerry/lpzrobots_mac/ode_robots:$DYLD_LIBRARY_PATH
   ./start
   ```

2. Once the simulation window appears, click on it to focus it.

3. Press Ctrl+M (hold Control key and press M)

4. The MatrixViz window should appear alongside the simulation.

## Expected behavior:
- MatrixViz should launch and display the neural network visualization
- The window should appear with geometry offset from the main window
- No errors should appear in the terminal

## What was fixed:
The issue was that when the geometry parameter was passed (like "-geometry +812+300"), the `-novideo` option was not being included. This caused matrixviz to fail to start properly. Now it always includes `-novideo` when launching from within the simulation.

## Alternative test:
You can also test with command line launch to verify it works:
```bash
./start -m
```
This should launch both the simulation and MatrixViz at startup.