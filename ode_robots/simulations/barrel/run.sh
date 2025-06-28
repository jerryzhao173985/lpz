#!/bin/bash
# Set library paths for macOS
export DYLD_LIBRARY_PATH="/Users/jerry/lpzrobot_mac/selforg:/Users/jerry/lpzrobot_mac/ode_robots:$DYLD_LIBRARY_PATH"
# Run the simulation with all arguments passed to this script
./start "$@"