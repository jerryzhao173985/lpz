#!/bin/bash
echo "Testing LPZRobots GUI Tools on macOS ARM64"
echo "=========================================="
echo

# Set up environment
export PATH="/Users/jerry/lpzrobots_mac/guilogger:/Users/jerry/lpzrobots_mac/matrixviz:$PATH"
export QT_PLUGIN_PATH=/opt/homebrew/opt/qt@5/plugins
export QT_MAC_WANTS_LAYER=1

cd /Users/jerry/lpzrobots_mac/ode_robots/simulations/template_sphererobot

echo "1. Testing Guilogger"
echo "--------------------"
echo "Starting simulation with guilogger..."
echo "Commands:"
echo "  - Click on channel checkboxes to display plots"
echo "  - Press Ctrl+G to toggle guilogger"
echo "  - Press Ctrl+C to stop"
echo
./start -g &
SIM_PID=$!

sleep 5

echo
echo "2. Checking processes..."
if ps aux | grep -v grep | grep guilogger > /dev/null; then
    echo "✓ Guilogger is running"
else
    echo "✗ Guilogger not found"
fi

if ps aux | grep -v grep | grep gnuplot > /dev/null; then
    echo "✓ Gnuplot processes detected"
else
    echo "! No gnuplot processes detected"
fi

echo
echo "Press Enter to test Matrixviz..."
read

# Kill guilogger test
kill $SIM_PID 2>/dev/null
pkill -f "guilogger" 2>/dev/null
pkill -f "gnuplot" 2>/dev/null

echo
echo "3. Testing Matrixviz"
echo "--------------------"
echo "Starting simulation with matrixviz..."
echo "Commands:"
echo "  - Press Ctrl+M to toggle matrixviz"
echo "  - Press Ctrl+C to stop"
echo
./start -m &
SIM_PID=$!

sleep 5

echo
echo "Checking processes..."
if ps aux | grep -v grep | grep matrixviz > /dev/null; then
    echo "✓ Matrixviz is running"
else
    echo "✗ Matrixviz not found"
fi

echo
echo "Press Enter to clean up..."
read

# Cleanup
kill $SIM_PID 2>/dev/null
pkill -f "start.real" 2>/dev/null
pkill -f "guilogger" 2>/dev/null
pkill -f "matrixviz" 2>/dev/null
pkill -f "gnuplot" 2>/dev/null

echo
echo "Test complete!"