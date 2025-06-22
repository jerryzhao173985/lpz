#!/bin/bash
echo "========================================"
echo "Testing All LPZRobots Fixes"
echo "========================================"
echo

# Set up environment
export PATH="/Users/jerry/lpzrobots_mac/guilogger:/Users/jerry/lpzrobots_mac/matrixviz:$PATH"
export QT_PLUGIN_PATH=/opt/homebrew/opt/qt@5/plugins
export QT_MAC_WANTS_LAYER=1

cd /Users/jerry/lpzrobots_mac/ode_robots/simulations/template_sphererobot

echo "1. Testing Viewport Fix"
echo "-----------------------"
echo "The simulation should fill the entire window."
echo "Check the debug output for viewport dimensions."
echo
echo "Press Enter to start..."
read

timeout 15 ./start &
SIM_PID=$!
sleep 15
kill $SIM_PID 2>/dev/null

echo
echo "2. Testing Matrixviz with Direct Launch"
echo "---------------------------------------"
echo "Running: ./start -m"
echo "Matrixviz should open successfully."
echo
echo "Press Enter to start..."
read

./start -m &
SIM_PID=$!
echo "Press Enter to stop..."
read
kill $SIM_PID 2>/dev/null

echo
echo "3. Testing Matrixviz with Ctrl+M"
echo "---------------------------------"
echo "Start simulation, then press Ctrl+M to toggle matrixviz."
echo "Check terminal for 'MatrixViz-Stream opened using:' message"
echo
echo "Press Enter to start..."
read

./start &
SIM_PID=$!
echo "Now press Ctrl+M in the simulation window..."
echo "Press Enter here when done testing..."
read
kill $SIM_PID 2>/dev/null

echo
echo "4. Testing Guilogger Window Numbering"
echo "-------------------------------------"
echo "Start simulation with guilogger."
echo "Click on multiple channels to open plot windows."
echo "Each window should show a unique number (window 0, window 1, etc.)"
echo
echo "Press Enter to start..."
read

./start -g &
SIM_PID=$!
echo "Click on channels to open multiple plot windows..."
echo "Press Enter here when done testing..."
read
kill $SIM_PID 2>/dev/null

# Cleanup
pkill -f "start.real" 2>/dev/null
pkill -f "guilogger" 2>/dev/null
pkill -f "matrixviz" 2>/dev/null
pkill -f "gnuplot" 2>/dev/null

echo
echo "========================================"
echo "Test Summary"
echo "========================================"
echo
echo "Issues that should be fixed:"
echo "1. ✓ Viewport fills entire window (not just 1/4)"
echo "2. ✓ Gnuplot windows show unique numbers"
echo "3. ✓ Matrixviz launches with ./start -m"
echo "4. ✓ Matrixviz launches with Ctrl+M"
echo "5. ✓ No excessive debug output"
echo "6. ✓ Clean exit without ODE errors"
echo
echo "If any issues remain, check the terminal output for error messages."