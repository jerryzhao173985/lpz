# Clangd Troubleshooting Guide

## Current Status (2025-07-13 15:26)

### Issue
- Homebrew clangd is not starting
- Xcode clangd keeps taking over
- Microsoft C++ extensions may still be interfering

### Diagnosis Results
1. ✅ Homebrew clangd is installed and working (`/opt/homebrew/opt/llvm/bin/clangd`)
2. ✅ Configuration in settings.json is correct
3. ✅ compile_commands.json exists with 283 entries
4. ❌ VSCode is not launching the correct clangd
5. ⚠️ Microsoft C++ extensions are still installed (though disabled in settings)

## Manual Solutions

### Solution 1: Force Uninstall Microsoft C++ Extensions
```bash
# Completely uninstall (not just disable) the extensions
code --uninstall-extension ms-vscode.cpptools
code --uninstall-extension ms-vscode.cpptools-extension-pack
code --uninstall-extension ms-vscode.cpptools-themes

# Then reload VSCode
```

### Solution 2: Manual clangd Start
1. Open integrated terminal in VSCode
2. Run:
```bash
/opt/homebrew/opt/llvm/bin/clangd \
  --compile-commands-dir=build/macos-arm64 \
  --background-index \
  --clang-tidy \
  --header-insertion=never \
  -j=12 \
  --pch-storage=memory
```

### Solution 3: Check VSCode Output
1. Open Output panel: `View → Output` (Cmd+Shift+U)
2. Select "clangd" from dropdown
3. Look for error messages about why it's not starting

### Solution 4: Developer Tools Console
1. Help → Toggle Developer Tools
2. Go to Console tab
3. Look for extension errors
4. Search for "clangd" or "cpptools"

## Alternative Approach

If clangd still won't start with VSCode:

### Use Cursor or Another Editor
Since the cpptools processes are from Cursor editor, there might be a conflict.

### Create a Clean VSCode Profile
```bash
# Start VSCode with a clean profile
code --user-data-dir=/tmp/vscode-clean --extensions-dir=/tmp/vscode-ext
```

Then install ONLY clangd extension.

## Performance Without Clangd

Even without the optimized clangd running, you still have:
- ✅ Code formatting with clang-format
- ✅ CMake integration
- ✅ Build system working
- ❌ Slower IntelliSense (using Xcode's clangd)
- ❌ Not using M4 Max optimizations

## Next Steps

1. **Check Output Panel** for clangd errors
2. **Uninstall** (not just disable) MS C++ extensions
3. **Open workspace file** (lpzrobots.code-workspace)
4. **Report** any error messages you see

The configuration is correct; the issue is with extension conflicts or VSCode not picking up the path correctly.