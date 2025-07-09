#!/usr/bin/env python3
"""Add explicit to single-parameter constructors to prevent implicit conversions"""

import re
import os

def add_explicit_to_constructors(file_path):
    """Add explicit keyword to single-parameter constructors"""
    
    with open(file_path, 'r', encoding='utf-8') as f:
        lines = f.readlines()
    
    modified = False
    new_lines = []
    
    for line in lines:
        original_line = line
        
        # Pattern: Constructor with single parameter (not copy constructor)
        # ClassName(Type param) but not ClassName(const ClassName&)
        constructor_pattern = r'^(\s*)(\w+)\s*\(([^,)]+)\)\s*([:{].*)?$'
        match = re.match(constructor_pattern, line)
        
        if match:
            indent = match.group(1)
            class_name = match.group(2)
            param = match.group(3).strip()
            rest = match.group(4) or ''
            
            # Check if it's a single parameter constructor (not copy constructor)
            # and doesn't already have explicit
            if (not re.search(r'const\s+' + class_name + r'\s*&', param) and
                'explicit' not in line and
                'operator' not in line and
                '=' not in param):  # Not default parameter
                
                # Add explicit
                line = f"{indent}explicit {class_name}({param}){rest}\n"
                modified = True
        
        new_lines.append(line)
    
    if modified:
        with open(file_path, 'w', encoding='utf-8') as f:
            f.writelines(new_lines)
        return True
    return False

def main():
    # Priority files with single-parameter constructors
    target_files = [
        'ode_robots/osg/old/motionblurcallback.h',
        'selforg/matrix/matrixpool.h',
        'selforg/matrix/matrix_iterators.h',
        'selforg/utils/optional_extensions.h',
        'ode_robots/simulations/template_XMLImportSim/XMLHelper.h',
    ]
    
    # Also search for more files
    additional_files = []
    for root, dirs, files in os.walk('.'):
        # Skip certain directories
        skip_dirs = ['.git', '.cache', 'build', 'dist', 'backup_']
        dirs[:] = [d for d in dirs if not any(skip in d for skip in skip_dirs)]
        
        for file in files:
            if file.endswith(('.h', '.hpp')):
                file_path = os.path.join(root, file)
                if file_path not in target_files:
                    additional_files.append(file_path)
    
    fixed_count = 0
    
    print("Adding explicit to single-parameter constructors...")
    
    # Fix priority files first
    for file_path in target_files:
        if os.path.exists(file_path):
            if add_explicit_to_constructors(file_path):
                print(f"Fixed: {file_path}")
                fixed_count += 1
    
    # Check additional files (limit to prevent too many changes)
    checked = 0
    for file_path in additional_files[:50]:  # Check first 50 files
        if os.path.exists(file_path):
            checked += 1
            if add_explicit_to_constructors(file_path):
                print(f"Fixed: {file_path}")
                fixed_count += 1
    
    print(f"\nTotal files fixed: {fixed_count} (checked {checked + len(target_files)} files)")

if __name__ == "__main__":
    main()