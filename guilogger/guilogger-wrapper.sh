#!/bin/bash
# Wrapper script for guilogger to ensure proper Qt environment

# Check if guilogger binary exists
GUILOGGER_BIN="/Users/jerry/lpzrobots_mac/guilogger/src/bin/guilogger.app/Contents/MacOS/guilogger"
if [ ! -f "$GUILOGGER_BIN" ]; then
    echo "Error: guilogger binary not found at $GUILOGGER_BIN"
    echo "Please run 'make' in the guilogger directory first."
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

# Run guilogger with all arguments passed through
exec "$GUILOGGER_BIN" "$@"