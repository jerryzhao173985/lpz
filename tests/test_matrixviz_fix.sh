#!/bin/bash
echo "========================================"
echo "Testing Matrixviz Fix"
echo "========================================"
echo

# Source the environment setup
source ./setup_lpzrobots_env.sh

cd /Users/jerry/lpzrobots_mac/ode_robots/simulations/template_sphererobot

echo
echo "1. Testing Direct Matrixviz Launch"
echo "----------------------------------"
echo "Running: ./start -m"
echo "Matrixviz should open with proper window positioning"
echo
./start -m &
SIM_PID=$!

sleep 5
echo "Check if matrixviz opened successfully."
echo "Press Enter to continue..."
read

kill $SIM_PID 2>/dev/null
pkill -f matrixviz 2>/dev/null

echo
echo "2. Testing Matrixviz with Ctrl+M"
echo "---------------------------------"
echo "Start simulation, then press Ctrl+M"
echo "Matrixviz should open to the right of the simulation window"
echo
./start &
SIM_PID=$!

echo "Simulation started. Now:"
echo "1. Press Ctrl+M in the simulation window"
echo "2. Matrixviz should appear"
echo
echo "Press Enter here when done testing..."
read

kill $SIM_PID 2>/dev/null
pkill -f matrixviz 2>/dev/null
pkill -f "start.real" 2>/dev/null

echo
echo "========================================"
echo "Test Complete"
echo "========================================"
echo
echo "If matrixviz works with both methods, the fix is successful!"
echo
echo "For production deployment:"
echo "1. Source setup_lpzrobots_env.sh in your shell profile"
echo "2. Or run: source /path/to/lpzrobots/setup_lpzrobots_env.sh"
echo "3. Then GUI tools will work from any directory"