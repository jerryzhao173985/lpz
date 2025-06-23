#!/usr/bin/env python3
"""Fix remaining sprintf calls in the codebase."""

import re
import os

def fix_sprintf_in_file(filepath):
    """Replace sprintf with snprintf in a file."""
    if not os.path.exists(filepath):
        print(f"File not found: {filepath}")
        return False
    
    # Create backup
    backup_path = filepath + '.bak.sprintf_final'
    if not os.path.exists(backup_path):
        os.system(f'cp "{filepath}" "{backup_path}"')
    
    with open(filepath, 'r', encoding='utf-8', errors='ignore') as f:
        content = f.read()
    
    # Find sprintf calls and their buffers
    modified = False
    lines = content.split('\n')
    
    # First pass: find buffer declarations
    buffer_sizes = {}
    for i, line in enumerate(lines):
        # Look for char buffer declarations
        match = re.search(r'char\s+(\w+)\[(\d+)\]', line)
        if match:
            buffer_name = match.group(1)
            buffer_size = match.group(2)
            buffer_sizes[buffer_name] = buffer_size
    
    # Second pass: replace sprintf with snprintf
    for i, line in enumerate(lines):
        if 'sprintf' in line and 'snprintf' not in line:
            # Try to find the buffer name
            match = re.search(r'sprintf\s*\(\s*(\w+)\s*,', line)
            if match:
                buffer_name = match.group(1)
                # Use sizeof if it's a known buffer, otherwise use a reasonable size
                if buffer_name in buffer_sizes:
                    size_expr = f'sizeof({buffer_name})'
                else:
                    # For cmd buffers and others, use a reasonable size
                    size_expr = '1024' if 'cmd' in buffer_name else 'sizeof(' + buffer_name + ')'
                
                new_line = re.sub(
                    r'sprintf\s*\(\s*' + buffer_name + r'\s*,',
                    f'snprintf({buffer_name}, {size_expr},',
                    line
                )
                if new_line != line:
                    lines[i] = new_line
                    modified = True
                    print(f"  Line {i+1}: Fixed sprintf for buffer '{buffer_name}'")
    
    if modified:
        with open(filepath, 'w', encoding='utf-8') as f:
            f.write('\n'.join(lines))
        return True
    
    return False

def main():
    files_to_fix = [
        '/Users/jerry/lpzrobot_mac/selforg/utils/plotoption.cpp',
        '/Users/jerry/lpzrobot_mac/selforg/utils/stl_adds.cpp'
    ]
    
    for filepath in files_to_fix:
        print(f"\nProcessing {filepath}:")
        if fix_sprintf_in_file(filepath):
            print("  Fixed sprintf warnings")
        else:
            print("  No changes needed")

if __name__ == '__main__':
    main()