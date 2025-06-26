#!/bin/bash
# Fix misplaced explicit keywords in configurator

cd /Users/jerry/lpzrobot_mac/configurator/src

# Fix all patterns of misplaced explicit
find . -name "*.h" -o -name "*.cpp" | while read file; do
    # Backup original
    cp "$file" "$file.bak"
    
    # Fix patterns like "void explicit", "double explicit", etc.
    sed -i '' -E 's/(void|double|float|int|bool|QString|QColor|string|charArray|virtual void|virtual double|virtual int) explicit /\1 /g' "$file"
    
    # Fix explicit if, while, switch
    sed -i '' 's/explicit if (/if (/g' "$file"
    sed -i '' 's/explicit while (/while (/g' "$file"
    sed -i '' 's/explicit switch (/switch (/g' "$file"
    
    # Clean up if no changes
    if diff "$file" "$file.bak" > /dev/null 2>&1; then
        rm "$file.bak"
    else
        echo "Fixed: $file"
        rm "$file.bak"
    fi
done

echo "Done fixing explicit keywords"