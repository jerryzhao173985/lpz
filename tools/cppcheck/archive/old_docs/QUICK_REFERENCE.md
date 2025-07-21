# Cppcheck Quick Reference

## 🚀 Common Commands

```bash
# Quick analysis (development)
./tools/cppcheck/cppcheck quick

# Check changed files only
./tools/cppcheck/cppcheck incremental

# C++17 migration analysis
./tools/cppcheck/cppcheck cpp17

# Memory safety check
./tools/cppcheck/cppcheck memory

# Performance analysis
./tools/cppcheck/cppcheck performance

# Full analysis (slow)
./tools/cppcheck/cppcheck full

# Check specific files
./tools/cppcheck/cppcheck quick --files src/robot.cpp src/controller.cpp

# Generate HTML report
./tools/cppcheck/cppcheck cpp17 --format html

# List all profiles
./tools/cppcheck/cppcheck list
```

## 🔧 Automated Fixes

```bash
# Preview all fixes (safe - no changes)
python3 tools/cppcheck/scripts/autofix.py --dry-run

# Apply specific fixes
python3 tools/cppcheck/scripts/autofix.py --fix nullptr --apply
python3 tools/cppcheck/scripts/autofix.py --fix override --apply
python3 tools/cppcheck/scripts/autofix.py --fix using --apply
python3 tools/cppcheck/scripts/autofix.py --fix auto --apply

# Fix specific files only
python3 tools/cppcheck/scripts/autofix.py --fix nullptr --files src/robot.cpp --apply
```

## 📋 Inline Suppressions

```cpp
// Suppress single line
int x; // cppcheck-suppress uninitvar

// Suppress with comment
// cppcheck-suppress nullPointer - false positive, checked above
if (ptr) ptr->method();

// Suppress block
// cppcheck-suppress-begin unusedFunction
void debug_function() { }
void test_function() { }
// cppcheck-suppress-end unusedFunction

// Suppress entire file
// cppcheck-suppress-file unusedFunction
```

## 🎯 Profile Comparison

| Profile | Speed | Coverage | Use Case |
|---------|-------|----------|----------|
| quick_check | ⚡⚡⚡ | Basic | Development |
| cpp17_migration | ⚡⚡ | Modernization | Refactoring |
| memory_safety | ⚡ | Deep | Bug hunting |
| performance | ⚡⚡ | Optimization | Tuning |
| comprehensive | 🐌 | Everything | Release |

## 🛑 Error Thresholds

```bash
# Fail if any errors
./tools/cppcheck/cppcheck quick --threshold 0

# Allow up to 10 errors
./tools/cppcheck/cppcheck quick --threshold 10

# Check specific severity
python3 tools/cppcheck/scripts/analyze.py --profile quick_check --threshold 5
```

## 🔍 Common Issues & Fixes

### NULL → nullptr
```cpp
// Before
if (ptr == NULL) { }

// After
if (ptr == nullptr) { }
```

### Missing override
```cpp
// Before
virtual void update();

// After
void update() override;
```

### typedef → using
```cpp
// Before
typedef std::vector<int> IntVector;

// After
using IntVector = std::vector<int>;
```

### Explicit constructor
```cpp
// Before
class Foo {
    Foo(int x);
};

// After
class Foo {
    explicit Foo(int x);
};
```

### Pass by const reference
```cpp
// Before
void process(std::string name);

// After
void process(const std::string& name);
```

## 🔧 Git Integration

```bash
# Install pre-commit hook
./tools/cppcheck/hooks/install.sh

# Skip pre-commit check once
git commit --no-verify

# Uninstall hook
rm .git/hooks/pre-commit
```

## 📊 Reports Location

```
tools/cppcheck/reports/
├── quick_check/
│   └── 20240115_120000/
│       ├── report.txt
│       └── report.html
├── cpp17_migration/
├── memory_safety/
└── comprehensive/
```

## ⚙️ Environment Variables

```bash
# Disable colors
export CPPCHECK_NO_COLOR=1

# Set custom cache directory
export CPPCHECK_CACHE_DIR=/tmp/cppcheck_cache

# Enable debug output
export CPPCHECK_DEBUG=1

# Set analysis timeout (seconds)
export CPPCHECK_TIMEOUT=300
```

## 🚨 Emergency Commands

```bash
# Clear all cache
rm -rf tools/cppcheck/cache/*

# Reset to defaults
git checkout tools/cppcheck/profiles/

# Kill stuck analysis
pkill -f cppcheck

# Analyze single file with maximum verbosity
cppcheck --enable=all --verbose --debug src/problem_file.cpp
```

## 📝 VS Code Integration

```json
// settings.json
{
    "task.autoDetect": "off",
    "editor.formatOnSave": true,
    "[cpp]": {
        "editor.defaultFormatter": "ms-vscode.cpptools"
    }
}
```

Keyboard shortcut for current file check:
```json
// keybindings.json
{
    "key": "ctrl+shift+c",
    "command": "workbench.action.tasks.runTask",
    "args": "Cppcheck Current File"
}
```

## 🔗 Useful Links

- [Full Documentation](./README.md)
- [Developer Guide](./DEVELOPER_GUIDE.md)
- [Cppcheck Manual](http://cppcheck.sourceforge.net/manual.pdf)
- [Project Issues](https://github.com/georgmartius/lpzrobots/issues)