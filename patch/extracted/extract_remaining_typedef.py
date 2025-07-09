#!/usr/bin/env python3
"""
Extract remaining typedef conversions from the patch.
Focus on the typedefs that are still in the current codebase.
"""

import re
import sys

# Specific files we know still have typedefs
remaining_typedef_files = [
    'selforg/tests/unit_test.hpp',
    'selforg/abstractrobot.h',
    'selforg/utils/globaldatabase.h',
    'selforg/utils/lyapunov.h',
    'selforg/utils/mediator.h',
    'selforg/utils/backcallervector.h',
    'selforg/utils/mediatorcollegue.h',
]

def extract_specific_typedef_changes(input_file, output_file):
    with open(input_file, 'r') as f:
        lines = f.readlines()
    
    output_lines = []
    i = 0
    extracted_files = []
    
    while i < len(lines):
        line = lines[i]
        
        # Found a new file diff
        if line.startswith('diff --git'):
            file_start = i
            current_file = line.split()[2].replace('a/', '').replace('b/', '')
            
            # Check if this is one of our target files
            include_file = False
            for target in remaining_typedef_files:
                if target in current_file:
                    include_file = True
                    break
            
            if include_file:
                # Collect the entire file diff
                j = i + 1
                while j < len(lines) and not lines[j].startswith('diff --git'):
                    j += 1
                
                # Check if there are actual typedef changes
                has_typedef_change = False
                for k in range(file_start, j):
                    if lines[k].startswith('-') and 'typedef' in lines[k]:
                        has_typedef_change = True
                        break
                
                if has_typedef_change:
                    # Add the entire file diff
                    for k in range(file_start, j):
                        output_lines.append(lines[k])
                    extracted_files.append(current_file)
                
                i = j
            else:
                # Skip to next file
                j = i + 1
                while j < len(lines) and not lines[j].startswith('diff --git'):
                    j += 1
                i = j
        else:
            i += 1
    
    # Write the output
    with open(output_file, 'w') as f:
        f.writelines(output_lines)
    
    return len(extracted_files), len(output_lines)

if __name__ == "__main__":
    input_file = 'patch/extracted/source_changes.patch'
    output_file = 'patch/extracted/patch_01b_remaining_typedef.patch'
    
    print("Extracting remaining typedef changes...")
    file_count, line_count = extract_specific_typedef_changes(input_file, output_file)
    
    print(f"Extracted remaining typedef changes:")
    print(f"  - Files with changes: {file_count}")
    print(f"  - Total lines: {line_count}")
    print(f"  - Output written to: {output_file}")
    
    if file_count == 0:
        print("\nNo typedef changes found for the remaining files.")
        print("These typedefs may need manual conversion.")