#!/usr/bin/env python3
"""Fix all remaining C-style casts comprehensively."""

import re
import os
import subprocess

def fix_cstyle_casts_comprehensive(filepath):
    """Replace all C-style casts with appropriate C++ casts."""
    if not os.path.exists(filepath):
        return False
    
    # Skip backup files
    if '.bak' in filepath:
        return False
    
    # Create backup
    backup_path = filepath + '.bak.cast2'
    if not os.path.exists(backup_path):
        subprocess.run(['cp', filepath, backup_path], check=True)
    
    with open(filepath, 'r', encoding='utf-8', errors='ignore') as f:
        content = f.read()
    
    changes = []
    modified = content
    
    # More comprehensive patterns
    patterns = [
        # Pattern: ( type * ) malloc/calloc/realloc
        (r'\(\s*(\w+)\s*\*\s*\)\s*(malloc|calloc|realloc)\s*\(', r'static_cast<\1*>(\2('),
        (r'\(\s*(const\s+\w+)\s*\*\s*\)\s*(malloc|calloc|realloc)\s*\(', r'static_cast<\1*>(\2('),
        (r'\(\s*(D)\s*\*\s*\)\s*(malloc|calloc|realloc)\s*\(', r'static_cast<\1*>(\2('),
        
        # Pattern: (type*) var
        (r'\(\s*(\w+)\s*\*\s*\)\s*(\w+)(?!\()', r'static_cast<\1*>(\2)'),
        (r'\(\s*(const\s+\w+)\s*\*\s*\)\s*(\w+)(?!\()', r'static_cast<\1*>(\2)'),
        (r'\(\s*(void)\s*\*\s*\)\s*(\w+)(?!\()', r'static_cast<\1*>(\2)'),
        
        # Pattern: (type) var
        (r'\(\s*(int|double|float|long|short|char|bool|size_t)\s*\)\s*(\w+)', r'static_cast<\1>(\2)'),
        (r'\(\s*(unsigned\s+int|unsigned\s+long|unsigned\s+short)\s*\)\s*(\w+)', r'static_cast<\1>(\2)'),
        
        # Pattern: (type) (expression)
        (r'\(\s*(int|double|float|long|short|char|bool|size_t)\s*\)\s*\(([^)]+)\)', r'static_cast<\1>(\2)'),
    ]
    
    for pattern, replacement in patterns:
        matches = list(re.finditer(pattern, modified))
        for match in reversed(matches):
            # Skip if already a C++ cast
            context = modified[max(0, match.start()-20):match.start()]
            if any(cast in context for cast in ['static_cast', 'dynamic_cast', 'reinterpret_cast', 'const_cast']):
                continue
            
            new_text = re.sub(pattern, replacement, match.group(0))
            modified = modified[:match.start()] + new_text + modified[match.end():]
            changes.append(('C-style cast', match.start()))
    
    if changes:
        print(f"  Fixed {len(changes)} C-style casts")
        with open(filepath, 'w', encoding='utf-8') as f:
            f.write(modified)
        return True
    
    return False

def main():
    # Get all files with C-style cast warnings
    result = subprocess.run(
        ['cppcheck', '--enable=all', '--std=c++17', '--suppress=missingIncludeSystem', '--quiet', 'selforg'],
        stderr=subprocess.PIPE,
        text=True
    )
    
    files_to_fix = set()
    for line in result.stderr.splitlines():
        if 'cstyleCast' in line:
            filepath = line.split(':')[0]
            if os.path.exists(filepath):
                files_to_fix.add(filepath)
    
    print(f"Found {len(files_to_fix)} files with C-style casts")
    
    modified_count = 0
    for filepath in sorted(files_to_fix)[:20]:  # Process first 20 files
        print(f"\nProcessing: {filepath}")
        if fix_cstyle_casts_comprehensive(filepath):
            modified_count += 1
    
    print(f"\nTotal files modified: {modified_count}")

if __name__ == '__main__':
    main()