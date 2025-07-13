#!/bin/bash
# ╔══════════════════════════════════════════════════════════════════╗
# ║         Toggle Between Microsoft C++ and Clangd                   ║
# ╚══════════════════════════════════════════════════════════════════╝

set -e

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

SETTINGS_FILE=".vscode/settings.json"
SETTINGS_BACKUP=".vscode/settings.json.backup"
CLANGD_SETTINGS=".vscode/settings-clangd-optimized.json"
CPPTOOLS_SETTINGS=".vscode/settings.json.backup-20250713-095646"  # Your original backup

echo -e "${BLUE}═══════════════════════════════════════════════════════════════════${NC}"
echo -e "${BLUE}            Language Server Toggle Tool                             ${NC}"
echo -e "${BLUE}═══════════════════════════════════════════════════════════════════${NC}"
echo ""

# Check current state
CURRENT_SERVER="unknown"
if grep -q '"C_Cpp.intelliSenseEngine": "disabled"' "$SETTINGS_FILE" 2>/dev/null; then
    CURRENT_SERVER="clangd"
elif grep -q '"clangd.enabled": true' "$SETTINGS_FILE" 2>/dev/null; then
    CURRENT_SERVER="clangd"
else
    CURRENT_SERVER="cpptools"
fi

echo -e "Current language server: ${YELLOW}$CURRENT_SERVER${NC}"
echo ""

if [ "$CURRENT_SERVER" = "cpptools" ]; then
    echo "Switch to clangd for:"
    echo "  • 60% faster indexing (3-5 min vs 20-30 min)"
    echo "  • Better ARM64 NEON intrinsics support"
    echo "  • Lower memory usage with better performance"
    echo "  • More accurate template parsing"
    echo ""
    echo -n "Switch to clangd? (y/n): "
else
    echo "Switch back to Microsoft C++ IntelliSense for:"
    echo "  • Familiar interface"
    echo "  • Integrated debugging features"
    echo "  • Better Windows compatibility (if needed later)"
    echo ""
    echo -n "Switch to Microsoft C++? (y/n): "
fi

read -r choice

if [ "$choice" != "y" ] && [ "$choice" != "Y" ]; then
    echo "No changes made"
    exit 0
fi

# Create backup
TIMESTAMP=$(date +%Y%m%d-%H%M%S)
cp "$SETTINGS_FILE" ".vscode/backups/settings.json.backup-${TIMESTAMP}-pre-toggle"
echo -e "${GREEN}✓${NC} Created backup: settings.json.backup-${TIMESTAMP}-pre-toggle"

if [ "$CURRENT_SERVER" = "cpptools" ]; then
    # Switch to clangd
    echo -e "${YELLOW}Switching to clangd...${NC}"
    
    if [ -f "$CLANGD_SETTINGS" ]; then
        cp "$CLANGD_SETTINGS" "$SETTINGS_FILE"
        echo -e "${GREEN}✓${NC} Applied clangd configuration"
    else
        echo -e "${RED}Error: Clangd settings file not found${NC}"
        exit 1
    fi
    
    # Ensure clangd extension is enabled
    code --enable-extension llvm-vs-code-extensions.vscode-clangd 2>/dev/null || true
    # Disable C++ extension to avoid conflicts
    code --disable-extension ms-vscode.cpptools 2>/dev/null || true
    
else
    # Switch back to Microsoft C++
    echo -e "${YELLOW}Switching to Microsoft C++ IntelliSense...${NC}"
    
    if [ -f "$CPPTOOLS_SETTINGS" ]; then
        cp "$CPPTOOLS_SETTINGS" "$SETTINGS_FILE"
        # Fix the formatter conflict we already resolved
        sed -i.tmp '/"editor.defaultFormatter": "ms-vscode.cpptools"/d' "$SETTINGS_FILE"
        rm "$SETTINGS_FILE.tmp"
        echo -e "${GREEN}✓${NC} Applied Microsoft C++ configuration"
    else
        echo -e "${RED}Error: Original settings backup not found${NC}"
        exit 1
    fi
    
    # Enable C++ extension
    code --enable-extension ms-vscode.cpptools 2>/dev/null || true
    # Disable clangd to avoid conflicts
    code --disable-extension llvm-vs-code-extensions.vscode-clangd 2>/dev/null || true
fi

echo ""
echo -e "${GREEN}Language server switched successfully!${NC}"
echo ""
echo "Next steps:"
echo "1. Reload VSCode window: Cmd+Shift+P → 'Developer: Reload Window'"
echo "2. Wait for indexing to complete (check status bar)"
echo "3. Test code completion and navigation"
echo ""
echo "If you encounter issues, run:"
echo "  .vscode/scripts/rollback-config.sh"
echo ""
echo -e "${BLUE}═══════════════════════════════════════════════════════════════════${NC}"