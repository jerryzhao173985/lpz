# Cppcheck Usage Guide

## Installation Verification

```bash
# Check installation
which cppcheck
cppcheck --version

# Validate scripts
cd tools/cppcheck
python3 scripts/validate_scripts.py

# Test run
./cppcheck quick --files /tmp/test.cpp
```

## Common Workflows

### 1. Development Workflow

```bash
# Before coding - baseline
./cppcheck quick

# After changes - incremental
./cppcheck incremental

# Before commit - strict
./cppcheck quick --threshold 0
```

### 2. C++17 Modernization

```bash
# Find opportunities
./cppcheck cpp17 --format html

# Preview fixes
python3 scripts/autofix.py --fix nullptr --dry-run

# Apply fixes with backup
python3 scripts/autofix.py --fix nullptr --apply

# Verify changes
git diff
./cppcheck incremental
```

### 3. Memory Safety Audit

```bash
# Deep analysis
./cppcheck memory --format html

# Focus on specific component
./cppcheck memory --files selforg/**/*.cpp

# CI integration
./cppcheck memory --threshold 0 --format json
```

### 4. Performance Optimization

```bash
# Find bottlenecks
./cppcheck performance --format html

# Check specific patterns
grep "passedByValue" reports/performance/*/report.txt
```

### 5. Pre-Release Validation

```bash
# Full analysis
./cppcheck full --format html

# Generate metrics
python3 scripts/metrics.py --import reports/comprehensive/latest

# View trends
python3 scripts/metrics.py --trend --days 30
```

## Real-World Examples

### Example 1: Fix Missing Override

**Issue Found**:
```cpp
class Derived : public Base {
    virtual void process() { }  // Missing override
};
```

**Commands**:
```bash
# Find all missing overrides
./cppcheck cpp17 --format json
cat reports/cpp17_migration/latest/report.json | jq '.issues[] | select(.id=="missingOverride")'

# Generate fix
python3 scripts/fix_generator.py reports/cpp17_migration/latest/report.json

# Apply fix
python3 scripts/autofix.py --fix override --files src/derived.cpp --apply
```

**Result**:
```cpp
class Derived : public Base {
    virtual void process() override { }  // Fixed
};
```

### Example 2: Memory Leak Detection

**Code**:
```cpp
void process() {
    int* data = new int[100];
    if (error) return;  // Leak!
    delete[] data;
}
```

**Commands**:
```bash
# Detect leak
./cppcheck memory --files src/leaky.cpp

# View in dashboard
open reports/memory_safety/latest/report.html
```

### Example 3: Performance Issue

**Code**:
```cpp
void process(std::vector<Data> items) {  // Pass by value
    for (const auto& item : items) { }
}
```

**Fix Process**:
```bash
# Detect issue
./cppcheck performance

# View suggestion
# Dashboard shows: "Pass by const reference"

# Apply manually (high impact on API)
void process(const std::vector<Data>& items) {
```

## Dashboard Navigation

### Keyboard Shortcuts
- `j/k` - Navigate issues
- `Space` - Expand code preview
- `f` - Show fix suggestion
- `/` - Focus search
- `Esc` - Close modal
- `?` - Show help

### Filtering
```javascript
// Search syntax
error:nullPointer     // Specific error
file:matrix.cpp      // Specific file
line:>100           // Line number range
severity:error      // By severity
```

### Export Options
1. **CSV Export**: For spreadsheet analysis
2. **JSON Export**: For custom tools
3. **Copy Issue**: Right-click → Copy

## CI/CD Integration

### GitHub Actions

```yaml
name: Code Quality
on: [push, pull_request]

jobs:
  cppcheck:
    runs-on: ubuntu-latest
    steps:
    - uses: actions/checkout@v3
    
    - name: Install Cppcheck
      run: |
        sudo apt-get update
        sudo apt-get install -y cppcheck
    
    - name: Quick Check
      run: |
        ./tools/cppcheck/cppcheck quick --threshold 0
    
    - name: Upload Report
      if: failure()
      uses: actions/upload-artifact@v3
      with:
        name: cppcheck-report
        path: tools/cppcheck/reports/
```

### Pre-commit Hook

```bash
#!/bin/bash
# .git/hooks/pre-commit

# Get staged C++ files
STAGED_FILES=$(git diff --cached --name-only --diff-filter=ACM | grep -E '\.(cpp|h|hpp|cc|cxx)$')

if [ -z "$STAGED_FILES" ]; then
    exit 0
fi

# Run incremental check
echo "Running cppcheck..."
./tools/cppcheck/cppcheck incremental --threshold 0

if [ $? -ne 0 ]; then
    echo "❌ Cppcheck found issues. Please fix before committing."
    echo "Run './tools/cppcheck/cppcheck incremental --format html' to see details"
    exit 1
fi

echo "✅ Cppcheck passed"
```

### GitLab CI

```yaml
cppcheck:
  stage: test
  script:
    - ./tools/cppcheck/cppcheck quick --format json
    - python3 tools/cppcheck/scripts/metrics.py --import reports/
  artifacts:
    reports:
      junit: reports/junit.xml
    paths:
      - tools/cppcheck/reports/
```

## Metrics Tracking

### Import Results
```bash
# Single analysis
python3 scripts/metrics.py --import reports/quick_check/20250721_140218

# Batch import
find reports -name "report.json" -exec python3 scripts/metrics.py --import {} \;
```

### View Trends
```bash
# Command line
python3 scripts/metrics.py --trend

# Export for graphing
python3 scripts/metrics.py --export-csv metrics.csv

# HTML dashboard
python3 scripts/metrics.py --export-html metrics.html
```

### Database Queries
```bash
# Direct SQL
sqlite3 metrics.db "
SELECT DATE(timestamp) as date, 
       SUM(errors) as total_errors,
       SUM(warnings) as total_warnings
FROM analyses 
GROUP BY DATE(timestamp)
ORDER BY date DESC
LIMIT 30"
```

## Advanced Usage

### Custom Profiles

Create `profiles/custom.json`:
```json
{
  "name": "custom",
  "description": "Project-specific checks",
  "checks": {
    "enable": ["warning", "style"],
    "disable": ["unusedFunction"]
  },
  "specific": [
    "nullPointer",
    "uninitvar",
    "modernize-use-nullptr"
  ],
  "suppressions": [
    "missingIncludeSystem"
  ],
  "settings": {
    "max-configs": 20,
    "check-level": "exhaustive"
  }
}
```

Use:
```bash
./cppcheck custom --format html
```

### Suppression Rules

Edit `configs/base.xml`:
```xml
<suppressions>
  <!-- Suppress in external headers -->
  <suppress>
    <id>*</id>
    <fileName>/usr/include/*</fileName>
  </suppress>
  
  <!-- Project-specific -->
  <suppress>
    <id>unusedFunction</id>
    <fileName>*/test/*</fileName>
  </suppress>
</suppressions>
```

### Batch Processing

```bash
# Analyze multiple components
for component in selforg ode_robots opende; do
    echo "Analyzing $component..."
    ./cppcheck quick --files "$component/**/*.cpp" \
        --format json > "reports/${component}_analysis.json"
done

# Merge results
python3 -c "
import json, glob
all_issues = []
for f in glob.glob('reports/*_analysis.json'):
    all_issues.extend(json.load(open(f))['issues'])
json.dump({'issues': all_issues}, open('reports/merged.json', 'w'))
"
```

## Best Practices

### 1. Start with Quick Check
- Fastest feedback
- Catches common issues
- Good for development

### 2. Use Incremental Mode
- 10x faster
- Same accuracy
- Ideal for large codebases

### 3. Review Fix Suggestions
- Check confidence score
- Understand the fix
- Test after applying

### 4. Track Metrics
- Monitor trends
- Set quality gates
- Celebrate improvements

### 5. Customize for Your Project
- Create custom profiles
- Add suppressions wisely
- Document decisions

## Troubleshooting

### Issue: Analysis too slow
```bash
# Check what's being analyzed
./cppcheck quick --verbose | grep "Checking"

# Use incremental
./cppcheck incremental

# Reduce scope
./cppcheck quick --files "src/core/**/*.cpp"
```

### Issue: Too many false positives
```bash
# Add suppressions
echo "unusedFunction:*/generated/*" >> configs/suppressions.txt

# Use specific profile
./cppcheck quick  # Instead of comprehensive
```

### Issue: Dashboard not loading
```bash
# Check JSON validity
python3 -m json.tool reports/latest/report.json

# Regenerate
rm -rf reports/latest
./cppcheck quick --format html

# Use basic dashboard
python3 scripts/generate_basic_report.py reports/latest/report.json
```