#!/bin/bash
# Pre-commit hook to prevent backup files from being committed
# Created 2025-07-12 to fix CI failures caused by editor backup files

set -e

echo "Checking for backup files..."

# Check for backup files with .!*!.* pattern
backup_files=$(git ls-files | grep -E '\.!.*!\.' || true)

if [ -n "$backup_files" ]; then
    echo "ERROR: Backup files found in git repository!"
    echo "The following backup files should not be committed:"
    echo "$backup_files"
    echo ""
    echo "Please remove these files and add them to .gitignore:"
    echo "  git rm <file>"
    echo "  git commit -m 'Remove backup files'"
    echo ""
    echo "Add the following patterns to .gitignore to prevent future issues:"
    echo "  .!*!.*"
    echo "  *~"
    exit 1
fi

echo "✓ No backup files found"
exit 0