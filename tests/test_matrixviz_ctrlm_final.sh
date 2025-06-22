#!/bin/bash
# Final test script for MatrixViz Ctrl+M functionality

echo "=== MatrixViz Ctrl+M Fix Test ==="
echo ""
echo "This script will test if MatrixViz properly appears when using Ctrl+M"
echo ""
echo "Instructions:"
echo "1. The simulation will start"
echo "2. Click on the simulation window to give it focus"
echo "3. Press Ctrl+M (hold Control and press M)"
echo "4. MatrixViz should appear and be visible/focused"
echo ""
echo "Press Enter to start the test..."
read

# Set up environment
export DYLD_LIBRARY_PATH=/Users/jerry/lpzrobots_mac/ode_robots:$DYLD_LIBRARY_PATH

# Change to simulation directory
cd /Users/jerry/lpzrobots_mac/ode_robots/simulations/template_sphererobot

# Run the simulation
echo "Starting simulation..."
./start

echo ""
echo "Test complete. Did MatrixViz appear when you pressed Ctrl+M? (y/n)"
read response

if [ "$response" = "y" ]; then
    echo "Great! The fix is working."
else
    echo "If MatrixViz didn't appear, check the console output for any error messages."
    echo "You can also try running: ./start -m"
    echo "to verify MatrixViz works when launched at startup."
fi