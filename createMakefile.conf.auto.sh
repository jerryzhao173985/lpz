#!/bin/bash
# Automated configuration script with CLI options for lpzrobots

# Default values
PREFIX="${PREFIX:-$HOME/lpzrobots}"
TYPE="${TYPE:-USER}"
INTERACTIVE="${INTERACTIVE:-no}"

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --prefix=*)
            PREFIX="${1#*=}"
            shift
            ;;
        --type=*)
            TYPE="${1#*=}"
            shift
            ;;
        --interactive)
            INTERACTIVE="yes"
            shift
            ;;
        --help)
            echo "Usage: $0 [options]"
            echo "Options:"
            echo "  --prefix=PATH     Installation prefix (default: $HOME/lpzrobots)"
            echo "  --type=TYPE       Installation type: USER or DEVEL (default: USER)"
            echo "  --interactive     Run in interactive mode"
            exit 0
            ;;
        *)
            echo "Unknown option: $1"
            exit 1
            ;;
    esac
done

# If interactive mode, use original script
if [ "$INTERACTIVE" = "yes" ]; then
    exec ./createMakefile.conf.sh "$PREFIX" "$TYPE"
fi

# Validate installation type
if [ "$TYPE" != "USER" ] && [ "$TYPE" != "DEVEL" ]; then
    echo "Error: TYPE must be USER or DEVEL"
    exit 1
fi

# Validate and create directories
echo "Configuring lpzrobots installation..."
echo "  PREFIX: $PREFIX"
echo "  TYPE: $TYPE"

# Create necessary directories
mkdir -p "$PREFIX/bin" "$PREFIX/lib" "$PREFIX/include" "$PREFIX/share/lpzrobots"

# Check if we can write to prefix
if [ ! -w "$PREFIX" ]; then
    echo "Error: Cannot write to $PREFIX"
    echo "Please choose a different prefix or create the directory with appropriate permissions"
    exit 1
fi

# Add to PATH if needed
if ! echo $PATH | grep -q "$PREFIX/bin"; then
    echo "Note: $PREFIX/bin is not in PATH"
    echo "Adding to ~/.bashrc and ~/.zshrc if they exist..."
    
    if [ -f ~/.bashrc ]; then
        if ! grep -q "$PREFIX/bin" ~/.bashrc; then
            echo "export PATH=$PREFIX/bin:\$PATH" >> ~/.bashrc
        fi
    fi
    
    if [ -f ~/.zshrc ]; then
        if ! grep -q "$PREFIX/bin" ~/.zshrc; then
            echo "export PATH=$PREFIX/bin:\$PATH" >> ~/.zshrc
        fi
    fi
    
    # Also export for current session
    export PATH=$PREFIX/bin:$PATH
fi

# Generate Makefile.conf
cat > Makefile.conf << EOF
# Automatically generated configuration for lpzrobots
# Generated on: $(date)
# To reconfigure, run: make conf-auto

# Installation prefix
PREFIX=$PREFIX

# Installation type (USER or DEVEL)
TYPE=$TYPE

# Additional flags for strict compilation
CXXFLAGS += -Wall -Wextra -pedantic -Wno-unused-parameter
CFLAGS += -Wall -Wextra -pedantic -Wno-unused-parameter

# Parallel build jobs (auto-detected)
JOBS = $(sysctl -n hw.ncpu 2>/dev/null || nproc 2>/dev/null || echo 4)
EOF

# Generate ode_robots/install_prefix.conf
mkdir -p ode_robots
cat > ode_robots/install_prefix.conf << EOF
// Automatically generated file for lpzrobots
// Generated on: $(date)
#define PREFIX "$PREFIX"
EOF

# Generate install_prefix.conf
cat > install_prefix.conf << EOF
# installation type: user or devel
INSTALLTYPE=$TYPE

# installation prefix:
# e.g. /usr/local or /home/theuser
PREFIX=$PREFIX
EOF

# Create build directories
mkdir -p build_logs

echo "Configuration complete!"
echo ""
echo "Summary:"
echo "  Installation prefix: $PREFIX"
echo "  Installation type: $TYPE"
echo "  Configuration saved to: Makefile.conf"
echo ""
echo "Next steps:"
echo "  make all          - Build and install everything"
echo "  make build-all-auto - Automated build with warning fixes"
echo ""