#!/bin/bash
# Debug script to test simulation
export DYLD_LIBRARY_PATH="/Users/jerry/lpzrobot_mac/selforg:/Users/jerry/lpzrobot_mac/ode_robots:$DYLD_LIBRARY_PATH"
echo "Starting simulation with debug output..."
echo "Library path: $DYLD_LIBRARY_PATH"
echo "Running: ./start -noshadow -nographics"
./start -noshadow -nographics