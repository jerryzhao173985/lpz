#!/bin/bash

# Fix "explicit if" errors in ga_tools files
echo "Fixing 'explicit if' syntax errors..."

# Fix in ga_tools files
find ga_tools -name "*.cpp" -exec sed -i '' 's/explicit if(/if (/g' {} \;

echo "Done fixing 'explicit if' syntax errors."