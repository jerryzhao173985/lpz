# Cppcheck Infrastructure Development Guide

## Development Setup

```bash
# Clone and setup
git clone <repo>
cd tools/cppcheck

# Verify Python 3.6+
python3 --version

# Install cppcheck
brew install cppcheck  # macOS
apt install cppcheck   # Linux

# Run tests
python3 scripts/validate_scripts.py
python3 scripts/test_scripts.py
```

## Architecture Deep Dive

### Core Components

#### 1. Wrapper Script (`cppcheck`)
```bash
#!/bin/bash
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ANALYZE_SCRIPT="$SCRIPT_DIR/scripts/analyze.py"

case "$1" in
    quick|full|cpp17|memory|performance|incremental)
        exec python3 "$ANALYZE_SCRIPT" --profile "$1" "${@:2}"
        ;;
esac
```

**Purpose**: Unified entry point, profile routing

#### 2. Analysis Engine (`scripts/analyze.py`)

**Key Classes**:
```python
class CppcheckAnalyzer:
    def __init__(self, profile_name: str):
        self.profile = self._load_profile(profile_name)
        self.cache = self._init_cache()
        self.project_root = self._find_project_root()
```

**Critical Methods**:
- `_get_changed_files()`: Git diff + hash comparison
- `_run_cppcheck()`: Subprocess with timeout
- `_parse_results()`: XML/text parsing
- `_generate_report()`: Format routing

#### 3. Dashboard Generators

**Hierarchy**:
```
BasicHTMLGenerator
    ↓
EnhancedDashboardGenerator
    ↓
UltimateDashboardGenerator
```

**Pattern**: Progressive enhancement
```python
try:
    from generate_ultimate_report import UltimateDashboardGenerator
    return UltimateDashboardGenerator(results).generate()
except ImportError:
    # Fallback to enhanced/basic
```

### Data Flow

```
Input Files → Cppcheck Binary → XML Output → Parser → JSON → Dashboard
     ↓              ↓               ↓          ↓        ↓         ↓
  Hash Cache   Subprocess      ErrorHandler  Filter  Transform  HTML
```

## Adding New Features

### 1. New Analysis Profile

Create `profiles/security.json`:
```json
{
  "name": "security",
  "description": "Security vulnerability detection",
  "checks": {
    "enable": ["warning", "security"],
    "addons": ["cert.py", "misra.py"]
  },
  "specific": [
    "bufferAccessOutOfBounds",
    "insecureCmdLineArgs",
    "uninitvar"
  ],
  "settings": {
    "check-level": "exhaustive",
    "bug-hunting": true
  }
}
```

Update wrapper:
```bash
# In cppcheck script
security)
    exec python3 "$ANALYZE_SCRIPT" --profile security "$@"
    ;;
```

### 2. New Fix Pattern

Add to `fix_generator.py`:
```python
def _fix_const_method(self, issue: Dict, context: Dict) -> Dict:
    """Add const to methods that don't modify state"""
    pattern = r'(\w+\s+\w+\s*\([^)]*\))\s*{' 
    replacement = r'\1 const {'
    
    # Detect if method modifies members
    if not self._modifies_state(context['lines']):
        return {
            'success': True,
            'pattern': pattern,
            'replacement': replacement,
            'confidence': 92,
            'description': 'Add const qualifier'
        }
```

Register pattern:
```python
self.fix_patterns = {
    'constMethod': self._fix_const_method,
    # ... existing patterns
}
```

### 3. New Dashboard Feature

Add to `generate_enhanced_dashboard.py`:
```javascript
// In _generate_javascript()
function addMetricsChart() {
    const data = [{
        x: issuesData.map(i => i.file),
        y: issuesData.map(i => i.complexity),
        type: 'bar'
    }];
    
    Plotly.newPlot('metricsChart', data);
}
```

Update HTML template:
```html
<div id="metricsChart" class="chart-container"></div>
```

## Testing

### Unit Tests

Create `scripts/test_analyze.py`:
```python
import unittest
from analyze import CppcheckAnalyzer

class TestAnalyzer(unittest.TestCase):
    def test_cache_hit(self):
        analyzer = CppcheckAnalyzer('quick_check')
        # Create known file
        test_file = Path('/tmp/test.cpp')
        test_file.write_text('int main() {}')
        
        # First run - cache miss
        result1 = analyzer._analyze_file(test_file)
        self.assertFalse(analyzer._is_cached(test_file))
        
        # Second run - cache hit
        result2 = analyzer._analyze_file(test_file)
        self.assertTrue(analyzer._is_cached(test_file))
        self.assertEqual(result1, result2)
```

### Integration Tests

```bash
#!/bin/bash
# test_integration.sh

# Test each profile
for profile in quick cpp17 memory performance; do
    echo "Testing $profile..."
    ./cppcheck $profile --files test/fixtures/*.cpp
    
    # Verify output exists
    [ -f "reports/$profile/latest/report.json" ] || exit 1
done

# Test dashboard generation
python3 scripts/generate_enhanced_dashboard.py \
    reports/quick_check/latest/report.json \
    /tmp/test_dashboard.html

# Verify HTML
grep -q "issuesData" /tmp/test_dashboard.html || exit 1
```

### Performance Tests

```python
# scripts/benchmark.py
import time
from pathlib import Path

def benchmark_analysis():
    files = list(Path('../../selforg').rglob('*.cpp'))[:100]
    
    # Without cache
    start = time.time()
    subprocess.run(['./cppcheck', 'quick', '--no-cache'])
    no_cache_time = time.time() - start
    
    # With cache (second run)
    start = time.time()
    subprocess.run(['./cppcheck', 'quick'])
    cache_time = time.time() - start
    
    print(f"Without cache: {no_cache_time:.2f}s")
    print(f"With cache: {cache_time:.2f}s")
    print(f"Speedup: {no_cache_time/cache_time:.1f}x")
```

## Debugging

### Enable Verbose Logging

```python
# In analyze.py
if os.environ.get('CPPCHECK_DEBUG'):
    logging.basicConfig(level=logging.DEBUG)
    
logger.debug(f"Analyzing {filepath}")
logger.debug(f"Cache hit: {is_cached}")
```

Use:
```bash
CPPCHECK_DEBUG=1 ./cppcheck quick
```

### Profile Execution

```python
# Add to analyze.py
if args.profile_execution:
    import cProfile
    pr = cProfile.Profile()
    pr.enable()
    # ... main code ...
    pr.disable()
    pr.dump_stats('cppcheck.prof')
```

Analyze:
```bash
python3 -m pstats cppcheck.prof
> sort cumtime
> stats 20
```

### Memory Profiling

```python
# memory_profile.py
from memory_profiler import profile

@profile
def analyze_large_file():
    analyzer = CppcheckAnalyzer('comprehensive')
    analyzer.analyze([Path('large_file.cpp')])
```

Run:
```bash
python3 -m memory_profiler memory_profile.py
```

## Common Patterns

### 1. Safe File Operations
```python
def safe_read_file(filepath: Path) -> List[str]:
    try:
        with open(filepath, 'r', encoding='utf-8', errors='ignore') as f:
            return f.readlines()
    except (IOError, OSError) as e:
        logger.warning(f"Cannot read {filepath}: {e}")
        return []
```

### 2. Process Timeout
```python
def run_with_timeout(cmd: List[str], timeout: int = 30) -> subprocess.CompletedProcess:
    try:
        return subprocess.run(
            cmd,
            capture_output=True,
            text=True,
            timeout=timeout,
            check=False
        )
    except subprocess.TimeoutExpired:
        logger.error(f"Command timed out: {' '.join(cmd)}")
        return subprocess.CompletedProcess(cmd, 1, '', 'Timeout')
```

### 3. JSON Error Handling
```python
def parse_json_safe(text: str) -> Dict:
    try:
        return json.loads(text)
    except json.JSONDecodeError as e:
        logger.error(f"Invalid JSON: {e}")
        # Return empty but valid structure
        return {'issues': [], 'stats': {}}
```

### 4. Path Validation
```python
def validate_project_path(filepath: Path, project_root: Path) -> bool:
    try:
        filepath = filepath.resolve()
        project_root = project_root.resolve()
        filepath.relative_to(project_root)
        return True
    except ValueError:
        return False
```

## Code Style Guide

### Python Style
- PEP 8 compliance
- Type hints where helpful
- Docstrings for public methods
- f-strings for formatting

### JavaScript Style
- ES6+ features
- Const by default
- Arrow functions for callbacks
- Template literals

### Error Messages
```python
# Good
raise ValueError(f"Profile '{profile_name}' not found in {PROFILES_DIR}")

# Bad
raise ValueError("Invalid profile")
```

## Release Process

### 1. Version Bump
```python
# In analyze.py
VERSION = "1.2.0"  # Semantic versioning
```

### 2. Test Suite
```bash
# Run all tests
./run_tests.sh

# Verify no regressions
python3 scripts/metrics.py --compare v1.1.0 v1.2.0
```

### 3. Documentation
- Update CHANGELOG.md
- Update command examples
- Verify all docs accurate

### 4. Package
```bash
# Create release archive
tar -czf cppcheck-infra-v1.2.0.tar.gz \
    --exclude='*.pyc' \
    --exclude='__pycache__' \
    --exclude='.git' \
    --exclude='reports' \
    tools/cppcheck/
```

## Performance Optimization

### 1. Lazy Imports
```python
def generate_ultimate_dashboard():
    # Import only when needed
    from plotly import graph_objects as go
    # Use go...
```

### 2. Generator Patterns
```python
def parse_large_file(filepath):
    with open(filepath) as f:
        for line in f:  # Generator, not list
            yield parse_line(line)
```

### 3. Caching Decorators
```python
from functools import lru_cache

@lru_cache(maxsize=1000)
def expensive_parse(content: str) -> Dict:
    # Parsing code
    return result
```

## Troubleshooting Development

### Issue: Import Errors
```python
# Add to scripts
import sys
from pathlib import Path
sys.path.insert(0, str(Path(__file__).parent))
```

### Issue: Subprocess Hangs
```python
# Use Popen for better control
proc = subprocess.Popen(cmd, stdout=PIPE, stderr=PIPE)
try:
    stdout, stderr = proc.communicate(timeout=30)
except subprocess.TimeoutExpired:
    proc.kill()
    stdout, stderr = proc.communicate()
```

### Issue: Memory Leaks
```python
# Explicit cleanup
def analyze_files(files):
    for f in files:
        result = analyze(f)
        process_result(result)
        # Explicit cleanup
        del result
        gc.collect()  # Force garbage collection
```