#!/bin/bash
# Test script for MatrixViz Ctrl+M after removing debug messages

echo "=== MatrixViz Ctrl+M Test (Clean Console) ==="
echo ""
echo "This test verifies that:"
echo "1. The continuous 'Agent Sphere1 selected' messages are gone"
echo "2. MatrixViz properly appears when pressing Ctrl+M"
echo ""
echo "Steps:"
echo "1. Start the simulation"
echo "2. Verify no continuous 'selected' messages in console"
echo "3. Press Ctrl+M to launch MatrixViz"
echo "4. MatrixViz should appear and work correctly"
echo ""
echo "Press Enter to start..."
read

# Set up environment
export DYLD_LIBRARY_PATH=/Users/jerry/lpzrobots_mac/ode_robots:$DYLD_LIBRARY_PATH
cd /Users/jerry/lpzrobots_mac/ode_robots/simulations/template_sphererobot

# Run the simulation
echo "Starting simulation..."
./start

echo ""
echo "Test complete. Please answer:"
echo "1. Were the continuous 'selected' messages gone? (y/n)"
read response1

echo "2. Did MatrixViz appear when you pressed Ctrl+M? (y/n)"
read response2

if [ "$response1" = "y" ] && [ "$response2" = "y" ]; then
    echo ""
    echo "Excellent! The fix worked."
    echo "The debug messages were interfering with the pipe communication to MatrixViz."
else
    echo ""
    echo "Please check if there are any error messages in the console."
fi