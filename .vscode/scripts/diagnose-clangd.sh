#!/bin/bash
# ╔══════════════════════════════════════════════════════════════════╗
# ║         Diagnose Clangd Startup Issues                             ║
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
echo -e "${BLUE}            Clangd Diagnostic Report                                ${NC}"
echo -e "${BLUE}═══════════════════════════════════════════════════════════════════${NC}"
echo ""

# 1. Test clangd directly
echo -e "${CYAN}1. Testing clangd executable directly...${NC}"
if [ -f "/opt/homebrew/opt/llvm/bin/clangd" ]; then
    echo -e "${GREEN}✓${NC} Clangd executable exists"
    
    # Test if it can run
    if /opt/homebrew/opt/llvm/bin/clangd --version > /dev/null 2>&1; then
        echo -e "${GREEN}✓${NC} Clangd runs successfully"
    else
        echo -e "${RED}✗${NC} Clangd fails to run"
        /opt/homebrew/opt/llvm/bin/clangd --version 2>&1 || true
    fi
else
    echo -e "${RED}✗${NC} Clangd executable not found at expected path"
fi
echo ""

# 2. Check PATH
echo -e "${CYAN}2. Checking PATH configuration...${NC}"
echo "Current PATH: $PATH"
if echo "$PATH" | grep -q "/opt/homebrew/opt/llvm/bin"; then
    echo -e "${GREEN}✓${NC} LLVM bin directory in PATH"
else
    echo -e "${YELLOW}⚠${NC} LLVM bin directory not in PATH"
fi
echo ""

# 3. Check for VSCode workspace settings
echo -e "${CYAN}3. Checking for workspace settings override...${NC}"
if [ -f ".vscode/settings.json" ]; then
    if grep -q '"files.associations"' .vscode/settings.json; then
        echo -e "${GREEN}✓${NC} C++ file associations configured"
        grep -A3 '"files.associations"' .vscode/settings.json | head -10
    fi
fi
echo ""

# 4. Test compile_commands.json accessibility
echo -e "${CYAN}4. Testing compile_commands.json...${NC}"
if [ -f "compile_commands.json" ]; then
    echo -e "${GREEN}✓${NC} Root compile_commands.json accessible"
    echo "   Size: $(wc -c < compile_commands.json) bytes"
fi

if [ -f "build/macos-arm64/compile_commands.json" ]; then
    echo -e "${GREEN}✓${NC} Build directory compile_commands.json accessible"
    echo "   Size: $(wc -c < build/macos-arm64/compile_commands.json) bytes"
fi
echo ""

# 5. Check for permission issues
echo -e "${CYAN}5. Checking permissions...${NC}"
CLANGD_PATH="/opt/homebrew/opt/llvm/bin/clangd"
if [ -f "$CLANGD_PATH" ]; then
    ls -la "$CLANGD_PATH" | awk '{print "   Permissions: " $1 " Owner: " $3}'
    if [ -x "$CLANGD_PATH" ]; then
        echo -e "${GREEN}✓${NC} Clangd is executable"
    else
        echo -e "${RED}✗${NC} Clangd is not executable"
    fi
fi
echo ""

# 6. Check VSCode extension status
echo -e "${CYAN}6. Extension conflict check...${NC}"
MS_CPP_COUNT=$(code --list-extensions | grep -c "ms-vscode.cpptools" || echo 0)
CLANGD_COUNT=$(code --list-extensions | grep -c "clangd" || echo 0)

echo "Microsoft C++ extensions: $MS_CPP_COUNT"
echo "Clangd extensions: $CLANGD_COUNT"

if [ $MS_CPP_COUNT -gt 0 ] && [ $CLANGD_COUNT -gt 0 ]; then
    echo -e "${YELLOW}⚠${NC} Both extensions are installed"
    echo "   This is OK since we disabled C++ IntelliSense"
fi
echo ""

# 7. Manual start test
echo -e "${CYAN}7. Testing manual clangd start...${NC}"
echo "Attempting to start clangd with project configuration..."

# Create a test request
cat > /tmp/clangd_test_input.txt << 'EOF'
Content-Length: 125

{"jsonrpc":"2.0","id":0,"method":"initialize","params":{"rootPath":".","capabilities":{},"initializationOptions":{}}}
EOF

# Try to start clangd
timeout 2 /opt/homebrew/opt/llvm/bin/clangd \
    --compile-commands-dir=build/macos-arm64 \
    --log=verbose \
    < /tmp/clangd_test_input.txt 2>&1 | head -20 || true

rm -f /tmp/clangd_test_input.txt
echo ""

# Summary
echo -e "${BLUE}═══════════════════════════════════════════════════════════════════${NC}"
echo -e "${BLUE}Diagnosis Summary${NC}"
echo -e "${BLUE}═══════════════════════════════════════════════════════════════════${NC}"

echo ""
echo "Potential issues to check:"
echo "1. Open Output panel: Cmd+Shift+U"
echo "2. Select 'clangd' from dropdown"
echo "3. Look for any error messages"
echo ""
echo "Try these actions:"
echo "1. Open a .cpp file (not .hpp)"
echo "2. Make a small edit and save"
echo "3. Check if clangd starts then"
echo ""
echo "If clangd still doesn't start:"
echo "- Check if clangd extension is enabled"
echo "- Try disabling and re-enabling clangd extension"
echo "- Check VSCode Developer Tools (Help → Toggle Developer Tools)"

echo -e "${BLUE}═══════════════════════════════════════════════════════════════════${NC}"