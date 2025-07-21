# Cppcheck Infrastructure Documentation Index

## Core Documentation

### For Users
- **[README.md](README.md)** - Quick start and command reference
- **[USAGE.md](USAGE.md)** - Practical workflows and examples
- **[docs/DASHBOARD_GUIDE.md](docs/DASHBOARD_GUIDE.md)** - Interactive dashboard features

### For Developers  
- **[DEVELOPMENT.md](DEVELOPMENT.md)** - Setup, testing, and contribution guide
- **[IMPLEMENTATION.md](IMPLEMENTATION.md)** - Technical implementation details
- **[RESEARCH.md](RESEARCH.md)** - Design decisions and performance research

## Quick Reference

### Commands
```bash
./cppcheck quick              # Fast analysis
./cppcheck cpp17 --format html # C++17 modernization
./cppcheck memory             # Memory safety check
./cppcheck incremental        # Changed files only
```

### Key Features
- **5 Analysis Profiles** - Specialized for different needs
- **10x Faster** - Smart incremental caching
- **Interactive Dashboard** - Code preview and fix suggestions
- **Automated Fixes** - Safe C++17 modernization
- **CI/CD Ready** - GitHub Actions and pre-commit hooks

## Architecture Overview

```
Input → Profile → Analysis → Cache → Report → Dashboard
  ↓        ↓         ↓        ↓       ↓         ↓
Files   JSON    Cppcheck   Hash   JSON    HTML+JS
```

## Component Map

| Component | Purpose | Lines |
|-----------|---------|-------|
| analyze.py | Core engine | 886 |
| generate_enhanced_dashboard.py | Interactive UI | 1344 |
| autofix.py | Automated fixes | 404 |
| metrics.py | Tracking/trends | 878 |
| fix_generator.py | Fix patterns | 418 |

## Support

- **Issues**: Check troubleshooting in [README.md](README.md#troubleshooting)
- **Development**: See [DEVELOPMENT.md](DEVELOPMENT.md)
- **Examples**: Browse [USAGE.md](USAGE.md#real-world-examples)