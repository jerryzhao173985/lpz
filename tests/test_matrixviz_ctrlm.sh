#!/bin/bash
# Test script for matrixviz Ctrl+M functionality

export LPZROBOTS_HOME=/Users/jerry/lpzrobots_mac
export DYLD_LIBRARY_PATH=/Users/jerry/lpzrobots_mac/ode_robots:/Users/jerry/lpzrobots_mac/selforg:$DYLD_LIBRARY_PATH

cd /Users/jerry/lpzrobots_mac/ode_robots/simulations/template_sphererobot

echo "Starting simulation..."
echo "After the simulation starts, press Ctrl+M to test MatrixViz launch"
echo "Watch for any error messages in the console"
echo ""

# Run simulation in background and capture output
./start 2>&1 | grep -E "(PlotOption|MatrixViz|matrixviz)" &

echo "Simulation started in background. Output filtered for MatrixViz messages."
echo "Press Enter to stop the test..."
read

# Kill the simulation
pkill -f "./start"
echo "Test completed."