#!/bin/bash

# Script to find and revert ALL files corrupted by the failed cast replacement script

echo "Finding and reverting all corrupted files..."

cd /Users/jerry/lpzrobot_mac

# Patterns that indicate corruption from the cast script
CORRUPTION_PATTERNS=(
    "static_cast<C>("
    "static_cast<CC>("
    "static_cast<CCC>("
    "ifstatic_cast"
    "forstatic_cast"
    "confstatic_cast"
    "buffersizestatic_cast"
    "interstatic_cast"
    "floatstatic_cast"
    "doublestatic_cast"
    "voidstatic_cast"
    "size_tstatic_cast"
)

# Find all .cpp and .h files with corruption patterns
echo "Searching for corrupted files..."
CORRUPTED_FILES=()

for pattern in "${CORRUPTION_PATTERNS[@]}"; do
    echo "Checking for pattern: $pattern"
    while IFS= read -r file; do
        if [[ ! " ${CORRUPTED_FILES[@]} " =~ " ${file} " ]]; then
            CORRUPTED_FILES+=("$file")
        fi
    done < <(grep -l "$pattern" selforg/**/*.{cpp,h} ode_robots/**/*.{cpp,h} utils/**/*.{cpp,h} ga_tools/**/*.{cpp,h} 2>/dev/null || true)
done

echo "Found ${#CORRUPTED_FILES[@]} corrupted files"

# Revert each corrupted file
for file in "${CORRUPTED_FILES[@]}"; do
    echo "Reverting: $file"
    git checkout -- "$file"
done

echo "Done reverting corrupted files."

# List files that are still modified
echo ""
echo "Files still modified after revert:"
git status --porcelain | grep -E "\.cpp|\.h" | grep -v "scripts/"