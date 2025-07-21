#!/bin/bash
# Script to add missing override specifiers to virtual functions
# This addresses the 880 missing override specifiers found by cppcheck

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

echo -e "${BLUE}=== Adding Override Specifiers to LPZRobots ===${NC}"
echo "This script will add 'override' to virtual function declarations"
echo ""

# Backup directory
BACKUP_DIR="backup_$(date +%Y%m%d_%H%M%S)"
mkdir -p "$BACKUP_DIR"

# Function to process a file
process_file() {
    local file="$1"
    local backup_file="$BACKUP_DIR/$(basename "$file")"
    
    # Skip if file doesn't exist
    if [ ! -f "$file" ]; then
        return
    fi
    
    # Create backup
    cp "$file" "$backup_file"
    
    # Count changes
    local changes=0
    
    # Pattern 1: virtual destructor without override
    # virtual ~ClassName(); -> virtual ~ClassName() override;
    if grep -q "virtual[[:space:]]*~.*)[[:space:]]*;" "$file"; then
        perl -i -pe 's/virtual(\s+~\w+\(\s*\))\s*;/virtual$1 override;/g' "$file"
        changes=$((changes + $(grep -c "override" "$file" 2>/dev/null || echo 0) - $(grep -c "override" "$backup_file" 2>/dev/null || echo 0)))
    fi
    
    # Pattern 2: virtual void/int/bool/etc function without override
    # virtual void funcName(...); -> virtual void funcName(...) override;
    perl -i -pe 's/virtual\s+(void|int|bool|double|float|const\s+\w+\*?|unsigned|size_t|matrix::Matrix)\s+(\w+\s*\([^)]*\))\s*;/virtual $1 $2 override;/g' "$file"
    
    # Pattern 3: virtual function returning pointer without override
    perl -i -pe 's/virtual\s+(\w+\s*\*+)\s*(\w+\s*\([^)]*\))\s*;/virtual $1 $2 override;/g' "$file"
    
    # Pattern 4: const virtual functions
    perl -i -pe 's/virtual\s+([\w\s\*]+)\s+(\w+\s*\([^)]*\))\s*const\s*;/virtual $1 $2 const override;/g' "$file"
    
    # Count actual changes
    if diff -q "$file" "$backup_file" > /dev/null; then
        rm "$backup_file"
        return 0
    else
        changes=$(diff "$backup_file" "$file" | grep -c "^>" || true)
        echo -e "${GREEN}Modified${NC}: $file (${changes} changes)"
        return $changes
    fi
}

# Components to process
COMPONENTS=(
    "selforg/controller"
    "selforg/utils"
    "ode_robots/robots"
    "ode_robots/sensors"
    "ode_robots/motors"
    "ga_tools"
)

total_files=0
total_changes=0

echo -e "\n${YELLOW}Processing files...${NC}"

for component in "${COMPONENTS[@]}"; do
    echo -e "\n${BLUE}Component: $component${NC}"
    
    # Find all header files
    while IFS= read -r -d '' file; do
        if process_file "$file"; then
            total_changes=$((total_changes + $?))
        fi
        total_files=$((total_files + 1))
    done < <(find "$component" -name "*.h" -o -name "*.hpp" -print0 2>/dev/null || true)
done

echo -e "\n${GREEN}=== Summary ===${NC}"
echo "Total files processed: $total_files"
echo "Total changes made: $total_changes"
echo "Backup directory: $BACKUP_DIR"

# Verify changes with cppcheck
echo -e "\n${YELLOW}Verifying changes with cppcheck...${NC}"
echo "Running quick check on selforg to verify improvements..."

# Run cppcheck on a sample to show improvement
sample_files="selforg/controller/abstractcontroller.h selforg/controller/sox.h selforg/controller/dep.h"
echo "Checking sample files: $sample_files"

cppcheck --enable=style --std=c++17 --suppress=missingInclude --template='{file}:{line}: {message}' $sample_files 2>&1 | grep -i "override" || echo "No missing override warnings in sample files!"

echo -e "\n${GREEN}Complete!${NC}"
echo "Next steps:"
echo "1. Review changes in your IDE/editor"
echo "2. Build and test the project"
echo "3. Run full cppcheck analysis to verify all issues are resolved"
echo "4. Commit changes with message: 'refactor: Add missing override specifiers (880 instances)'"

# Create a summary report
cat > "$BACKUP_DIR/CHANGES_SUMMARY.md" << EOF
# Override Specifier Migration Summary
Date: $(date)

## Files Modified
$(find . -newer "$BACKUP_DIR" -name "*.h" -o -name "*.hpp" | grep -v "$BACKUP_DIR" | sort)

## Backup Location
All original files backed up to: $BACKUP_DIR

## Verification
Run the following to verify all override issues are resolved:
\`\`\`bash
cppcheck --enable=style --std=c++17 --suppress=missingInclude selforg ode_robots ga_tools 2>&1 | grep -c "override"
\`\`\`

## Rollback
To rollback changes:
\`\`\`bash
for file in $BACKUP_DIR/*; do
    original="\$(find . -name "\$(basename "\$file")" | grep -v "$BACKUP_DIR" | head -1)"
    if [ -n "\$original" ]; then
        cp "\$file" "\$original"
    fi
done
\`\`\`
EOF

echo -e "\nSummary report created: ${BLUE}$BACKUP_DIR/CHANGES_SUMMARY.md${NC}"