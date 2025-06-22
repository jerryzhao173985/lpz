#\!/bin/bash
# Test script to verify viewport fix for Retina displays

echo "Testing LPZRobots viewport fix for macOS Retina displays"
echo "Default window size: 400x300"
echo ""

# Check if we're on macOS
if [[ "$OSTYPE" \!= "darwin"* ]]; then
    echo "Warning: This fix is specifically for macOS Retina displays"
fi

# Set up environment
export LPZROBOTS_HOME="/Users/jerry/lpzrobots_mac"
export PATH="$LPZROBOTS_HOME/guilogger:$LPZROBOTS_HOME/matrixviz:$PATH"

# Qt5 environment for macOS
if [[ "$OSTYPE" == "darwin"* ]]; then
    if [ -d "/opt/homebrew/opt/qt@5" ]; then
        export QT_PLUGIN_PATH=/opt/homebrew/opt/qt@5/plugins
    elif [ -d "/usr/local/opt/qt@5" ]; then
        export QT_PLUGIN_PATH=/usr/local/opt/qt@5/plugins
    fi
    export QT_MAC_WANTS_LAYER=1
fi

# Navigate to a simple simulation
cd "$LPZROBOTS_HOME/ode_robots/simulations/template_sphererobot"

echo "Building template_sphererobot simulation..."
make clean 2>/dev/null
if make; then
    echo ""
    echo "Build successful\! Running simulation..."
    echo ""
    echo "Expected behavior:"
    echo "- Window should open at 400x300 pixels"
    echo "- Simulation should render in FULL window (not just bottom-left quarter)"
    echo "- You should be able to zoom with mouse wheel/trackpad"
    echo "- Camera controls: 1=Static, 2=Follow, 3=TV mode"
    echo ""
    echo "Starting simulation..."
    ./start
else
    echo "Build failed. Please check that ode_robots library is built."
    echo "Run 'make' in $LPZROBOTS_HOME/ode_robots directory first."
fi
EOF < /dev/null