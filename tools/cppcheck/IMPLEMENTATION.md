# Cppcheck Infrastructure Implementation Details

## Core Implementation

### 1. Wrapper Script Implementation
```bash
#!/bin/bash
# tools/cppcheck/cppcheck (101 lines)

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ANALYZE_SCRIPT="$SCRIPT_DIR/scripts/analyze.py"

# Profile routing
case "$1" in
    quick|full|cpp17|memory|performance|incremental)
        exec python3 "$ANALYZE_SCRIPT" --profile "$1" "${@:2}"
        ;;
    list)
        exec python3 "$ANALYZE_SCRIPT" --list-profiles
        ;;
    help)
        show_help
        ;;
    *)
        echo "Unknown command: $1"
        exit 1
        ;;
esac
```

### 2. Analysis Engine Core
```python
# scripts/analyze.py (886 lines)

class CppcheckAnalyzer:
    def __init__(self, profile_name: str, verbose: bool = False):
        self.profile_name = profile_name
        self.verbose = verbose
        self.profile = self._load_profile(profile_name)
        self.cache = self._init_cache()
        self.project_root = self._find_project_root()
        self.stats = defaultdict(int)
```

#### Cache Implementation
```python
def _init_cache(self) -> Dict:
    cache_file = CACHE_DIR / f"{self.profile_name}_cache.json"
    if cache_file.exists():
        try:
            with open(cache_file) as f:
                return json.load(f)
        except:
            return {}
    return {}

def _save_cache(self):
    cache_file = CACHE_DIR / f"{self.profile_name}_cache.json"
    CACHE_DIR.mkdir(exist_ok=True)
    with open(cache_file, 'w') as f:
        json.dump(self.cache, f, indent=2)
```

#### Changed Files Detection
```python
def _get_changed_files(self) -> Set[Path]:
    changed = set()
    
    # Git diff
    try:
        result = subprocess.run(
            ['git', 'diff', '--name-only', '--diff-filter=ACMRT'],
            capture_output=True,
            text=True,
            cwd=PROJECT_ROOT
        )
        if result.returncode == 0:
            for line in result.stdout.strip().split('\n'):
                if line and any(line.endswith(ext) for ext in ['.cpp', '.h', '.hpp', '.cc', '.cxx']):
                    changed.add(PROJECT_ROOT / line)
    except:
        pass
    
    # Hash comparison
    for component in COMPONENTS:
        component_path = PROJECT_ROOT / component
        if component_path.exists():
            for filepath in component_path.rglob('*.[ch]pp'):
                file_key = str(filepath.relative_to(PROJECT_ROOT))
                file_hash = self._get_file_hash(filepath)
                
                if file_key not in self.cache or self.cache[file_key].get('hash') != file_hash:
                    changed.add(filepath)
    
    return changed
```

### 3. Cppcheck Execution
```python
def _run_cppcheck(self, files: List[Path]) -> Dict:
    cmd = self._build_command(files)
    
    try:
        result = subprocess.run(
            cmd,
            capture_output=True,
            text=True,
            timeout=300,  # 5 minutes max
            cwd=str(PROJECT_ROOT)
        )
        
        if result.returncode not in (0, 1):  # 1 = issues found
            raise RuntimeError(f"Cppcheck failed: {result.stderr}")
            
        return self._parse_output(result.stdout)
        
    except subprocess.TimeoutExpired:
        print(f"{Colors.RED}Analysis timed out{Colors.NC}")
        return {'issues': [], 'stats': {}}
```

#### Command Building
```python
def _build_command(self, files: List[Path]) -> List[str]:
    cmd = ['cppcheck']
    
    # Enable checks from profile
    if 'enable' in self.profile['checks']:
        enables = self.profile['checks']['enable']
        if isinstance(enables, list):
            cmd.extend([f'--enable={e}' for e in enables])
        else:
            cmd.append(f'--enable={enables}')
    
    # Add suppressions
    if BASE_CONFIG.exists():
        cmd.append(f'--suppress-xml={BASE_CONFIG}')
    
    # Performance settings
    cmd.extend([
        '--inline-suppr',
        f'-j{os.cpu_count() or 4}',
        '--quiet',
        '--template={file}:{line}:{column}: [{severity}] ({id}) {message}'
    ])
    
    # Add files
    for f in files:
        cmd.append(str(f))
    
    return cmd
```

### 4. Result Parsing
```python
def _parse_output(self, output: str) -> Dict:
    issues = []
    pattern = re.compile(
        r'^(.+?):(\d+):(\d+): \[(\w+)\] \((\w+)\) (.+)$',
        re.MULTILINE
    )
    
    for match in pattern.finditer(output):
        issue = {
            'file': match.group(1),
            'line': int(match.group(2)),
            'column': int(match.group(3)),
            'severity': match.group(4),
            'id': match.group(5),
            'message': match.group(6)
        }
        issues.append(issue)
        self.stats[issue['severity']] += 1
    
    return {
        'issues': issues,
        'stats': dict(self.stats)
    }
```

### 5. Dashboard Generation

#### Enhanced Dashboard Generator
```python
# scripts/generate_enhanced_dashboard.py (1344 lines)

class EnhancedDashboardGenerator:
    def __init__(self, analysis_results: Dict):
        self.results = analysis_results
        self.project_root = self._find_project_root()
        self.context_extractor = CodeContextExtractor(self.project_root)
        self.fix_generator = FixGenerator()
```

#### Code Context Extraction
```python
def extract_context(self, filepath: str, line_number: int, context_size: int = 15) -> Dict:
    try:
        with open(filepath, 'r', encoding='utf-8', errors='ignore') as f:
            lines = f.readlines()
    except:
        return self._empty_context()
    
    # Find function boundaries
    func_start, func_end = self._find_function_boundaries(lines, line_number - 1)
    
    # Find class info
    class_info = self._find_class_info(lines, line_number - 1)
    
    # Smart boundary calculation
    if func_start is not None:
        if func_end - func_start <= context_size * 2:
            start_line = func_start
            end_line = func_end + 1
        else:
            start_line = max(func_start, line_number - context_size - 1)
            end_line = min(func_end + 1, line_number + context_size)
    else:
        start_line = max(0, line_number - context_size - 1)
        end_line = min(len(lines), line_number + context_size)
    
    return {
        'lines': lines[start_line:end_line],
        'start_line': start_line + 1,
        'target_line': line_number - start_line,
        'function': self._extract_function_name(lines, func_start) if func_start else None,
        'class': class_info
    }
```

#### Fix Generation
```python
# scripts/fix_generator.py (418 lines)

class FixGenerator:
    def __init__(self):
        self.fix_patterns = {
            'noExplicitConstructor': self._fix_explicit_constructor,
            'missingOverride': self._fix_missing_override,
            'useNullptr': self._fix_use_nullptr,
            'passedByValue': self._fix_passed_by_value,
            'unusedVariable': self._fix_unused_variable,
            'uninitMemberVar': self._fix_uninit_member,
            'redundantAssignment': self._fix_redundant_assignment,
            'uselessCallsCompare': self._fix_useless_compare,
            'postfixOperator': self._fix_postfix_operator,
            'useStlAlgorithm': self._fix_use_stl_algorithm
        }
```

#### Pattern Implementation Example
```python
def _fix_missing_override(self, issue: Dict, context: Dict) -> Dict:
    lines = context['lines']
    target_idx = context['target_line']
    
    if target_idx >= len(lines):
        return self._no_fix("Target line out of range")
    
    line = lines[target_idx]
    
    # Pattern to match virtual function declaration
    pattern = r'(.*virtual\s+.*?)(\s*)(;|\s*{|\s*=\s*0\s*;)'
    match = re.match(pattern, line)
    
    if match:
        # Check if override already present
        if 'override' in line:
            return self._no_fix("Override already present")
        
        # Add override before semicolon/brace
        fixed_line = match.group(1) + match.group(2) + ' override' + match.group(3)
        
        fixed_lines = lines.copy()
        fixed_lines[target_idx] = fixed_line
        
        return {
            'success': True,
            'description': 'Add override specifier to virtual function',
            'confidence': 98,
            'fixed_lines': fixed_lines,
            'diff': self._generate_diff(lines, fixed_lines, context)
        }
    
    return self._no_fix("Could not match virtual function pattern")
```

### 6. Metrics Database
```python
# scripts/metrics.py (878 lines)

class MetricsTracker:
    def __init__(self, db_path: Path = Path('metrics.db')):
        self.db_path = db_path
        self.conn = sqlite3.connect(str(db_path))
        self._init_database()
```

#### Schema Implementation
```python
def _init_database(self):
    self.conn.executescript("""
        CREATE TABLE IF NOT EXISTS analyses (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,
            profile TEXT NOT NULL,
            component TEXT,
            total_issues INTEGER,
            errors INTEGER,
            warnings INTEGER,
            style INTEGER,
            performance INTEGER,
            portability INTEGER,
            information INTEGER,
            files_analyzed INTEGER,
            files_cached INTEGER,
            analysis_time REAL,
            cache_hit_rate REAL,
            version TEXT
        );
        
        CREATE TABLE IF NOT EXISTS issue_trends (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,
            issue_type TEXT NOT NULL,
            issue_id TEXT NOT NULL,
            count INTEGER,
            component TEXT,
            severity TEXT,
            file_pattern TEXT
        );
        
        CREATE TABLE IF NOT EXISTS file_metrics (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,
            filepath TEXT NOT NULL,
            total_issues INTEGER,
            complexity INTEGER,
            lines_of_code INTEGER,
            last_modified DATETIME
        );
        
        CREATE INDEX IF NOT EXISTS idx_analyses_timestamp 
            ON analyses(timestamp);
        CREATE INDEX IF NOT EXISTS idx_trends_type 
            ON issue_trends(issue_type);
        CREATE INDEX IF NOT EXISTS idx_file_metrics_path 
            ON file_metrics(filepath);
    """)
```

### 7. Autofix Implementation
```python
# scripts/autofix.py (404 lines)

class CppModernizer:
    def __init__(self, dry_run: bool = True, verbose: bool = False):
        self.dry_run = dry_run
        self.verbose = verbose
        self.backup_dir = None
        self.report = {
            'files_analyzed': 0,
            'files_modified': 0,
            'changes_made': 0,
            'errors': []
        }
```

#### Fix Application
```python
def fix_nullptr(self, files: Optional[List[Path]] = None):
    """Replace NULL with nullptr"""
    if files is None:
        files = self._get_cpp_files()
    
    self._print_header("NULL → nullptr Migration")
    
    patterns = [
        (r'\bNULL\b', 'nullptr'),
        (r'(\w+)\s*==\s*0\b', r'\1 == nullptr'),
        (r'0\s*==\s*(\w+)', r'nullptr == \1'),
        (r'(\w+)\s*!=\s*0\b', r'\1 != nullptr'),
        (r'0\s*!=\s*(\w+)', r'nullptr != \1')
    ]
    
    for filepath in files:
        changes = self._apply_patterns_to_file(filepath, patterns)
        if changes > 0:
            self.report['changes_made'] += changes
            self.report['files_modified'] += 1
            print(f"  {Colors.GREEN}✓{Colors.NC} {filepath}: {changes} replacements")
```

### 8. JavaScript Implementation (Dashboard)
```javascript
// In generate_enhanced_dashboard.py

const DashboardController = {
    currentPage: 1,
    itemsPerPage: 100,
    currentFilter: 'all',
    searchTerm: '',
    
    init() {
        this.bindEvents();
        this.loadIssues();
        this.setupKeyboardShortcuts();
    },
    
    bindEvents() {
        document.getElementById('searchInput').addEventListener('input', 
            debounce((e) => this.search(e.target.value), 300));
        
        document.querySelectorAll('.filter-btn').forEach(btn => {
            btn.addEventListener('click', (e) => this.filter(e.target.dataset.filter));
        });
    },
    
    setupKeyboardShortcuts() {
        document.addEventListener('keydown', (e) => {
            if (e.target.tagName === 'INPUT') return;
            
            switch(e.key) {
                case 'j': this.selectNext(); break;
                case 'k': this.selectPrevious(); break;
                case ' ': e.preventDefault(); this.expandSelected(); break;
                case 'f': this.showFix(); break;
                case '/': e.preventDefault(); this.focusSearch(); break;
                case '?': this.showHelp(); break;
            }
        });
    }
};
```

### 9. Performance Optimizations

#### Parallel Processing
```python
def analyze_files_parallel(self, files: List[Path]) -> List[Dict]:
    with ThreadPoolExecutor(max_workers=os.cpu_count()) as executor:
        # Submit all tasks
        future_to_file = {
            executor.submit(self._analyze_single_file, f): f 
            for f in files if not self._is_cached(f)
        }
        
        # Collect results
        results = []
        for future in as_completed(future_to_file):
            try:
                result = future.result(timeout=30)
                results.extend(result)
            except Exception as e:
                print(f"Error analyzing {future_to_file[future]}: {e}")
        
        return results
```

#### Memory Efficient Processing
```python
def process_large_report(self, report_path: Path):
    """Process large JSON reports without loading all into memory"""
    with open(report_path) as f:
        # Parse JSON streaming
        parser = ijson.parse(f)
        
        for prefix, event, value in parser:
            if prefix.endswith('.issue'):
                # Process single issue
                yield self._process_issue(value)
```

### 10. Error Recovery
```python
def safe_analyze(self, filepath: Path) -> List[Dict]:
    """Analyze with multiple fallback strategies"""
    try:
        # Try normal analysis
        return self._run_cppcheck([filepath])
    except subprocess.TimeoutExpired:
        # Try with reduced checks
        self.profile['checks'] = {'enable': ['warning']}
        try:
            return self._run_cppcheck([filepath])
        except:
            # Final fallback - return cached or empty
            return self.cache.get(str(filepath), {'issues': []})
    except Exception as e:
        logger.error(f"Analysis failed for {filepath}: {e}")
        return {'issues': []}
```

## Security Implementation

### Path Traversal Protection
```python
def validate_filepath(self, filepath: Path) -> Path:
    """Ensure file is within project boundaries"""
    try:
        resolved = filepath.resolve()
        project = self.project_root.resolve()
        
        # Check if path is within project
        resolved.relative_to(project)
        
        # Check file type
        if resolved.suffix not in ALLOWED_EXTENSIONS:
            raise ValueError(f"Invalid file type: {resolved.suffix}")
        
        # Check not symlink to outside
        if resolved.is_symlink():
            target = Path(os.readlink(resolved)).resolve()
            target.relative_to(project)
        
        return resolved
        
    except ValueError:
        raise ValueError(f"Path outside project: {filepath}")
```

### Command Injection Prevention
```python
def build_safe_command(self, files: List[str]) -> List[str]:
    """Build command without shell injection risk"""
    cmd = ['cppcheck']
    
    # Validate each argument
    for arg in self.profile.get('args', []):
        if not re.match(r'^--[\w-]+=?[\w/.,-]*$', arg):
            raise ValueError(f"Invalid argument: {arg}")
        cmd.append(arg)
    
    # Add files as separate arguments
    for f in files:
        # No shell metacharacters
        if any(c in f for c in ';|&$`'):
            raise ValueError(f"Invalid filename: {f}")
        cmd.append(f)
    
    return cmd
```

## Production Optimizations

### 1. Lazy Loading
```python
_ultimate_generator = None

def get_ultimate_generator():
    global _ultimate_generator
    if _ultimate_generator is None:
        from generate_ultimate_report import UltimateDashboardGenerator
        _ultimate_generator = UltimateDashboardGenerator
    return _ultimate_generator
```

### 2. Resource Cleanup
```python
class AnalysisContext:
    def __enter__(self):
        self.temp_files = []
        return self
    
    def __exit__(self, exc_type, exc_val, exc_tb):
        # Cleanup temp files
        for tf in self.temp_files:
            try:
                os.unlink(tf)
            except:
                pass
        
        # Force garbage collection
        gc.collect()
```

### 3. Stream Processing
```python
def analyze_streaming(self, files: Iterator[Path]) -> Iterator[Dict]:
    """Process files as stream to limit memory usage"""
    for batch in chunked(files, 100):  # Process in batches
        results = self._analyze_batch(batch)
        for result in results:
            yield result
        
        # Cleanup between batches
        self._cleanup_cache()
```