# Cppcheck Usage Guide for LPZRobots C++17 Migration

## Overview

This guide explains how to use cppcheck effectively for the LPZRobots C++17 migration and modernization effort. Cppcheck is configured to understand the project's structure, dependencies, and calling patterns through compile_commands.json.

## Prerequisites

1. Install cppcheck:
```bash
brew install cppcheck
# Optional: HTML report generator
pip install cppcheck-htmlreport
```

2. Generate compile_commands.json:
```bash
cmake -B build/cppcheck -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DBUILD_GUI_TOOLS=OFF
```

## Available Scripts

### 1. `scripts/cppcheck_full.sh` - Comprehensive Analysis
The most thorough analysis with all features enabled:
- Cross-translation-unit (CTU) analysis
- Bug hunting mode
- CERT and MISRA addons
- Full call graph analysis
- Inheritance and member tracking

```bash
./scripts/cppcheck_full.sh
```

**Use when**: You need a complete code review or before major releases.

### 2. `scripts/cppcheck_analysis.sh` - Standard Analysis
Balanced analysis with multiple focused reports:
- C++17 modernization opportunities
- Memory safety issues
- Performance analysis
- Component-specific deep analysis

```bash
./scripts/cppcheck_analysis.sh
```

**Use when**: Regular development, CI/CD pipelines.

### 3. `scripts/cppcheck_cpp17_migration.sh` - C++17 Migration Focus
Specifically targets C++17 modernization:
- NULL to nullptr
- typedef to using
- auto usage opportunities
- STL algorithm usage
- Smart pointer opportunities
- override specifiers

```bash
./scripts/cppcheck_cpp17_migration.sh
```

**Use when**: Planning modernization work.

### 4. `scripts/cppcheck_quick.sh` - Quick Component Analysis
Fast, focused analysis on specific components:

```bash
# Analyze specific component
./scripts/cppcheck_quick.sh selforg

# Focus on specific aspect
./scripts/cppcheck_quick.sh ode_robots cpp17
./scripts/cppcheck_quick.sh ga_tools memory
./scripts/cppcheck_quick.sh selforg perf
```

**Use when**: Quick checks during development.

## Key Features Utilized

### 1. Compile Commands Integration
```bash
cppcheck --project=build/cppcheck/compile_commands.json
```
This ensures cppcheck understands:
- Exact compiler flags
- Include paths
- Preprocessor definitions
- File dependencies

### 2. Cross-Translation-Unit Analysis
```bash
--max-ctu-depth=10
```
Analyzes function calls across files:
- Tracks data flow between compilation units
- Detects issues in function call chains
- Understands class hierarchies

### 3. Library Configuration
```bash
--library=std,posix,qt
```
Proper understanding of:
- STL containers and algorithms
- POSIX functions
- Qt framework specifics

### 4. Suppressions
Two types of suppressions are used:
- **cppcheck_suppressions.txt**: File-based suppressions
- **.cppcheck**: Project configuration with inline suppressions

## Understanding the Output

### Severity Levels
- **error**: Bugs, undefined behavior, memory corruption
- **warning**: Defensive programming issues, potential bugs
- **style**: Code modernization, readability
- **performance**: Optimization opportunities
- **portability**: Platform-specific issues
- **information**: Informational messages

### Output Format
```
file.cpp:123:45: [severity] (id) message
  --> file.cpp:123:45: additional information
```

## C++17 Migration Priorities

Based on cppcheck analysis, focus on:

### 1. High Priority (Safety/Correctness)
- **nullptr usage**: Replace NULL and 0 pointer literals
- **override specifiers**: Add to all virtual overrides
- **Memory management**: Convert to smart pointers

### 2. Medium Priority (Performance)
- **Pass by const reference**: Large objects passed by value
- **STL algorithms**: Replace manual loops
- **Move semantics**: Add move constructors/operators

### 3. Low Priority (Style)
- **auto usage**: Type deduction for readability
- **using declarations**: Replace typedef
- **Range-based for**: Modern loop syntax

## Example Commands

### Full project analysis with specific focus:
```bash
cppcheck \
    --project=build/cppcheck/compile_commands.json \
    --enable=all \
    --std=c++17 \
    --suppress=missingInclude \
    --suppress=unusedFunction \
    --max-ctu-depth=10 \
    --output-file=analysis.txt
```

### Component-specific with modernization focus:
```bash
cppcheck selforg \
    --enable=style,performance,modernize \
    --std=c++17 \
    --template='{file}:{line}: [{id}] {message}' \
    2>&1 | grep -E "(modernize-|performance-|readability-)"
```

### Memory safety check:
```bash
cppcheck \
    --project=build/cppcheck/compile_commands.json \
    --enable=warning,error \
    --std=c++17 \
    --template='{file}:{line}: {message}' \
    2>&1 | grep -E "(leak|delete|free|malloc|nullPointer)"
```

## Integration with Development Workflow

### 1. Pre-commit Check
Add to .git/hooks/pre-commit:
```bash
#!/bin/bash
files=$(git diff --cached --name-only --diff-filter=ACM | grep -E '\.(cpp|h|hpp)$')
if [ -n "$files" ]; then
    cppcheck --quiet --enable=error,warning --std=c++17 $files
fi
```

### 2. CI/CD Integration
Add to GitHub Actions:
```yaml
- name: Run Cppcheck
  run: |
    ./scripts/cppcheck_analysis.sh
    if grep -q "\[error\]" build/cppcheck_reports/*.txt; then
      exit 1
    fi
```

### 3. VS Code Integration
Add to tasks.json:
```json
{
    "label": "Cppcheck Current File",
    "type": "shell",
    "command": "cppcheck",
    "args": [
        "--enable=all",
        "--std=c++17",
        "--suppress=missingInclude",
        "${file}"
    ]
}
```

## Best Practices

1. **Regular Analysis**: Run weekly full analysis
2. **Component Focus**: Analyze modified components before commits
3. **Fix Incrementally**: Address errors first, then warnings
4. **Document Suppressions**: Explain why issues are suppressed
5. **Track Progress**: Monitor issue counts over time

## Interpreting Results for LPZRobots

### Common Patterns
1. **Matrix operations**: Many pass-by-value issues due to matrix library
2. **Controller inheritance**: Missing override specifiers common
3. **Legacy C code**: NULL usage in OpenDE integration
4. **Qt integration**: Some false positives from MOC files

### Component-Specific Notes
- **selforg**: Focus on matrix optimizations
- **ode_robots**: Many inheritance issues
- **ga_tools**: Template-heavy, some false positives
- **guilogger/matrixviz**: Qt-specific suppressions needed

## Next Steps After Analysis

1. Review `ACTION_ITEMS.md` in report directory
2. Create fix branches for specific issue categories
3. Use automated tools where possible (clang-tidy fix-it)
4. Verify fixes don't break functionality
5. Update this guide with new patterns found

## Enhanced Dashboard Features

The new infrastructure includes an interactive dashboard with:
- **Click-to-expand code preview** - See full function/class context
- **Fix suggestions with diffs** - GitHub-style diff visualization
- **Confidence scoring** - Know which fixes are safe to apply
- **Keyboard shortcuts** - j/k navigation, space to expand, f for fix
- **One-click application** - Apply fixes directly from dashboard

Generate the enhanced dashboard:
```bash
./tools/cppcheck/cppcheck cpp17 --format html
# Dashboard will be in tools/cppcheck/reports/{profile}/latest/report.html
```