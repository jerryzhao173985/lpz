#!/bin/bash
# Wrapper script for matrixviz to ensure proper Qt environment

# Check if matrixviz binary exists
MATRIXVIZ_BIN="/Users/jerry/lpzrobots_mac/matrixviz/bin/matrixviz.app/Contents/MacOS/matrixviz"
if [ ! -f "$MATRIXVIZ_BIN" ]; then
    echo "Error: matrixviz binary not found at $MATRIXVIZ_BIN"
    echo "Please run 'make' in the matrixviz directory first."
    exit 1
fi

# Set Qt plugin path
export QT_PLUGIN_PATH=/opt/homebrew/opt/qt@5/plugins

# Enable high DPI support
export QT_ENABLE_HIGHDPI_SCALING=1
export QT_AUTO_SCREEN_SCALE_FACTOR=1

# macOS specific settings
export QT_MAC_WANTS_LAYER=1

# Check if Qt plugins exist
if [ ! -d "$QT_PLUGIN_PATH" ]; then
    echo "Warning: Qt plugin path not found at $QT_PLUGIN_PATH"
    echo "Qt may not display properly. Install Qt5 with: brew install qt@5"
fi

# Initialize configuration if needed
CONFIG_DIR="$HOME/.lpzrobots"
if [ ! -d "$CONFIG_DIR" ]; then
    echo "Initializing LPZRobots configuration directory..."
    /Users/jerry/lpzrobots_mac/init_lpzrobots_config.sh
fi

# Run matrixviz with all arguments passed through
# Don't use exec to maintain shell process for popen compatibility
"$MATRIXVIZ_BIN" "$@"