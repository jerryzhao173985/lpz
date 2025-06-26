#!/bin/bash
# Fix misplaced explicit keywords in matrixviz

cd /Users/jerry/lpzrobot_mac/matrixviz/src

# Remove explicit from method declarations (not constructors)
find . -name "*.h" -o -name "*.cpp" | while read file; do
    # Backup original
    cp "$file" "$file.bak"
    
    # Fix patterns like "void explicit", "double explicit", "T explicit", etc.
    sed -i '' -E 's/(void|double|float|int|bool|QString|QColor|string|charArray|T|VectorElementPlotChannel\*|GroupPlotChannel\*|VectorPlotChannel\*|MatrixPlotChannel\*|AbstractPlotChannel\*) explicit /\1 /g' "$file"
    
    # Fix "virtual void explicit"
    sed -i '' -E 's/virtual (void|double|float|int|bool|QString|QColor|string|charArray|T|VectorElementPlotChannel\*|GroupPlotChannel\*|VectorPlotChannel\*|MatrixPlotChannel\*|AbstractPlotChannel\*) explicit /virtual \1 /g' "$file"
    
    # Fix misplaced explicit in if statements
    sed -i '' 's/explicit if (/if (/g' "$file"
    
    # Clean up if no changes
    if diff "$file" "$file.bak" > /dev/null; then
        rm "$file.bak"
    else
        echo "Fixed: $file"
        rm "$file.bak"
    fi
done

echo "Done fixing explicit keywords"