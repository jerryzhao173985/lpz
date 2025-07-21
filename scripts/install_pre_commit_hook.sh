#!/bin/bash
# Script to install cppcheck pre-commit hook for C++17 compliance

set -e

# Colors
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

echo -e "${BLUE}=== Installing Cppcheck Pre-commit Hook ===${NC}"

# Check if .git exists
if [ ! -d ".git" ]; then
    echo "Error: Not in a git repository root"
    exit 1
fi

# Create hooks directory if it doesn't exist
mkdir -p .git/hooks

# Create the pre-commit hook
cat > .git/hooks/pre-commit << 'EOF'
#!/bin/bash
# Pre-commit hook for cppcheck C++17 compliance

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

# Check if cppcheck is installed
if ! command -v cppcheck &> /dev/null; then
    echo -e "${YELLOW}Warning: cppcheck not installed. Skipping static analysis.${NC}"
    exit 0
fi

# Get list of C++ files to be committed
files=$(git diff --cached --name-only --diff-filter=ACM | grep -E '\.(cpp|h|hpp)$' || true)

if [ -z "$files" ]; then
    exit 0
fi

echo -e "${YELLOW}Running cppcheck on staged C++ files...${NC}"

# Run cppcheck with specific checks
issues_found=0
temp_file=$(mktemp)

for file in $files; do
    if [ -f "$file" ]; then
        # Check for critical issues
        cppcheck --enable=warning,style,performance \
                 --std=c++17 \
                 --suppress=missingInclude \
                 --suppress=unmatchedSuppression \
                 --suppress=unusedFunction \
                 --template='{file}:{line}: [{severity}] {message}' \
                 --quiet \
                 "$file" 2>&1 | tee -a "$temp_file"
        
        # Check specific modernization issues
        if grep -q "\bNULL\b" "$file"; then
            echo "$file: [style] Use nullptr instead of NULL"
            issues_found=1
        fi
        
        if grep -q "typedef.*;" "$file" && ! grep -q "typedef.*(" "$file"; then
            echo "$file: [style] Consider using 'using' instead of 'typedef'"
        fi
    fi
done

# Check for critical issues
if grep -E "\[error\]|\[warning\]" "$temp_file" > /dev/null 2>&1; then
    issues_found=1
fi

# Check for missing override specifiers
if grep -i "override" "$temp_file" | grep -i "missing" > /dev/null 2>&1; then
    echo -e "\n${RED}Critical: Missing override specifiers detected${NC}"
    echo "Run ./scripts/add_override_specifiers.sh to fix automatically"
    issues_found=1
fi

rm -f "$temp_file"

if [ $issues_found -eq 1 ]; then
    echo -e "\n${RED}Commit aborted due to cppcheck issues.${NC}"
    echo "Please fix the issues above and try again."
    echo -e "\n${YELLOW}Helpful scripts:${NC}"
    echo "  ./scripts/add_override_specifiers.sh - Add missing override"
    echo "  ./scripts/nullptr_migration.sh - Replace NULL with nullptr"
    exit 1
fi

echo -e "${GREEN}Cppcheck passed!${NC}"
exit 0
EOF

# Make the hook executable
chmod +x .git/hooks/pre-commit

echo -e "${GREEN}Pre-commit hook installed successfully!${NC}"
echo ""
echo "The hook will:"
echo "  - Run cppcheck on staged C++ files"
echo "  - Check for NULL usage (should be nullptr)"
echo "  - Check for missing override specifiers"
echo "  - Check for typedef usage (should be using)"
echo "  - Block commits with errors or warnings"
echo ""
echo -e "${YELLOW}To bypass the hook (not recommended):${NC}"
echo "  git commit --no-verify"
echo ""
echo -e "${BLUE}To test the hook:${NC}"
echo "  1. Make a change to a C++ file"
echo "  2. git add <file>"
echo "  3. git commit -m 'test'"