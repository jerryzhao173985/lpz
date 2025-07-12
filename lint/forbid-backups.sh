#!/bin/bash
# Pre-commit hook to prevent backup files from being committed
# Usage: ./lint/forbid-backups.sh
# Exit code: 0 if no backup files found, 1 if backup files found

set -e

echo "🔍 Checking for backup files that should not be committed..."

# Check for backup files with .!*!.* pattern
backup_files=$(git ls-files | grep -E '\.!.*!\.' || true)

if [ -n "$backup_files" ]; then
    echo "❌ Found backup files that should not be committed:"
    echo "$backup_files"
    echo ""
    echo "These appear to be editor backup files. Please remove them:"
    echo "  find . -name '.!*!.*' -type f -delete"
    echo ""
    echo "And ensure they're added to .gitignore:"
    echo "  echo '.!*!.*' >> .gitignore"
    exit 1
fi

echo "✓ No backup files found"
exit 0