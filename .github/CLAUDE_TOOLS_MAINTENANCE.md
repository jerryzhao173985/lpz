# Claude Tools Maintenance Guide

## Overview

This guide explains how to maintain Claude AI tool configurations for the LPZRobots project.

## Single Source of Truth

All Claude tools are defined in:
```
.github/claude-allowed-tools-lpzrobots-complete.txt
```

This file is the **single source of truth** for all allowed tools.

## Automatic Synchronization

### Using the Sync Script

To update both workflow files from the source:
```bash
.github/scripts/sync-claude-tools.sh
```

This script will:
1. Read tools from `claude-allowed-tools-lpzrobots-complete.txt`
2. Update both `claude.yml` and `claude-code-review.yml`
3. Show a summary of changes

### Automatic Checking

The workflow `check-claude-tools-sync.yml` automatically verifies that:
- Both workflows have identical tool lists
- Tool lists match the source file
- No manual edits have caused divergence

## Adding New Tools

1. **Edit the source file**:
   ```bash
   vi .github/claude-allowed-tools-lpzrobots-complete.txt
   ```

2. **Add tools in the correct format**:
   ```
   Bash(new-command:*),
   ```

3. **Run the sync script**:
   ```bash
   .github/scripts/sync-claude-tools.sh
   ```

4. **Verify and commit**:
   ```bash
   git diff .github/workflows/
   git add -A .github/
   git commit -m "feat: Add new-command to Claude tools"
   ```

## Tool Categories

Tools are organized in the source file by category:
- Core shells and sudo
- Version control
- File and shell essentials
- Build systems
- LPZRobots specific scripts
- Compilers and binutils
- Static analysis and formatting
- Debugging and profiling
- Documentation and helpers
- Platform-specific tools
- Claude workflow helpers

## Best Practices

1. **Never edit workflow files directly** for tool changes
2. **Always use the sync script** to maintain consistency
3. **Test new tools** before adding them
4. **Document why** a tool is needed in commit messages
5. **Keep tools organized** by category in the source file

## Troubleshooting

### Tools Out of Sync
If the CI check fails with "tools don't match":
```bash
# Fix by running sync
.github/scripts/sync-claude-tools.sh
git add .github/workflows/*.yml
git commit -m "fix: Sync Claude tools across workflows"
```

### Adding Platform-Specific Tools
For macOS-only tools:
```
# macOS specific (needed for macOS development)
Bash(otool:*),Bash(install_name_tool:*),
```

For Linux-only tools:
```
# Linux specific
Bash(strace:*),Bash(ltrace:*),
```

## Security Considerations

When adding tools, consider:
- Does this tool have destructive capabilities?
- Is it necessary for the development workflow?
- Are there safer alternatives?

Claude has built-in safeguards, but we should still be thoughtful about what tools we allow.

## Quick Reference

- **Source file**: `.github/claude-allowed-tools-lpzrobots-complete.txt`
- **Sync script**: `.github/scripts/sync-claude-tools.sh`
- **Check workflow**: `.github/workflows/check-claude-tools-sync.yml`
- **Documentation**: `.github/CLAUDE_TOOLS_FINAL_SUMMARY.md`