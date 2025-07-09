#!/bin/bash
# Fix misplaced explicit keywords on if statements

find . -name "*.cpp" -o -name "*.h" | while read file; do
    if grep -q "explicit if" "$file"; then
        echo "Fixing $file"
        sed -i '' 's/explicit if(/if(/g' "$file"
    fi
done

echo "Fixed all explicit if statements"