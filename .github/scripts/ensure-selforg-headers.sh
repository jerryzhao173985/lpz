#!/bin/bash
# Script to ensure selforg headers are properly set up for CI builds
# This handles the complex case where headers use relative includes

set -e

SRCROOT="$(pwd)"
PREFIX="${HOME}/lpzrobots"

echo "Ensuring selforg headers are properly configured..."

# First, make sure selforg has created its internal header links
if [ -d "selforg" ]; then
    echo "Creating selforg internal header links..."
    cd selforg
    make create_header_links || echo "Failed to create header links"
    cd ..
fi

# Now ensure the headers are accessible in all the places they might be needed
# 1. In the installation prefix (for when ode_robots is rebuilt)
if [ -d "$PREFIX/include" ]; then
    # Instead of symlinking individual files, we need to preserve the directory structure
    # This is critical for relative includes to work
    if [ ! -d "$PREFIX/include/selforg" ]; then
        echo "Setting up selforg headers in $PREFIX/include/selforg..."
        mkdir -p "$PREFIX/include/selforg"
        
        # Create symlinks that preserve directory structure
        for dir in controller matrix utils statistictools wirings; do
            if [ -d "selforg/$dir" ]; then
                ln -sf "$SRCROOT/selforg/$dir" "$PREFIX/include/selforg/$dir"
                echo "  Linked $dir directory"
            fi
        done
        
        # Link top-level headers
        for header in selforg/*.h; do
            if [ -f "$header" ]; then
                ln -sf "$SRCROOT/$header" "$PREFIX/include/selforg/$(basename $header)"
            fi
        done
        echo "Selforg headers set up in prefix"
    fi
fi

# 2. In the ode_robots include directory (for local builds)
if [ -d "ode_robots" ]; then
    mkdir -p ode_robots/include/selforg
    
    # Create similar structure in ode_robots
    for dir in controller matrix utils statistictools wirings; do
        if [ -d "selforg/$dir" ] && [ ! -L "ode_robots/include/selforg/$dir" ]; then
            ln -sf "../../../selforg/$dir" "ode_robots/include/selforg/$dir"
            echo "  Linked $dir to ode_robots/include/selforg/"
        fi
    done
    
    # Link top-level headers
    for header in selforg/*.h; do
        if [ -f "$header" ]; then
            ln -sf "../../../$header" "ode_robots/include/selforg/$(basename $header)"
        fi
    done
fi

echo "Selforg header setup complete"