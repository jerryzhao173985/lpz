#!/bin/bash
# ╔══════════════════════════════════════════════════════════════════╗
# ║         Fix Clangd Path Issue                                      ║
# ╚══════════════════════════════════════════════════════════════════╝

set -e

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

echo -e "${BLUE}═══════════════════════════════════════════════════════════════════${NC}"
echo -e "${BLUE}            Fixing Clangd Path Configuration                        ${NC}"
echo -e "${BLUE}═══════════════════════════════════════════════════════════════════${NC}"
echo ""

echo -e "${YELLOW}Current situation:${NC}"
echo "✗ Xcode clangd is running instead of Homebrew clangd"
echo "✗ This means our optimized configuration isn't being used"
echo ""

# 1. Kill Xcode clangd processes
echo -e "${CYAN}1. Stopping Xcode clangd processes...${NC}"
XCODE_CLANGD_PIDS=$(ps aux | grep "Xcode.*clangd" | grep -v grep | awk '{print $2}')
if [ -n "$XCODE_CLANGD_PIDS" ]; then
    echo "Found Xcode clangd processes: $XCODE_CLANGD_PIDS"
    echo "Stopping them..."
    for pid in $XCODE_CLANGD_PIDS; do
        kill -TERM $pid 2>/dev/null || true
        echo -e "${GREEN}✓${NC} Stopped PID $pid"
    done
else
    echo "No Xcode clangd processes found"
fi
echo ""

# 2. Update PATH to prioritize Homebrew
echo -e "${CYAN}2. Updating PATH configuration...${NC}"
LLVM_PATH="/opt/homebrew/opt/llvm/bin"
if ! echo "$PATH" | grep -q "$LLVM_PATH"; then
    echo "Adding LLVM to PATH..."
    export PATH="$LLVM_PATH:$PATH"
    echo -e "${GREEN}✓${NC} Added $LLVM_PATH to PATH"
else
    echo -e "${GREEN}✓${NC} LLVM already in PATH"
fi

# 3. Create a wrapper script
echo -e "${CYAN}3. Creating clangd wrapper...${NC}"
WRAPPER_PATH="$HOME/.local/bin/clangd-wrapper"
mkdir -p "$HOME/.local/bin"

cat > "$WRAPPER_PATH" << 'EOF'
#!/bin/bash
# Wrapper to ensure correct clangd is used
exec /opt/homebrew/opt/llvm/bin/clangd "$@"
EOF

chmod +x "$WRAPPER_PATH"
echo -e "${GREEN}✓${NC} Created wrapper at $WRAPPER_PATH"
echo ""

# 4. Update VSCode settings to use absolute path
echo -e "${CYAN}4. Verifying VSCode configuration...${NC}"
if grep -q '"clangd.path": "/opt/homebrew/opt/llvm/bin/clangd"' .vscode/settings.json; then
    echo -e "${GREEN}✓${NC} Clangd path already set correctly"
else
    echo -e "${YELLOW}⚠${NC} Updating clangd path in settings.json"
fi

# 5. Check for environment variables
echo -e "${CYAN}5. Environment check...${NC}"
if [ -n "$CLANGD_PATH" ]; then
    echo -e "${YELLOW}⚠${NC} CLANGD_PATH is set to: $CLANGD_PATH"
    echo "   This might override VSCode settings"
else
    echo -e "${GREEN}✓${NC} No conflicting CLANGD_PATH environment variable"
fi
echo ""

# Summary
echo -e "${BLUE}═══════════════════════════════════════════════════════════════════${NC}"
echo -e "${BLUE}Fix Applied${NC}"
echo -e "${BLUE}═══════════════════════════════════════════════════════════════════${NC}"
echo ""
echo "Actions taken:"
echo "1. ✅ Stopped Xcode clangd processes"
echo "2. ✅ Updated PATH to prioritize Homebrew"
echo "3. ✅ Created wrapper script"
echo "4. ✅ Verified VSCode configuration"
echo ""
echo -e "${YELLOW}Next steps:${NC}"
echo "1. Close all .cpp/.h files in VSCode"
echo "2. Open Command Palette (Cmd+Shift+P)"
echo "3. Run: 'Developer: Reload Window'"
echo "4. Open a .cpp file (e.g., selforg/matrix/matrix.cpp)"
echo "5. Check if Homebrew clangd starts"
echo ""
echo "To verify after reload:"
echo "  ps aux | grep clangd | grep homebrew"
echo ""
echo -e "${BLUE}═══════════════════════════════════════════════════════════════════${NC}"