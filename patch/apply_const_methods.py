#!/usr/bin/env python3
"""Add const to getter methods that don't modify state"""

import re
import os

def add_const_to_getters(file_path):
    """Add const to getter methods"""
    
    with open(file_path, 'r', encoding='utf-8') as f:
        content = f.read()
    
    original = content
    
    # Pattern for simple getter methods that should be const
    # Matches: type getName() { return member; }
    # But not if already const
    patterns = [
        # Simple getters returning member variables
        (r'(\b(?:int|double|float|bool|size_t|unsigned)\s+(?:get|is|has)\w+\s*\(\s*\)\s*{\s*return\s+\w+;\s*})(?!\s*const)',
         r'\1 const'),
        # Getters returning references
        (r'(\b\w+&\s+(?:get|is|has)\w+\s*\(\s*\)\s*{\s*return\s+\w+;\s*})(?!\s*const)',
         r'\1 const'),
        # Virtual getters
        (r'(virtual\s+(?:int|double|float|bool|size_t|unsigned)\s+(?:get|is|has)\w+\s*\(\s*\)\s*(?:override\s*)?{\s*return\s+\w+;\s*})(?!\s*const)',
         r'\1 const'),
    ]
    
    for pattern, replacement in patterns:
        content = re.sub(pattern, replacement, content)
    
    if content != original:
        with open(file_path, 'w', encoding='utf-8') as f:
            f.write(content)
        return True
    return False

def main():
    # Files identified with non-const getters
    target_files = [
        'selforg/matrix/matrixpool.h',
        'tests/bdd/robot_behavior_scenarios.cpp',
        'tests/integration/homeokinetic_robot_integration_test.cpp',
    ]
    
    fixed_count = 0
    
    print("Adding const to getter methods...")
    for file_path in target_files:
        if os.path.exists(file_path):
            if add_const_to_getters(file_path):
                print(f"Fixed: {file_path}")
                fixed_count += 1
        else:
            print(f"File not found: {file_path}")
    
    print(f"\nTotal files fixed: {fixed_count}")

if __name__ == "__main__":
    main()