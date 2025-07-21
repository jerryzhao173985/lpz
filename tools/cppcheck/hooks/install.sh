#!/bin/bash
# Install cppcheck pre-commit hook

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/../../.." && pwd)"
GIT_HOOKS_DIR="$PROJECT_ROOT/.git/hooks"

# Colors
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

echo "Installing cppcheck pre-commit hook..."

# Check if .git directory exists
if [ ! -d "$PROJECT_ROOT/.git" ]; then
    echo "Error: Not in a git repository"
    exit 1
fi

# Create hooks directory if it doesn't exist
mkdir -p "$GIT_HOOKS_DIR"

# Install pre-commit hook
if [ -f "$GIT_HOOKS_DIR/pre-commit" ]; then
    echo -e "${YELLOW}Warning: pre-commit hook already exists${NC}"
    echo "Creating backup at $GIT_HOOKS_DIR/pre-commit.backup"
    cp "$GIT_HOOKS_DIR/pre-commit" "$GIT_HOOKS_DIR/pre-commit.backup"
fi

# Create a wrapper that calls our hook
cat > "$GIT_HOOKS_DIR/pre-commit" << EOF
#!/bin/bash
# LPZRobots pre-commit hook wrapper

# Run cppcheck analysis
$SCRIPT_DIR/pre-commit
CPPCHECK_RESULT=\$?

# Run other pre-commit hooks if they exist
if [ -f "$GIT_HOOKS_DIR/pre-commit.backup" ]; then
    "$GIT_HOOKS_DIR/pre-commit.backup"
    OTHER_RESULT=\$?
else
    OTHER_RESULT=0
fi

# Exit with error if either check failed
if [ \$CPPCHECK_RESULT -ne 0 ] || [ \$OTHER_RESULT -ne 0 ]; then
    exit 1
fi

exit 0
EOF

chmod +x "$GIT_HOOKS_DIR/pre-commit"

echo -e "${GREEN}Pre-commit hook installed successfully!${NC}"
echo "The hook will run cppcheck on staged C++ files before each commit."
echo "Use 'git commit --no-verify' to skip the check if needed."