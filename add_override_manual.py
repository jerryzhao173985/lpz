#!/usr/bin/env python3
"""
Script to add override keyword to virtual functions based on cppcheck output.
This is a safer alternative when clang-tidy is not available.
"""

import re
import os
import sys

def parse_cppcheck_output(filename):
    """Parse cppcheck output to extract override issues."""
    issues = []
    with open(filename, 'r') as f:
        for line in f:
            # Match lines like: controller/abstractiafcontroller.h:95:16: style: The function 'init' overrides...
            match = re.match(r'([^:]+):(\d+):(\d+): style: The (function|destructor) \'([^\']+)\' overrides.*\[missingOverride\]', line)
            if match:
                issues.append({
                    'file': match.group(1),
                    'line': int(match.group(2)),
                    'col': int(match.group(3)),
                    'type': match.group(4),
                    'name': match.group(5)
                })
    return issues

def fix_override_in_file(filename, issues):
    """Add override keyword to functions in the given file."""
    # Group issues by line number
    issues_by_line = {}
    for issue in issues:
        if issue['file'] == filename:
            issues_by_line[issue['line']] = issue
    
    if not issues_by_line:
        return False
    
    # Read the file
    with open(filename, 'r') as f:
        lines = f.readlines()
    
    modified = False
    for line_num, issue in sorted(issues_by_line.items(), reverse=True):
        line_idx = line_num - 1  # Convert to 0-based index
        if line_idx >= len(lines):
            continue
            
        line = lines[line_idx]
        function_name = issue['name']
        
        # Skip if already has override
        if 'override' in line:
            continue
        
        # For destructors
        if issue['type'] == 'destructor':
            # Pattern: virtual ~ClassName() { ... } or virtual ~ClassName();
            pattern = r'(virtual\s+~\w+\s*\([^)]*\)\s*(?:const\s*)?)(\s*[{;])'
            replacement = r'\1 override\2'
        else:
            # For regular functions
            # Pattern to match function declarations
            pattern = r'(virtual\s+\S+\s+' + re.escape(function_name) + r'\s*\([^)]*\)\s*(?:const\s*)?)(\s*[{;])'
            replacement = r'\1 override\2'
        
        new_line = re.sub(pattern, replacement, line)
        
        if new_line != line:
            lines[line_idx] = new_line
            modified = True
            print(f"  Fixed: {function_name} at line {line_num}")
    
    if modified:
        # Create backup
        backup_name = filename + '.bak.override'
        os.rename(filename, backup_name)
        
        # Write modified file
        with open(filename, 'w') as f:
            f.writelines(lines)
        
        print(f"✓ Modified {filename} (backup: {backup_name})")
        return True
    
    return False

def main():
    # Parse cppcheck output
    issues = parse_cppcheck_output('selforg_issues.txt')
    print(f"Found {len(issues)} missing override issues")
    
    # Prepend selforg/ to all file paths
    for issue in issues:
        if not issue['file'].startswith('selforg/'):
            issue['file'] = 'selforg/' + issue['file']
    
    # Group by file
    files_to_fix = {}
    for issue in issues:
        if issue['file'] not in files_to_fix:
            files_to_fix[issue['file']] = []
        files_to_fix[issue['file']].append(issue)
    
    print(f"Files to fix: {len(files_to_fix)}")
    
    # Fix each file
    fixed_count = 0
    for filename, file_issues in files_to_fix.items():
        if os.path.exists(filename):
            print(f"\nProcessing {filename} ({len(file_issues)} issues)...")
            if fix_override_in_file(filename, file_issues):
                fixed_count += 1
        else:
            print(f"✗ File not found: {filename}")
    
    print(f"\nFixed {fixed_count} files")

if __name__ == '__main__':
    main()