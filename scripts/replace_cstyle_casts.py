#!/usr/bin/env python3
"""
Script to replace C-style casts with C++ casts in source code
"""

import re
import sys
import os
from pathlib import Path

def determine_cast_type(old_cast, context):
    """Determine the appropriate C++ cast based on the C-style cast and context"""
    # Extract the type being cast to
    match = re.match(r'\(\s*([^)]+)\s*\)', old_cast)
    if not match:
        return None
    
    cast_type = match.group(1).strip()
    
    # For void* conversions and malloc/realloc, use static_cast
    if 'void' in context or 'malloc' in context or 'realloc' in context:
        return 'static_cast'
    
    # For pointer type conversions between related types
    if '*' in cast_type:
        # For const_cast scenarios
        if 'const' in cast_type or 'const' in context:
            return 'reinterpret_cast'  # Often safer than const_cast for raw pointers
        return 'static_cast'
    
    # For numeric conversions
    if cast_type in ['int', 'I', 'unsigned int', 'double', 'D', 'float']:
        return 'static_cast'
    
    # Default to static_cast
    return 'static_cast'

def replace_cstyle_cast(line):
    """Replace C-style casts in a line with appropriate C++ casts"""
    # Pattern to match C-style casts
    # Matches (type) or ( type ) or (type*) or (type *) etc.
    pattern = r'\(\s*([A-Za-z_][\w\s\*&]*?)\s*\)'
    
    def replacement(match):
        full_match = match.group(0)
        cast_content = match.group(1).strip()
        
        # Skip function calls and declarations
        if '(' in cast_content or ')' in cast_content:
            return full_match
            
        # Get context around the cast
        start = max(0, match.start() - 50)
        end = min(len(line), match.end() + 50)
        context = line[start:end]
        
        # Determine if this is actually a cast (has something after it)
        after_match = line[match.end():match.end()+2]
        if not after_match or after_match[0] in [';', ',', ')', '}', '\n']:
            return full_match
            
        # Check if there's an expression/variable after the cast
        if after_match[0] not in [' ', '\t'] and not after_match[0].isalnum():
            if after_match[0] not in ['_', '*', '&', '(']:
                return full_match
        
        # Determine the appropriate C++ cast
        cpp_cast = determine_cast_type(full_match, context)
        if not cpp_cast:
            return full_match
            
        # Format the C++ cast
        return f'{cpp_cast}<{cast_content}>'
    
    # First pass: Replace obvious C-style casts
    modified_line = line
    
    # Specific patterns for common casts
    # Pattern 1: (type*) malloc/realloc patterns
    modified_line = re.sub(
        r'\(\s*([\w\s]+\*)\s*\)\s*(malloc|realloc)',
        r'static_cast<\1>(\2',
        modified_line
    )
    
    # Pattern 2: (type) variable patterns  
    modified_line = re.sub(
        r'\(\s*(int|I|double|D|float|unsigned int)\s*\)\s*([a-zA-Z_]\w*)',
        r'static_cast<\1>(\2)',
        modified_line
    )
    
    # Pattern 3: (char*) or similar pointer casts
    modified_line = re.sub(
        r'\(\s*(char|void|const char|const void)\s*\*\s*\)\s*([a-zA-Z_]\w*)',
        r'static_cast<\1*>(\2)',
        modified_line
    )
    
    return modified_line

def process_file(filepath):
    """Process a single C++ file to replace C-style casts"""
    print(f"Processing {filepath}")
    
    with open(filepath, 'r') as f:
        lines = f.readlines()
    
    modified = False
    new_lines = []
    changes = []
    
    for line_num, line in enumerate(lines, 1):
        new_line = replace_cstyle_cast(line)
        if new_line != line:
            modified = True
            changes.append(f"  Line {line_num}: {line.strip()} -> {new_line.strip()}")
        new_lines.append(new_line)
    
    if modified:
        # Create backup
        backup_path = f"{filepath}.bak"
        with open(backup_path, 'w') as f:
            f.writelines(lines)
        
        # Write modified file
        with open(filepath, 'w') as f:
            f.writelines(new_lines)
            
        print(f"  Modified {len(changes)} lines:")
        for change in changes[:10]:  # Show first 10 changes
            print(change)
        if len(changes) > 10:
            print(f"  ... and {len(changes) - 10} more changes")
        print(f"  Backup saved to {backup_path}")
        return True
    else:
        print("  No C-style casts found")
        return False

def main():
    if len(sys.argv) < 2:
        print("Usage: python3 replace_cstyle_casts.py <file_or_directory>")
        sys.exit(1)
    
    path = sys.argv[1]
    
    if os.path.isfile(path):
        process_file(path)
    elif os.path.isdir(path):
        cpp_files = []
        for root, dirs, files in os.walk(path):
            # Skip build directories
            dirs[:] = [d for d in dirs if d not in ['build', '.git']]
            
            for file in files:
                if file.endswith(('.cpp', '.h', '.hpp')):
                    cpp_files.append(os.path.join(root, file))
        
        print(f"Found {len(cpp_files)} C++ files to process")
        
        modified_count = 0
        for cpp_file in cpp_files:
            if process_file(cpp_file):
                modified_count += 1
        
        print(f"\nModified {modified_count} files")
    else:
        print(f"Error: {path} is not a valid file or directory")
        sys.exit(1)

if __name__ == "__main__":
    main()