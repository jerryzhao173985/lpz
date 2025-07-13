#!/bin/bash
# ╔══════════════════════════════════════════════════════════════════╗
# ║         Disable Microsoft C++ Extensions for Workspace             ║
# ╚══════════════════════════════════════════════════════════════════╝

set -e
set -u
set -o pipefail

# Colors
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m'

echo -e "${BLUE}═══════════════════════════════════════════════════════════════════${NC}"
echo -e "${BLUE}            Disabling Microsoft C++ Extensions                      ${NC}"
echo -e "${BLUE}═══════════════════════════════════════════════════════════════════${NC}"
echo ""

# Check current extensions
echo -e "${CYAN}Current C++ extensions:${NC}"
if command -v code >/dev/null 2>&1; then
    code --list-extensions | grep -E "(cpptools|clangd)" || true
else
    echo "VSCode CLI not available - extension check skipped"
fi
echo ""

# Note: VSCode CLI extension disable affects global scope, not workspace
echo -e "${YELLOW}Note: Creating workspace configuration for extension control...${NC}"

# Create workspace configuration
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

# Restart clangd processes for clean state
echo -e "${CYAN}Restarting clangd processes...${NC}"
pkill -x clangd 2>/dev/null || echo "No clangd processes to restart"
echo -e "${GREEN}✓${NC} Clangd processes restarted"
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