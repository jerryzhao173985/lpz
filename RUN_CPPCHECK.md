# Quick Cppcheck Commands for LPZRobots

## Most Useful Commands

### 1. Quick C++17 Migration Check
```bash
# See what needs modernization
./scripts/cppcheck_cpp17_migration.sh
```

### 2. Check Specific Component
```bash
# Quick analysis of selforg
./scripts/cppcheck_quick.sh selforg

# Memory issues in ode_robots
./scripts/cppcheck_quick.sh ode_robots memory

# Performance in ga_tools
./scripts/cppcheck_quick.sh ga_tools perf
```

### 3. Full Analysis (Comprehensive)
```bash
# Complete analysis with all checks
./scripts/cppcheck_full.sh
```

### 4. Standard Development Check
```bash
# Balanced analysis for regular development
./scripts/cppcheck_analysis.sh
```

## Direct Command Examples

### Basic Check with Project Understanding
```bash
cppcheck --project=build/cppcheck/compile_commands.json \
         --enable=warning,error,performance \
         --std=c++17 \
         --suppress=missingInclude \
         --quiet
```

### Find C++17 Modernization Opportunities
```bash
cppcheck --project=build/cppcheck/compile_commands.json \
         --enable=style,performance \
         --std=c++17 \
         --suppress=missingInclude \
         --template='{file}:{line}: {message}' \
         2>&1 | grep -E "(useAuto|modernize|nullptr|override)"
```

### Memory Safety Focus
```bash
cppcheck --project=build/cppcheck/compile_commands.json \
         --enable=warning,error \
         --std=c++17 \
         --suppress=missingInclude \
         2>&1 | grep -E "(leak|delete|nullptr|uninit)"
```

## Setup Required

First time only:
```bash
# Install cppcheck
brew install cppcheck

# Generate compile_commands.json
cmake -B build/cppcheck -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DBUILD_GUI_TOOLS=OFF

# Make scripts executable
chmod +x scripts/cppcheck_*.sh
```

## Output Location

All reports are saved to:
```
build/cppcheck_reports/
├── comprehensive_YYYYMMDD_HHMMSS/
├── cpp17_migration/
└── *.txt (individual reports)
```

## Tips

1. Start with `cppcheck_quick.sh` for fast feedback
2. Use `cppcheck_cpp17_migration.sh` to plan modernization work
3. Run `cppcheck_full.sh` before major commits
4. Check reports in `build/cppcheck_reports/` for detailed analysis

## New Infrastructure Features

The cppcheck infrastructure now includes:
- **Interactive Dashboard** - Click issues to see full code context
- **Automated Fixes** - One-click fixes with diff preview
- **Keyboard Navigation** - j/k to navigate, space to expand
- **CI/CD Integration** - GitHub Actions workflow ready
- **Smart Caching** - 10x faster incremental analysis

See `docs/CPPCHECK_USAGE_GUIDE.md` for complete documentation.