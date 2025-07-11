#\!/bin/bash
# Test that a user can build a simulation with installed LPZRobots packages
# This simulates what a real user would do after installing LPZRobots

set -e

echo "=== User Simulation Build Test ==="
echo "Testing that installed packages work correctly for users..."

# User would have these in their environment
export PREFIX="${PREFIX:-$HOME/lpzrobots}"
export PATH="$PREFIX/bin:$PATH"

# Save the project root for reference
PROJECT_ROOT="${PROJECT_ROOT:-$(pwd)}"

# Navigate to a simulation (as a user would)
cd ode_robots/simulations/template_sphererobot

echo "=== Environment Check ==="
echo "PREFIX: $PREFIX"
echo "PATH: $PATH"
echo "PROJECT_ROOT: $PROJECT_ROOT"
echo "Current directory: $(pwd)"

# Check that required config scripts are available
echo ""
echo "=== Checking installed tools ==="
for tool in selforg-config ode_robots-config; do
    echo -n "$tool: "
    if which $tool >/dev/null 2>&1; then
        echo "✓ Found at $(which $tool)"
        $tool --version || true
    else
        echo "✗ Not found in PATH"
        echo "ERROR: $tool is required but not found in PATH"
        exit 1
    fi
done

# ode-dbl-config is optional (system ODE might be used)
echo -n "ode-dbl-config: "
if which ode-dbl-config >/dev/null 2>&1; then
    echo "✓ Found at $(which ode-dbl-config)"
    ode-dbl-config --version || true
else
    echo "✗ Not found (will use system ODE)"
fi

# Show what the config scripts provide
echo ""
echo "=== Config script outputs ==="
echo "selforg-config --cflags:"
selforg-config --cflags

echo ""
echo "ode_robots-config --cflags:"
ode_robots-config --cflags

echo ""
echo "selforg-config --libs:"
selforg-config --libs

echo ""
echo "ode_robots-config --libs:"
ode_robots-config --libs

# Check that Makefile exists
echo ""
echo "=== Checking simulation structure ==="
if [ \! -f "Makefile" ]; then
    echo "ERROR: No Makefile found in simulation directory"
    echo "Available files:"
    ls -la
    exit 1
fi

# Clean any previous build
echo ""
echo "=== Building simulation ==="
echo "Cleaning previous build..."
make clean || true

# Build the simulation (this is what users do)
echo ""
echo "Building with: make"
if make; then
    echo ""
    echo "✓ Build completed successfully"
    
    # Check that the binary was created
    if [ -f "start" ]; then
        echo "✓ Simulation binary 'start' created"
        ls -la start
        
        # Verify it's executable
        if [ -x "start" ]; then
            echo "✓ Binary is executable"
            
            # Show library dependencies to verify linking
            echo ""
            echo "=== Checking library dependencies ==="
            if command -v ldd >/dev/null 2>&1; then
                echo "Library dependencies:"
                ldd ./start | grep -E "(selforg|ode_robots|ode)" || true
            elif command -v otool >/dev/null 2>&1; then
                echo "Library dependencies (macOS):"
                otool -L ./start | grep -E "(selforg|ode_robots|ode)" || true
            fi
            
            echo ""
            echo "=== SUCCESS: User installation test passed\! ==="
            echo "Users can successfully build simulations with the installed LPZRobots packages."
            exit 0
        else
            echo "✗ Binary exists but is not executable"
            exit 1
        fi
    else
        echo "✗ Simulation binary 'start' not found"
        echo "Build output files:"
        ls -la
        exit 1
    fi
else
    echo ""
    echo "✗ Build failed"
    echo ""
    echo "=== Build Error Debugging ==="
    
    # Show the actual compile command that failed
    echo "Attempting verbose build to show error:"
    make VERBOSE=1 2>&1 | tail -50
    
    echo ""
    echo "=== Makefile Analysis ==="
    echo "First 30 lines of Makefile:"
    head -30 Makefile
    
    echo ""
    echo "=== Directory Structure ==="
    echo "Current directory contents:"
    ls -la
    
    echo ""
    echo "Parent directory contents:"
    ls -la ..
    
    exit 1
fi
