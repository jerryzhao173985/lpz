#!/bin/bash
# ╔══════════════════════════════════════════════════════════════════╗
# ║         Test Current IntelliSense Performance                      ║
# ╚══════════════════════════════════════════════════════════════════╝

set -e

# Colors
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m'

echo -e "${BLUE}═══════════════════════════════════════════════════════════════════${NC}"
echo -e "${BLUE}            IntelliSense Performance Test                           ${NC}"
echo -e "${BLUE}═══════════════════════════════════════════════════════════════════${NC}"
echo ""

echo -e "${CYAN}Current Language Server Status:${NC}"
echo ""

# Check which clangd is running
CLANGD_PROCS=$(ps aux | grep clangd | grep -v grep | head -5)
if echo "$CLANGD_PROCS" | grep -q "Xcode"; then
    echo -e "${YELLOW}⚠ Using Xcode clangd (not optimized)${NC}"
elif echo "$CLANGD_PROCS" | grep -q "homebrew"; then
    echo -e "${GREEN}✓ Using Homebrew clangd (optimized)${NC}"
else
    echo -e "${YELLOW}⚠ No clangd process found${NC}"
fi

echo ""
echo "Active clangd processes:"
ps aux | grep clangd | grep -v grep | awk '{print "  PID: " $2 " Memory: " $6/1024 "MB CPU: " $3 "%"}'

echo ""
echo -e "${CYAN}Manual Performance Tests:${NC}"
echo ""
echo "1. ${YELLOW}Code Completion Test:${NC}"
echo "   - Open: selforg/matrix/matrix.cpp"
echo "   - Type: Matrix m; m."
echo "   - Time how long until suggestions appear"
echo ""
echo "2. ${YELLOW}Go to Definition Test:${NC}"
echo "   - Find any class name or function"
echo "   - Press F12"
echo "   - Time how long until jump occurs"
echo ""
echo "3. ${YELLOW}Find References Test:${NC}"
echo "   - Click on a method name"
echo "   - Press Shift+F12"
echo "   - Time how long until results appear"
echo ""
echo "4. ${YELLOW}Error Detection Test:${NC}"
echo "   - Type: int x ="
echo "   - Time how long until red squiggle appears"
echo ""

echo -e "${CYAN}System Resources:${NC}"
echo -n "Free memory: "
vm_stat | grep "Pages free" | awk '{print $3 * 4096 / 1024 / 1024 / 1024 " GB"}'
echo -n "VSCode memory: "
ps aux | grep "Visual Studio Code" | grep -v grep | awk '{sum += $6} END {print sum/1024 " MB total"}'

echo ""
echo -e "${BLUE}═══════════════════════════════════════════════════════════════════${NC}"
echo ""
echo "Expected Performance:"
echo "- With Xcode clangd: 200-500ms completions"
echo "- With Homebrew clangd: <100ms completions"
echo ""
echo "Current setup is functional for development!"
echo -e "${BLUE}═══════════════════════════════════════════════════════════════════${NC}"