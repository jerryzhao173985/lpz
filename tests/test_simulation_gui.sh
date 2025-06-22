#!/bin/bash
# Test script for running simulation with GUI tools

echo "Testing LPZRobots simulation with GUI tools"
echo "=========================================="

# Set up environment
export QT_PLUGIN_PATH=/opt/homebrew/opt/qt@5/plugins
export QT_MAC_WANTS_LAYER=1

echo ""
echo "1. Testing simulation with guilogger..."
echo "Starting simulation in one terminal and guilogger in another"
echo ""

# Start simulation with guilogger output
cd /Users/jerry/lpzrobots_mac/ode_robots/simulations/template_sphererobot
echo "Running: ./start -g 1 -nographics -simtime 0.1"
echo "This will run the simulation headless and pipe data to guilogger"
echo ""

# Run simulation with guilogger
./start -g 1 -nographics -simtime 0.1 | /Users/jerry/lpzrobots_mac/guilogger/src/bin/guilogger.app/Contents/MacOS/guilogger -m pipe &
GUILOGGER_PID=$!

echo "Guilogger started with PID $GUILOGGER_PID"
echo "Waiting for simulation to complete..."

# Wait a bit
sleep 10

# Check if guilogger is still running
if ps -p $GUILOGGER_PID > /dev/null; then
    echo "Guilogger is still running - terminating..."
    kill $GUILOGGER_PID
else
    echo "Guilogger has exited"
fi

echo ""
echo "2. Testing simulation with matrixviz..."
echo "Running: ./start -m 1 -nographics -simtime 0.1"
echo ""

# Run simulation with matrixviz
./start -m 1 -nographics -simtime 0.1 | /Users/jerry/lpzrobots_mac/matrixviz/bin/matrixviz -novideo &
MATRIXVIZ_PID=$!

echo "Matrixviz started with PID $MATRIXVIZ_PID"
echo "Waiting for simulation to complete..."

# Wait a bit
sleep 10

# Check if matrixviz is still running
if ps -p $MATRIXVIZ_PID > /dev/null; then
    echo "Matrixviz is still running - terminating..."
    kill $MATRIXVIZ_PID
else
    echo "Matrixviz has exited"
fi

echo ""
echo "Test completed!"
echo ""
echo "To run interactive tests:"
echo "1. Run simulation with graphics: ./start"
echo "2. In another terminal, run guilogger: ./start -g 1 | /Users/jerry/lpzrobots_mac/guilogger/guilogger-wrapper.sh"
echo "3. Or run matrixviz: ./start -m 1 | /Users/jerry/lpzrobots_mac/matrixviz/matrixviz-wrapper.sh"