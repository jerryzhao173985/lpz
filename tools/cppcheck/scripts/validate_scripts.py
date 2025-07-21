#!/usr/bin/env python3
"""
Validate all Python scripts for production readiness
"""

import os
import sys
import subprocess
from pathlib import Path
import ast
import tempfile
import platform

class Colors:
    RED = '\033[0;31m'
    GREEN = '\033[0;32m'
    YELLOW = '\033[1;33m'
    BLUE = '\033[0;34m'
    CYAN = '\033[0;36m'
    NC = '\033[0m'

def check_script(script_path: Path) -> dict:
    """Check a single Python script for common issues"""
    issues = []
    warnings = []
    
    # Check if file exists and is readable
    if not script_path.exists():
        return {'error': f'File does not exist: {script_path}'}
    
    if not os.access(script_path, os.R_OK):
        return {'error': f'File is not readable: {script_path}'}
    
    # Check shebang
    with open(script_path, 'r', encoding='utf-8', errors='ignore') as f:
        first_line = f.readline().strip()
        if not first_line.startswith('#!/usr/bin/env python3'):
            issues.append('Missing or incorrect shebang line')
        
        content = first_line + '\n' + f.read()
    
    # Check executable bit
    if not os.access(script_path, os.X_OK):
        warnings.append('File is not executable (missing +x permission)')
    
    # Parse AST to check for common issues
    try:
        tree = ast.parse(content)
        
        # Check for bare except clauses
        for node in ast.walk(tree):
            if isinstance(node, ast.ExceptHandler) and node.type is None:
                issues.append(f'Bare except clause at line {node.lineno}')
        
        # Check for hardcoded paths
        for node in ast.walk(tree):
            if isinstance(node, ast.Constant) and isinstance(node.value, str):
                if any(path in node.value for path in ['/Users/', '/home/', 'C:\\']):
                    issues.append(f'Possible hardcoded path at line {node.lineno}: {node.value[:50]}...')
        
    except SyntaxError as e:
        issues.append(f'Syntax error: {e}')
    
    # Check imports
    missing_imports = []
    try:
        # Try to compile
        compile(content, script_path, 'exec')
        
        # Check if script can be imported (in isolated environment)
        result = subprocess.run(
            [sys.executable, '-c', f'import sys; sys.path.insert(0, "{script_path.parent}"); import {script_path.stem}'],
            capture_output=True,
            text=True,
            timeout=5
        )
        if result.returncode != 0 and 'ModuleNotFoundError' in result.stderr:
            # Extract missing module
            import re
            match = re.search(r"No module named '(\w+)'", result.stderr)
            if match:
                missing_imports.append(match.group(1))
    except Exception as e:
        warnings.append(f'Could not check imports: {str(e)}')
    
    # Check for proper main guard
    if '__main__' in content and 'if __name__' not in content:
        warnings.append('Missing proper if __name__ == "__main__" guard')
    
    # Check for docstring
    try:
        tree = ast.parse(content)
        if not ast.get_docstring(tree):
            warnings.append('Missing module-level docstring')
    except:
        pass
    
    # Check for proper error handling in main()
    if 'def main(' in content:
        if 'sys.exit(main())' not in content and 'return' in content:
            warnings.append('main() function should be called with sys.exit(main())')
    
    # Platform-specific checks
    if platform.system() == 'Windows':
        # Check for Unix-specific commands
        unix_commands = ['grep', 'sed', 'awk', 'ls', 'chmod']
        for cmd in unix_commands:
            if f"'{cmd}" in content or f'"{cmd}' in content:
                warnings.append(f'Uses Unix command "{cmd}" which may not work on Windows')
    
    return {
        'issues': issues,
        'warnings': warnings,
        'missing_imports': missing_imports
    }

def main():
    """Validate all Python scripts in the current directory"""
    script_dir = Path(__file__).parent
    all_good = True
    
    print(f"{Colors.BLUE}Validating Python scripts for production readiness...{Colors.NC}\n")
    
    # Find all Python scripts
    scripts = sorted(script_dir.glob('*.py'))
    
    for script in scripts:
        if script.name == 'validate_scripts.py':
            continue
            
        print(f"Checking {Colors.CYAN}{script.name}{Colors.NC}...")
        result = check_script(script)
        
        if 'error' in result:
            print(f"  {Colors.RED}✗ ERROR: {result['error']}{Colors.NC}")
            all_good = False
        else:
            if result['issues']:
                all_good = False
                for issue in result['issues']:
                    print(f"  {Colors.RED}✗ ISSUE: {issue}{Colors.NC}")
            
            if result['warnings']:
                for warning in result['warnings']:
                    print(f"  {Colors.YELLOW}⚠ WARNING: {warning}{Colors.NC}")
            
            if result['missing_imports']:
                for imp in result['missing_imports']:
                    print(f"  {Colors.YELLOW}⚠ MISSING IMPORT: {imp}{Colors.NC}")
            
            if not result['issues'] and not result['warnings'] and not result['missing_imports']:
                print(f"  {Colors.GREEN}✓ All checks passed{Colors.NC}")
    
    # Additional checks
    print(f"\n{Colors.BLUE}Running additional checks...{Colors.NC}")
    
    # Check Python version compatibility
    print(f"Python version: {sys.version}")
    if sys.version_info < (3, 6):
        print(f"{Colors.RED}✗ Python 3.6+ required{Colors.NC}")
        all_good = False
    else:
        print(f"{Colors.GREEN}✓ Python version OK{Colors.NC}")
    
    # Check for __pycache__ cleanup
    pycache_dirs = list(script_dir.glob('**/__pycache__'))
    if pycache_dirs:
        print(f"{Colors.YELLOW}⚠ Found {len(pycache_dirs)} __pycache__ directories{Colors.NC}")
    
    # Summary
    print(f"\n{Colors.BLUE}Summary:{Colors.NC}")
    if all_good:
        print(f"{Colors.GREEN}✓ All scripts are production-ready!{Colors.NC}")
        return 0
    else:
        print(f"{Colors.RED}✗ Some issues need to be addressed{Colors.NC}")
        return 1

if __name__ == '__main__':
    sys.exit(main())
