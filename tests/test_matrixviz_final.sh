#!/bin/bash
echo "========================================="
echo "Final Matrixviz Test"
echo "========================================="
echo

# Set up environment
export LPZROBOTS_HOME="/Users/jerry/lpzrobots_mac"
export PATH="$PATH:$LPZROBOTS_HOME/guilogger:$LPZROBOTS_HOME/matrixviz"

echo "Environment:"
echo "  LPZROBOTS_HOME: $LPZROBOTS_HOME"
echo "  PATH includes matrixviz: $(echo $PATH | grep -q matrixviz && echo "YES" || echo "NO")"
echo

echo "Checking executables:"
echo -n "  matrixviz in PATH: "
which matrixviz || echo "NOT FOUND"

echo -n "  guilogger in PATH: "
which guilogger || echo "NOT FOUND"

echo
echo "Direct executable check:"
if [ -x "$LPZROBOTS_HOME/matrixviz/matrixviz" ]; then
    echo "  ✓ $LPZROBOTS_HOME/matrixviz/matrixviz is executable"
else
    echo "  ✗ $LPZROBOTS_HOME/matrixviz/matrixviz is NOT executable"
fi

if [ -x "$LPZROBOTS_HOME/guilogger/matrixviz" ]; then
    echo "  ✓ $LPZROBOTS_HOME/guilogger/matrixviz is executable (wrapper)"
else
    echo "  ✗ $LPZROBOTS_HOME/guilogger/matrixviz is NOT executable"
fi

echo
echo "Testing matrixviz launch:"
cd /Users/jerry/lpzrobots_mac/ode_robots/simulations/template_sphererobot

echo
echo "1. Testing with './start -m' (should work):"
echo "   Press Ctrl+C to stop after matrixviz opens..."
./start -m &
PID=$!
sleep 3
kill $PID 2>/dev/null
pkill -f matrixviz 2>/dev/null

echo
echo "2. Now test with Ctrl+M inside simulation:"
echo "   - Simulation will start"
echo "   - Press Ctrl+M in the simulation window"
echo "   - Matrixviz should open"
echo "   - Press Enter here when done testing"
echo
./start &
PID=$!
read -p "Press Enter when done testing Ctrl+M..."
kill $PID 2>/dev/null
pkill -f matrixviz 2>/dev/null
pkill -f "start.real" 2>/dev/null

echo
echo "========================================="
echo "Test Complete"
echo "========================================="