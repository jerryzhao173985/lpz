#!/usr/bin/env python3
"""
Script to apply safe C++17 modernizations to the codebase.
"""

import re
import os
import sys

def modernize_auto(lines):
    """Replace obvious iterator declarations with auto."""
    modified = False
    new_lines = []
    
    for line in lines:
        # Pattern: std::vector<Type>::iterator it = container.begin()
        pattern = r'(std::\w+<[^>]+>::(?:const_)?iterator)\s+(\w+)\s*=\s*(\w+)\.(begin|end|find)'
        replacement = r'auto \2 = \3.\4'
        
        if re.search(pattern, line):
            new_line = re.sub(pattern, replacement, line)
            if new_line != line:
                modified = True
                line = new_line
        
        new_lines.append(line)
    
    return new_lines, modified

def modernize_nullptr(lines):
    """Replace NULL with nullptr."""
    modified = False
    new_lines = []
    
    for line in lines:
        # Skip preprocessor directives and comments
        if line.strip().startswith('#') or '//' in line or '/*' in line:
            new_lines.append(line)
            continue
        
        # Replace NULL with nullptr
        if 'NULL' in line:
            new_line = re.sub(r'\bNULL\b', 'nullptr', line)
            if new_line != line:
                modified = True
                line = new_line
        
        new_lines.append(line)
    
    return new_lines, modified

def modernize_using(lines):
    """Replace typedef with using for simple cases."""
    modified = False
    new_lines = []
    
    for line in lines:
        # Pattern: typedef Type Alias;
        pattern = r'typedef\s+([^;]+)\s+(\w+)\s*;'
        match = re.match(pattern, line.strip())
        
        if match and '(' not in match.group(1):  # Skip function pointers
            type_str = match.group(1).strip()
            alias = match.group(2)
            indent = line[:len(line) - len(line.lstrip())]
            new_line = f'{indent}using {alias} = {type_str};\n'
            new_lines.append(new_line)
            modified = True
        else:
            new_lines.append(line)
    
    return new_lines, modified

def add_constexpr(lines):
    """Add constexpr to simple constant functions."""
    modified = False
    new_lines = []
    
    for i, line in enumerate(lines):
        # Pattern: static double/int/float functionName() { return value; }
        pattern = r'(\s*static\s+)(double|int|float|bool)\s+(\w+)\s*\(\s*\)\s*{\s*return\s+[^;]+;\s*}'
        match = re.match(pattern, line)
        
        if match:
            indent = match.group(1)
            ret_type = match.group(2)
            new_line = line.replace(f'static {ret_type}', f'static constexpr {ret_type}')
            new_lines.append(new_line)
            modified = True
        else:
            new_lines.append(line)
    
    return new_lines, modified

def modernize_file(filename):
    """Apply C++17 modernizations to a single file."""
    if not os.path.exists(filename):
        return False
    
    with open(filename, 'r', encoding='utf-8', errors='ignore') as f:
        lines = f.readlines()
    
    original_lines = lines[:]
    modified = False
    
    # Apply modernizations
    lines, mod1 = modernize_nullptr(lines)
    modified |= mod1
    
    lines, mod2 = modernize_auto(lines)
    modified |= mod2
    
    if filename.endswith('.h'):
        lines, mod3 = modernize_using(lines)
        modified |= mod3
        
        lines, mod4 = add_constexpr(lines)
        modified |= mod4
    
    if modified:
        # Create backup
        backup_name = filename + '.bak.cpp17'
        with open(backup_name, 'w') as f:
            f.writelines(original_lines)
        
        # Write modified file
        with open(filename, 'w') as f:
            f.writelines(lines)
        
        print(f"✓ Modernized {filename}")
        return True
    
    return False

def main():
    if len(sys.argv) < 2:
        print("Usage: modernize_cpp17.py <directory>")
        sys.exit(1)
    
    directory = sys.argv[1]
    
    # Find all C++ files
    cpp_files = []
    for root, dirs, files in os.walk(directory):
        dirs[:] = [d for d in dirs if d not in ['build', 'build_opt', 'build_dbg']]
        for file in files:
            if file.endswith(('.cpp', '.h')) and not file.endswith('.bak'):
                cpp_files.append(os.path.join(root, file))
    
    print(f"Found {len(cpp_files)} C++ files")
    
    # Process files
    modernized_count = 0
    for filepath in cpp_files[:20]:  # Process first 20 files
        if modernize_file(filepath):
            modernized_count += 1
    
    print(f"\nModernized {modernized_count} files")

if __name__ == '__main__':
    main()