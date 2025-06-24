#!/bin/bash

# Master script to fix all ga_tools issues
# Author: Assistant
# Date: 2025-06-24

set -e  # Exit on error

echo "==================================================================="
echo "GA_TOOLS COMPREHENSIVE FIX SCRIPT"
echo "==================================================================="
echo "This script will fix all known issues in ga_tools:"
echo "- Remove corrupted files with '.!' in names"
echo "- Fix 'const const' syntax errors"
echo "- Fix uninitialized variables"
echo "- Fix AST/syntax errors"
echo "- Apply const correctness"
echo "==================================================================="

# Check if we're in the right directory
if [ ! -d "ga_tools" ]; then
    echo "Error: ga_tools directory not found!"
    echo "Please run this script from the lpzrobots directory"
    exit 1
fi

# Create timestamped backup
TIMESTAMP=$(date +%Y%m%d_%H%M%S)
BACKUP_DIR="ga_tools_backup_$TIMESTAMP"
echo -e "\n[BACKUP] Creating backup: $BACKUP_DIR"
cp -r ga_tools "$BACKUP_DIR"
echo "Backup created successfully"

# Counter for fixes
FIX_COUNT=0

# 1. Remove corrupted files
echo -e "\n[FIX 1] Removing corrupted files with '.!' in names..."
CORRUPTED_COUNT=$(find ga_tools -name "*.!*" -type f 2>/dev/null | wc -l | tr -d ' ')
if [ "$CORRUPTED_COUNT" -gt 0 ]; then
    find ga_tools -name "*.!*" -type f -delete
    echo "✓ Removed $CORRUPTED_COUNT corrupted files"
    ((FIX_COUNT++))
else
    echo "✓ No corrupted files found"
fi

# 2. Fix "const const" syntax errors
echo -e "\n[FIX 2] Fixing 'const const' syntax errors..."
CONST_CONST_FILES=$(find ga_tools \( -name "*.h" -o -name "*.cpp" \) -exec grep -l "const const" {} \; 2>/dev/null | wc -l | tr -d ' ')
if [ "$CONST_CONST_FILES" -gt 0 ]; then
    find ga_tools \( -name "*.h" -o -name "*.cpp" \) -exec grep -l "const const" {} \; | while read -r file; do
        sed -i '' 's/const const/const/g' "$file"
        echo "  Fixed: $file"
        ((FIX_COUNT++))
    done
    echo "✓ Fixed $CONST_CONST_FILES files"
else
    echo "✓ No 'const const' errors found"
fi

# 3. Fix uninitialized member variables in StandartGenerationSizeStrategy
echo -e "\n[FIX 3] Fixing uninitialized member variables..."
if [ -f "ga_tools/GenerationSizeStrategies/StandartGenerationSizeStrategy.cpp" ]; then
    # Check if already initialized
    if ! grep -q "m_best_first = 0.0;" "ga_tools/GenerationSizeStrategies/StandartGenerationSizeStrategy.cpp"; then
        sed -i '' '/m_firstIsSet = false;/a\
\	m_best_first = 0.0;\
\	m_best_new = 0.0;\
\	m_best_old = 0.0;' "ga_tools/GenerationSizeStrategies/StandartGenerationSizeStrategy.cpp"
        echo "✓ Fixed member initialization in StandartGenerationSizeStrategy.cpp"
        ((FIX_COUNT++))
    else
        echo "✓ Member variables already initialized"
    fi
fi

# 4. Fix uninitialized 'fit' variables
echo -e "\n[FIX 4] Fixing uninitialized 'fit' variables..."
FILES_TO_FIX=(
    "ga_tools/simulations/template_tasked_GA_Simulation/main.cpp"
    "ga_tools/examples/robot_chain_tasked_GA_Simulation/main.cpp"
)
for file in "${FILES_TO_FIX[@]}"; do
    if [ -f "$file" ]; then
        if grep -q "double fit;" "$file"; then
            sed -i '' 's/double fit;/double fit = 0.0;/' "$file"
            echo "✓ Fixed uninitialized 'fit' in: $(basename "$file")"
            ((FIX_COUNT++))
        fi
    fi
done

# 5. Fix AST errors (malformed sizeof and sqrt casts)
echo -e "\n[FIX 5] Fixing AST/syntax errors..."

# Fix SingletonGenEngine.cpp
if [ -f "ga_tools/SingletonGenEngine.cpp" ]; then
    if grep -q "static_cast<int>(sizeof)(RESTORE_GA_TEMPLATE" "ga_tools/SingletonGenEngine.cpp"; then
        sed -i '' 's/static_cast<int>(sizeof)(RESTORE_GA_TEMPLATE<int>)/static_cast<int>(sizeof(RESTORE_GA_TEMPLATE<int>))/' "ga_tools/SingletonGenEngine.cpp"
        echo "✓ Fixed AST error in SingletonGenEngine.cpp"
        ((FIX_COUNT++))
    fi
fi

# Fix template_cycled_GA_Simulation/main.cpp
if [ -f "ga_tools/simulations/template_cycled_GA_Simulation/main.cpp" ]; then
    if grep -q "static_cast<int>(sqrt)(numberIndividuals)" "ga_tools/simulations/template_cycled_GA_Simulation/main.cpp"; then
        sed -i '' 's/static_cast<int>(sqrt)(numberIndividuals)/static_cast<int>(sqrt(numberIndividuals))/g' "ga_tools/simulations/template_cycled_GA_Simulation/main.cpp"
        echo "✓ Fixed syntax error in template_cycled_GA_Simulation/main.cpp"
        ((FIX_COUNT++))
    fi
fi

# 6. Fix virtual destructor override syntax (for C++11 compatibility)
echo -e "\n[FIX 6] Fixing virtual destructor syntax..."
OVERRIDE_COUNT=$(find ga_tools -name "*.h" -exec grep -l "virtual ~.*override" {} \; 2>/dev/null | wc -l | tr -d ' ')
if [ "$OVERRIDE_COUNT" -gt 0 ]; then
    find ga_tools -name "*.h" -exec grep -l "virtual ~.*override" {} \; | while read -r file; do
        sed -i '' 's/virtual ~\([^)]*\) override;/virtual ~\1;/g' "$file"
    done
    echo "✓ Fixed $OVERRIDE_COUNT files with override on destructors"
    ((FIX_COUNT++))
fi

# 7. Generate detailed report
echo -e "\n[REPORT] Generating fix report..."
cat > ga_tools/FIX_REPORT_${TIMESTAMP}.txt << EOF
GA_TOOLS FIX REPORT
==================
Timestamp: $(date)
Total fixes applied: $FIX_COUNT
Backup location: $BACKUP_DIR

SUMMARY OF FIXES:
1. Corrupted files removed: $CORRUPTED_COUNT
2. 'const const' errors fixed: $CONST_CONST_FILES files
3. Uninitialized member variables: Fixed in StandartGenerationSizeStrategy
4. Uninitialized 'fit' variables: Fixed in simulation files
5. AST/syntax errors: Fixed in SingletonGenEngine.cpp and template_cycled_GA_Simulation
6. Virtual destructor syntax: Fixed $OVERRIDE_COUNT files

NEXT STEPS:
1. cd ga_tools
2. make clean
3. make
4. If compilation succeeds, run: cppcheck --enable=all --std=c++17 .

RESTORATION:
If needed, restore from backup:
  rm -rf ga_tools
  mv $BACKUP_DIR ga_tools

EOF

echo "✓ Report saved to: ga_tools/FIX_REPORT_${TIMESTAMP}.txt"

# 8. Final summary
echo -e "\n==================================================================="
echo "FIX SUMMARY"
echo "==================================================================="
echo "Total fixes applied: $FIX_COUNT"
echo "Backup created in: $BACKUP_DIR"
echo "Report saved to: ga_tools/FIX_REPORT_${TIMESTAMP}.txt"
echo ""
echo "To test the fixes:"
echo "  cd ga_tools"
echo "  make clean && make"
echo ""
echo "To verify with cppcheck:"
echo "  cd ga_tools"
echo "  cppcheck --enable=all --std=c++17 --suppress=missingInclude ."
echo "==================================================================="