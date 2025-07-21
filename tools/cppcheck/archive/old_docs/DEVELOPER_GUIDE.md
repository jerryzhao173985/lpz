# Cppcheck Developer Guide for LPZRobots

This guide provides in-depth information for developers working with the LPZRobots cppcheck infrastructure.

## 🎯 Philosophy and Goals

Our static analysis approach is designed to:
1. **Minimize false positives** - Only report actionable issues
2. **Maximize developer productivity** - Fast feedback loops
3. **Support gradual modernization** - C++17 migration without breaking changes
4. **Ensure code quality** - Catch bugs before runtime
5. **Enable automation** - Reduce manual review burden

## 🏗️ Architecture Overview

### Component Relationships
```
┌─────────────────┐     ┌──────────────┐     ┌─────────────┐
│ cppcheck wrapper│────▶│  analyze.py  │────▶│  cppcheck   │
└─────────────────┘     └──────────────┘     └─────────────┘
         │                      │                     │
         │                      ▼                     ▼
         │              ┌──────────────┐     ┌─────────────┐
         │              │   Profiles   │     │ compile_cmds│
         │              └──────────────┘     └─────────────┘
         │                      │
         ▼                      ▼
┌─────────────────┐     ┌──────────────┐
│    autofix.py   │     │    Cache     │
└─────────────────┘     └──────────────┘
```

### Key Design Decisions

1. **Python-based orchestration**: Provides flexibility and cross-platform support
2. **JSON profiles**: Easy to modify and version control
3. **Compile commands integration**: Ensures analysis uses actual build flags
4. **Incremental analysis**: Critical for large codebases
5. **Modular suppressions**: Different profiles have different noise tolerance

## 🔧 Advanced Configuration

### Creating Custom Profiles

Profile structure:
```json
{
  "name": "Profile Name",
  "description": "What this profile does",
  "extends": "base",  // Optional: inherit from another profile
  "checks": {
    "enable": ["warning", "style"],  // Categories to enable
    "disable": ["information"],      // Categories to disable
    "specific": [                    // Specific check IDs
      "nullPointer",
      "memleak",
      "uninitvar"
    ]
  },
  "settings": {
    "check-level": "normal|exhaustive",
    "max-configs": 12,
    "max-ctu-depth": 10,
    "bug-hunting": false,
    "inconclusive": false
  },
  "suppressions": [
    "missingInclude",
    "unusedFunction"
  ],
  "paths": {
    "include": ["src", "include"],
    "exclude": ["tests", "third_party"]
  },
  "output": {
    "template": "{file}:{line}: {message}",
    "quiet": false,
    "verbose": false
  }
}
```

### Performance Tuning

#### For Large Codebases
```python
# In analyze.py, adjust these settings:
CACHE_EXPIRY_DAYS = 7  # How long to keep cached results
MAX_PARALLEL_JOBS = multiprocessing.cpu_count()
CHUNK_SIZE = 50  # Files per analysis batch
```

#### Memory Usage
```bash
# Limit cppcheck memory usage
export CPPCHECK_MAX_CTU_DEPTH=5
export CPPCHECK_MAX_CONFIGS=10

# Run with memory profiling
/usr/bin/time -l ./tools/cppcheck/cppcheck full
```

### Suppression Strategies

#### 1. Global Suppressions (base.xml)
```xml
<suppressions>
    <suppression>missingIncludeSystem</suppression>
    <suppression>unmatchedSuppression</suppression>
</suppressions>
```

#### 2. Profile-Specific (profiles/*.json)
```json
"suppressions": [
    "unusedFunction",
    "constParameter"
]
```

#### 3. Inline Suppressions (in code)
```cpp
// cppcheck-suppress uninitvar
int x;  // Intentionally uninitialized

// cppcheck-suppress-begin unusedFunction
void debug_only_function() { }
// cppcheck-suppress-end unusedFunction
```

#### 4. File-Level Suppressions
```cpp
// cppcheck-suppress-file unusedFunction
// At the top of a file to suppress for entire file
```

## 🔍 Deep Dive: Analysis Features

### Cross-Translation Unit (CTU) Analysis

CTU enables cppcheck to understand relationships across files:

```cpp
// file1.cpp
void setGlobalState(int x) {
    globalVar = x;  // CTU tracks this assignment
}

// file2.cpp
void useGlobalState() {
    if (globalVar == 0) {  // CTU knows possible values
        // ...
    }
}
```

Enable with:
```json
"settings": {
    "max-ctu-depth": 20  // Higher = more thorough but slower
}
```

### Bug Hunting Mode

Experimental deep analysis using Z3 solver:
```json
"settings": {
    "bug-hunting": true,
    "bug-hunting-check-function-max-time": 300
}
```

Best for:
- Buffer overflow detection
- Integer overflow analysis
- Complex conditional logic

### Value Flow Analysis

Tracks value ranges through code:
```cpp
void example(int x) {
    if (x > 10) {
        // Cppcheck knows x >= 11 here
        int arr[5];
        arr[x] = 0;  // Warns: array index out of bounds
    }
}
```

## 🛠️ Extending the System

### Adding New Automated Fixes

In `autofix.py`, add a new fix method:
```python
def fix_my_pattern(self, files: Optional[List[Path]] = None):
    """Fix description"""
    print(f"{Colors.BLUE}Fixing my pattern...{Colors.NC}")
    
    patterns = [
        (r'old_pattern', 'new_pattern'),
    ]
    
    # Implementation similar to existing fixes
    # ...
```

### Custom Check Rules

Create custom rules using regular expressions:
```xml
<!-- In custom_rules.xml -->
<rules>
    <rule>
        <pattern>sprintf\s*\(</pattern>
        <id>unsafeSprintf</id>
        <severity>warning</severity>
        <summary>Use snprintf instead of sprintf</summary>
    </rule>
</rules>
```

Use in profile:
```json
"rule-files": ["custom_rules.xml"]
```

### Integration with Other Tools

#### ClangFormat Integration
```python
# In autofix.py, after applying fixes:
subprocess.run(['clang-format', '-i', str(filepath)])
```

#### Clang-Tidy Coordination
```bash
# Run both tools in sequence
./tools/cppcheck/cppcheck quick && \
clang-tidy -p build src/*.cpp
```

## 📊 Metrics and Reporting

### Custom Metrics Collection

Add to `analyze.py`:
```python
def collect_custom_metrics(self, issues: List[Dict]) -> Dict:
    metrics = {
        'complexity_issues': 0,
        'api_misuse': 0,
        'deprecated_calls': 0
    }
    
    for issue in issues:
        if 'complexity' in issue['message']:
            metrics['complexity_issues'] += 1
        # ... more categorization
    
    return metrics
```

### Trend Analysis

Track improvement over time:
```python
# metrics_tracker.py
import sqlite3
from datetime import datetime

class MetricsTracker:
    def __init__(self, db_path='metrics.db'):
        self.conn = sqlite3.connect(db_path)
        self.create_tables()
        
    def record_analysis(self, profile, stats):
        self.conn.execute("""
            INSERT INTO analyses (timestamp, profile, errors, warnings)
            VALUES (?, ?, ?, ?)
        """, (datetime.now(), profile, stats['errors'], stats['warnings']))
```

## 🐛 Debugging Cppcheck Issues

### Verbose Diagnostics
```bash
# Maximum verbosity
cppcheck --debug --debug-warnings --verbose \
    --enable=all src/file.cpp

# Check preprocessor output
cppcheck -E src/file.cpp > preprocessed.cpp
```

### Common Problems and Solutions

#### "Too many configurations"
```bash
# Limit configurations
cppcheck --max-configs=1 --platform=unix64 src/
```

#### "Cannot find include file"
```bash
# Add include paths explicitly
cppcheck -I/path/to/includes --suppress=missingInclude src/
```

#### "Analysis hangs"
```bash
# Set timeouts
cppcheck --max-ctu-depth=2 --check-level=normal src/
```

## 🔒 Security Considerations

### Running on Untrusted Code
```python
# In analyze.py, add sandboxing:
import resource

# Limit CPU time (seconds)
resource.setrlimit(resource.RLIMIT_CPU, (300, 300))

# Limit memory (bytes)
resource.setrlimit(resource.RLIMIT_AS, (2 * 1024 * 1024 * 1024, -1))
```

### Suppression Security
Never suppress:
- Buffer overflows
- Format string vulnerabilities
- SQL injection patterns
- Command injection risks

## 📈 Performance Optimization

### Profiling the Analysis
```bash
# Profile Python scripts
python3 -m cProfile -o profile.out tools/cppcheck/scripts/analyze.py

# Analyze profile
python3 -m pstats profile.out
```

### Caching Strategy
```python
# Implement smart cache invalidation
def should_invalidate_cache(self, file_path: Path) -> bool:
    # Check file hash
    current_hash = self._get_file_hash(file_path)
    cached_hash = self.cache.get(str(file_path), {}).get('hash')
    
    if current_hash != cached_hash:
        return True
        
    # Check include dependencies
    includes = self._get_includes(file_path)
    for inc in includes:
        if self._get_file_hash(inc) != self.cache.get(str(inc), {}).get('hash'):
            return True
            
    return False
```

## 🎓 Best Practices

### 1. Profile Selection
- Development: `quick_check`
- Pre-commit: `quick_check` with specific files
- PR review: `cpp17_migration` + `memory_safety`
- Release: `comprehensive`

### 2. Suppression Philosophy
- Suppress at the narrowest scope possible
- Document why suppressions are needed
- Review suppressions quarterly
- Never suppress security-critical checks

### 3. Fix Prioritization
1. Security issues (buffer overflows, injections)
2. Memory safety (leaks, use-after-free)
3. Logic errors (null derefs, uninitialized vars)
4. Performance (unnecessary copies, inefficient algorithms)
5. Style (naming, formatting)

### 4. Incremental Adoption
```bash
# Week 1: Fix all errors
./tools/cppcheck/cppcheck quick --threshold 0

# Week 2: Fix critical warnings
python3 tools/cppcheck/scripts/autofix.py --fix nullptr --apply

# Week 3: Start performance improvements
./tools/cppcheck/cppcheck performance
```

## 🔗 Integration Examples

### VS Code Integration
```json
// .vscode/tasks.json
{
    "version": "2.0.0",
    "tasks": [
        {
            "label": "Cppcheck Current File",
            "type": "shell",
            "command": "${workspaceFolder}/tools/cppcheck/cppcheck",
            "args": ["quick", "--files", "${file}"],
            "problemMatcher": {
                "pattern": {
                    "regexp": "^(.*):(\\d+):\\s+\\[(\\w+)\\]\\s+(.*)$",
                    "file": 1,
                    "line": 2,
                    "severity": 3,
                    "message": 4
                }
            }
        }
    ]
}
```

### Pre-Push Hook
```bash
#!/bin/bash
# .git/hooks/pre-push

# Run comprehensive check before pushing
./tools/cppcheck/cppcheck cpp17 --threshold 50 || {
    echo "Too many C++17 issues. Fix before pushing."
    exit 1
}
```

## 📚 Further Reading

- [Cppcheck Internals](http://cppcheck.sourceforge.net/devinfo/)
- [Static Analysis in Large Projects](https://www.viva64.com/en/b/0532/)
- [C++ Core Guidelines Checker](https://docs.microsoft.com/en-us/cpp/code-quality/using-the-cpp-core-guidelines-checkers)

---

For questions or improvements, please open an issue or submit a PR to the LPZRobots repository.