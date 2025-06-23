#!/usr/bin/env python3
"""
Simple tool to add override keywords based on cppcheck output.
This version focuses on reliability over complexity.
"""

import re
import sys
import subprocess
from collections import defaultdict

def get_missing_overrides(directory):
    """Get list of files and line numbers that need override keyword."""
    print(f"Running cppcheck on {directory}...")
    
    result = subprocess.run(
        ['cppcheck', '--enable=style', '--suppress=missingInclude', 
         '--inline-suppr', '--quiet', '--force', directory],
        capture_output=True,
        text=True
    )
    
    # Group by file
    files_to_fix = defaultdict(list)
    
    for line in result.stderr.split('\n'):
        if 'missingOverride' in line:
            # Parse the line
            match = re.match(r'([^:]+):(\d+):(\d+): style: The (\w+) \'(\w+)\' overrides', line)
            if match:
                filepath = match.group(1)
                line_num = int(match.group(2))
                col_num = int(match.group(3))
                decl_type = match.group(4)  # function, destructor, etc.
                func_name = match.group(5)
                files_to_fix[filepath].append({
                    'line': line_num,
                    'col': col_num,
                    'type': decl_type,
                    'name': func_name
                })
    
    return files_to_fix

def fix_file(filepath, issues):
    """Fix override keywords in a single file."""
    print(f"\nFixing {filepath} ({len(issues)} issues)...")
    
    with open(filepath, 'r') as f:
        lines = f.readlines()
    
    # Sort issues by line number in reverse order to avoid index shifting
    issues.sort(key=lambda x: x['line'], reverse=True)
    
    fixed_count = 0
    for issue in issues:
        line_idx = issue['line'] - 1
        
        if line_idx >= len(lines):
            print(f"  ✗ Line {issue['line']} out of range")
            continue
            
        line = lines[line_idx]
        
        # Skip if already has override
        if 'override' in line:
            print(f"  - Line {issue['line']}: Already has override")
            continue
        
        # For inline functions with body on same line
        if '{' in line and '}' in line:
            # Add override before the opening brace
            new_line = re.sub(r'(\s*)\{', r' override\1{', line)
            if new_line != line:
                lines[line_idx] = new_line
                fixed_count += 1
                print(f"  ✓ Fixed {issue['name']} on line {issue['line']} (inline)")
                continue
        
        # For declarations ending with semicolon
        if ';' in line:
            # Handle pure virtual
            if '= 0' in line or '=0' in line:
                new_line = re.sub(r'(\s*=\s*0\s*);', r'\1 override;', line)
            else:
                new_line = re.sub(r'(\s*);', r' override;', line)
            
            if new_line != line:
                lines[line_idx] = new_line
                fixed_count += 1
                print(f"  ✓ Fixed {issue['name']} on line {issue['line']}")
                continue
        
        # For functions with opening brace on same line
        if '{' in line:
            new_line = re.sub(r'(\s*)\{', r' override\1{', line)
            if new_line != line:
                lines[line_idx] = new_line
                fixed_count += 1
                print(f"  ✓ Fixed {issue['name']} on line {issue['line']}")
                continue
        
        # For multi-line declarations, check next few lines
        fixed = False
        for offset in range(1, 5):
            if line_idx + offset < len(lines):
                next_line = lines[line_idx + offset]
                if ';' in next_line or '{' in next_line:
                    if ';' in next_line:
                        if '= 0' in next_line or '=0' in next_line:
                            new_line = re.sub(r'(\s*=\s*0\s*);', r'\1 override;', next_line)
                        else:
                            new_line = re.sub(r'(\s*);', r' override;', next_line)
                    else:
                        new_line = re.sub(r'(\s*)\{', r' override\1{', next_line)
                    
                    if new_line != next_line:
                        lines[line_idx + offset] = new_line
                        fixed_count += 1
                        fixed = True
                        print(f"  ✓ Fixed {issue['name']} on line {issue['line']} (multi-line)")
                    break
        
        if not fixed:
            print(f"  ✗ Could not fix {issue['name']} on line {issue['line']}")
    
    # Write back if we fixed anything
    if fixed_count > 0:
        with open(filepath, 'w') as f:
            f.writelines(lines)
        print(f"  Wrote {fixed_count} fixes to {filepath}")
    
    return fixed_count

def main(directory='.', max_files=10):
    """Main function to fix override keywords."""
    files_to_fix = get_missing_overrides(directory)
    
    print(f"\nFound {len(files_to_fix)} files with missing override keywords")
    print(f"Total issues: {sum(len(issues) for issues in files_to_fix.values())}")
    
    if max_files and len(files_to_fix) > max_files:
        print(f"\nLimiting to first {max_files} files")
        files_to_fix = dict(list(files_to_fix.items())[:max_files])
    
    total_fixed = 0
    for filepath, issues in files_to_fix.items():
        fixed = fix_file(filepath, issues)
        total_fixed += fixed
    
    print(f"\n=== Summary ===")
    print(f"Total fixes applied: {total_fixed}")
    print(f"Files modified: {len([f for f, i in files_to_fix.items() if fix_file(f, i) > 0])}")

if __name__ == "__main__":
    if len(sys.argv) > 1:
        directory = sys.argv[1]
    else:
        directory = '.'
    
    main(directory, max_files=5)  # Process 5 files at a time