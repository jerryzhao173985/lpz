# Clangd Path Issue - Resolution Guide

## Problem Identified
VSCode is using Xcode's clangd instead of Homebrew's optimized version.

## Root Cause
The clangd extension is finding Xcode's clangd first in the system PATH before our specified path.

## Solution Steps

### Option 1: Disable Microsoft C++ Extension (Recommended)
Since we're using clangd, we should disable the Microsoft C++ extension completely:

1. **Open Extensions** (Cmd+Shift+X)
2. Search for "C/C++" by Microsoft
3. Click on it and select **Disable (Workspace)**
4. This prevents any conflicts

### Option 2: Force Homebrew Clangd Path
Update the clangd arguments to ensure it uses the correct binary:

1. **Close all C++ files** in VSCode
2. **Open Command Palette** (Cmd+Shift+P)
3. Run: **"clangd: Restart language server"**
4. If that doesn't exist, try **"Developer: Reload Window"**

### Option 3: Check Extension Settings
1. Open Command Palette (Cmd+Shift+P)
2. Run: **"Preferences: Open Settings (UI)"**
3. Search for "clangd.path"
4. Ensure it shows: `/opt/homebrew/opt/llvm/bin/clangd`
5. If not, click "Edit in settings.json" and verify

## Verification Commands

After making changes:

```bash
# Check which clangd is running
ps aux | grep clangd | grep -v grep | grep -v Xcode

# Should show:
# /opt/homebrew/opt/llvm/bin/clangd

# If still showing Xcode, try:
killall clangd
# Then open a .cpp file in VSCode
```

## Alternative Workaround

If the above doesn't work, we can create a launch configuration:

1. Create `.vscode/tasks.json`:
```json
{
    "version": "2.0.0",
    "tasks": [
        {
            "label": "Start Homebrew Clangd",
            "type": "shell",
            "command": "/opt/homebrew/opt/llvm/bin/clangd",
            "args": [
                "--compile-commands-dir=${workspaceFolder}/build/macos-arm64",
                "--background-index",
                "--clang-tidy"
            ],
            "isBackground": true
        }
    ]
}
```

## Why This Matters

- **Xcode clangd**: Generic, not optimized for our project
- **Homebrew clangd**: 
  - Configured with 12 threads for M4 Max
  - 16GB memory limit
  - Project-specific settings
  - ARM64 optimizations

## Status Check

Run this to see current status:
```bash
.vscode/scripts/validate-clangd.sh
```