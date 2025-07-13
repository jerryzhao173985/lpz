#!/bin/bash
# ╔══════════════════════════════════════════════════════════════════╗
# ║         Disable Microsoft C++ Extensions for Workspace             ║
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
echo -e "${BLUE}            Disabling Microsoft C++ Extensions                      ${NC}"
echo -e "${BLUE}═══════════════════════════════════════════════════════════════════${NC}"
echo ""

# Create workspace settings directory if not exists
WORKSPACE_SETTINGS=".vscode/.code-workspace"
mkdir -p "$(dirname "$WORKSPACE_SETTINGS")"

# Check current extensions
echo -e "${CYAN}Current C++ extensions:${NC}"
code --list-extensions | grep -E "(cpptools|clangd)" || true
echo ""

# Disable Microsoft C++ extensions for this workspace
echo -e "${YELLOW}Disabling Microsoft C++ extensions...${NC}"

# Method 1: Using VSCode CLI (may not work for workspace-specific)
code --disable-extension ms-vscode.cpptools 2>/dev/null || true
code --disable-extension ms-vscode.cpptools-extension-pack 2>/dev/null || true
code --disable-extension ms-vscode.cpptools-themes 2>/dev/null || true

# Method 2: Create workspace configuration
echo -e "${CYAN}Creating workspace configuration...${NC}"
cat > lpzrobots.code-workspace << 'EOF'
{
    "folders": [
        {
            "path": "."
        }
    ],
    "settings": {
        // Force clangd only
        "C_Cpp.intelliSenseEngine": "disabled",
        "clangd.enabled": true
    },
    "extensions": {
        "recommendations": [
            "llvm-vs-code-extensions.vscode-clangd"
        ],
        "unwantedRecommendations": [
            "ms-vscode.cpptools",
            "ms-vscode.cpptools-extension-pack",
            "ms-vscode.cpptools-themes"
        ]
    }
}
EOF

echo -e "${GREEN}✓${NC} Created lpzrobots.code-workspace"
echo ""

# Kill all clangd processes
echo -e "${CYAN}Stopping all clangd processes...${NC}"
killall clangd 2>/dev/null || echo "No clangd to kill"
echo -e "${GREEN}✓${NC} Stopped all clangd processes"
echo ""

# Summary
echo -e "${BLUE}═══════════════════════════════════════════════════════════════════${NC}"
echo -e "${GREEN}Extensions disabled!${NC}"
echo ""
echo "Next steps:"
echo ""
echo "1. ${YELLOW}Open the workspace file:${NC}"
echo "   code lpzrobots.code-workspace"
echo ""
echo "2. ${YELLOW}Or manually in VSCode:${NC}"
echo "   - File → Open Workspace from File..."
echo "   - Select: lpzrobots.code-workspace"
echo ""
echo "3. ${YELLOW}When prompted:${NC}"
echo "   - Click 'Yes' to install recommended extensions"
echo "   - This will ensure only clangd is active"
echo ""
echo "4. ${YELLOW}Verify clangd starts:${NC}"
echo "   ps aux | grep homebrew.*clangd"
echo ""
echo -e "${BLUE}═══════════════════════════════════════════════════════════════════${NC}"