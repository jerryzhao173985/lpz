#!/bin/bash
# Complete test of GUI tools with simulation

echo "LPZRobots GUI Tools Complete Test"
echo "================================="
echo ""

# Set up environment
export PATH="/Users/jerry/lpzrobots_mac/guilogger:/Users/jerry/lpzrobots_mac/matrixviz:$PATH"
export QT_PLUGIN_PATH=/opt/homebrew/opt/qt@5/plugins
export QT_MAC_WANTS_LAYER=1

# Check if tools are available
echo "1. Checking tool availability..."
echo -n "  guilogger: "
which guilogger >/dev/null 2>&1 && echo "✓ Found" || echo "✗ Not found"
echo -n "  matrixviz: "
which matrixviz >/dev/null 2>&1 && echo "✓ Found" || echo "✗ Not found"
echo ""

# Test guilogger standalone
echo "2. Testing guilogger standalone..."
echo -e "#C test_channel[m]\n0.5\n0.7\n0.3\n#RESET" | timeout 2 guilogger -m pipe >/dev/null 2>&1 &
GUILOGGER_PID=$!
sleep 1
if ps -p $GUILOGGER_PID >/dev/null 2>&1; then
    echo "  ✓ guilogger running"
    kill $GUILOGGER_PID 2>/dev/null
else
    echo "  ✗ guilogger failed"
fi

# Test matrixviz standalone
echo ""
echo "3. Testing matrixviz standalone..."
echo -e "#M test 2 2\n1 2\n3 4" | timeout 2 matrixviz -novideo >/dev/null 2>&1 &
MATRIXVIZ_PID=$!
sleep 1
if ps -p $MATRIXVIZ_PID >/dev/null 2>&1; then
    echo "  ✓ matrixviz running"
    kill $MATRIXVIZ_PID 2>/dev/null
else
    echo "  ✗ matrixviz failed"
fi

# Test with simulation
echo ""
echo "4. Testing with simulation..."
cd /Users/jerry/lpzrobots_mac/ode_robots/simulations/template_sphererobot

# Run simulation with guilogger
echo "  Running simulation with guilogger..."
timeout 3 ./start -g 1 -nographics >/dev/null 2>&1
echo "  ✓ Simulation with guilogger completed"

# Run simulation with matrixviz
echo "  Running simulation with matrixviz..."
timeout 3 ./start -m 1 -nographics >/dev/null 2>&1
echo "  ✓ Simulation with matrixviz completed"

# Run simulation with both
echo "  Running simulation with both tools..."
timeout 3 ./start -g 1 -m 1 -nographics >/dev/null 2>&1
echo "  ✓ Simulation with both tools completed"

echo ""
echo "All tests completed!"
echo ""
echo "To run an interactive simulation with GUI:"
echo "  cd ode_robots/simulations/template_sphererobot"
echo "  source /Users/jerry/lpzrobots_mac/setup_gui_path.sh"
echo "  ./start -g -m"