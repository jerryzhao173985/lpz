#!/bin/bash
# Test gnuplot with different terminals

echo "Testing gnuplot terminals..."

# Test 1: Basic gnuplot with default terminal
echo "1. Testing default terminal..."
gnuplot -e "plot sin(x); pause 3" 2>&1 | head -10

# Test 2: Test with dumb terminal (ASCII)
echo -e "\n2. Testing dumb terminal (should work)..."
gnuplot -e "set terminal dumb; plot sin(x)" 2>&1

# Test 3: Test interactive mode
echo -e "\n3. Creating test plot file..."
cat > /tmp/test_plot.gnu << 'EOF'
set terminal dumb
set title "Test Plot"
plot sin(x) title "sin(x)", cos(x) title "cos(x)"
EOF

gnuplot /tmp/test_plot.gnu

echo -e "\n4. Testing PNG output..."
gnuplot -e "set terminal png; set output '/tmp/test.png'; plot sin(x); set output" 2>&1
if [ -f /tmp/test.png ]; then
    echo "PNG file created successfully"
    ls -la /tmp/test.png
else
    echo "Failed to create PNG file"
fi

echo -e "\nDone testing."