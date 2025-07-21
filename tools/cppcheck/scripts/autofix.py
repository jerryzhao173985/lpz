#!/usr/bin/env python3
"""
LPZRobots C++17 Automated Fix Generator
Automatically applies safe C++17 modernizations based on cppcheck findings
"""

import argparse
import re
import os
import sys
import subprocess
import shutil
from pathlib import Path
from typing import List, Dict, Tuple, Optional
import json
from datetime import datetime

# Constants
SCRIPT_DIR = Path(__file__).parent
TOOLS_DIR = SCRIPT_DIR.parent
PROJECT_ROOT = TOOLS_DIR.parent

# Colors
class Colors:
    RED = '\033[0;31m'
    GREEN = '\033[0;32m'
    YELLOW = '\033[1;33m'
    BLUE = '\033[0;34m'
    MAGENTA = '\033[0;35m'
    NC = '\033[0m'

class CppModernizer:
    def __init__(self, dry_run: bool = True, verbose: bool = False):
        self.dry_run = dry_run
        self.verbose = verbose
        self.fixes_applied = 0
        self.files_modified = set()
        self.backup_dir = TOOLS_DIR / 'backups' / datetime.now().strftime('%Y%m%d_%H%M%S')
        
    def backup_file(self, filepath: Path):
        """Create backup of file before modification"""
        if not self.dry_run:
            backup_path = self.backup_dir / filepath.relative_to(PROJECT_ROOT)
            backup_path.parent.mkdir(parents=True, exist_ok=True)
            shutil.copy2(filepath, backup_path)
            
    def apply_fix(self, filepath: Path, line_num: int, old_text: str, new_text: str, fix_type: str):
        """Apply a single fix to a file"""
        if self.dry_run:
            print(f"{Colors.YELLOW}[DRY RUN]{Colors.NC} {filepath}:{line_num} - {fix_type}")
            print(f"  - {old_text}")
            print(f"  + {new_text}")
            return True
            
        # Read file
        with open(filepath, 'r') as f:
            lines = f.readlines()
            
        # Apply fix
        if line_num <= len(lines):
            # Create backup on first modification
            if filepath not in self.files_modified:
                self.backup_file(filepath)
                self.files_modified.add(filepath)
                
            # Replace line
            lines[line_num - 1] = lines[line_num - 1].replace(old_text, new_text)
            
            # Write back
            with open(filepath, 'w') as f:
                f.writelines(lines)
                
            self.fixes_applied += 1
            if self.verbose:
                print(f"{Colors.GREEN}Applied:{Colors.NC} {filepath}:{line_num} - {fix_type}")
            return True
            
        return False
        
    def fix_nullptr(self, files: Optional[List[Path]] = None):
        """Replace NULL with nullptr"""
        print(f"{Colors.BLUE}Fixing NULL → nullptr...{Colors.NC}")
        
        # Patterns to replace
        patterns = [
            (r'\bNULL\b', 'nullptr'),
            (r'\b0\s*==\s*(\w+)', r'nullptr == \1'),
            (r'(\w+)\s*==\s*0\b', r'\1 == nullptr'),
            (r'\b0\s*!=\s*(\w+)', r'nullptr != \1'),
            (r'(\w+)\s*!=\s*0\b', r'\1 != nullptr'),
        ]
        
        if not files:
            # Find all C++ files
            files = []
            for ext in ['*.cpp', '*.h', '*.hpp', '*.cc', '*.cxx']:
                files.extend(PROJECT_ROOT.rglob(ext))
                
        for filepath in files:
            # Skip excluded directories
            if any(part in str(filepath) for part in ['build', '.git', 'opende/OPCODE', 'tests']):
                continue
                
            with open(filepath, 'r') as f:
                content = f.read()
                lines = content.split('\n')
                
            for line_num, line in enumerate(lines, 1):
                # Skip comments and strings
                if '//' in line or '/*' in line or '"' in line:
                    continue
                    
                for pattern, replacement in patterns:
                    if re.search(pattern, line):
                        new_line = re.sub(pattern, replacement, line)
                        if new_line != line:
                            self.apply_fix(filepath, line_num, line.strip(), new_line.strip(), 'nullptr')
                            
    def fix_override(self, files: Optional[List[Path]] = None):
        """Add missing override specifiers"""
        print(f"{Colors.BLUE}Adding missing override specifiers...{Colors.NC}")
        
        # Run cppcheck to find missing overrides
        cmd = [
            'cppcheck',
            '--enable=style',
            '--std=c++17',
            '--template={file}:{line}:{message}',
            '--quiet'
        ]
        
        if files:
            cmd.extend([str(f) for f in files])
        else:
            cmd.extend(['selforg', 'ode_robots', 'ga_tools'])
            
        result = subprocess.run(cmd, capture_output=True, text=True, cwd=PROJECT_ROOT)
        
        # Parse output for override suggestions
        for line in result.stderr.split('\n'):
            if 'override' in line.lower() and ':' in line:
                parts = line.split(':', 2)
                if len(parts) >= 2:
                    filepath = PROJECT_ROOT / parts[0]
                    try:
                        line_num = int(parts[1])
                        
                        # Read the specific line
                        with open(filepath, 'r') as f:
                            lines = f.readlines()
                            
                        if line_num <= len(lines):
                            original_line = lines[line_num - 1]
                            
                            # Add override before the semicolon or opening brace
                            if 'virtual' in original_line and 'override' not in original_line:
                                if ';' in original_line:
                                    new_line = original_line.replace(';', ' override;')
                                elif '{' in original_line:
                                    new_line = original_line.replace('{', ' override {')
                                else:
                                    continue
                                    
                                self.apply_fix(filepath, line_num, original_line.strip(), new_line.strip(), 'override')
                    except (ValueError, FileNotFoundError):
                        continue
                        
    def fix_using(self, files: Optional[List[Path]] = None):
        """Convert typedef to using"""
        print(f"{Colors.BLUE}Converting typedef → using...{Colors.NC}")
        
        if not files:
            files = []
            for ext in ['*.h', '*.hpp']:
                files.extend(PROJECT_ROOT.rglob(ext))
                
        for filepath in files:
            if any(part in str(filepath) for part in ['build', '.git', 'opende/OPCODE']):
                continue
                
            with open(filepath, 'r') as f:
                lines = f.readlines()
                
            for line_num, line in enumerate(lines, 1):
                # Match typedef patterns
                # typedef type alias;
                match = re.match(r'^(\s*)typedef\s+(.+?)\s+(\w+)\s*;', line)
                if match:
                    indent, type_spec, alias = match.groups()
                    
                    # Skip function pointer typedefs (complex)
                    if '(' in type_spec and ')' in type_spec:
                        continue
                        
                    new_line = f"{indent}using {alias} = {type_spec};"
                    self.apply_fix(filepath, line_num, line.strip(), new_line.strip(), 'using')
                    
    def fix_auto(self, files: Optional[List[Path]] = None):
        """Replace verbose type declarations with auto"""
        print(f"{Colors.BLUE}Adding auto for iterator declarations...{Colors.NC}")
        
        if not files:
            files = []
            for ext in ['*.cpp', '*.cc', '*.cxx']:
                files.extend(PROJECT_ROOT.rglob(ext))
                
        # Iterator patterns
        iterator_patterns = [
            # std::vector<T>::iterator it = vec.begin()
            (r'(\s*)std::\w+<[^>]+>::(const_)?iterator\s+(\w+)\s*=\s*(.+?)\.(begin|end|find)\s*\(',
             r'\1auto \3 = \4.\5('),
            # std::map<K,V>::iterator it = map.find()
            (r'(\s*)std::map<[^>]+>::(const_)?iterator\s+(\w+)\s*=\s*(.+?)\.find\s*\(',
             r'\1auto \3 = \4.find('),
            # for (std::vector<T>::iterator it = ...)
            (r'for\s*\(\s*std::\w+<[^>]+>::(const_)?iterator\s+(\w+)\s*=',
             r'for (auto \2 ='),
        ]
        
        for filepath in files:
            if any(part in str(filepath) for part in ['build', '.git', 'tests']):
                continue
                
            with open(filepath, 'r') as f:
                lines = f.readlines()
                
            for line_num, line in enumerate(lines, 1):
                for pattern, replacement in iterator_patterns:
                    match = re.search(pattern, line)
                    if match:
                        new_line = re.sub(pattern, replacement, line)
                        self.apply_fix(filepath, line_num, line.strip(), new_line.strip(), 'auto')
                        
    def fix_range_for(self, files: Optional[List[Path]] = None):
        """Convert index-based loops to range-based for loops"""
        print(f"{Colors.BLUE}Converting to range-based for loops...{Colors.NC}")
        
        if not files:
            files = []
            for ext in ['*.cpp', '*.cc', '*.cxx']:
                files.extend(PROJECT_ROOT.rglob(ext))
                
        for filepath in files:
            if any(part in str(filepath) for part in ['build', '.git', 'tests']):
                continue
                
            with open(filepath, 'r') as f:
                content = f.read()
                
            # Find simple index-based loops
            # for (int i = 0; i < vec.size(); i++)
            pattern = r'for\s*\(\s*(?:unsigned\s+)?(?:int|size_t)\s+(\w+)\s*=\s*0\s*;\s*\1\s*<\s*(\w+)\.size\(\)\s*;\s*(?:\+\+\1|\1\+\+)\s*\)'
            
            # This is complex and needs context awareness, so we'll just report opportunities
            matches = re.finditer(pattern, content)
            for match in matches:
                line_num = content[:match.start()].count('\n') + 1
                print(f"{Colors.YELLOW}Range-for opportunity:{Colors.NC} {filepath}:{line_num}")
                if self.verbose:
                    print(f"  Found: {match.group()}")
                    
    def fix_explicit(self, files: Optional[List[Path]] = None):
        """Add explicit to single-argument constructors"""
        print(f"{Colors.BLUE}Adding explicit to single-argument constructors...{Colors.NC}")
        
        if not files:
            files = []
            for ext in ['*.h', '*.hpp']:
                files.extend(PROJECT_ROOT.rglob(ext))
                
        for filepath in files:
            if any(part in str(filepath) for part in ['build', '.git', 'opende']):
                continue
                
            with open(filepath, 'r') as f:
                lines = f.readlines()
                
            in_class = False
            for line_num, line in enumerate(lines, 1):
                # Track class context
                if re.match(r'\s*class\s+\w+', line):
                    in_class = True
                elif re.match(r'\s*};', line):
                    in_class = False
                    
                if in_class:
                    # Match single-argument constructor without explicit
                    # ClassName(type arg);
                    match = re.match(r'^(\s*)(\w+)\s*\(\s*(?:const\s+)?(\w+(?:\s*[&*])?\s+\w+)\s*\)\s*;', line)
                    if match and 'explicit' not in line and 'virtual' not in line:
                        indent, ctor_name, param = match.groups()
                        # Check if it's likely a constructor (starts with uppercase)
                        if ctor_name[0].isupper():
                            new_line = f"{indent}explicit {ctor_name}({param});"
                            self.apply_fix(filepath, line_num, line.strip(), new_line.strip(), 'explicit')
                            
    def fix_constexpr(self, files: Optional[List[Path]] = None):
        """Replace const with constexpr for compile-time constants"""
        print(f"{Colors.BLUE}Adding constexpr for compile-time constants...{Colors.NC}")
        
        if not files:
            files = []
            for ext in ['*.h', '*.hpp', '*.cpp']:
                files.extend(PROJECT_ROOT.rglob(ext))
                
        for filepath in files:
            if any(part in str(filepath) for part in ['build', '.git', 'tests']):
                continue
                
            with open(filepath, 'r') as f:
                lines = f.readlines()
                
            for line_num, line in enumerate(lines, 1):
                # Match const numeric literals
                # const double PI = 3.14159;
                match = re.match(r'^(\s*)const\s+(int|unsigned|long|double|float)\s+(\w+)\s*=\s*([\d.]+)\s*;', line)
                if match and 'constexpr' not in line:
                    indent, type_name, var_name, value = match.groups()
                    # Check if it's likely a compile-time constant (uppercase name)
                    if var_name.isupper() or '_' in var_name:
                        new_line = f"{indent}constexpr {type_name} {var_name} = {value};"
                        self.apply_fix(filepath, line_num, line.strip(), new_line.strip(), 'constexpr')
                        
    def generate_report(self):
        """Generate a report of all fixes"""
        report_path = TOOLS_DIR / 'reports' / 'autofix' / f'report_{datetime.now().strftime("%Y%m%d_%H%M%S")}.md'
        report_path.parent.mkdir(parents=True, exist_ok=True)
        
        with open(report_path, 'w') as f:
            f.write(f"# C++17 Automated Fixes Report\n")
            f.write(f"Generated: {datetime.now()}\n")
            f.write(f"Mode: {'DRY RUN' if self.dry_run else 'APPLIED'}\n\n")
            
            f.write(f"## Summary\n")
            f.write(f"- Fixes {'identified' if self.dry_run else 'applied'}: {self.fixes_applied}\n")
            f.write(f"- Files {'to be modified' if self.dry_run else 'modified'}: {len(self.files_modified)}\n")
            
            if not self.dry_run and self.backup_dir.exists():
                f.write(f"- Backups created in: {self.backup_dir}\n")
                
            f.write(f"\n## Modified Files\n")
            for filepath in sorted(self.files_modified):
                f.write(f"- {filepath.relative_to(PROJECT_ROOT)}\n")
                
        print(f"\n{Colors.GREEN}Report generated: {report_path}{Colors.NC}")
        return report_path

def main():
    parser = argparse.ArgumentParser(description='LPZRobots C++17 Automated Fix Generator')
    parser.add_argument('--fix', choices=['all', 'nullptr', 'override', 'using', 'auto', 
                                          'range-for', 'explicit', 'constexpr'],
                        default='all', help='Type of fixes to apply')
    parser.add_argument('--files', nargs='+', help='Specific files to fix')
    parser.add_argument('--dry-run', action='store_true', default=True,
                        help='Show what would be fixed without modifying files (default)')
    parser.add_argument('--apply', action='store_true',
                        help='Actually apply the fixes (use with caution!)')
    parser.add_argument('--verbose', '-v', action='store_true',
                        help='Verbose output')
    
    args = parser.parse_args()
    
    # Safety check
    if args.apply:
        print(f"{Colors.YELLOW}WARNING: This will modify files!{Colors.NC}")
        response = input("Are you sure you want to apply fixes? (yes/no): ")
        if response.lower() != 'yes':
            print("Aborted.")
            return 0
            
    # Create modernizer
    modernizer = CppModernizer(dry_run=not args.apply, verbose=args.verbose)
    
    # Convert file paths
    files = [Path(f) for f in args.files] if args.files else None
    
    # Apply fixes
    if args.fix == 'all':
        modernizer.fix_nullptr(files)
        modernizer.fix_override(files)
        modernizer.fix_using(files)
        modernizer.fix_auto(files)
        modernizer.fix_explicit(files)
        modernizer.fix_constexpr(files)
        modernizer.fix_range_for(files)  # Just reports opportunities
    else:
        fix_method = getattr(modernizer, f'fix_{args.fix.replace("-", "_")}')
        fix_method(files)
        
    # Generate report
    modernizer.generate_report()
    
    # Summary
    print(f"\n{Colors.BLUE}Summary:{Colors.NC}")
    print(f"  Fixes {'identified' if not args.apply else 'applied'}: {modernizer.fixes_applied}")
    print(f"  Files {'to be modified' if not args.apply else 'modified'}: {len(modernizer.files_modified)}")
    
    if not args.apply:
        print(f"\n{Colors.YELLOW}This was a dry run. Use --apply to actually modify files.{Colors.NC}")
        
    return 0

if __name__ == '__main__':
    sys.exit(main())
