#\!/bin/bash
# Test script for guilogger and matrixviz

echo "Testing GUI tools fixes..."
echo

# Add tools to PATH
export PATH="/Users/jerry/lpzrobots_mac/guilogger:/Users/jerry/lpzrobots_mac/matrixviz:$PATH"
export QT_PLUGIN_PATH=/opt/homebrew/opt/qt@5/plugins
export QT_MAC_WANTS_LAYER=1

echo "1. Checking gnuplot availability..."
if command -v gnuplot &> /dev/null; then
    echo "✓ gnuplot is installed at: $(which gnuplot)"
    echo "  Version: $(gnuplot --version)"
else
    echo "✗ gnuplot is not installed"
fi

echo
echo "2. Testing guilogger plot functionality..."

# Create a test data file
cat > /tmp/guilogger_test.data << EOFDATA
#N test
#C x
#C y
#C z
1.0 2.0 3.0
2.0 4.0 6.0
3.0 6.0 9.0
4.0 8.0 12.0
5.0 10.0 15.0
EOFDATA

# Run guilogger with test data in background
echo "Starting guilogger with test data..."
(cat /tmp/guilogger_test.data; sleep 5) | timeout 10 guilogger -m pipe &
GUILOGGER_PID=$\!

sleep 2

# Check if guilogger is running
if ps -p $GUILOGGER_PID > /dev/null 2>&1; then
    echo "✓ Guilogger is running (PID: $GUILOGGER_PID)"
    echo "  Try clicking on channels to display plots"
    
    # Check for gnuplot processes
    sleep 3
    if ps aux | grep -v grep | grep gnuplot > /dev/null; then
        echo "✓ Gnuplot processes detected - plots should be visible"
    else
        echo "\! No gnuplot processes detected yet"
        echo "  Click on channel checkboxes in guilogger to display plots"
    fi
else
    echo "✗ Guilogger is not running - it may have crashed"
fi

echo
echo "3. Testing matrixviz..."

# Create test data for matrixviz
cat > /tmp/matrixviz_test.data << EOFDATA2
#C A[3,3]
#N test
1.0 0.5 0.2 0.3 0.8 0.1 0.6 0.4 0.9
0.9 0.4 0.6 0.1 0.8 0.3 0.2 0.5 1.0
EOFDATA2

echo "Starting matrixviz with test data..."
(cat /tmp/matrixviz_test.data; sleep 5) | timeout 10 /Users/jerry/lpzrobots_mac/matrixviz/bin/matrixviz.app/Contents/MacOS/matrixviz -noCtrlC &
MATRIXVIZ_PID=$\!

sleep 2

# Check if matrixviz is running
if ps -p $MATRIXVIZ_PID > /dev/null 2>&1; then
    echo "✓ Matrixviz is running (PID: $MATRIXVIZ_PID)"
    echo "  You should see a visualization window"
else
    echo "✗ Matrixviz crashed or exited"
fi

echo
echo "4. Testing with simulation..."
echo "Run the following command to test with actual simulation:"
echo "cd /Users/jerry/lpzrobots_mac/ode_robots/simulations/template_sphererobot"
echo "./start -g -m"
echo
echo "Then:"
echo "- In guilogger: Click on channel checkboxes to display plots"
echo "- Use Ctrl+G in simulation to toggle guilogger"
echo "- Use Ctrl+M in simulation to toggle matrixviz"

# Wait for user input
echo
echo "Press Enter to clean up test processes..."
read

# Cleanup
kill $GUILOGGER_PID 2>/dev/null
kill $MATRIXVIZ_PID 2>/dev/null
pkill gnuplot 2>/dev/null
rm -f /tmp/guilogger_test.data /tmp/matrixviz_test.data

echo "Test complete."
