#!/usr/bin/env python3
"""
Convert typedef declarations to using declarations for C++17 modernization.
Batch version that processes files without user interaction.
"""

import os
import re
import sys

def is_function_pointer_typedef(line):
    """Check if a typedef is for a function pointer."""
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
    try:
        with open(filepath, 'r') as f:
            lines = f.readlines()
    except Exception as e:
        print(f"Error reading {filepath}: {e}")
        return False
    
    modified = False
    new_lines = []
    i = 0
    conversions = []
    
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
            conversions.append((line.strip(), converted))
        else:
            new_lines.append(line)
        
        i += 1
    
    if modified:
        try:
            with open(filepath, 'w') as f:
                f.writelines(new_lines)
            for old, new in conversions:
                print(f"  {old} → {new}")
            return True
        except Exception as e:
            print(f"Error writing {filepath}: {e}")
            return False
    
    return False

def main():
    """Main function to convert typedefs in specified files."""
    if len(sys.argv) < 2:
        print("Usage: python3 convert_typedef_to_using_batch.py <file1> [file2] ...")
        return
    
    files_to_process = sys.argv[1:]
    converted_count = 0
    
    for filepath in files_to_process:
        if not os.path.exists(filepath):
            print(f"File not found: {filepath}")
            continue
            
        print(f"\nProcessing: {filepath}")
        if process_file(filepath):
            converted_count += 1
            print(f"✓ Converted")
        else:
            print(f"  No typedefs to convert")
    
    print(f"\n✓ Conversion complete: {converted_count} files modified")

if __name__ == '__main__':
    main()