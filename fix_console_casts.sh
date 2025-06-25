#!/bin/bash
# Script to fix remaining C-style (signed) casts in console.cpp files

echo "Fixing C-style casts in console.cpp files..."

# Find all console.cpp files with (signed) casts
files=$(find . -name "console.cpp" -path "*/simulations/*" | xargs grep -l "(signed)")

for file in $files; do
    echo "Processing: $file"
    
    # Fix (signed) casts to static_cast<int>
    # Using sed with backup
    sed -i.bak 's/(signed)/static_cast<int>/g' "$file"
    
    # Check if changes were made
    if ! diff -q "$file" "$file.bak" >/dev/null; then
        echo "  ✓ Fixed casts in $file"
        rm "$file.bak"  # Remove backup if successful
    else
        echo "  - No changes needed in $file"
        rm "$file.bak"
    fi
done

echo "Done! All C-style (signed) casts have been replaced with static_cast<int>."

# Count remaining warnings
remaining=$(find . -name "*.cpp" -o -name "*.h" | xargs grep -c "(signed)" 2>/dev/null | grep -v ":0" | wc -l)
echo "Files with remaining (signed) casts: $remaining"