#!/bin/bash
# Test script to verify CMake file locking prevents race conditions

echo "=== Testing CMake Parallel Configuration for Race Conditions ==="
echo "This test will create multiple CMake configurations in parallel"
echo "to verify that file locking prevents symlink race conditions."
echo

# Number of parallel builds
NUM_BUILDS=5

# Clean up any previous test builds
echo "Cleaning up previous test builds..."
for i in $(seq 1 $NUM_BUILDS); do
    rm -rf build/race-test-$i 2>/dev/null
done

# Function to run CMake configuration
run_cmake_config() {
    local build_num=$1
    echo "Starting configuration $build_num..."
    cmake -B build/race-test-$build_num -DBUILD_GUI_TOOLS=OFF -DBUILD_TESTS=OFF &>/dev/null
    if [ $? -eq 0 ]; then
        echo "Configuration $build_num completed successfully"
    else
        echo "Configuration $build_num FAILED"
        return 1
    fi
}

# Start parallel configurations
echo
echo "Starting $NUM_BUILDS parallel CMake configurations..."
for i in $(seq 1 $NUM_BUILDS); do
    run_cmake_config $i &
done

# Wait for all to complete
echo "Waiting for all configurations to complete..."
wait

# Check for consistency
echo
echo "=== Checking for Race Conditions ==="

# Get reference symlinks from first build
if [ ! -d "build/race-test-1/include" ]; then
    # No symlinks created in build dir, check source include dir
    INCLUDE_DIR="include"
else
    INCLUDE_DIR="build/race-test-1/include"
fi

# Count symlinks in each component
echo "Comparing symlink counts across builds..."
components="selforg ode_robots ga_tools"
all_consistent=true

for component in $components; do
    if [ -d "$INCLUDE_DIR/$component" ]; then
        reference_count=$(find "$INCLUDE_DIR/$component" -type l 2>/dev/null | wc -l)
        echo -n "  $component: $reference_count symlinks"
        
        # Check if counts match across all builds
        mismatch=false
        for i in $(seq 2 $NUM_BUILDS); do
            if [ -d "build/race-test-$i/include/$component" ]; then
                count=$(find "build/race-test-$i/include/$component" -type l 2>/dev/null | wc -l)
            else
                count=$(find "$INCLUDE_DIR/$component" -type l 2>/dev/null | wc -l)
            fi
            
            if [ "$count" != "$reference_count" ]; then
                echo -n " [Build $i has $count - MISMATCH!]"
                mismatch=true
                all_consistent=false
            fi
        done
        
        if [ "$mismatch" = false ]; then
            echo " ✓"
        else
            echo " ✗"
        fi
    fi
done

# Check for lock timeout warnings
echo
echo "Checking for lock timeout warnings..."
timeout_warnings=0
for i in $(seq 1 $NUM_BUILDS); do
    if [ -f "build/race-test-$i/CMakeFiles/CMakeOutput.log" ]; then
        if grep -q "lock timeout\|Failed to acquire lock" "build/race-test-$i/CMakeFiles/CMakeOutput.log" 2>/dev/null; then
            echo "  Build $i had lock timeout warnings"
            timeout_warnings=$((timeout_warnings + 1))
        fi
    fi
done

if [ $timeout_warnings -eq 0 ]; then
    echo "  No lock timeout warnings found ✓"
fi

# Summary
echo
echo "=== Summary ==="
if [ "$all_consistent" = true ] && [ $timeout_warnings -eq 0 ]; then
    echo "SUCCESS: File locking is working correctly!"
    echo "All parallel configurations produced consistent results."
else
    echo "ISSUES DETECTED:"
    if [ "$all_consistent" = false ]; then
        echo "  - Inconsistent symlink counts (possible race condition)"
    fi
    if [ $timeout_warnings -gt 0 ]; then
        echo "  - Lock timeout warnings in $timeout_warnings builds"
    fi
fi

# Cleanup
echo
echo "Cleaning up test builds..."
for i in $(seq 1 $NUM_BUILDS); do
    rm -rf build/race-test-$i
done

echo "Test completed."