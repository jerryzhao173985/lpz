#!/bin/bash
# Comprehensive script to fix all remaining warnings in LPZRobots

echo "=== Starting comprehensive warning fixes ==="
echo "This will fix:"
echo "1. Missing override specifiers"
echo "2. C-style casts"
echo "3. Uninitialized member variables"
echo "4. Variable scope issues"
echo "5. Const-correctness issues"
echo ""

# Make scripts executable
chmod +x fix_override_warnings.py
chmod +x fix_cstyle_casts.py
chmod +x fix_uninit_members_comprehensive.py

# Step 1: Fix missing override specifiers
echo "Step 1: Fixing missing override specifiers..."
python3 fix_override_warnings.py
echo ""

# Step 2: Fix C-style casts
echo "Step 2: Fixing C-style casts..."
python3 fix_cstyle_casts.py
echo ""

# Step 3: Fix uninitialized member variables
echo "Step 3: Fixing uninitialized member variables..."
python3 fix_uninit_members_comprehensive.py
echo ""

# Step 4: Test compilation
echo "Step 4: Testing compilation..."
cd selforg
make clean
make -j4 2>&1 | tee ../selforg_build_test.log
cd ..

# Count remaining warnings
echo ""
echo "=== Compilation Results ==="
if [ -f selforg_build_test.log ]; then
    echo "Counting remaining warnings..."
    grep -c "warning:" selforg_build_test.log || echo "No warnings found!"
    
    # Show summary of warning types
    echo ""
    echo "Warning types still present:"
    grep "warning:" selforg_build_test.log | sed 's/.*warning: //' | sed 's/ \[.*//' | sort | uniq -c | sort -nr | head -20
fi

echo ""
echo "=== Fix complete ==="
echo "Next steps:"
echo "1. Review the changes with 'git diff'"
echo "2. Test the simulations to ensure functionality"
echo "3. Commit the changes"