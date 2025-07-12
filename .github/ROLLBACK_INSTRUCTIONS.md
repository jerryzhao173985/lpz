# Rollback Instructions for Claude Tools

If the Claude GitHub Actions fail with the current tool configuration, here's how to rollback:

## Quick Rollback to Previous State

```bash
# Rollback to before tool changes
git revert 3496036 125e3aa

# Or reset to specific commit before changes
git reset --hard a5a22f0
```

## Manual Fix if Needed

If the `Bash(command:*)` format doesn't work, you may need to use specific commands:

Instead of:
```
Bash(git:*)
```

Use:
```
Bash(git add:*),Bash(git commit:*),Bash(git push:*),Bash(git pull:*)
```

## Testing the Configuration

To test if Claude works with current settings:
1. Create a test issue
2. Comment "@claude can you run `ls -la`?"
3. Check if Claude responds

## Current Configuration Summary

- **Model**: claude-opus-4-20250514 (Opus 4)
- **Tools Format**: Bash(command:*) for all subcommands
- **Total Tools**: 202 commands
- **Trigger**: @claude in issues/PRs

## If Rollback is Needed

The original working configuration from commit 733c519 had:
- Specific git commands listed separately
- Fewer total tools
- Same model (after commit 4aedd7e)

## Contact

If issues persist, check:
- https://github.com/anthropics/claude-code-action/issues
- GitHub Actions logs for specific error messages