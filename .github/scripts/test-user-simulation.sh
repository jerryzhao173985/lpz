#!/bin/bash
# Test script for building a simulation with user-installed lpzrobots packages
# This simulates what a real user would do after installing lpzrobots

set -e

# Configuration
PREFIX="${PREFIX:-$HOME/lpzrobots}"
PROJECT_ROOT="${PROJECT_ROOT:-$(pwd)}"

echo "=== Testing User Simulation Build ==="
echo "PREFIX: $PREFIX"
echo "PROJECT_ROOT: $PROJECT_ROOT"

# Set up environment as a user would
export PATH="$PREFIX/bin:$PATH"
export LD_LIBRARY_PATH="$PREFIX/lib:$LD_LIBRARY_PATH"

# Check that required tools are available
echo "=== Checking installed tools ==="
for tool in selforg-config ode_robots-config; do
    if which $tool >/dev/null 2>&1; then
        echo "✓ $tool found at: $(which $tool)"
        echo "  $tool --version: $($tool --version 2>&1 || echo 'no version')"
        echo "  $tool --cflags: $($tool --cflags)"
        echo "  $tool --libs: $($tool --libs)"
    else
        echo "✗ $tool not found in PATH"
        exit 1
    fi
done

# Check for ode-dbl-config (optional, might use system ODE)
if which ode-dbl-config >/dev/null 2>&1; then
    echo "✓ ode-dbl-config found at: $(which ode-dbl-config)"
else
    echo "⚠ ode-dbl-config not found, will use system ODE"
fi

# Create a test directory (simulating user workspace)
TEST_DIR="/tmp/lpzrobots_test_$$"
mkdir -p "$TEST_DIR"
cd "$TEST_DIR"

echo "=== Creating test simulation in $TEST_DIR ==="

# Copy a template simulation
if [ -d "$PROJECT_ROOT/ode_robots/simulations/template_sphererobot" ]; then
    cp -r "$PROJECT_ROOT/ode_robots/simulations/template_sphererobot" test_sim
    cd test_sim
else
    echo "ERROR: template_sphererobot not found"
    exit 1
fi

# Clean any existing build
make clean || true

# Show what the Makefile will use
echo "=== Build configuration ==="
echo "Makefile first 20 lines:"
head -20 Makefile

echo "=== Attempting build ==="
# Try to build with verbose output
if make VERBOSE=1; then
    echo "✓ Build successful!"
    if [ -f start ]; then
        echo "✓ Executable 'start' created"
        ls -la start
        echo "=== SUCCESS: User simulation test passed! ==="
        # Clean up
        cd /
        rm -rf "$TEST_DIR"
        exit 0
    else
        echo "✗ Executable 'start' not found"
        ls -la
        exit 1
    fi
else
    echo "✗ Build failed"
    echo "=== Debugging information ==="
    
    # Show the actual compile command that failed
    echo "Failed compile command:"
    make -n main.o 2>&1 | head -10
    
    # Check include paths
    echo "Include paths being used:"
    make -n main.o 2>&1 | grep -o -- '-I[^ ]*' | sort -u || true
    
    # Check for missing headers
    echo "Looking for common headers:"
    for header in odehandle.h simulation.h matrix.h; do
        echo -n "  $header: "
        if find "$PREFIX/include" -name "$header" 2>/dev/null | head -1; then
            :
        else
            echo "NOT FOUND in $PREFIX/include"
        fi
    done
    
    exit 1
fi