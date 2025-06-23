#!/bin/bash

# Script to fix remaining C++17 modernization issues

echo "=== Fixing Remaining C++17 Issues ==="

# Fix incorrect nullptr replacements for integers
echo "Fixing incorrect nullptr replacements..."
find . -name "*.cpp" -o -name "*.h" | while read -r file; do
    [ -f "$file" ] && [ ! -L "$file" ] || continue
    
    # Fix comparisons with nullptr that should be 0
    sed -i.bak -E 's/([a-zA-Z_][a-zA-Z0-9_]*Steps\s*==\s*)nullptr/\10/g' "$file"
    sed -i.bak -E 's/([a-zA-Z_][a-zA-Z0-9_]*[Cc]ount\s*==\s*)nullptr/\10/g' "$file"
    sed -i.bak -E 's/([a-zA-Z_][a-zA-Z0-9_]*[Nn]umber\s*==\s*)nullptr/\10/g' "$file"
    sed -i.bak -E 's/([a-zA-Z_][a-zA-Z0-9_]*[Ss]ize\s*==\s*)nullptr/\10/g' "$file"
    
    # Clean up backups
    if ! diff -q "$file" "$file.bak" >/dev/null 2>&1; then
        rm "$file.bak"
    else
        mv "$file.bak" "$file"
    fi
done

# Fix deprecated random_shuffle
echo "Replacing deprecated random_shuffle with shuffle..."
find . -name "*.cpp" -o -name "*.h" | while read -r file; do
    [ -f "$file" ] && [ ! -L "$file" ] || continue
    
    # Replace random_shuffle with shuffle
    if grep -q "random_shuffle" "$file"; then
        # Add <random> include if not present
        if ! grep -q "#include <random>" "$file"; then
            sed -i.bak '/#include <algorithm>/a\
#include <random>' "$file"
        fi
        
        # Replace random_shuffle calls
        sed -i.bak -E 's/std::random_shuffle\(([^,]+),\s*([^)]+)\)/std::shuffle(\1, \2, std::mt19937{std::random_device{}()})/g' "$file"
        sed -i.bak -E 's/random_shuffle\(([^,]+),\s*([^)]+)\)/std::shuffle(\1, \2, std::mt19937{std::random_device{}()})/g' "$file"
        
        if ! diff -q "$file" "$file.bak" >/dev/null 2>&1; then
            rm "$file.bak"
            echo "  Fixed random_shuffle in $file"
        else
            mv "$file.bak" "$file"
        fi
    fi
done

# Fix auto_ptr usage (deprecated in C++11, removed in C++17)
echo "Replacing deprecated auto_ptr with unique_ptr..."
find . -name "*.cpp" -o -name "*.h" | while read -r file; do
    [ -f "$file" ] && [ ! -L "$file" ] || continue
    
    if grep -q "auto_ptr" "$file"; then
        sed -i.bak 's/std::auto_ptr/std::unique_ptr/g' "$file"
        sed -i.bak 's/auto_ptr/std::unique_ptr/g' "$file"
        
        if ! diff -q "$file" "$file.bak" >/dev/null 2>&1; then
            rm "$file.bak"
            echo "  Fixed auto_ptr in $file"
        else
            mv "$file.bak" "$file"
        fi
    fi
done

# Fix bind1st/bind2nd (deprecated in C++11, removed in C++17)
echo "Replacing deprecated bind1st/bind2nd with lambdas..."
find . -name "*.cpp" -o -name "*.h" | while read -r file; do
    [ -f "$file" ] && [ ! -L "$file" ] || continue
    
    if grep -q "bind1st\|bind2nd" "$file"; then
        echo "  Warning: Found bind1st/bind2nd in $file - manual fix needed"
    fi
done

echo
echo "=== Summary ==="
echo "Fixed:"
echo "  - Incorrect nullptr replacements for integer comparisons"
echo "  - random_shuffle → shuffle with proper random engine"
echo "  - auto_ptr → unique_ptr"
echo
echo "Manual review needed for:"
echo "  - bind1st/bind2nd usage (replace with lambdas)"
echo "  - Any remaining compiler errors"
echo
echo "Done!"