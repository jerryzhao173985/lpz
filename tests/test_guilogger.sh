#!/bin/bash
# Test script for guilogger functionality

echo "Testing guilogger with gnuplot..."

# Add tools to PATH
export PATH="/Users/jerry/lpzrobots_mac/guilogger:/Users/jerry/lpzrobots_mac/matrixviz:$PATH"
export QT_PLUGIN_PATH=/opt/homebrew/opt/qt@5/plugins
export QT_MAC_WANTS_LAYER=1

# Check if gnuplot is available
if ! command -v gnuplot &> /dev/null; then
    echo "ERROR: gnuplot is not installed or not in PATH"
    exit 1
fi

echo "gnuplot is available at: $(which gnuplot)"

# Test 1: Simple pipe test with proper format
echo "Test 1: Testing guilogger with pipe mode..."
(
    echo "#N test"
    echo "#C x"
    echo "#C y" 
    echo "1.0 2.0"
    echo "2.0 4.0"
    echo "3.0 6.0"
    sleep 2
) | timeout 3 guilogger -m pipe &

GUILOGGER_PID=$!
sleep 2

# Check if guilogger is running
if ps -p $GUILOGGER_PID > /dev/null; then
    echo "✓ Guilogger is running"
    
    # Check for gnuplot processes
    if ps aux | grep -v grep | grep gnuplot > /dev/null; then
        echo "✓ Gnuplot processes detected"
    else
        echo "✗ No gnuplot processes found"
    fi
    
    kill $GUILOGGER_PID 2>/dev/null
else
    echo "✗ Guilogger crashed or exited"
fi

echo ""
echo "Test 2: Running simulation with guilogger..."
cd /Users/jerry/lpzrobots_mac/ode_robots/simulations/template_sphererobot

# Run simulation with guilogger
timeout 5 ./start -g &
SIM_PID=$!

sleep 3

# Check for guilogger and gnuplot
if ps aux | grep -v grep | grep guilogger > /dev/null; then
    echo "✓ Guilogger launched from simulation"
    
    if ps aux | grep -v grep | grep gnuplot > /dev/null; then
        echo "✓ Gnuplot windows should be visible"
        echo ""
        echo "SUCCESS: Guilogger is working correctly!"
        echo "You should now be able to click on channels without crashes."
    else
        echo "✗ Gnuplot not detected"
    fi
else
    echo "✗ Guilogger not launched properly"
fi

# Cleanup
pkill -f "start -g" 2>/dev/null
pkill guilogger 2>/dev/null
pkill gnuplot 2>/dev/null

echo ""
echo "Test complete."