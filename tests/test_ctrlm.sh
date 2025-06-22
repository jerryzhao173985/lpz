#!/bin/bash
# Test script to run simulation and monitor Ctrl+M behavior
echo "Starting simulation..."
echo "When the simulation window opens:"
echo "1. Click on the simulation window to give it focus"
echo "2. Press Ctrl+M"
echo "3. Watch the terminal for debug output"
echo ""
export DYLD_LIBRARY_PATH=/Users/jerry/lpzrobots_mac/ode_robots:$DYLD_LIBRARY_PATH
./start