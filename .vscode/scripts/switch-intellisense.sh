#!/bin/bash
# ╔══════════════════════════════════════════════════════════════════╗
# ║              Switch IntelliSense Engine (MS C++ ↔ clangd)        ║
# ╚══════════════════════════════════════════════════════════════════╝

set -e

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

SETTINGS_FILE=".vscode/settings.json"
CLANGD_SETTINGS=".vscode/settings-clangd.json"

echo -e "${BLUE}═══════════════════════════════════════════════════════════════════${NC}"
echo -e "${BLUE}            Switch IntelliSense Engine                              ${NC}"
echo -e "${BLUE}═══════════════════════════════════════════════════════════════════${NC}"
echo ""

# Check current state
if [ ! -f "$SETTINGS_FILE" ]; then
    echo -e "${RED}Error: $SETTINGS_FILE not found${NC}"
    exit 1
fi

# Detect current engine
CURRENT_ENGINE="unknown"
if grep -q '"C_Cpp.intelliSenseEngine": "disabled"' "$SETTINGS_FILE"; then
    CURRENT_ENGINE="clangd"
elif grep -q '"clangd.enabled": true' "$SETTINGS_FILE"; then
    CURRENT_ENGINE="clangd"
else
    CURRENT_ENGINE="ms-cpp"
fi

echo -e "Current engine: ${YELLOW}$CURRENT_ENGINE${NC}"
echo ""
echo "Available engines:"
echo "  1. Microsoft C++ IntelliSense (default)"
echo "  2. clangd (faster, more accurate)"
echo ""
echo -n "Select engine (1-2) or 'q' to quit: "
read -r CHOICE

case $CHOICE in
    1)
        echo ""
        echo -e "${YELLOW}Switching to Microsoft C++ IntelliSense...${NC}"
        
        # Backup current settings
        cp "$SETTINGS_FILE" "$SETTINGS_FILE.bak"
        
        # Remove clangd settings
        if [[ "$OSTYPE" == "darwin"* ]]; then
            # macOS sed
            sed -i '' '/"C_Cpp.intelliSenseEngine": "disabled"/d' "$SETTINGS_FILE"
            sed -i '' '/"clangd.enabled": true/d' "$SETTINGS_FILE"
            sed -i '' '/"clangd.path":/d' "$SETTINGS_FILE"
            sed -i '' '/"clangd.arguments":/,/\]/d' "$SETTINGS_FILE"
            sed -i '' '/"clangd\./d' "$SETTINGS_FILE"
        else
            # GNU sed
            sed -i '/"C_Cpp.intelliSenseEngine": "disabled"/d' "$SETTINGS_FILE"
            sed -i '/"clangd.enabled": true/d' "$SETTINGS_FILE"
            sed -i '/"clangd.path":/d' "$SETTINGS_FILE"
            sed -i '/"clangd.arguments":/,/\]/d' "$SETTINGS_FILE"
            sed -i '/"clangd\./d' "$SETTINGS_FILE"
        fi
        
        # Ensure MS IntelliSense is enabled
        if ! grep -q '"C_Cpp.intelliSenseEngine"' "$SETTINGS_FILE"; then
            # Add to settings
            if [[ "$OSTYPE" == "darwin"* ]]; then
                sed -i '' '/{/a\
    "C_Cpp.intelliSenseEngine": "default",
' "$SETTINGS_FILE"
            else
                sed -i '/{/a\    "C_Cpp.intelliSenseEngine": "default",' "$SETTINGS_FILE"
            fi
        fi
        
        echo -e "${GREEN}✓${NC} Switched to Microsoft C++ IntelliSense"
        echo ""
        echo "Please reload VSCode window: Cmd+R (in command palette)"
        ;;
        
    2)
        echo ""
        echo -e "${YELLOW}Switching to clangd...${NC}"
        
        # Check if clangd extension is installed
        if command -v code >/dev/null 2>&1; then
            if ! code --list-extensions | grep -q "llvm-vs-code-extensions.vscode-clangd"; then
                echo -e "${YELLOW}Installing clangd extension...${NC}"
                code --install-extension llvm-vs-code-extensions.vscode-clangd
            fi
        fi
        
        # Backup current settings
        cp "$SETTINGS_FILE" "$SETTINGS_FILE.bak"
        
        # Check if clangd settings file exists
        if [ -f "$CLANGD_SETTINGS" ]; then
            echo -e "${YELLOW}Merging clangd settings...${NC}"
            
            # This is a simplified merge - in practice, you'd want a proper JSON merge
            echo -e "${YELLOW}Note: Please manually merge settings from $CLANGD_SETTINGS${NC}"
            echo -e "${YELLOW}Key settings to add:${NC}"
            echo '    "C_Cpp.intelliSenseEngine": "disabled",'
            echo '    "clangd.enabled": true,'
            echo '    "clangd.path": "${env:HOMEBREW_PREFIX:/opt/homebrew}/opt/llvm/bin/clangd",'
            echo ""
        fi
        
        # Add basic clangd settings
        if [[ "$OSTYPE" == "darwin"* ]]; then
            sed -i '' '/{/a\
    "C_Cpp.intelliSenseEngine": "disabled",\
    "clangd.enabled": true,
' "$SETTINGS_FILE"
        else
            sed -i '/{/a\    "C_Cpp.intelliSenseEngine": "disabled",\n    "clangd.enabled": true,' "$SETTINGS_FILE"
        fi
        
        echo -e "${GREEN}✓${NC} Switched to clangd"
        echo ""
        echo "Next steps:"
        echo "1. Reload VSCode window: Cmd+R (in command palette)"
        echo "2. If clangd fails to start, check the path in settings"
        echo "3. See $CLANGD_SETTINGS for advanced configuration"
        ;;
        
    q|Q)
        echo "Cancelled"
        exit 0
        ;;
        
    *)
        echo -e "${RED}Invalid choice${NC}"
        exit 1
        ;;
esac

echo ""
echo -e "${BLUE}═══════════════════════════════════════════════════════════════════${NC}"