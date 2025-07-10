#!/usr/bin/env python3
"""
Convert typedef declarations to using declarations for C++17 modernization.
This script handles simple type aliases and container type aliases.
Function pointer typedefs are skipped as they require more careful conversion.
"""

import os
import re
import sys

def is_function_pointer_typedef(line):
    """Check if a typedef is for a function pointer."""
    # Function pointer patterns include (*name) or specific function signatures
    function_patterns = [
        r'\(\s*\*',  # Contains (*
        r'typedef.*\(.*\)\s*\(',  # Function signature pattern
    ]
    return any(re.search(pattern, line) for pattern in function_patterns)

def is_struct_typedef(lines, index):
    """Check if this is a typedef struct pattern."""
    line = lines[index].strip()
    
    # Check for typedef struct on same line
    if re.match(r'typedef\s+struct\s*\w*\s*{', line):
        return True
    
    # Check for typedef struct on next line
    if line == 'typedef struct' or re.match(r'typedef\s+struct\s+\w+\s*$', line):
        if index + 1 < len(lines):
            next_line = lines[index + 1].strip()
            if next_line.startswith('{'):
                return True
    
    return False

def convert_simple_typedef(line):
    """Convert a simple typedef to using declaration."""
    # Match pattern: typedef Type Alias;
    match = re.match(r'^\s*typedef\s+(.+?)\s+(\w+)\s*;\s*$', line)
    if match:
        type_part = match.group(1).strip()
        alias_name = match.group(2).strip()
        
        # Skip function pointers
        if is_function_pointer_typedef(line):
            return None
            
        # Convert to using declaration
        return f'using {alias_name} = {type_part};'
    
    return None

def process_file(filepath):
    """Process a single file to convert typedefs to using declarations."""
    with open(filepath, 'r') as f:
        lines = f.readlines()
    
    modified = False
    new_lines = []
    i = 0
    
    while i < len(lines):
        line = lines[i]
        
        # Skip typedef struct patterns
        if is_struct_typedef(lines, i):
            new_lines.append(line)
            i += 1
            continue
        
        # Try to convert simple typedef
        converted = convert_simple_typedef(line)
        if converted:
            # Preserve indentation
            indent = len(line) - len(line.lstrip())
            new_lines.append(' ' * indent + converted + '\n')
            modified = True
        else:
            new_lines.append(line)
        
        i += 1
    
    if modified:
        with open(filepath, 'w') as f:
            f.writelines(new_lines)
        return True
    
    return False

def find_typedef_files(directory):
    """Find all header files containing typedef declarations."""
    typedef_files = []
    
    for root, dirs, files in os.walk(directory):
        # Skip build directories and external dependencies
        dirs[:] = [d for d in dirs if d not in ['build', '.git', 'dist', 'opende']]
        
        for file in files:
            if file.endswith(('.h', '.hpp')):
                filepath = os.path.join(root, file)
                try:
                    with open(filepath, 'r') as f:
                        content = f.read()
                        # Look for typedef but exclude typedef struct patterns
                        if re.search(r'\btypedef\b(?!\s+struct)', content):
                            # Double check it's not all typedef struct
                            non_struct_typedefs = re.findall(r'^\s*typedef\s+(?!struct).*$', content, re.MULTILINE)
                            if non_struct_typedefs:
                                typedef_files.append(filepath)
                except Exception as e:
                    print(f"Error reading {filepath}: {e}")
    
    return typedef_files

def main():
    """Main function to convert typedefs in the codebase."""
    # Start from the current directory or specified path
    start_dir = sys.argv[1] if len(sys.argv) > 1 else '.'
    
    print(f"Searching for typedef declarations in {start_dir}...")
    typedef_files = find_typedef_files(start_dir)
    
    print(f"\nFound {len(typedef_files)} files with typedef declarations")
    
    # Show first 10 files
    print("\nFiles to process:")
    for i, file in enumerate(typedef_files[:10]):
        print(f"  {file}")
    if len(typedef_files) > 10:
        print(f"  ... and {len(typedef_files) - 10} more files")
    
    # Ask for confirmation
    response = input("\nProceed with conversion? (y/n): ")
    if response.lower() != 'y':
        print("Conversion cancelled.")
        return
    
    # Process files
    converted_count = 0
    for filepath in typedef_files:
        if process_file(filepath):
            converted_count += 1
            print(f"✓ Converted: {filepath}")
    
    print(f"\nConversion complete: {converted_count} files modified")

if __name__ == '__main__':
    main()