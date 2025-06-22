#!/bin/bash
# Quick build script for ode_robots library - builds only standard variant

echo "=== Quick Build for ode_robots ==="
echo "Building standard library only (no opt/dbg variants)"

# Set up environment
export CXXFLAGS="-DGL_SILENCE_DEPRECATION -w"  # -w suppresses all warnings
export MAKEFLAGS="-j8"  # Use 8 parallel jobs

# Kill any existing builds
killall make g++ 2>/dev/null

# Count source files
TOTAL_SOURCES=$(find . -name "*.cpp" -type f | wc -l | tr -d ' ')
echo "Total source files: $TOTAL_SOURCES"

# Build only the standard library
echo "Starting build..."
time make libode_robots.a 2>&1 | grep -E "(^g\+\+|^ar|error:|Error)" | tee quick_build.log

# Check if library was created
if [ -f libode_robots.a ]; then
    echo "SUCCESS: libode_robots.a created"
    ls -lh libode_robots.a
else
    # Check how many object files we have
    OBJ_COUNT=$(find build -name "*.o" -type f 2>/dev/null | wc -l | tr -d ' ')
    echo "Library not complete yet. Object files compiled: $OBJ_COUNT / ~$TOTAL_SOURCES"
    echo "Run this script again to continue building."
fi