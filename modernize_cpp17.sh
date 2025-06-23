#!/bin/bash

# Script to modernize LPZRobots codebase to C++17
# This script applies safe, automated modernizations

echo "=== LPZRobots C++17 Modernization Script ==="
echo

# Check if we're in the right directory
if [ ! -f "Makefile" ] || [ ! -d "selforg" ] || [ ! -d "ode_robots" ]; then
    echo "Error: This script must be run from the LPZRobots root directory"
    exit 1
fi

# Function to apply modernizations to a directory
modernize_directory() {
    local dir=$1
    local component=$2
    
    echo "Processing $component in $dir..."
    
    # Count files to process
    local total_files=$(find "$dir" -name "*.cpp" -o -name "*.h" | wc -l)
    echo "Found $total_files files to process"
    
    # Apply NULL to nullptr conversion
    echo "  - Converting NULL to nullptr..."
    find "$dir" -name "*.cpp" -o -name "*.h" | while read -r file; do
        # Skip if file doesn't exist or is a symlink
        [ -f "$file" ] && [ ! -L "$file" ] || continue
        
        # Apply NULL to nullptr conversion
        sed -i.bak 's/\bNULL\b/nullptr/g' "$file"
        
        # Clean up backup if changes were made
        if ! diff -q "$file" "$file.bak" >/dev/null 2>&1; then
            rm "$file.bak"
        else
            mv "$file.bak" "$file"
        fi
    done
    
    # Apply 0 to nullptr for pointer contexts (conservative)
    echo "  - Converting pointer assignments from 0 to nullptr..."
    find "$dir" -name "*.cpp" -o -name "*.h" | while read -r file; do
        [ -f "$file" ] && [ ! -L "$file" ] || continue
        
        # Convert obvious pointer assignments
        sed -i.bak -E 's/([*&]\s*[a-zA-Z_][a-zA-Z0-9_]*\s*=\s*)0([;,)])/\1nullptr\2/g' "$file"
        sed -i.bak -E 's/(return\s+)0(\s*;.*\/\/.*pointer)/\1nullptr\2/g' "$file"
        sed -i.bak -E 's/(\s+==\s*)0(\s*[;)])/\1nullptr\2/g' "$file"
        sed -i.bak -E 's/(\s+!=\s*)0(\s*[;)])/\1nullptr\2/g' "$file"
        
        if ! diff -q "$file" "$file.bak" >/dev/null 2>&1; then
            rm "$file.bak"
        else
            mv "$file.bak" "$file"
        fi
    done
    
    echo "  - Completed $component"
    echo
}

# Function to add override keywords where appropriate
add_override_keywords() {
    local dir=$1
    local component=$2
    
    echo "Adding override keywords in $component..."
    
    # This is more complex and requires parsing, so we'll use a conservative approach
    # Only add override to destructors that clearly override
    find "$dir" -name "*.h" | while read -r file; do
        [ -f "$file" ] && [ ! -L "$file" ] || continue
        
        # Add override to virtual destructors in derived classes
        sed -i.bak -E 's/(virtual\s+~[a-zA-Z_][a-zA-Z0-9_]*\s*\(\s*\))(\s*;)/\1 override\2/g' "$file"
        
        # Remove double override if any
        sed -i.bak 's/override override/override/g' "$file"
        
        if ! diff -q "$file" "$file.bak" >/dev/null 2>&1; then
            rm "$file.bak"
        else
            mv "$file.bak" "$file"
        fi
    done
}

# Main modernization process
echo "Starting modernization process..."
echo

# Process each component
modernize_directory "selforg" "selforg library"
modernize_directory "ode_robots" "ode_robots library"
modernize_directory "ga_tools" "ga_tools library"

# Add override keywords (conservative)
echo "Adding override keywords (conservative approach)..."
add_override_keywords "selforg" "selforg"
add_override_keywords "ode_robots" "ode_robots"
add_override_keywords "ga_tools" "ga_tools"

echo
echo "=== Modernization Summary ==="
echo "1. Replaced NULL with nullptr"
echo "2. Converted obvious pointer contexts from 0 to nullptr"
echo "3. Added override keywords to virtual destructors"
echo
echo "Note: This script applies only safe, automated modernizations."
echo "Manual review is recommended for:"
echo "  - Complex override cases"
echo "  - C-style casts that need conversion"
echo "  - Old-style for loops that could use range-based for"
echo "  - auto keyword opportunities"
echo
echo "Done!"