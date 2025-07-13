#!/bin/bash
# ╔══════════════════════════════════════════════════════════════════╗
# ║         Force Homebrew Clangd to Start                             ║
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
echo -e "${BLUE}            Forcing Homebrew Clangd                                 ${NC}"
echo -e "${BLUE}═══════════════════════════════════════════════════════════════════${NC}"
echo ""

# 1. Kill ALL clangd processes
echo -e "${CYAN}1. Stopping ALL clangd processes...${NC}"
killall clangd 2>/dev/null || echo "No clangd processes to kill"
sleep 1
echo -e "${GREEN}✓${NC} All clangd processes stopped"
echo ""

# 2. Test manual start
echo -e "${CYAN}2. Testing manual clangd start...${NC}"
echo "Starting clangd in background for 5 seconds..."
/opt/homebrew/opt/llvm/bin/clangd \
    --compile-commands-dir=build/macos-arm64 \
    --background-index \
    --log=info \
    2>&1 | head -10 &

CLANGD_PID=$!
sleep 2

if ps -p $CLANGD_PID > /dev/null; then
    echo -e "${GREEN}✓${NC} Homebrew clangd can start successfully"
    kill $CLANGD_PID 2>/dev/null || true
else
    echo -e "${RED}✗${NC} Homebrew clangd failed to start"
fi
echo ""

# 3. Create a launcher script
echo -e "${CYAN}3. Creating VSCode launcher configuration...${NC}"
cat > .vscode/clangd-launcher.sh << 'EOF'
#!/bin/bash
# Force Homebrew clangd
exec /opt/homebrew/opt/llvm/bin/clangd "$@"
EOF
chmod +x .vscode/clangd-launcher.sh
echo -e "${GREEN}✓${NC} Created launcher script"
echo ""

# 4. Update settings to use launcher
echo -e "${CYAN}4. Checking alternative configurations...${NC}"

# Check if we need to disable any conflicting extensions
CONFLICTING_EXTS=$(code --list-extensions | grep -E "cpptools|intellisense" | grep -v clangd || true)
if [ -n "$CONFLICTING_EXTS" ]; then
    echo -e "${YELLOW}⚠${NC} Found potentially conflicting extensions:"
    echo "$CONFLICTING_EXTS"
    echo ""
    echo "Consider disabling these extensions:"
    for ext in $CONFLICTING_EXTS; do
        echo "  code --disable-extension $ext"
    done
fi
echo ""

# 5. Environment variable check
echo -e "${CYAN}5. Setting environment variables...${NC}"
export PATH="/opt/homebrew/opt/llvm/bin:$PATH"
echo -e "${GREEN}✓${NC} Updated PATH"

# Create a debug wrapper
cat > .vscode/debug-clangd.sh << 'EOF'
#!/bin/bash
echo "Clangd wrapper called with args: $@" >> /tmp/clangd-debug.log
exec /opt/homebrew/opt/llvm/bin/clangd "$@" 2>&1 | tee -a /tmp/clangd-debug.log
EOF
chmod +x .vscode/debug-clangd.sh

echo ""
echo -e "${BLUE}═══════════════════════════════════════════════════════════════════${NC}"
echo -e "${BLUE}Manual Override Instructions${NC}"
echo -e "${BLUE}═══════════════════════════════════════════════════════════════════${NC}"
echo ""
echo "If clangd still doesn't start automatically:"
echo ""
echo "1. ${YELLOW}Disable Microsoft C++ Extension:${NC}"
echo "   - Open Extensions (Cmd+Shift+X)"
echo "   - Find 'C/C++' by Microsoft"
echo "   - Click gear icon → Disable (Workspace)"
echo ""
echo "2. ${YELLOW}Force clangd restart:${NC}"
echo "   - Command Palette (Cmd+Shift+P)"
echo "   - Run: 'clangd: Restart language server'"
echo ""
echo "3. ${YELLOW}Check Output panel:${NC}"
echo "   - View → Output (Cmd+Shift+U)"
echo "   - Select 'clangd' from dropdown"
echo "   - Look for error messages"
echo ""
echo "4. ${YELLOW}Alternative: Start manually:${NC}"
echo "   In terminal, run:"
echo "   /opt/homebrew/opt/llvm/bin/clangd \\"
echo "     --compile-commands-dir=build/macos-arm64 \\"
echo "     --background-index"
echo ""
echo -e "${BLUE}═══════════════════════════════════════════════════════════════════${NC}"