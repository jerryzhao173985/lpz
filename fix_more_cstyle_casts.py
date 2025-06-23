#!/usr/bin/env python3
"""Fix more C-style casts in matrix and utils."""

import re
import os

def fix_cstyle_cast_in_file(filepath, line_num, old_pattern, new_pattern):
    """Fix a specific C-style cast at a given line."""
    if not os.path.exists(filepath):
        return False
    
    with open(filepath, 'r') as f:
        lines = f.readlines()
    
    if line_num > 0 and line_num <= len(lines):
        if old_pattern in lines[line_num - 1]:
            lines[line_num - 1] = lines[line_num - 1].replace(old_pattern, new_pattern)
            with open(filepath, 'w') as f:
                f.writelines(lines)
            return True
    
    return False

def main():
    # Specific fixes based on cppcheck output
    fixes = [
        # matrix.cpp fixes
        ("selforg/matrix/matrix.cpp", 72, "(D*) malloc", "static_cast<D*>(malloc"),
        ("selforg/matrix/matrix.cpp", 258, "(D*) malloc", "static_cast<D*>(malloc"),
        ("selforg/matrix/matrix.cpp", 567, "(D*) realloc", "static_cast<D*>(realloc"),
        ("selforg/matrix/matrix.cpp", 580, "(D*) malloc", "static_cast<D*>(malloc"),
        ("selforg/matrix/matrix.cpp", 599, "(double*)", "static_cast<double*>"),
        ("selforg/matrix/matrix.cpp", 746, "(D*) malloc", "static_cast<D*>(malloc"),
        
        # backcaller.cpp fixes
        ("selforg/utils/backcaller.cpp", 115, "(char*)calloc", "static_cast<char*>(calloc"),
        ("selforg/utils/backcaller.cpp", 116, "(char*)calloc", "static_cast<char*>(calloc"),
        ("selforg/utils/backcaller.cpp", 117, "(char*)calloc", "static_cast<char*>(calloc"),
        ("selforg/utils/backcaller.cpp", 119, "(char*)calloc", "static_cast<char*>(calloc"),
        ("selforg/utils/backcaller.cpp", 120, "(char*)calloc", "static_cast<char*>(calloc"),
        ("selforg/utils/backcaller.cpp", 121, "(char*)calloc", "static_cast<char*>(calloc"),
        
        # configurable.cpp
        ("selforg/utils/configurable.cpp", 384, "(char*)malloc", "static_cast<char*>(malloc"),
        
        # noisegenerator.h
        ("selforg/utils/noisegenerator.h", 174, "(double*)malloc", "static_cast<double*>(malloc"),
        ("selforg/utils/noisegenerator.h", 227, "(double*)malloc", "static_cast<double*>(malloc"),
        
        # controller_misc.cpp
        ("selforg/utils/controller_misc.cpp", 18, "(double*)malloc", "static_cast<double*>(malloc"),
        ("selforg/utils/controller_misc.cpp", 33, "(double*)malloc", "static_cast<double*>(malloc"),
        ("selforg/utils/controller_misc.cpp", 44, "(double*)malloc", "static_cast<double*>(malloc"),
    ]
    
    modified_count = 0
    for filepath, line_num, old_pattern, new_pattern in fixes:
        print(f"Fixing {filepath}:{line_num} - {old_pattern} → {new_pattern}")
        if fix_cstyle_cast_in_file(filepath, line_num, old_pattern, new_pattern):
            modified_count += 1
            print("  ✓ Fixed")
        else:
            print("  ✗ Failed")
    
    print(f"\nTotal fixes applied: {modified_count}")

if __name__ == '__main__':
    main()