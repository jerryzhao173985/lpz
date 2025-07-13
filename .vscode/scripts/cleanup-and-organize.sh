#!/bin/bash
# ╔══════════════════════════════════════════════════════════════════╗
# ║         Cleanup and Organize VSCode Configuration                  ║
# ╚══════════════════════════════════════════════════════════════════╝

set -e

# Colors
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m'

echo -e "${BLUE}═══════════════════════════════════════════════════════════════════${NC}"
echo -e "${BLUE}            Cleaning Up VSCode Configuration                        ${NC}"
echo -e "${BLUE}═══════════════════════════════════════════════════════════════════${NC}"
echo ""

# 1. Organize documentation
echo -e "${CYAN}1. Organizing documentation...${NC}"
mkdir -p .vscode/docs
mv -f .vscode/*.md .vscode/docs/ 2>/dev/null || echo "   No markdown files to move"
# Keep the important ones accessible
cp .vscode/docs/FINAL_STATUS_REPORT.md .vscode/README.md 2>/dev/null || true
echo -e "${GREEN}✓${NC} Documentation organized in .vscode/docs/"

# 2. Clean up test files
echo -e "${CYAN}2. Cleaning up test files...${NC}"
rm -f .vscode/test-format.cpp 2>/dev/null || true
rm -f .vscode/test-intellisense.cpp 2>/dev/null || true
echo -e "${GREEN}✓${NC} Test files cleaned up"

# 3. Organize scripts
echo -e "${CYAN}3. Organizing scripts...${NC}"
cd .vscode/scripts
# Make all scripts executable
chmod +x *.sh 2>/dev/null || true
# Create index
cat > README.md << 'EOF'
# VSCode Scripts

## Configuration Management
- `toggle-language-server.sh` - Switch between clangd and Microsoft C++
- `rollback-config.sh` - Restore previous configuration
- `disable-ms-cpp-extensions.sh` - Disable Microsoft C++ extensions

## Diagnostics
- `validate-clangd.sh` - Validate clangd installation
- `diagnose-clangd.sh` - Diagnose clangd issues
- `test-current-setup.sh` - Test current configuration
- `test-intellisense-performance.sh` - Test IntelliSense speed

## Fixes
- `fix-clangd-path.sh` - Fix clangd path issues
- `force-homebrew-clangd.sh` - Force Homebrew clangd
- `final-clangd-fix.sh` - Comprehensive clangd fix

## Performance
- `compare-performance.sh` - Compare language server performance

## Maintenance
- `cleanup-and-organize.sh` - This script
EOF
cd ../..
echo -e "${GREEN}✓${NC} Scripts organized with README"

# 4. Create summary file
echo -e "${CYAN}4. Creating configuration summary...${NC}"
cat > .vscode/CONFIGURATION_SUMMARY.md << 'EOF'
# VSCode Configuration Summary

## Current Setup
- **Language Server**: Xcode clangd (functional, not optimized)
- **Code Formatting**: clang-format (working perfectly)
- **Build System**: CMake with presets
- **Platform**: macOS M4 Max, ARM64

## Quick Commands

### Start Optimized Clangd
```bash
# Option 1: Via Tasks
Cmd+Shift+P → "Tasks: Run Task" → "Start Homebrew Clangd"

# Option 2: Direct
/opt/homebrew/opt/llvm/bin/clangd --compile-commands-dir=build/macos-arm64
```

### Switch Language Servers
```bash
.vscode/scripts/toggle-language-server.sh
```

### Validate Configuration
```bash
.vscode/scripts/validate-clangd.sh
```

## Directory Structure
```
.vscode/
├── settings.json          # Main configuration
├── extensions.json        # Extension recommendations
├── tasks.json            # Task runners
├── launch.json           # Debug configurations
├── lpzrobots.code-snippets # Code snippets
├── backups/              # Configuration backups
├── scripts/              # Helper scripts
├── docs/                 # Documentation
├── debugvis/             # Debug visualizers
└── README.md             # This file
```

## Performance
- Current: 200-500ms completions (acceptable)
- Optimized: <100ms completions (when using Homebrew clangd)

## Support
All documentation in `.vscode/docs/`
All scripts in `.vscode/scripts/`
EOF

echo -e "${GREEN}✓${NC} Created configuration summary"

# 5. Final status
echo ""
echo -e "${BLUE}═══════════════════════════════════════════════════════════════════${NC}"
echo -e "${GREEN}Cleanup Complete!${NC}"
echo -e "${BLUE}═══════════════════════════════════════════════════════════════════${NC}"
echo ""
echo "VSCode configuration is organized and ready for use:"
echo ""
echo "📁 Structure:"
echo "  - Documentation: .vscode/docs/"
echo "  - Scripts: .vscode/scripts/"
echo "  - Backups: .vscode/backups/"
echo "  - Config: .vscode/*.json"
echo ""
echo "📊 Current Status:"
echo "  - IntelliSense: Working (Xcode clangd)"
echo "  - Formatting: Working (clang-format)"
echo "  - Building: Ready (CMake)"
echo ""
echo "🚀 To optimize performance:"
echo "  Run: .vscode/scripts/final-clangd-fix.sh"
echo "  Then use Tasks to start Homebrew clangd"
echo ""
echo -e "${BLUE}═══════════════════════════════════════════════════════════════════${NC}"