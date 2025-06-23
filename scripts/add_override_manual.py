#!/usr/bin/env python3
"""
Script to add 'override' keyword to virtual functions in derived classes
"""

import os
import re
import sys
from pathlib import Path

def is_destructor(func_declaration):
    """Check if the function is a destructor"""
    return '~' in func_declaration

def is_pure_virtual(func_declaration):
    """Check if the function is pure virtual"""
    return '= 0' in func_declaration

def has_override(func_declaration):
    """Check if the function already has override keyword"""
    return 'override' in func_declaration

def add_override_to_function(func_declaration):
    """Add override keyword to a virtual function declaration"""
    if is_pure_virtual(func_declaration):
        # For pure virtual, add override before = 0
        return func_declaration.replace('= 0', 'override = 0')
    elif func_declaration.rstrip().endswith(';'):
        # Add override before the semicolon
        return func_declaration.rstrip()[:-1] + ' override;'
    else:
        # For multi-line declarations, we'll need more complex handling
        return func_declaration

def process_header_file(filepath):
    """Process a header file to add override keywords"""
    with open(filepath, 'r') as f:
        content = f.read()
    
    original_content = content
    modified = False
    
    # Pattern to match virtual function declarations in derived classes
    # This is a simplified pattern and may need refinement
    virtual_pattern = re.compile(
        r'(\s+virtual\s+.*?\s*\([^)]*\)\s*(?:const\s*)?(?:=\s*0\s*)?;)',
        re.MULTILINE | re.DOTALL
    )
    
    # Find all virtual function declarations
    matches = list(virtual_pattern.finditer(content))
    
    # Process matches in reverse order to avoid offset issues
    for match in reversed(matches):
        func_declaration = match.group(1)
        
        # Skip if already has override
        if has_override(func_declaration):
            continue
            
        # Skip destructors (they typically don't need override)
        if is_destructor(func_declaration):
            continue
        
        # Add override keyword
        new_declaration = add_override_to_function(func_declaration)
        
        if new_declaration != func_declaration:
            content = content[:match.start(1)] + new_declaration + content[match.end(1):]
            modified = True
    
    # Save if modified
    if modified:
        with open(filepath, 'w') as f:
            f.write(content)
        return True
    
    return False

def find_header_files(directory):
    """Find all header files in directory"""
    header_files = []
    for root, dirs, files in os.walk(directory):
        # Skip build directories
        dirs[:] = [d for d in dirs if d not in ['build', 'build_opt', 'build_dbg', '.git']]
        
        for file in files:
            if file.endswith(('.h', '.hpp')):
                header_files.append(os.path.join(root, file))
    
    return header_files

def main():
    if len(sys.argv) < 2:
        print("Usage: python3 add_override_manual.py <directory>")
        sys.exit(1)
    
    directory = sys.argv[1]
    if not os.path.isdir(directory):
        print(f"Error: {directory} is not a valid directory")
        sys.exit(1)
    
    print(f"Processing header files in {directory}...")
    
    header_files = find_header_files(directory)
    print(f"Found {len(header_files)} header files")
    
    modified_count = 0
    for header_file in header_files:
        try:
            if process_header_file(header_file):
                modified_count += 1
                print(f"  Modified: {os.path.relpath(header_file, directory)}")
        except Exception as e:
            print(f"  Error processing {header_file}: {e}")
    
    print(f"\nModified {modified_count} files")

if __name__ == "__main__":
    main()