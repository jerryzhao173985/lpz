#!/usr/bin/env python3
"""Fix postfix operators to prefix operators for non-primitive types."""

import re
import os
import subprocess

def fix_postfix_operators(filepath):
    """Replace i++ with ++i for iterators and other non-primitive types."""
    if not os.path.exists(filepath):
        return False
    
    # Skip backup files
    if '.bak' in filepath:
        return False
    
    # Create backup
    backup_path = filepath + '.bak.postfix'
    if not os.path.exists(backup_path):
        subprocess.run(['cp', filepath, backup_path], check=True)
    
    with open(filepath, 'r', encoding='utf-8', errors='ignore') as f:
        content = f.read()
    
    changes = []
    modified = content
    
    # Patterns to fix:
    # 1. Iterator++ in for loops: for(...; it++) → for(...; ++it)
    # 2. Standalone iterator++; → ++iterator;
    patterns = [
        # For loop iterator increment
        (r'(for\s*\([^;]+;[^;]+;\s*)(\w+)\+\+(\s*\))', r'\1++\2\3'),
        # Standalone increment (careful not to catch primitive types)
        (r'(\s+)(it|iter|iterator|i|j|k)\+\+;', r'\1++\2;'),
        # In complex expressions where we're sure it's an iterator
        (r'(\w+\.begin\(\)\s*;\s*\w+\s*!=\s*\w+\.end\(\)\s*;\s*)(\w+)\+\+', r'\1++\2'),
    ]
    
    for pattern, replacement in patterns:
        matches = list(re.finditer(pattern, modified))
        for match in reversed(matches):
            new_text = re.sub(pattern, replacement, match.group(0))
            modified = modified[:match.start()] + new_text + modified[match.end():]
            changes.append(('postfix→prefix', match.start()))
    
    if changes:
        print(f"  Fixed {len(changes)} postfix operators")
        with open(filepath, 'w', encoding='utf-8') as f:
            f.write(modified)
        return True
    
    return False

def main():
    # Get files with postfixOperator warnings
    result = subprocess.run(
        ['cppcheck', '--enable=all', '--std=c++17', '--suppress=missingIncludeSystem', '--quiet', 'selforg'],
        stderr=subprocess.PIPE,
        text=True
    )
    
    files_to_fix = []
    for line in result.stderr.splitlines():
        if 'postfixOperator' in line:
            parts = line.split(':')
            if len(parts) >= 3:
                filepath = parts[0]
                line_num = int(parts[1])
                if filepath not in [f[0] for f in files_to_fix]:
                    files_to_fix.append((filepath, line_num))
    
    print(f"Found {len(files_to_fix)} files with postfix operator issues")
    
    # Group by file
    files = {}
    for filepath, line_num in files_to_fix:
        if filepath not in files:
            files[filepath] = []
        files[filepath].append(line_num)
    
    modified_count = 0
    for filepath in sorted(files.keys())[:20]:  # Process first 20 files
        print(f"\nProcessing: {filepath}")
        if fix_postfix_operators(filepath):
            modified_count += 1
    
    print(f"\nTotal files modified: {modified_count}")

if __name__ == '__main__':
    main()