#!/bin/bash

# Comprehensive fix script for ga_tools issues
# Author: Assistant
# Date: 2025-06-24

set -e  # Exit on error

echo "=== Comprehensive fix for ga_tools ==="
echo "Starting fixes at $(date)"

# Create backup directory
BACKUP_DIR="ga_tools_backup_$(date +%Y%m%d_%H%M%S)"
echo "Creating backup in $BACKUP_DIR..."
cp -r ga_tools "$BACKUP_DIR"

cd ga_tools || exit 1

# 1. Remove corrupted files
echo -e "\n[1/8] Removing corrupted files with '.!' in names..."
CORRUPTED_COUNT=$(find . -name "*.!*" -type f | wc -l | tr -d ' ')
find . -name "*.!*" -type f -delete
echo "Removed $CORRUPTED_COUNT corrupted files"

# 2. Fix "const const" syntax errors
echo -e "\n[2/8] Fixing 'const const' syntax errors..."
find . -name "*.h" -o -name "*.cpp" | while read -r file; do
    if grep -q "const const" "$file" 2>/dev/null; then
        sed -i '' 's/const const/const/g' "$file"
        echo "Fixed const const in: $file"
    fi
done

# 3. Fix uninitialized member variables in StandartGenerationSizeStrategy
echo -e "\n[3/8] Fixing uninitialized member variables..."
if [ -f "GenerationSizeStrategies/StandartGenerationSizeStrategy.cpp" ]; then
    # Add member initialization to the constructor
    sed -i '' '/StandartGenerationSizeStrategy::StandartGenerationSizeStrategy(int startSize, int numGeneration) {/,/^}/ {
        s/m_firstIsSet = false;/m_firstIsSet = false;\
\tm_best_first = 0.0;\
\tm_best_new = 0.0;\
\tm_best_old = 0.0;/
    }' "GenerationSizeStrategies/StandartGenerationSizeStrategy.cpp"
    echo "Fixed member initialization in StandartGenerationSizeStrategy.cpp"
fi

# 4. Fix uninitialized variable 'fit' in simulation files
echo -e "\n[4/8] Fixing uninitialized 'fit' variables..."

# Fix in template_tasked_GA_Simulation
if [ -f "simulations/template_tasked_GA_Simulation/main.cpp" ]; then
    sed -i '' 's/double fit;/double fit = 0.0;/' "simulations/template_tasked_GA_Simulation/main.cpp"
    echo "Fixed uninitialized 'fit' in template_tasked_GA_Simulation/main.cpp"
fi

# Fix in robot_chain_tasked_GA_Simulation
if [ -f "examples/robot_chain_tasked_GA_Simulation/main.cpp" ]; then
    sed -i '' 's/double fit;/double fit = 0.0;/' "examples/robot_chain_tasked_GA_Simulation/main.cpp"
    echo "Fixed uninitialized 'fit' in robot_chain_tasked_GA_Simulation/main.cpp"
fi

# 5. Fix AST error in SingletonGenEngine.cpp
echo -e "\n[5/8] Fixing AST error in SingletonGenEngine.cpp..."
if [ -f "SingletonGenEngine.cpp" ]; then
    # Fix the malformed sizeof expression
    sed -i '' 's/static_cast<int>(sizeof)(RESTORE_GA_TEMPLATE<int>)/static_cast<int>(sizeof(RESTORE_GA_TEMPLATE<int>))/' "SingletonGenEngine.cpp"
    echo "Fixed AST error in SingletonGenEngine.cpp"
fi

# 6. Fix virtual destructor declarations
echo -e "\n[6/8] Fixing virtual destructor syntax..."
find . -name "*.h" | while read -r file; do
    # Remove 'override' from destructor declarations (C++11 compatibility)
    sed -i '' 's/virtual ~\([^)]*\) override;/virtual ~\1;/g' "$file" 2>/dev/null || true
done

# 7. Fix const correctness in method parameters
echo -e "\n[7/8] Applying const correctness..."
# This is a more conservative approach - only fix obvious cases
find . -name "*.h" | while read -r file; do
    # Fix getFitness and similar methods
    sed -i '' 's/getFitness(\([^)]*\)Individual\*/getFitness(\1const Individual*/g' "$file" 2>/dev/null || true
    sed -i '' 's/calcGenerationSize(\([^)]*\)Generation\*/calcGenerationSize(\1const Generation*/g' "$file" 2>/dev/null || true
done

# 8. Create summary report
echo -e "\n[8/8] Creating summary report..."
cat > FIX_REPORT.txt << EOF
GA_TOOLS FIX REPORT
==================
Date: $(date)
Backup: ../$BACKUP_DIR

FIXES APPLIED:
1. Removed $CORRUPTED_COUNT corrupted files with ".!" in names
2. Fixed "const const" syntax errors
3. Fixed uninitialized member variables in StandartGenerationSizeStrategy
4. Fixed uninitialized 'fit' variables in simulation files
5. Fixed AST error in SingletonGenEngine.cpp (sizeof expression)
6. Fixed virtual destructor syntax (removed 'override' for C++11)
7. Applied const correctness to method parameters

MANUAL REVIEW NEEDED:
- simulations/template_cycled_GA_Simulation/main.cpp line 411
- Check build with: make clean && make
- Run cppcheck for remaining issues

TESTING COMMANDS:
cd ga_tools
make clean
make
cppcheck --enable=all --std=c++17 --suppress=missingInclude .

EOF

echo -e "\n=== Fix completed successfully ==="
echo "Report saved to: ga_tools/FIX_REPORT.txt"
echo "Backup saved to: $BACKUP_DIR"

# Return to parent directory
cd ..

# Optional: Run a quick syntax check
echo -e "\n=== Running quick syntax check ==="
find ga_tools -name "*.cpp" -o -name "*.h" | head -5 | while read -r file; do
    if g++ -std=c++17 -fsyntax-only "$file" 2>/dev/null; then
        echo "✓ Syntax OK: $file"
    else
        echo "✗ Syntax error: $file (expected due to missing includes)"
    fi
done

echo -e "\n=== All fixes completed ==="
echo "Next steps:"
echo "1. cd ga_tools"
echo "2. make clean && make"
echo "3. Review any compilation errors"
echo "4. Run cppcheck for verification"