#!/bin/bash
# Verify lpzrobots builds without warnings

echo "=== Building lpzrobots with strict warning checks ==="

# Set strict compiler flags
export CXXFLAGS="-std=c++17 -Wall -Wextra -Wpedantic -Werror"

# Clean build
make clean-all

# Build each component
echo "Building selforg..."
if make selforg 2>&1 | tee selforg_build.log | grep -E "warning:|error:"; then
    echo "Warnings/errors found in selforg!"
else
    echo "selforg built cleanly!"
fi

echo "Building ga_tools..."
if make ga_tools 2>&1 | tee ga_tools_build.log | grep -E "warning:|error:"; then
    echo "Warnings/errors found in ga_tools!"
else
    echo "ga_tools built cleanly!"
fi

echo "Building opende..."
if make opende 2>&1 | tee opende_build.log | grep -E "warning:|error:"; then
    echo "Warnings/errors found in opende!"
else
    echo "opende built cleanly!"
fi

echo "Building ode_robots..."
if make ode_robots 2>&1 | tee ode_robots_build.log | grep -E "warning:|error:"; then
    echo "Warnings/errors found in ode_robots!"
else
    echo "ode_robots built cleanly!"
fi

echo "=== Build verification complete ==="
