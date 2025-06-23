#!/bin/bash

# Script to revert ALL controller files that have been corrupted

echo "Reverting all corrupted controller files..."

cd /Users/jerry/lpzrobot_mac

# Find all .cpp and .h files in controller directory with 'static_cast' corruption pattern
echo "Finding corrupted files..."
grep -l "static_cast<.*>(" selforg/controller/*.cpp selforg/controller/*.h 2>/dev/null | while read file; do
    echo "Reverting $file..."
    git checkout -- "$file"
done

# Also revert files with other corruption patterns
grep -l "confstatic_cast\|ifstatic_cast\|forstatic_cast" selforg/controller/*.cpp selforg/controller/*.h 2>/dev/null | while read file; do
    echo "Reverting $file..."
    git checkout -- "$file"
done

# List of known problematic files to revert
PROBLEM_FILES=(
    "selforg/controller/homeokinbase.h"
    "selforg/controller/dep.cpp" 
    "selforg/controller/dep.h"
    "selforg/controller/derbigcontroller.cpp"
    "selforg/controller/derbigcontroller.h"
    "selforg/controller/dercontroller.cpp"
    "selforg/controller/dercontroller.h"
    "selforg/controller/derinf.cpp"
    "selforg/controller/derinf.h"
    "selforg/controller/derlininvert.cpp"
    "selforg/controller/derlininvert.h"
    "selforg/controller/derlinunivers.cpp"
    "selforg/controller/derlinunivers.h"
    "selforg/controller/derpseudosensor.cpp"
    "selforg/controller/derpseudosensor.h"
)

for file in "${PROBLEM_FILES[@]}"; do
    if [ -f "$file" ]; then
        echo "Reverting $file..."
        git checkout -- "$file"
    fi
done

echo "Done reverting controller files."