#!/bin/bash
# ╔══════════════════════════════════════════════════════════════════╗
# ║              Rollback VSCode Configuration Script                 ║
# ╚══════════════════════════════════════════════════════════════════╝

set -e

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

echo -e "${BLUE}═══════════════════════════════════════════════════════════════════${NC}"
echo -e "${BLUE}            VSCode Configuration Rollback Tool                      ${NC}"
echo -e "${BLUE}═══════════════════════════════════════════════════════════════════${NC}"
echo ""

# Check if backups directory exists
if [ ! -d ".vscode/backups" ]; then
    echo -e "${RED}Error: No backups directory found${NC}"
    exit 1
fi

# List available backups
echo -e "${YELLOW}Available backups:${NC}"
backups=($(ls -1 .vscode/backups/settings.json.backup-* 2>/dev/null | sort -r))

if [ ${#backups[@]} -eq 0 ]; then
    echo -e "${RED}No backups found${NC}"
    exit 1
fi

for i in "${!backups[@]}"; do
    timestamp=$(echo "${backups[$i]}" | sed 's/.*backup-//' | sed 's/\([0-9]\{8\}\)-\([0-9]\{6\}\)/\1 \2/')
    echo "  $((i+1)). Backup from $timestamp"
done

echo ""
echo -n "Select backup to restore (1-${#backups[@]}) or 'q' to quit: "
read -r choice

if [ "$choice" = "q" ] || [ "$choice" = "Q" ]; then
    echo "Rollback cancelled"
    exit 0
fi

# Validate choice
if ! [[ "$choice" =~ ^[0-9]+$ ]] || [ "$choice" -lt 1 ] || [ "$choice" -gt ${#backups[@]} ]; then
    echo -e "${RED}Invalid choice${NC}"
    exit 1
fi

# Get selected backup timestamp
selected_backup="${backups[$((choice-1))]}"
timestamp=$(echo "$selected_backup" | grep -o '[0-9]\{8\}-[0-9]\{6\}')

echo ""
echo -e "${YELLOW}This will restore configuration from $timestamp${NC}"
echo -e "${YELLOW}Current configuration will be backed up first${NC}"
echo ""
echo -n "Continue? (y/n): "
read -r confirm

if [ "$confirm" != "y" ] && [ "$confirm" != "Y" ]; then
    echo "Rollback cancelled"
    exit 0
fi

# Create backup of current state
CURRENT_TIMESTAMP=$(date +%Y%m%d-%H%M%S)
echo ""
echo -e "${BLUE}Creating backup of current configuration...${NC}"

if [ -f ".vscode/settings.json" ]; then
    cp .vscode/settings.json .vscode/backups/settings.json.backup-${CURRENT_TIMESTAMP}-pre-rollback
fi
if [ -f ".vscode/c_cpp_properties.json" ]; then
    cp .vscode/c_cpp_properties.json .vscode/backups/c_cpp_properties.json.backup-${CURRENT_TIMESTAMP}-pre-rollback
fi
if [ -f ".clang-format" ]; then
    cp .clang-format .vscode/backups/clang-format.backup-${CURRENT_TIMESTAMP}-pre-rollback
fi
if [ -f ".clangd" ]; then
    cp .clangd .vscode/backups/clangd.backup-${CURRENT_TIMESTAMP}-pre-rollback
fi

# Restore selected backup
echo -e "${BLUE}Restoring configuration from $timestamp...${NC}"

# Restore settings.json
if [ -f ".vscode/backups/settings.json.backup-${timestamp}" ]; then
    cp .vscode/backups/settings.json.backup-${timestamp} .vscode/settings.json
    echo -e "${GREEN}✓${NC} Restored settings.json"
else
    echo -e "${YELLOW}⚠${NC} No settings.json in this backup"
fi

# Restore c_cpp_properties.json
if [ -f ".vscode/backups/c_cpp_properties.json.backup-${timestamp}" ]; then
    cp .vscode/backups/c_cpp_properties.json.backup-${timestamp} .vscode/c_cpp_properties.json
    echo -e "${GREEN}✓${NC} Restored c_cpp_properties.json"
else
    echo -e "${YELLOW}⚠${NC} No c_cpp_properties.json in this backup"
fi

# Restore .clang-format
if [ -f ".vscode/backups/clang-format.backup-${timestamp}" ]; then
    cp .vscode/backups/clang-format.backup-${timestamp} .clang-format
    echo -e "${GREEN}✓${NC} Restored .clang-format"
else
    echo -e "${YELLOW}⚠${NC} No .clang-format in this backup"
fi

# Restore .clangd
if [ -f ".vscode/backups/clangd.backup-${timestamp}" ]; then
    cp .vscode/backups/clangd.backup-${timestamp} .clangd
    echo -e "${GREEN}✓${NC} Restored .clangd"
else
    echo -e "${YELLOW}⚠${NC} No .clangd in this backup"
fi

echo ""
echo -e "${GREEN}Configuration rolled back successfully!${NC}"
echo ""
echo "Next steps:"
echo "1. Reload VSCode window: Cmd+R (in command palette)"
echo "2. Check that formatting and IntelliSense work correctly"
echo ""
echo -e "${BLUE}═══════════════════════════════════════════════════════════════════${NC}"