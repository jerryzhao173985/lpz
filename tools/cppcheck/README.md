# LPZRobots Cppcheck Infrastructure

A comprehensive, modular static analysis system designed specifically for the LPZRobots C++ codebase, focusing on C++17 modernization, memory safety, and performance optimization.

## 🚀 Quick Start

```bash
# List available analysis profiles
./tools/cppcheck/cppcheck list

# Run quick analysis (for development)
./tools/cppcheck/cppcheck quick

# Run C++17 migration analysis
./tools/cppcheck/cppcheck cpp17 --format html

# Run incremental analysis on changed files
./tools/cppcheck/cppcheck incremental

# Run comprehensive analysis
./tools/cppcheck/cppcheck full
```

## 📋 Features

### 1. **Modular Profile System**
- **quick_check**: Fast analysis for development (~10s)
- **cpp17_migration**: Identifies C++17 modernization opportunities
- **memory_safety**: Deep analysis for memory leaks and pointer issues
- **performance**: Finds optimization opportunities
- **comprehensive**: Full analysis with all checks enabled

### 2. **Advanced Capabilities**
- **Incremental Analysis**: Only analyzes changed files (10x faster)
- **Smart Caching**: Remembers previous results to skip unchanged files
- **Parallel Processing**: Uses all CPU cores for faster analysis
- **Cross-Translation Unit (CTU)**: Understands relationships across files
- **Compile Commands Integration**: Uses actual build flags for accuracy

### 3. **Automated Fixes**
```bash
# Preview C++17 modernizations (dry run)
python3 tools/cppcheck/scripts/autofix.py --fix all --dry-run

# Apply specific fixes
python3 tools/cppcheck/scripts/autofix.py --fix nullptr --apply
python3 tools/cppcheck/scripts/autofix.py --fix override --apply
python3 tools/cppcheck/scripts/autofix.py --fix using --apply
```

### 4. **CI/CD Integration**
- GitHub Actions workflows for automated checking
- Pre-commit hooks for local validation
- Configurable error thresholds
- PR commenting with analysis results

## 📁 Directory Structure

```
tools/cppcheck/
├── configs/          # Base configurations
│   └── base.xml     # Foundation config for all profiles
├── profiles/         # Analysis profiles (JSON)
│   ├── quick_check.json
│   ├── cpp17_migration.json
│   ├── memory_safety.json
│   ├── performance.json
│   └── comprehensive.json
├── scripts/          # Core analysis scripts
│   ├── analyze.py   # Main analysis engine
│   └── autofix.py   # Automated fix generator
├── hooks/           # Git integration
│   ├── pre-commit   # Pre-commit hook
│   └── install.sh   # Hook installer
├── reports/         # Analysis reports (auto-generated)
├── cache/           # Incremental analysis cache
└── cppcheck         # Main wrapper script
```

## 🔧 Installation

### Prerequisites
```bash
# macOS
brew install cppcheck python3

# Ubuntu/Debian
sudo apt-get install cppcheck python3 python3-pip

# Optional: For HTML reports
pip3 install cppcheck-htmlreport pygments
```

### Setup
```bash
# 1. Ensure cppcheck is in your PATH
which cppcheck

# 2. Generate compile_commands.json (recommended)
cmake -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

# 3. Install pre-commit hook (optional)
./tools/cppcheck/hooks/install.sh
```

## 📊 Analysis Profiles

### Quick Check Profile
- **Purpose**: Fast feedback during development
- **Time**: ~10-30 seconds
- **Checks**: Errors and warnings only
- **Use Case**: Pre-commit, rapid iteration

### C++17 Migration Profile
- **Purpose**: Identify modernization opportunities
- **Checks**: nullptr, override, auto, using, constexpr
- **Output**: Prioritized list of changes
- **Use Case**: Gradual codebase modernization

### Memory Safety Profile
- **Purpose**: Find memory-related bugs
- **Checks**: Leaks, buffer overflows, null pointers
- **Features**: Bug hunting mode, exhaustive analysis
- **Use Case**: Pre-release validation

### Performance Profile
- **Purpose**: Identify optimization opportunities
- **Checks**: Pass-by-value, STL usage, redundant operations
- **Output**: Performance metrics and suggestions
- **Use Case**: Performance tuning

### Comprehensive Profile
- **Purpose**: Complete code analysis
- **Time**: 30+ minutes
- **Checks**: All available checks
- **Use Case**: Weekly/monthly deep analysis

## 🛠️ Advanced Usage

### Incremental Analysis
```bash
# Analyze only files changed since last commit
./tools/cppcheck/cppcheck incremental

# Analyze specific files
./tools/cppcheck/cppcheck quick --files src/robot.cpp src/controller.cpp
```

### Custom Profiles
Create a new profile in `profiles/my_profile.json`:
```json
{
  "name": "My Custom Profile",
  "extends": "base",
  "checks": {
    "enable": ["warning", "style"],
    "specific": ["passedByValue", "unusedFunction"]
  },
  "suppressions": ["missingInclude"]
}
```

### CI/CD Integration
```yaml
# In your CI pipeline
- name: Run Cppcheck
  run: |
    python3 tools/cppcheck/scripts/analyze.py \
      --profile quick_check \
      --threshold 10 \
      --format json
```

### Report Formats
```bash
# Text report (default)
./tools/cppcheck/cppcheck cpp17 --format text

# HTML report with visualizations
./tools/cppcheck/cppcheck cpp17 --format html

# JSON for programmatic processing
./tools/cppcheck/cppcheck cpp17 --format json
```

## 🔍 Understanding Results

### Severity Levels
- **error**: Must fix - will cause runtime problems
- **warning**: Should fix - potential bugs or bad practices
- **style**: Consider fixing - code quality improvements
- **performance**: Optimization opportunities
- **portability**: Cross-platform compatibility issues

### Common Issues and Fixes

#### 1. Missing nullptr
```cpp
// Before
if (ptr == NULL) { }

// After (use autofix)
if (ptr == nullptr) { }
```

#### 2. Missing override
```cpp
// Before
virtual void update();

// After (use autofix)
void update() override;
```

#### 3. Pass by value
```cpp
// Before
void setName(std::string name);

// After
void setName(const std::string& name);
```

## 📈 Metrics and Progress Tracking

The analysis system tracks:
- Number of issues by category
- Files with most issues
- Fix rate over time
- Performance metrics

Access metrics:
```bash
# View latest statistics
cat tools/cppcheck/reports/*/STATISTICS.md

# Generate trend report
python3 tools/cppcheck/scripts/metrics.py --trend
```

## 🤝 Contributing

### Adding New Checks
1. Create or modify a profile in `profiles/`
2. Add specific check IDs to the profile
3. Test with a small set of files first
4. Document the new checks

### Improving Autofix
1. Add new fix patterns to `autofix.py`
2. Ensure fixes are safe and preserve functionality
3. Always create backups before applying
4. Test on multiple code patterns

## 🐛 Troubleshooting

### Common Issues

**"compile_commands.json not found"**
```bash
cmake -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
```

**"cppcheck: command not found"**
```bash
# Install cppcheck
brew install cppcheck  # macOS
sudo apt-get install cppcheck  # Ubuntu
```

**Analysis takes too long**
```bash
# Use quick profile or incremental mode
./tools/cppcheck/cppcheck quick
./tools/cppcheck/cppcheck incremental
```

**Too many false positives**
- Edit the profile to add suppressions
- Use inline suppressions: `// cppcheck-suppress unusedFunction`
- Update `base.xml` exclude paths

## 📚 References

- [Cppcheck Manual](http://cppcheck.sourceforge.net/manual.pdf)
- [C++ Core Guidelines](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines)
- [LPZRobots Coding Standards](../../docs/CODING_STANDARDS.md)

## 📝 License

This infrastructure is part of the LPZRobots project and follows the same license terms.