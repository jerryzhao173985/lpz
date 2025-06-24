#!/bin/bash
# Script to check for compilation warnings in LPZRobots

echo "Checking compilation warnings for LPZRobots..."
echo "============================================"

# Create temporary log file
LOGFILE="compilation_warnings_$(date +%Y%m%d_%H%M%S).log"

# Function to extract and categorize warnings
extract_warnings() {
    local component=$1
    local logfile=$2
    
    echo -e "\n=== $component Warnings ===" >> "$LOGFILE"
    
    # Count different warning types
    override_count=$(grep -c "override" "$logfile" 2>/dev/null || echo 0)
    cast_count=$(grep -c "old-style-cast\|cast" "$logfile" 2>/dev/null || echo 0)
    init_count=$(grep -c "uninitialized\|will be initialized after" "$logfile" 2>/dev/null || echo 0)
    shadow_count=$(grep -c "shadow" "$logfile" 2>/dev/null || echo 0)
    const_count=$(grep -c "const" "$logfile" 2>/dev/null || echo 0)
    
    echo "Override warnings: $override_count" >> "$LOGFILE"
    echo "Cast warnings: $cast_count" >> "$LOGFILE"
    echo "Initialization warnings: $init_count" >> "$LOGFILE"
    echo "Shadow warnings: $shadow_count" >> "$LOGFILE"
    echo "Const warnings: $const_count" >> "$LOGFILE"
    
    # Extract actual warning messages
    echo -e "\nDetailed warnings:" >> "$LOGFILE"
    grep -E "warning:|error:" "$logfile" >> "$LOGFILE" 2>/dev/null || echo "No warnings found" >> "$LOGFILE"
}

# Check selforg
echo "Checking selforg..."
cd selforg 2>/dev/null
if [ $? -eq 0 ]; then
    make clean > /dev/null 2>&1
    make 2>&1 | tee selforg_build.log | grep -E "warning:|error:"
    extract_warnings "selforg" "selforg_build.log"
    rm -f selforg_build.log
    cd ..
else
    echo "selforg directory not found" >> "$LOGFILE"
fi

# Check ode_robots
echo -e "\nChecking ode_robots..."
cd ode_robots 2>/dev/null
if [ $? -eq 0 ]; then
    make clean > /dev/null 2>&1
    make 2>&1 | tee ode_robots_build.log | grep -E "warning:|error:"
    extract_warnings "ode_robots" "ode_robots_build.log"
    rm -f ode_robots_build.log
    cd ..
else
    echo "ode_robots directory not found" >> "$LOGFILE"
fi

# Check a sample simulation
echo -e "\nChecking sample simulation..."
cd ode_robots/simulations/template_sphererobot 2>/dev/null
if [ $? -eq 0 ]; then
    make clean > /dev/null 2>&1
    make 2>&1 | tee simulation_build.log | grep -E "warning:|error:"
    extract_warnings "template_sphererobot" "simulation_build.log"
    rm -f simulation_build.log
    cd ../../..
else
    echo "template_sphererobot not found" >> "$LOGFILE"
fi

# Summary
echo -e "\n\n=== SUMMARY ===" >> "$LOGFILE"
echo "Total override warnings: $(grep -c "override" "$LOGFILE" 2>/dev/null || echo 0)" >> "$LOGFILE"
echo "Total cast warnings: $(grep -c "cast" "$LOGFILE" 2>/dev/null || echo 0)" >> "$LOGFILE"
echo "Total initialization warnings: $(grep -c "initialized" "$LOGFILE" 2>/dev/null || echo 0)" >> "$LOGFILE"
echo "Total shadow warnings: $(grep -c "shadow" "$LOGFILE" 2>/dev/null || echo 0)" >> "$LOGFILE"

echo -e "\nWarning analysis complete. Results saved to: $LOGFILE"
echo "To view results: cat $LOGFILE"