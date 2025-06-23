#!/usr/bin/env python3
"""Fix C-style casts to C++ style casts."""

import re
import os
import subprocess

def fix_cstyle_casts(filepath):
    """Replace C-style casts with appropriate C++ casts."""
    if not os.path.exists(filepath):
        return False
    
    # Skip backup files
    if '.bak' in filepath:
        return False
    
    # Create backup
    backup_path = filepath + '.bak.cast'
    if not os.path.exists(backup_path):
        subprocess.run(['cp', filepath, backup_path], check=True)
    
    with open(filepath, 'r', encoding='utf-8', errors='ignore') as f:
        content = f.read()
    
    changes = []
    modified = content
    
    # Pattern for C-style casts: (type)expression
    patterns = [
        # Pointer casts
        (r'\((\w+)\s*\*\s*\)\s*(\w+)', r'static_cast<\1*>(\2)'),
        (r'\((const\s+\w+)\s*\*\s*\)\s*(\w+)', r'static_cast<\1*>(\2)'),
        (r'\((\w+)\s*\*\s*\)\s*\(([^)]+)\)', r'static_cast<\1*>(\2)'),
        (r'\((void)\s*\*\s*\)\s*(\w+)', r'static_cast<\1*>(\2)'),
        # Basic type casts
        (r'\((int|double|float|long|short|char|bool)\)\s*(\w+)', r'static_cast<\1>(\2)'),
        (r'\((int|double|float|long|short|char|bool)\)\s*\(([^)]+)\)', r'static_cast<\1>(\2)'),
        # Unsigned type casts
        (r'\((unsigned\s+int|unsigned\s+long|unsigned\s+short|size_t)\)\s*(\w+)', r'static_cast<\1>(\2)'),
        (r'\((unsigned\s+int|unsigned\s+long|unsigned\s+short|size_t)\)\s*\(([^)]+)\)', r'static_cast<\1>(\2)'),
    ]
    
    for pattern, replacement in patterns:
        matches = list(re.finditer(pattern, modified))
        for match in reversed(matches):
            # Skip if it's already a C++ cast
            if 'static_cast' in modified[max(0, match.start()-20):match.start()]:
                continue
            if 'dynamic_cast' in modified[max(0, match.start()-20):match.start()]:
                continue
            if 'reinterpret_cast' in modified[max(0, match.start()-20):match.start()]:
                continue
            if 'const_cast' in modified[max(0, match.start()-20):match.start()]:
                continue
            
            new_text = re.sub(pattern, replacement, match.group(0))
            modified = modified[:match.start()] + new_text + modified[match.end():]
            changes.append(('C-style cast → static_cast', match.start()))
    
    # Special case for malloc/calloc casts
    malloc_pattern = r'\((\w+\s*\*)\)\s*(malloc|calloc)\s*\('
    malloc_matches = list(re.finditer(malloc_pattern, modified))
    for match in reversed(malloc_matches):
        type_str = match.group(1).strip()
        func = match.group(2)
        new_text = f'static_cast<{type_str}>({func}('
        modified = modified[:match.start()] + new_text + modified[match.start() + len(match.group(0)):]
        changes.append(('malloc cast → static_cast', match.start()))
    
    if changes:
        print(f"  Fixed {len(changes)} C-style casts")
        with open(filepath, 'w', encoding='utf-8') as f:
            f.write(modified)
        return True
    
    return False

def main():
    # Process specific files with C-style cast warnings
    files_to_fix = [
        'selforg/agent.cpp',
        'selforg/controller/abstractiafcontroller.h',
        'selforg/controller/regularisation.h',
        'selforg/controller/crossmotorcoupling.h',
        'selforg/controller/dep.cpp',
        'selforg/controller/discretecontrolleradapter.cpp',
        'selforg/controller/measureadapter.cpp'
    ]
    
    modified_count = 0
    for filepath in files_to_fix:
        print(f"\nProcessing: {filepath}")
        if fix_cstyle_casts(filepath):
            modified_count += 1
    
    print(f"\nTotal files modified: {modified_count}")

if __name__ == '__main__':
    main()