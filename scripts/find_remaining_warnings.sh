#!/bin/bash
# Script to find all remaining warnings

echo "=== Finding Remaining Warnings ==="

# C-style casts
echo -e "\n1. C-style casts still remaining:"
find . -name "*.cpp" -o -name "*.h" | \
    grep -v build | grep -v ".git" | \
    xargs grep -h "([A-Za-z_].*\*)" | \
    grep -v "static_cast" | grep -v "reinterpret_cast" | \
    grep -v "dynamic_cast" | grep -v "const_cast" | \
    grep -v "^\s*//" | \
    wc -l

# Missing override keywords
echo -e "\n2. Virtual functions without override:"
grep -r "virtual.*(" . --include="*.h" | \
    grep -v "override" | grep -v "= 0" | \
    grep -v build | grep -v ".git" | \
    wc -l

# Uninitialized members (common patterns)
echo -e "\n3. Potential uninitialized members:"
grep -r "class\|struct" . --include="*.h" -A 20 | \
    grep -E "^\s*(int|float|double|bool|size_t|unsigned)\s+\w+;" | \
    grep -v "static" | wc -l

# Const correctness in range loops
echo -e "\n4. Range loops without const ref:"
grep -r "for\s*(\s*auto\s\+\w\+\s*:" . --include="*.cpp" --include="*.h" | \
    grep -v "auto&" | grep -v "const auto&" | \
    wc -l

# Shadow variables
echo -e "\n5. Potential shadow variables:"
cppcheck --enable=style --quiet . 2>&1 | grep "shadows" | wc -l

# Buffer overflows
echo -e "\n6. Potential buffer issues:"
grep -r "sprintf\|strcpy\|strcat" . --include="*.cpp" --include="*.h" | \
    grep -v build | wc -l

echo -e "\n=== Top files with warnings ==="
# Find files with most C-style casts
echo -e "\nFiles with most C-style casts:"
find . -name "*.cpp" -o -name "*.h" | \
    grep -v build | grep -v ".git" | \
    while read f; do
        count=$(grep -c "([A-Za-z_].*\*)" "$f" 2>/dev/null || echo 0)
        if [ $count -gt 0 ]; then
            echo "$count $f"
        fi
    done | sort -nr | head -10