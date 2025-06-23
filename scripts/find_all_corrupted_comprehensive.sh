#!/bin/bash

# Script to find ALL corrupted files from failed cast script

echo "Finding all corrupted files comprehensively..."

cd /Users/jerry/lpzrobot_mac

# Find files with cast corruption patterns
echo "Searching for cast corruption patterns..."

# Find files with patterns like 'static_cast<something>(' or 'reinterpret_cast<something>(' without proper syntax
CORRUPTED_FILES=()

# Pattern 1: static_cast with wrong syntax
while IFS= read -r file; do
    if [[ ! " ${CORRUPTED_FILES[@]} " =~ " ${file} " ]]; then
        CORRUPTED_FILES+=("$file")
    fi
done < <(grep -rEln "(static_cast|reinterpret_cast)<[^>]+>\(" selforg/ ode_robots/ utils/ ga_tools/ 2>/dev/null | grep -E "\.(cpp|h)$" || true)

# Pattern 2: Cast in function names or variable names
while IFS= read -r file; do
    if [[ ! " ${CORRUPTED_FILES[@]} " =~ " ${file} " ]]; then
        CORRUPTED_FILES+=("$file")
    fi
done < <(grep -rEln "[a-zA-Z_]+(static_cast|reinterpret_cast)" selforg/ ode_robots/ utils/ ga_tools/ 2>/dev/null | grep -E "\.(cpp|h)$" || true)

# Pattern 3: Corrupted brackets
while IFS= read -r file; do
    if [[ ! " ${CORRUPTED_FILES[@]} " =~ " ${file} " ]]; then
        CORRUPTED_FILES+=("$file")
    fi
done < <(grep -rEln "static_cast<[^>]+>\({" selforg/ ode_robots/ utils/ ga_tools/ 2>/dev/null | grep -E "\.(cpp|h)$" || true)

# Pattern 4: sizeof corruptions
while IFS= read -r file; do
    if [[ ! " ${CORRUPTED_FILES[@]} " =~ " ${file} " ]]; then
        CORRUPTED_FILES+=("$file")
    fi
done < <(grep -rEln "sizeof(static_cast|reinterpret_cast)" selforg/ ode_robots/ utils/ ga_tools/ 2>/dev/null | grep -E "\.(cpp|h)$" || true)

# Pattern 5: if/for/while corruptions
while IFS= read -r file; do
    if [[ ! " ${CORRUPTED_FILES[@]} " =~ " ${file} " ]]; then
        CORRUPTED_FILES+=("$file")
    fi
done < <(grep -rEln "if static_cast|for static_cast|while static_cast" selforg/ ode_robots/ utils/ ga_tools/ 2>/dev/null | grep -E "\.(cpp|h)$" || true)

echo "Found ${#CORRUPTED_FILES[@]} potentially corrupted files"

# Revert each file
echo "Reverting corrupted files..."
for file in "${CORRUPTED_FILES[@]}"; do
    echo "Reverting: $file"
    git checkout -- "$file" 2>/dev/null || echo "  Could not revert $file (may be new or already clean)"
done

echo "Done."