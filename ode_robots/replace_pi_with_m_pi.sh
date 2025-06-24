#!/bin/bash

# Script to replace PI with M_PI in ode_robots source files
# This script will:
# 1. Find all occurrences of PI (not M_PI)
# 2. Replace them with M_PI
# 3. Add necessary includes for M_PI if not present

echo "Replacing PI with M_PI in ode_robots source files..."

# Files that need PI -> M_PI replacement based on our search
files_to_update=(
    "osg/cameramanipulator.cpp"
    "osg/cameramanipulatorTV.cpp"
    "osg/cameramanipulatorFollow.cpp"
    "osg/cameramanipulatorRace.cpp"
)

# Create backup directory
backup_dir="backup_$(date +%Y%m%d_%H%M%S)"
mkdir -p "$backup_dir"

# Function to check if file includes cmath or math.h
has_math_include() {
    local file="$1"
    grep -q "#include\s*<cmath>" "$file" || grep -q "#include\s*<math\.h>" "$file"
}

# Function to add cmath include after the last include
add_cmath_include() {
    local file="$1"
    # Find the last include line number
    last_include=$(grep -n "#include" "$file" | tail -1 | cut -d: -f1)
    if [ -n "$last_include" ]; then
        # Insert #include <cmath> after the last include
        sed -i.bak "${last_include}a\\
#include <cmath>  // For M_PI" "$file"
        rm "${file}.bak"
    else
        # If no includes found, add at the beginning of file after header guard
        sed -i.bak '/#define/a\\
#include <cmath>  // For M_PI' "$file"
        rm "${file}.bak"
    fi
}

# Process each file
for file in "${files_to_update[@]}"; do
    if [ -f "$file" ]; then
        echo "Processing $file..."
        
        # Create backup
        cp "$file" "$backup_dir/"
        
        # Check if file has PI usage (not M_PI)
        if grep -q "\bPI\b" "$file" && ! grep -q "M_PI" "$file"; then
            # Check if file includes math headers
            if ! has_math_include "$file"; then
                echo "  Adding #include <cmath> to $file"
                add_cmath_include "$file"
            fi
            
            # Replace PI with M_PI
            # Using word boundaries to avoid replacing parts of other identifiers
            sed -i.bak 's/\bPI\b/M_PI/g' "$file"
            
            # Show what was changed
            echo "  Replaced PI with M_PI in $file"
            echo "  Changes made:"
            diff -u "$backup_dir/$(basename $file)" "$file" | grep "^[+-]" | grep -v "^[+-][+-][+-]" | head -10
            
            # Remove sed backup
            rm "${file}.bak"
        else
            echo "  No PI to replace in $file (or already uses M_PI)"
        fi
    else
        echo "Warning: $file not found"
    fi
done

echo ""
echo "Backup files saved in: $backup_dir/"
echo ""

# Verify the changes compile
echo "To verify the changes compile correctly, run:"
echo "  cd osg && make clean && make"
echo ""
echo "If there are any compilation errors, you can restore from backup:"
echo "  cp $backup_dir/* osg/"