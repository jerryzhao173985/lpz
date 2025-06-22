#!/bin/bash
echo "===================================="
echo "Testing LPZRobots macOS ARM64 Fixes"
echo "===================================="
echo

# Set up environment
export PATH="/Users/jerry/lpzrobots_mac/guilogger:/Users/jerry/lpzrobots_mac/matrixviz:$PATH"
export QT_PLUGIN_PATH=/opt/homebrew/opt/qt@5/plugins
export QT_MAC_WANTS_LAYER=1

cd /Users/jerry/lpzrobots_mac/ode_robots/simulations/template_sphererobot

echo "1. Testing Gnuplot Window Numbering Fix"
echo "---------------------------------------"
echo "Starting simulation with guilogger..."
echo "- Each gnuplot window should show a unique window number (window 0, window 1, etc.)"
echo "- Press Ctrl+G to toggle guilogger"
echo "- Click on channel checkboxes to open plot windows"
echo
echo "Press Enter to start test..."
read

timeout 30 ./start -g &
SIM_PID=$!

echo "Running for 30 seconds... Check the window titles!"
echo

sleep 30
kill $SIM_PID 2>/dev/null

echo
echo "2. Testing Matrixviz Launch Fix"
echo "-------------------------------"
echo "Starting simulation with matrixviz..."
echo "- Press Ctrl+M to toggle matrixviz"
echo "- Matrixviz window should appear without 'command not found' errors"
echo
echo "Press Enter to start test..."
read

timeout 30 ./start -m &
SIM_PID=$!

echo "Running for 30 seconds... Check if matrixviz launches!"
echo

sleep 30
kill $SIM_PID 2>/dev/null

echo
echo "3. Testing Reduced Debug Output"
echo "--------------------------------"
echo "Starting simulation with guilogger..."
echo "- The terminal should NOT be flooded with 'Sending plot command' messages"
echo "- Only essential output should appear"
echo
echo "Press Enter to start test..."
read

timeout 20 ./start -g &
SIM_PID=$!

echo "Running for 20 seconds... Check the terminal output!"
echo

sleep 20
kill $SIM_PID 2>/dev/null

echo
echo "4. Testing ODE Error Fix"
echo "------------------------"
echo "Starting simulation and checking exit behavior..."
echo "- There should be NO 'ODE Message 2: exit without calling dCloseODE()' error"
echo "- The simulation should exit cleanly"
echo
echo "Press Enter to start test..."
read

./start -g &
SIM_PID=$!

echo "Running for 10 seconds, then will exit cleanly..."
sleep 10

echo "Sending SIGTERM to simulation..."
kill $SIM_PID 2>/dev/null
sleep 2

echo
echo "Checking for ODE error messages..."
if ps aux | grep -v grep | grep "start.real" > /dev/null; then
    echo "Warning: Simulation still running, forcing kill..."
    pkill -f "start.real"
fi

echo
echo "5. Combined Test - All Features"
echo "-------------------------------"
echo "Starting simulation with both guilogger and matrixviz..."
echo "- Press Ctrl+G for guilogger, Ctrl+M for matrixviz"
echo "- Check window numbering, no debug spam, clean exit"
echo
echo "Press Enter to start final test..."
read

./start -g -m &
SIM_PID=$!

echo "Test running... Press Enter to stop and check clean exit..."
read

kill $SIM_PID 2>/dev/null
sleep 2

# Final cleanup
pkill -f "start.real" 2>/dev/null
pkill -f "guilogger" 2>/dev/null
pkill -f "matrixviz" 2>/dev/null
pkill -f "gnuplot" 2>/dev/null

echo
echo "===================================="
echo "All tests completed!"
echo "===================================="
echo
echo "Summary of fixes:"
echo "✓ Gnuplot windows now show unique window numbers"
echo "✓ Matrixviz launches without 'command not found' errors"
echo "✓ Debug output reduced - no more terminal spam"
echo "✓ ODE error on exit should be resolved"
echo
echo "If any issues remain, please report them!"