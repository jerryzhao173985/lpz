#!/bin/bash
# Script to replace NULL with nullptr for C++17 modernization
# Safe replacement with verification

set -e

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

echo -e "${BLUE}=== NULL to nullptr Migration Script ===${NC}"
echo "This script will safely replace NULL with nullptr"
echo ""

# Backup directory
BACKUP_DIR="backup_nullptr_$(date +%Y%m%d_%H%M%S)"
mkdir -p "$BACKUP_DIR"

# Components to process
COMPONENTS=(
    "selforg"
    "ode_robots"
    "ga_tools"
    "ecbrobots"
    "real_robots"
)

# Function to process a file
process_file() {
    local file="$1"
    local backup_file="$BACKUP_DIR/$(echo "$file" | tr '/' '_')"
    
    # Skip binary files
    if ! file "$file" | grep -q "text"; then
        return 0
    fi
    
    # Check if file contains NULL
    if ! grep -q "NULL" "$file"; then
        return 0
    fi
    
    # Skip if file already uses nullptr extensively
    if grep -q "nullptr" "$file"; then
        echo -e "${YELLOW}Warning${NC}: $file already contains nullptr. Skipping to avoid conflicts."
        return 0
    fi
    
    # Create backup
    cp "$file" "$backup_file"
    
    # Count NULL occurrences
    local null_count=$(grep -o "NULL" "$file" | wc -l)
    
    # Perform replacements with context checking
    # Pattern 1: NULL in assignments and comparisons
    perl -i -pe 's/(\s*=\s*)NULL(\s*[;,)])/\1nullptr\2/g' "$file"
    perl -i -pe 's/(\s*==\s*)NULL(\s*)/\1nullptr\2/g' "$file"
    perl -i -pe 's/(\s*!=\s*)NULL(\s*)/\1nullptr\2/g' "$file"
    perl -i -pe 's/NULL(\s*==\s*)/nullptr\1/g' "$file"
    perl -i -pe 's/NULL(\s*!=\s*)/nullptr\1/g' "$file"
    
    # Pattern 2: NULL in function arguments
    perl -i -pe 's/\(([^)]*,\s*)NULL(\s*[,)])/\(\1nullptr\2/g' "$file"
    perl -i -pe 's/\(\s*NULL(\s*[,)])/\(nullptr\1/g' "$file"
    
    # Pattern 3: NULL in return statements
    perl -i -pe 's/return\s+NULL\s*;/return nullptr;/g' "$file"
    
    # Pattern 4: NULL in conditionals
    perl -i -pe 's/if\s*\(\s*NULL\s*\)/if (nullptr)/g' "$file"
    perl -i -pe 's/if\s*\(\s*([^)]+)\s*==\s*NULL\s*\)/if (\1 == nullptr)/g' "$file"
    perl -i -pe 's/if\s*\(\s*([^)]+)\s*!=\s*NULL\s*\)/if (\1 != nullptr)/g' "$file"
    
    # Pattern 5: NULL in ternary operators
    perl -i -pe 's/\?\s*NULL\s*:/? nullptr :/g' "$file"
    perl -i -pe 's/:\s*NULL\s*([;,)])/: nullptr\1/g' "$file"
    
    # Pattern 6: NULL in initializers
    perl -i -pe 's/\{\s*NULL\s*\}/{nullptr}/g' "$file"
    
    # Count changes
    if diff -q "$file" "$backup_file" > /dev/null; then
        rm "$backup_file"
        return 0
    else
        local new_null_count=$(grep -o "NULL" "$file" | wc -l || echo 0)
        local changes=$((null_count - new_null_count))
        echo -e "${GREEN}Modified${NC}: $file (replaced $changes NULL occurrences)"
        return 1
    fi
}

# Function to verify changes
verify_file() {
    local file="$1"
    
    # Check for remaining NULL (excluding NOTNULL, NULL_, etc.)
    local remaining=$(grep -o "\bNULL\b" "$file" | wc -l || echo 0)
    
    if [ "$remaining" -gt 0 ]; then
        echo -e "${YELLOW}Warning${NC}: $file still contains $remaining NULL occurrences"
        grep -n "\bNULL\b" "$file" | head -5
    fi
}

# Main processing
total_files=0
modified_files=0

echo -e "\n${YELLOW}Processing files...${NC}"

for component in "${COMPONENTS[@]}"; do
    echo -e "\n${BLUE}Component: $component${NC}"
    
    # Process C++ source files
    while IFS= read -r -d '' file; do
        if process_file "$file"; then
            modified_files=$((modified_files + 1))
            verify_file "$file"
        fi
        total_files=$((total_files + 1))
    done < <(find "$component" \( -name "*.cpp" -o -name "*.h" -o -name "*.hpp" \) -print0 2>/dev/null || true)
done

echo -e "\n${GREEN}=== Summary ===${NC}"
echo "Total files scanned: $total_files"
echo "Files modified: $modified_files"
echo "Backup directory: $BACKUP_DIR"

# Create verification script
cat > "$BACKUP_DIR/verify_nullptr.sh" << 'EOF'
#!/bin/bash
# Verification script for nullptr migration

echo "Checking for remaining NULL usage..."
echo "======================================"

# Find remaining NULL (excluding false positives)
grep -r "\bNULL\b" --include="*.cpp" --include="*.h" --include="*.hpp" \
    selforg ode_robots ga_tools 2>/dev/null | \
    grep -v "NOTNULL" | \
    grep -v "NULL_" | \
    grep -v "_NULL" | \
    grep -v "ifdef.*NULL" | \
    grep -v "ifndef.*NULL" | \
    grep -v "define.*NULL" | \
    head -20

echo ""
echo "If no output above, migration is complete!"
EOF

chmod +x "$BACKUP_DIR/verify_nullptr.sh"

# Create rollback script
cat > "$BACKUP_DIR/rollback.sh" << EOF
#!/bin/bash
# Rollback script for nullptr migration

echo "Rolling back nullptr changes..."
for backup in $BACKUP_DIR/*; do
    if [[ "\$backup" == *.sh ]] || [[ "\$backup" == *.md ]]; then
        continue
    fi
    original=\$(echo "\$backup" | sed "s|$BACKUP_DIR/||" | tr '_' '/')
    if [ -f "\$original" ]; then
        cp "\$backup" "\$original"
        echo "Restored: \$original"
    fi
done
echo "Rollback complete!"
EOF

chmod +x "$BACKUP_DIR/rollback.sh"

# Final report
cat > "$BACKUP_DIR/MIGRATION_REPORT.md" << EOF
# nullptr Migration Report
Date: $(date)

## Statistics
- Files scanned: $total_files
- Files modified: $modified_files
- Backup location: $BACKUP_DIR

## Verification
Run the verification script to check for remaining NULL usage:
\`\`\`bash
$BACKUP_DIR/verify_nullptr.sh
\`\`\`

## Build Test
After migration, test the build:
\`\`\`bash
cmake --build build --target selforg
\`\`\`

## Rollback
If needed, rollback all changes:
\`\`\`bash
$BACKUP_DIR/rollback.sh
\`\`\`

## Commit Message
\`\`\`
refactor: Replace NULL with nullptr for C++11/17 compliance

- Replaced NULL with nullptr in $modified_files files
- Improves type safety and modernizes codebase
- Part of C++17 migration effort
\`\`\`
EOF

echo -e "\n${GREEN}Migration complete!${NC}"
echo -e "Report saved to: ${BLUE}$BACKUP_DIR/MIGRATION_REPORT.md${NC}"
echo -e "\n${YELLOW}Next steps:${NC}"
echo "1. Run verification: $BACKUP_DIR/verify_nullptr.sh"
echo "2. Build and test the project"
echo "3. Review changes in your IDE"
echo "4. Commit if all tests pass"