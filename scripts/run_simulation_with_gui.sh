#!/bin/bash
# Script to run simulations with GUI tools properly configured

# Add our GUI tools to PATH temporarily
export PATH="/Users/jerry/lpzrobots_mac/guilogger:/Users/jerry/lpzrobots_mac/matrixviz:$PATH"

# Set up Qt environment
export QT_PLUGIN_PATH=/opt/homebrew/opt/qt@5/plugins
export QT_MAC_WANTS_LAYER=1

# Check if we're in a simulation directory
if [ ! -f "start" ]; then
    echo "Error: This script must be run from a simulation directory"
    echo "Example: cd ode_robots/simulations/template_sphererobot"
    exit 1
fi

echo "Starting simulation with GUI tools support..."
echo "Available options:"
echo "  -g    Enable guilogger"
echo "  -m    Enable matrixviz"
echo ""

# Pass all arguments to the simulation
./start "$@"