#!/bin/bash
# Pre-commit hook to prevent backup files from being committed
# Usage: ./lint/forbid-backups.sh

set -e

echo "🔍 Checking for backup files..."

# Check for backup files matching the problematic patterns
backup_files=$(git ls-files | grep -E '\.!.*!\.' || true)

if [ -n "$backup_files" ]; then
    echo "❌ ERROR: Backup files found in git index:"
    echo "$backup_files"
    echo ""
    echo "These backup files should not be committed. Remove them with:"
    echo "  git rm <filename>"
    echo "  # or to remove all backup files:"
    echo "  git ls-files | grep -E '\.!.*!\.' | xargs git rm"
    echo ""
    echo "Add the following patterns to .gitignore to prevent this:"
    echo "  .!*!.*"
    echo "  *~"
    exit 1
fi

echo "✓ No backup files found"
exit 0