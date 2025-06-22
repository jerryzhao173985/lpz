#!/bin/bash

echo "Testing matrixviz discovery..."

# Test 1: Direct PATH lookup
echo -e "\n1. Testing direct PATH lookup:"
export PATH="$PATH:/Users/jerry/lpzrobots_mac/matrixviz"
which matrixviz || echo "Not found in PATH"

# Test 2: LPZROBOTS_HOME environment variable
echo -e "\n2. Testing with LPZROBOTS_HOME:"
export LPZROBOTS_HOME="/Users/jerry/lpzrobots_mac"
echo "LPZROBOTS_HOME=$LPZROBOTS_HOME"

# Test 3: Direct execution
echo -e "\n3. Testing direct execution:"
if [ -x "/Users/jerry/lpzrobots_mac/matrixviz/matrixviz" ]; then
    echo "matrixviz is executable at /Users/jerry/lpzrobots_mac/matrixviz/matrixviz"
else
    echo "matrixviz NOT executable at /Users/jerry/lpzrobots_mac/matrixviz/matrixviz"
fi

# Test 4: Guilogger wrapper
echo -e "\n4. Testing guilogger wrapper:"
if [ -x "/Users/jerry/lpzrobots_mac/guilogger/matrixviz" ]; then
    echo "guilogger wrapper is executable at /Users/jerry/lpzrobots_mac/guilogger/matrixviz"
else
    echo "guilogger wrapper NOT executable at /Users/jerry/lpzrobots_mac/guilogger/matrixviz"
fi

# Test 5: Test popen command
echo -e "\n5. Testing popen-style command:"
sh -c "which matrixviz 2>&1" || echo "popen would fail to find matrixviz"

# Test 6: Test with environment
echo -e "\n6. Testing with full environment:"
env PATH="$PATH" LPZROBOTS_HOME="$LPZROBOTS_HOME" sh -c "which matrixviz 2>&1" || echo "Still not found"

echo -e "\nDone."