#!/usr/bin/env python3
"""Remove incorrect override keywords from AbstractController base class methods"""

import re
import os

def fix_abstractcontroller_override(file_path):
    """Remove override from base class virtual methods that don't override anything"""
    
    with open(file_path, 'r', encoding='utf-8') as f:
        content = f.read()
    
    original = content
    
    # These are base class methods that shouldn't have override
    methods_to_fix = [
        'sensorInfos',
        'motorInfos',
        'SIdx',
        'MIdx',
        'SInfo',
        'MInfo',
    ]
    
    # Remove override from these specific methods
    for method in methods_to_fix:
        # Pattern to match the method declaration with override
        pattern = rf'(virtual\s+\w+(?:\s*\*)?(?:\s+\w+::)?\s+{method}\s*\([^)]*\)(?:\s+const)?\s+)override(\s*;)'
        replacement = r'\1\2'
        content = re.sub(pattern, replacement, content)
    
    if content != original:
        with open(file_path, 'w', encoding='utf-8') as f:
            f.write(content)
        return True
    return False

def main():
    # Files to fix
    files_to_fix = [
        'selforg/controller/abstractcontroller.h',
        'include/selforg/abstractcontroller.h',
    ]
    
    fixed_count = 0
    
    print("Fixing AbstractController override issues...")
    for file_path in files_to_fix:
        if os.path.exists(file_path):
            if fix_abstractcontroller_override(file_path):
                print(f"Fixed: {file_path}")
                fixed_count += 1
    
    print(f"\nTotal files fixed: {fixed_count}")

if __name__ == "__main__":
    main()