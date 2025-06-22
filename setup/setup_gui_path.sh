#!/bin/bash
# Script to set up PATH for LPZRobots GUI tools
# Source this script to add GUI tools to your PATH

export PATH="/Users/jerry/lpzrobots_mac/guilogger:/Users/jerry/lpzrobots_mac/matrixviz:$PATH"
export QT_PLUGIN_PATH=/opt/homebrew/opt/qt@5/plugins
export QT_MAC_WANTS_LAYER=1

echo "LPZRobots GUI tools added to PATH"
echo ""
echo "You can now run simulations with GUI support:"
echo "  cd ode_robots/simulations/template_sphererobot"
echo "  ./start -g -m"
echo ""
echo "Or use individual tools:"
echo "  guilogger --help"
echo "  matrixviz --help"