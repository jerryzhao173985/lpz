#!/usr/bin/env python3
"""Replace sprintf with snprintf for security."""

import re
import os

def fix_sprintf_in_file(filepath):
    """Replace sprintf with snprintf in a file."""
    if not os.path.exists(filepath):
        print(f"File not found: {filepath}")
        return False
    
    # Create backup
    backup_path = filepath + '.bak.sprintf'
    if not os.path.exists(backup_path):
        os.system(f'cp "{filepath}" "{backup_path}"')
    
    with open(filepath, 'r', encoding='utf-8', errors='ignore') as f:
        content = f.read()
    
    # Find sprintf calls and replace with snprintf
    # Pattern: sprintf(buffer, format, ...)
    modified = False
    lines = content.split('\n')
    
    for i, line in enumerate(lines):
        if 'sprintf' in line and 'snprintf' not in line:
            # Try to find the buffer name
            match = re.search(r'sprintf\s*\(\s*(\w+)\s*,', line)
            if match:
                buffer_name = match.group(1)
                # Replace sprintf with snprintf, assuming reasonable buffer sizes
                # For char arrays, we'll use sizeof(buffer)
                new_line = re.sub(
                    r'sprintf\s*\(\s*' + buffer_name + r'\s*,',
                    f'snprintf({buffer_name}, sizeof({buffer_name}),',
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
        '/Users/jerry/lpzrobot_mac/selforg/controller/use_java_controller.cpp',
        '/Users/jerry/lpzrobot_mac/selforg/utils/configurable.cpp'
    ]
    
    for filepath in files_to_fix:
        print(f"\nProcessing {filepath}:")
        if fix_sprintf_in_file(filepath):
            print("  Fixed sprintf warnings")
        else:
            print("  No changes needed")

if __name__ == '__main__':
    main()