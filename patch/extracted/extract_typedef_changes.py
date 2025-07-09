#!/usr/bin/env python3
"""
Extract typedef to using declaration changes from the big patch.
This script carefully identifies and extracts only the relevant changes.
"""

import re
import sys

def is_typedef_change(lines, start_idx):
    """Check if a hunk contains typedef to using changes."""
    for i in range(start_idx, min(start_idx + 50, len(lines))):
        line = lines[i]
        # Look for typedef removal
        if line.startswith('-') and 'typedef' in line:
            # Check if there's a corresponding using declaration nearby
            for j in range(max(0, i-10), min(i+10, len(lines))):
                if lines[j].startswith('+') and 'using' in lines[j] and '=' in lines[j]:
                    return True
            return True
        # Look for using declaration addition
        if line.startswith('+') and 'using' in line and '=' in line:
            # Check if there's a typedef removal nearby
            for j in range(max(0, i-10), min(i+10, len(lines))):
                if lines[j].startswith('-') and 'typedef' in lines[j]:
                    return True
    return False

def extract_typedef_changes(input_file, output_file):
    with open(input_file, 'r') as f:
        lines = f.readlines()
    
    output_lines = []
    i = 0
    
    while i < len(lines):
        line = lines[i]
        
        # Found a new file diff
        if line.startswith('diff --git'):
            file_start = i
            file_has_typedef = False
            
            # Collect the entire file diff
            j = i + 1
            while j < len(lines) and not lines[j].startswith('diff --git'):
                # Check each hunk in this file
                if lines[j].startswith('@@'):
                    if is_typedef_change(lines, j):
                        file_has_typedef = True
                j += 1
            
            # If this file has typedef changes, include it
            if file_has_typedef:
                # Add the entire file diff
                for k in range(file_start, j):
                    output_lines.append(lines[k])
            
            i = j
        else:
            i += 1
    
    # Write the output
    with open(output_file, 'w') as f:
        f.writelines(output_lines)
    
    # Count the changes
    typedef_count = sum(1 for line in output_lines if line.startswith('-') and 'typedef' in line)
    using_count = sum(1 for line in output_lines if line.startswith('+') and 'using' in line and '=' in line)
    
    return typedef_count, using_count

if __name__ == "__main__":
    input_file = 'patch/extracted/source_changes.patch'
    output_file = 'patch/extracted/patch_01_typedef_to_using.patch'
    
    typedef_count, using_count = extract_typedef_changes(input_file, output_file)
    
    print(f"Extracted typedef to using changes:")
    print(f"  - Removed typedefs: {typedef_count}")
    print(f"  - Added using declarations: {using_count}")
    print(f"  - Output written to: {output_file}")