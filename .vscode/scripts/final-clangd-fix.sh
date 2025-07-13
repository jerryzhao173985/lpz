#!/bin/bash
# ╔══════════════════════════════════════════════════════════════════╗
# ║         Final Clangd Fix - Direct Path Override                    ║
# ╚══════════════════════════════════════════════════════════════════╝

set -e

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m'

echo -e "${BLUE}═══════════════════════════════════════════════════════════════════${NC}"
echo -e "${BLUE}            Final Clangd Configuration Fix                          ${NC}"
echo -e "${BLUE}═══════════════════════════════════════════════════════════════════${NC}"
echo ""

# 1. Create a symlink in a priority location
echo -e "${CYAN}1. Creating priority symlink...${NC}"
PRIORITY_BIN="$HOME/.local/bin"
mkdir -p "$PRIORITY_BIN"

# Remove old symlink if exists
rm -f "$PRIORITY_BIN/clangd"

# Create new symlink
ln -s /opt/homebrew/opt/llvm/bin/clangd "$PRIORITY_BIN/clangd"
echo -e "${GREEN}✓${NC} Created symlink at $PRIORITY_BIN/clangd"

# 2. Update PATH in shell config
echo -e "${CYAN}2. Updating shell PATH...${NC}"
SHELL_CONFIG="$HOME/.zshrc"
if ! grep -q "/.local/bin" "$SHELL_CONFIG"; then
    echo 'export PATH="$HOME/.local/bin:$PATH"' >> "$SHELL_CONFIG"
    echo -e "${GREEN}✓${NC} Added ~/.local/bin to PATH in .zshrc"
else
    echo -e "${GREEN}✓${NC} PATH already includes ~/.local/bin"
fi

# 3. Create VSCode tasks.json for manual start
echo -e "${CYAN}3. Creating VSCode tasks configuration...${NC}"
cat > .vscode/tasks.json << 'EOF'
{
    "version": "2.0.0",
    "tasks": [
        {
            "label": "Start Homebrew Clangd",
            "type": "shell",
            "command": "/opt/homebrew/opt/llvm/bin/clangd",
            "args": [
                "--background-index",
                "--compile-commands-dir=${workspaceFolder}/build/macos-arm64",
                "--header-insertion=never",
                "--clang-tidy",
                "--malloc-trim",
                "--pch-storage=memory",
                "-j=12",
                "--limit-results=100"
            ],
            "problemMatcher": [],
            "isBackground": true,
            "presentation": {
                "reveal": "never",
                "panel": "dedicated"
            }
        },
        {
            "label": "Kill All Clangd",
            "type": "shell",
            "command": "killall",
            "args": ["clangd"],
            "problemMatcher": []
        }
    ]
}
EOF
echo -e "${GREEN}✓${NC} Created tasks.json"

# 4. Create a launch configuration
echo -e "${CYAN}4. Creating launch configuration...${NC}"
cat > .vscode/launch.json << 'EOF'
{
    "version": "0.2.0",
    "configurations": [
        {
            "name": "Debug with clangd",
            "type": "lldb",
            "request": "launch",
            "program": "${workspaceFolder}/build/macos-arm64/${fileBasenameNoExtension}",
            "args": [],
            "cwd": "${workspaceFolder}",
            "preLaunchTask": "Start Homebrew Clangd"
        }
    ]
}
EOF
echo -e "${GREEN}✓${NC} Created launch.json"

# 5. Update settings with absolute path
echo -e "${CYAN}5. Verifying settings.json...${NC}"
# Check if clangd.path uses absolute path
if grep -q '"clangd.path": "/opt/homebrew/opt/llvm/bin/clangd"' .vscode/settings.json; then
    echo -e "${GREEN}✓${NC} settings.json already has absolute path"
else
    echo -e "${YELLOW}⚠${NC} Update clangd.path in settings.json to use absolute path"
fi

# 6. Kill all clangd processes again
echo -e "${CYAN}6. Stopping all clangd processes...${NC}"
killall clangd 2>/dev/null || echo "No clangd processes found"
sleep 1
echo -e "${GREEN}✓${NC} All clangd processes stopped"

# 7. Test direct execution
echo -e "${CYAN}7. Testing direct clangd execution...${NC}"
echo "Testing if Homebrew clangd can start..."
timeout 2 /opt/homebrew/opt/llvm/bin/clangd --version 2>&1 || true
echo ""

# Summary
echo -e "${BLUE}═══════════════════════════════════════════════════════════════════${NC}"
echo -e "${GREEN}Fix Applied!${NC}"
echo -e "${BLUE}═══════════════════════════════════════════════════════════════════${NC}"
echo ""
echo "Options to start Homebrew clangd:"
echo ""
echo "1. ${YELLOW}Using Tasks (Recommended):${NC}"
echo "   - Cmd+Shift+P → 'Tasks: Run Task'"
echo "   - Select 'Start Homebrew Clangd'"
echo ""
echo "2. ${YELLOW}Manual in Terminal:${NC}"
echo "   /opt/homebrew/opt/llvm/bin/clangd \\"
echo "     --compile-commands-dir=build/macos-arm64 \\"
echo "     --background-index"
echo ""
echo "3. ${YELLOW}Check if it's running:${NC}"
echo "   ps aux | grep homebrew.*clangd"
echo ""
echo "4. ${YELLOW}View clangd output:${NC}"
echo "   - View → Output (Cmd+Shift+U)"
echo "   - Select 'clangd' from dropdown"
echo ""
echo -e "${BLUE}═══════════════════════════════════════════════════════════════════${NC}"