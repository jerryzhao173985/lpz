#!/bin/bash
# Script to set up LPZRobots tools in PATH

LPZROBOTS_DIR="/Users/jerry/lpzrobots_mac"
INSTALL_DIR="/opt/lpzrobots/bin"

echo "Setting up LPZRobots tools in PATH..."

# Create installation directory
if [ ! -d "$INSTALL_DIR" ]; then
    echo "Creating installation directory: $INSTALL_DIR"
    sudo mkdir -p "$INSTALL_DIR"
fi

# Create symlinks for guilogger
if [ -f "$LPZROBOTS_DIR/guilogger/guilogger-wrapper.sh" ]; then
    echo "Creating symlink for guilogger..."
    sudo ln -sf "$LPZROBOTS_DIR/guilogger/guilogger-wrapper.sh" "$INSTALL_DIR/guilogger"
    echo "Created: $INSTALL_DIR/guilogger"
else
    echo "Warning: guilogger-wrapper.sh not found"
fi

# Create symlinks for matrixviz
if [ -f "$LPZROBOTS_DIR/matrixviz/matrixviz-wrapper.sh" ]; then
    echo "Creating symlink for matrixviz..."
    sudo ln -sf "$LPZROBOTS_DIR/matrixviz/matrixviz-wrapper.sh" "$INSTALL_DIR/matrixviz"
    echo "Created: $INSTALL_DIR/matrixviz"
else
    echo "Warning: matrixviz-wrapper.sh not found"
fi

# Check if PATH includes our directory
if [[ ":$PATH:" != *":$INSTALL_DIR:"* ]]; then
    echo ""
    echo "To add LPZRobots tools to your PATH permanently, add this line to your shell configuration:"
    echo ""
    echo "  export PATH=\$PATH:$INSTALL_DIR"
    echo ""
    echo "For zsh (default on macOS):"
    echo "  echo 'export PATH=\$PATH:$INSTALL_DIR' >> ~/.zshrc"
    echo ""
    echo "For bash:"
    echo "  echo 'export PATH=\$PATH:$INSTALL_DIR' >> ~/.bashrc"
    echo ""
    echo "Then reload your shell configuration:"
    echo "  source ~/.zshrc  # or ~/.bashrc"
else
    echo "PATH already includes $INSTALL_DIR"
fi

echo ""
echo "Setup complete! You can now use:"
echo "  guilogger [options]"
echo "  matrixviz [options]"