#!/bin/bash
# ╔══════════════════════════════════════════════════════════════════╗
# ║         Validate Clangd Installation and Configuration             ║
# ╚══════════════════════════════════════════════════════════════════╝

set -e

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

echo -e "${BLUE}═══════════════════════════════════════════════════════════════════${NC}"
echo -e "${BLUE}            Clangd Validation Suite                                 ${NC}"
echo -e "${BLUE}═══════════════════════════════════════════════════════════════════${NC}"
echo ""

# Function to check status
check_status() {
    if [ $? -eq 0 ]; then
        echo -e "${GREEN}✓${NC} $1"
    else
        echo -e "${RED}✗${NC} $1"
        return 1
    fi
}

# 1. Check clangd installation
echo -e "${YELLOW}1. Checking clangd installation...${NC}"
if [ -f "/opt/homebrew/opt/llvm/bin/clangd" ]; then
    echo -e "${GREEN}✓${NC} Homebrew clangd found"
    /opt/homebrew/opt/llvm/bin/clangd --version
else
    echo -e "${RED}✗${NC} Homebrew clangd not found!"
fi
echo ""

# 2. Check VSCode configuration
echo -e "${YELLOW}2. Checking VSCode configuration...${NC}"
if grep -q '"clangd.enabled": true' .vscode/settings.json; then
    echo -e "${GREEN}✓${NC} Clangd enabled in settings.json"
else
    echo -e "${RED}✗${NC} Clangd not enabled in settings.json"
fi

if grep -q '"C_Cpp.intelliSenseEngine": "disabled"' .vscode/settings.json; then
    echo -e "${GREEN}✓${NC} Microsoft C++ IntelliSense disabled"
else
    echo -e "${RED}✗${NC} Microsoft C++ IntelliSense still enabled"
fi
echo ""

# 3. Check compile_commands.json
echo -e "${YELLOW}3. Checking compile_commands.json...${NC}"
if [ -f "compile_commands.json" ]; then
    echo -e "${GREEN}✓${NC} compile_commands.json found in root"
    entries=$(grep -c '"file"' compile_commands.json || echo 0)
    echo "   Contains $entries compilation entries"
fi

if [ -f "build/macos-arm64/compile_commands.json" ]; then
    echo -e "${GREEN}✓${NC} compile_commands.json found in build directory"
fi
echo ""

# 4. Check clangd process
echo -e "${YELLOW}4. Checking clangd process...${NC}"
CLANGD_PID=$(ps aux | grep "/opt/homebrew/opt/llvm/bin/clangd" | grep -v grep | awk '{print $2}' | head -1)
if [ -n "$CLANGD_PID" ]; then
    echo -e "${GREEN}✓${NC} Homebrew clangd is running (PID: $CLANGD_PID)"
    ps aux | grep "$CLANGD_PID" | grep -v grep | awk '{print "   Memory: " $6/1024 "MB, CPU: " $3 "%"}'
else
    echo -e "${YELLOW}⚠${NC} Homebrew clangd not running yet"
    echo "   This is normal if VSCode hasn't been reloaded"
fi
echo ""

# 5. Check for conflicting processes
echo -e "${YELLOW}5. Checking for conflicts...${NC}"
CPPTOOLS_COUNT=$(ps aux | grep cpptools | grep -v grep | wc -l | xargs)
if [ "$CPPTOOLS_COUNT" -gt 0 ]; then
    echo -e "${YELLOW}⚠${NC} Found $CPPTOOLS_COUNT cpptools processes still running"
    echo "   These should stop after VSCode reload"
else
    echo -e "${GREEN}✓${NC} No cpptools processes found"
fi

XCODE_CLANGD=$(ps aux | grep "Xcode.*clangd" | grep -v grep | wc -l | xargs)
if [ "$XCODE_CLANGD" -gt 0 ]; then
    echo -e "${BLUE}ℹ${NC} Found $XCODE_CLANGD Xcode clangd processes"
    echo "   These are separate and won't interfere"
fi
echo ""

# 6. Test clangd directly
echo -e "${YELLOW}6. Testing clangd functionality...${NC}"
if [ -f "/opt/homebrew/opt/llvm/bin/clangd" ]; then
    # Create a simple test
    cat > /tmp/clangd_test.cpp << 'EOF'
#include <vector>
int main() {
    std::vector<int> v;
    v.push_back(42);
    return 0;
}
EOF
    
    # Test clangd can parse it
    echo '{"jsonrpc":"2.0","id":0,"method":"initialize","params":{}}' | \
        /opt/homebrew/opt/llvm/bin/clangd --compile-commands-dir=. 2>&1 | \
        grep -q "Content-Length" && echo -e "${GREEN}✓${NC} Clangd responds correctly" || echo -e "${RED}✗${NC} Clangd test failed"
    
    rm -f /tmp/clangd_test.cpp
fi
echo ""

# 7. Memory and performance check
echo -e "${YELLOW}7. System resources...${NC}"
echo -n "Available memory: "
vm_stat | grep "Pages free" | awk '{print $3 * 4096 / 1024 / 1024 / 1024 " GB"}'
echo -n "Total clangd memory usage: "
ps aux | grep clangd | grep -v grep | awk '{sum += $6} END {print sum/1024 " MB"}'
echo ""

# Summary
echo -e "${BLUE}═══════════════════════════════════════════════════════════════════${NC}"
echo -e "${BLUE}Summary${NC}"
echo -e "${BLUE}═══════════════════════════════════════════════════════════════════${NC}"

if [ -f "/opt/homebrew/opt/llvm/bin/clangd" ] && \
   grep -q '"clangd.enabled": true' .vscode/settings.json && \
   grep -q '"C_Cpp.intelliSenseEngine": "disabled"' .vscode/settings.json; then
    echo -e "${GREEN}✓ Clangd is properly configured${NC}"
    echo ""
    echo "Next steps:"
    echo "1. Reload VSCode: Cmd+Shift+P → 'Developer: Reload Window'"
    echo "2. Open a C++ file to trigger indexing"
    echo "3. Check status bar for 'clangd: indexing' progress"
else
    echo -e "${RED}✗ Configuration issues detected${NC}"
    echo "Please check the errors above"
fi

echo ""
echo -e "${BLUE}═══════════════════════════════════════════════════════════════════${NC}"