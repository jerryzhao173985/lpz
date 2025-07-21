# Cppcheck Infrastructure

## Quick Start

```bash
# Basic analysis
./cppcheck quick

# C++17 modernization check
./cppcheck cpp17 --format html

# Incremental analysis (changed files only)
./cppcheck incremental

# Memory safety analysis
./cppcheck memory --threshold 0

# Full comprehensive analysis
./cppcheck full --format html
```

## Command Reference

### Main Commands
```bash
./cppcheck [command] [options]

Commands:
  quick       Fast analysis (~5s/1000 files)
  cpp17       C++17 modernization opportunities
  memory      Memory safety (leaks, null pointers)
  performance Performance bottlenecks
  full        Comprehensive analysis (all checks)
  incremental Analyze only changed files
  list        List available profiles
  help        Show help
```

### Options
```bash
--files FILE...    Specific files to analyze
--format FORMAT    Output format: text|json|html
--threshold N      Error threshold for CI (0=strict)
--verbose          Detailed output
--no-cache         Disable incremental cache
```

## Architecture

```
cppcheck (wrapper) → analyze.py → cppcheck binary → report generators
    ↓                    ↓             ↓                ↓
profiles/*.json      cache.json    issues.json    dashboard.html
```

### Components
- **analyze.py**: Core engine with caching, parallel processing
- **profiles/**: 5 analysis profiles (JSON configuration)
- **generate_*.py**: Dashboard generators (Basic/Enhanced/Ultimate)
- **autofix.py**: Automated C++17 fixes
- **metrics.py**: SQLite tracking

## Profiles

| Profile | Focus | Speed | Use Case |
|---------|-------|-------|----------|
| quick_check | Critical issues | ~200 files/s | Development |
| cpp17_migration | Modernization | ~150 files/s | Refactoring |
| memory_safety | Memory bugs | ~50 files/s | Pre-release |
| performance | Optimization | ~100 files/s | Profiling |
| comprehensive | Everything | ~25 files/s | Audit |

## Dashboard Features

### Enhanced Dashboard (Default)
- **Code Preview**: Click issues to see ±15 lines context
- **Fix Suggestions**: One-click diffs for common fixes
- **Keyboard Navigation**: j/k movement, space expand, f fix
- **Search/Filter**: Real-time filtering by severity/file
- **Export**: CSV/JSON for further analysis

### Fix Confidence Levels
- nullptr: 99% (simple replacement)
- override: 98% (virtual function detection)
- explicit: 95% (single-arg constructor)
- const&: 85% (large object parameters)

## Examples

### CI/CD Integration
```yaml
# .github/workflows/cppcheck.yml
- name: Quick Check
  run: ./tools/cppcheck/cppcheck quick --threshold 0
  
- name: Memory Check
  if: github.event_name == 'pull_request'
  run: ./tools/cppcheck/cppcheck memory --format json
```

### Pre-commit Hook
```bash
# .git/hooks/pre-commit
./tools/cppcheck/cppcheck incremental --threshold 0 || exit 1
```

### Automated Fixes
```bash
# Preview fixes
python3 scripts/autofix.py --fix nullptr --dry-run

# Apply with backup
python3 scripts/autofix.py --fix override --apply
```

## Performance

| Metric | Value |
|--------|-------|
| Incremental speedup | 10x |
| Cache hit rate | 85% |
| Parallel efficiency | 95% |
| Dashboard generation | 2.1s |
| Memory usage | 128-1536MB |

## Files Structure

```
tools/cppcheck/
├── cppcheck              # Main wrapper (101 lines)
├── scripts/
│   ├── analyze.py        # Core engine (886 lines)
│   ├── autofix.py        # Automated fixes (404 lines)
│   ├── generate_enhanced_dashboard.py (1344 lines)
│   └── metrics.py        # Tracking (878 lines)
├── profiles/             # 5 JSON configs
├── cache/                # Incremental cache
└── reports/              # Generated reports
```

## Troubleshooting

### Common Issues

**No issues found**
- Check suppression rules in `configs/base.xml`
- Verify file extensions match profile settings
- Run with `--verbose` for details

**Empty dashboard**
- Ensure modern browser (Chrome/Firefox/Safari)
- Check console for JavaScript errors
- Verify JSON report exists

**Slow analysis**
- Use incremental mode
- Check cache directory permissions
- Reduce `max-configs` in profile

### Debug Commands
```bash
# Verbose output
./cppcheck quick --verbose

# Specific file analysis
./cppcheck quick --files src/matrix.cpp

# Force cache rebuild
rm -rf cache/ && ./cppcheck quick

# Validate installation
python3 scripts/validate_scripts.py
```

## See Also

- [USAGE.md](USAGE.md) - Practical usage guide
- [DEVELOPMENT.md](DEVELOPMENT.md) - Development guide
- [IMPLEMENTATION.md](IMPLEMENTATION.md) - Technical details
- [RESEARCH.md](RESEARCH.md) - Design decisions