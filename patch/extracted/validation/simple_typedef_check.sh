#!/bin/bash

echo "Checking typedef to using conversions in C++ files..."
echo "=================================================="

# Count actual typedef removals in C++ files
typedef_count=$(grep -E "^-.*typedef.*;" patch_01_typedef_to_using.patch | \
                grep -v "\.md" | \
                grep -v "\.txt" | \
                grep -v "//" | \
                grep -v "\*" | \
                wc -l)

# Count using additions
using_count=$(grep -E "^\+.*using.*=.*;" patch_01_typedef_to_using.patch | \
              grep -v "\.md" | \
              grep -v "\.txt" | \
              wc -l)

echo "Typedef removals: $typedef_count"
echo "Using additions: $using_count"
echo ""

# Show some examples
echo "Examples of conversions:"
echo "------------------------"

# Find paired changes
grep -B1 -A1 "^-.*typedef.*;" patch_01_typedef_to_using.patch | \
    grep -v "^--$" | \
    grep -B1 -A1 "^\+.*using.*=.*;" | \
    grep -v "\.md" | \
    grep -v "\.txt" | \
    head -30