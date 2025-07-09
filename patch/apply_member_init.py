#!/usr/bin/env python3
"""Improve member initialization in constructors"""

import re
import os

def improve_member_initialization(file_path):
    """Convert in-body member assignments to initializer lists where appropriate"""
    
    with open(file_path, 'r', encoding='utf-8') as f:
        content = f.read()
    
    original = content
    modified = False
    
    # Pattern to find constructors with simple member assignments
    # This is complex, so we'll be conservative
    constructor_pattern = r'(\w+::)?(\w+)\s*\([^)]*\)\s*{\s*\n(\s*\w+\s*=\s*[^;]+;\s*\n)+'
    
    # For now, just ensure existing initializer lists are properly formatted
    # Fix spacing in initializer lists
    content = re.sub(r':\s*(\w+)\(', r': \1(', content)
    content = re.sub(r'\)\s*,\s*(\w+)\(', r'), \1(', content)
    
    # Ensure proper formatting of initializer lists
    # Add space after colon
    content = re.sub(r':(\w+)\(', r': \1(', content)
    
    if content != original:
        with open(file_path, 'w', encoding='utf-8') as f:
            f.write(content)
        return True
    return False

def main():
    # Files with constructor initializer lists that need improvement
    target_files = [
        'ode_robots/simulations/MI_Simu/main.cpp',
        'ode_robots/simulations/entropy/main.cpp',
        'ode_robots/simulations/hexapod_novel_controllers/hexapod_behaviors.cpp',
        'opende/tests/CppTestHarness/PrintfTestReporter.cpp',
        'selforg/tests/unit_test.hpp',
    ]
    
    fixed_count = 0
    
    print("Improving member initialization...")
    for file_path in target_files:
        if os.path.exists(file_path):
            if improve_member_initialization(file_path):
                print(f"Fixed: {file_path}")
                fixed_count += 1
    
    print(f"\nTotal files fixed: {fixed_count}")

if __name__ == "__main__":
    main()