#!/bin/bash
# LPZRobots Environment Setup Script
# This script sets up the environment for LPZRobots GUI tools
# Source this file or add to your .bashrc/.zshrc

# Get the directory where LPZRobots is installed
# This assumes the script is in the LPZRobots root directory
LPZROBOTS_HOME="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Add GUI tools to PATH
export PATH="$LPZROBOTS_HOME/guilogger:$LPZROBOTS_HOME/matrixviz:$PATH"

# Qt5 environment for macOS
if [[ "$OSTYPE" == "darwin"* ]]; then
    # Check for Homebrew Qt5
    if [ -d "/opt/homebrew/opt/qt@5" ]; then
        export QT_PLUGIN_PATH=/opt/homebrew/opt/qt@5/plugins
    elif [ -d "/usr/local/opt/qt@5" ]; then
        export QT_PLUGIN_PATH=/usr/local/opt/qt@5/plugins
    fi
    export QT_MAC_WANTS_LAYER=1
fi

echo "LPZRobots environment configured:"
echo "  LPZROBOTS_HOME: $LPZROBOTS_HOME"
echo "  PATH includes: guilogger and matrixviz"

# Check if tools are available
if command -v guilogger &> /dev/null; then
    echo "  ✓ guilogger found"
else
    echo "  ✗ guilogger not found"
fi

if command -v matrixviz &> /dev/null; then
    echo "  ✓ matrixviz found"
else
    echo "  ✗ matrixviz not found - creating wrapper"
    # Create matrixviz wrapper if needed
    if [ -f "$LPZROBOTS_HOME/matrixviz/bin/matrixviz" ]; then
        mkdir -p "$LPZROBOTS_HOME/guilogger"
        cat > "$LPZROBOTS_HOME/guilogger/matrixviz" << 'EOF'
#!/bin/bash
# Wrapper script for matrixviz
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
LPZROBOTS_HOME="$(dirname "$SCRIPT_DIR")"
exec "$LPZROBOTS_HOME/matrixviz/bin/matrixviz" "$@"
EOF
        chmod +x "$LPZROBOTS_HOME/guilogger/matrixviz"
        echo "  Created matrixviz wrapper"
    fi
fi

# Function to run simulations with proper environment
lpzrun() {
    if [ $# -eq 0 ]; then
        echo "Usage: lpzrun <simulation_path> [options]"
        echo "Example: lpzrun ode_robots/simulations/template_sphererobot/start -g -m"
        return 1
    fi
    
    # Ensure environment is set
    export PATH="$LPZROBOTS_HOME/guilogger:$LPZROBOTS_HOME/matrixviz:$PATH"
    
    # Run the simulation
    "$@"
}