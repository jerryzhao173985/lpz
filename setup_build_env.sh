#!/bin/bash
# Setup script for lpzrobots build environment
# This script prepares the build environment for both local development and CI

set -e  # Exit on error

echo "Setting up lpzrobots build environment..."

# Detect platform
if [[ "$OSTYPE" == "darwin"* ]]; then
    PLATFORM="macOS"
elif [[ "$OSTYPE" == "linux-gnu"* ]]; then
    PLATFORM="Linux"
else
    echo "Unsupported platform: $OSTYPE"
    exit 1
fi

echo "Detected platform: $PLATFORM"

# Create ODE compatibility headers
setup_ode_compat() {
    echo "Setting up ODE compatibility layer..."
    mkdir -p include/ode-dbl
    
    # Try different locations for ODE headers
    ODE_FOUND=false
    
    # Check system locations
    if [ -d "/usr/include/ode" ]; then
        echo "Found system ODE in /usr/include/ode"
        ln -sf /usr/include/ode/*.h include/ode-dbl/
        ODE_FOUND=true
    elif command -v brew >/dev/null 2>&1 && [ -d "$(brew --prefix ode 2>/dev/null)/include/ode" ]; then
        echo "Found Homebrew ODE"
        ln -sf $(brew --prefix ode)/include/ode/*.h include/ode-dbl/
        ODE_FOUND=true
    elif [ -d "/opt/homebrew/include/ode" ]; then
        echo "Found ODE in /opt/homebrew"
        ln -sf /opt/homebrew/include/ode/*.h include/ode-dbl/
        ODE_FOUND=true
    fi
    
    # If system ODE not found, check if we have bundled ODE
    if [ "$ODE_FOUND" = false ]; then
        if [ -d "opende/ode/src" ]; then
            echo "No system ODE found, will use bundled ODE"
            # The bundled ODE headers will be set up during the build
            return 0
        else
            echo "WARNING: ODE headers not found!"
            echo "Please install ODE:"
            echo "  Ubuntu/Debian: sudo apt-get install libode-dev"
            echo "  macOS: brew install ode"
            echo ""
            echo "Or the build system will use the bundled ODE version."
        fi
    fi
}

# Check for required dependencies
check_dependencies() {
    echo "Checking dependencies..."
    
    MISSING_DEPS=""
    
    # Check for essential build tools
    command -v make >/dev/null 2>&1 || MISSING_DEPS="$MISSING_DEPS make"
    command -v g++ >/dev/null 2>&1 || command -v clang++ >/dev/null 2>&1 || MISSING_DEPS="$MISSING_DEPS g++/clang++"
    command -v m4 >/dev/null 2>&1 || MISSING_DEPS="$MISSING_DEPS m4"
    
    if [ -n "$MISSING_DEPS" ]; then
        echo "ERROR: Missing required dependencies:$MISSING_DEPS"
        echo ""
        echo "Please install missing dependencies:"
        if [ "$PLATFORM" = "Linux" ]; then
            echo "  sudo apt-get install build-essential m4"
        else
            echo "  brew install m4"
        fi
        exit 1
    fi
    
    echo "✓ Essential build tools found"
    
    # Check for optional but recommended dependencies
    echo ""
    echo "Checking optional dependencies..."
    command -v cmake >/dev/null 2>&1 && echo "✓ CMake found" || echo "✗ CMake not found (optional)"
    pkg-config --exists gsl 2>/dev/null && echo "✓ GSL found" || echo "✗ GSL not found (recommended)"
    
    # Check for Qt6 (for GUI tools)
    if command -v qmake6 >/dev/null 2>&1 || command -v qmake >/dev/null 2>&1; then
        echo "✓ Qt found"
    else
        echo "✗ Qt not found (needed for GUI tools)"
    fi
}

# Configure components for development mode
configure_components() {
    echo ""
    echo "Configuring components for development mode..."
    
    PREFIX="${PREFIX:-${HOME}/lpzrobots}"
    echo "Using prefix: $PREFIX"
    
    # Configure each component if it has a configure script
    for component in selforg opende ode_robots ga_tools; do
        if [ -d "$component" ] && [ -x "$component/configure" ]; then
            echo "Configuring $component..."
            (cd "$component" && ./configure --prefix="$PREFIX" --srcprefix="$(pwd)" --type=DEVEL) || echo "Warning: $component configuration failed"
        fi
    done
}

# Main setup
main() {
    # Change to script directory
    cd "$(dirname "$0")"
    
    # Run setup steps
    check_dependencies
    setup_ode_compat
    
    # Ask if user wants to configure components
    if [ -t 0 ]; then  # Check if running interactively
        echo ""
        read -p "Configure components for development mode? [y/N] " -n 1 -r
        echo
        if [[ $REPLY =~ ^[Yy]$ ]]; then
            configure_components
        fi
    fi
    
    echo ""
    echo "Build environment setup complete!"
    echo ""
    echo "Next steps:"
    echo "  1. For CMake build: cmake --preset=default && cmake --build build/default"
    echo "  2. For Make build: make conf && make all"
    echo ""
    echo "To build a specific component:"
    echo "  make selforg"
    echo "  make ode_robots"
    echo "  make ga_tools"
    echo ""
}

# Run main function
main "$@"