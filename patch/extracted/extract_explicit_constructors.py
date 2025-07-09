#!/usr/bin/env python3
"""
Extract explicit constructor additions from the patch.
Focus on actual constructor declarations that need explicit keyword.
"""

import re

def is_explicit_constructor_change(lines, start_idx):
    """Check if a hunk contains explicit constructor additions."""
    for i in range(start_idx, min(start_idx + 20, len(lines))):
        line = lines[i]
        
        # Look for constructor patterns that should be explicit
        if line.startswith('+') and 'explicit' in line:
            # Check if it's a constructor (has class name pattern)
            if re.search(r'explicit\s+\w+\s*\([^)]*\)', line):
                return True
                
        # Look for single-argument constructors without explicit
        if line.startswith('-') and not 'explicit' in line:
            # Pattern for single-arg constructor
            if re.search(r'^\s*\w+\s*\([^,)]+\)\s*[;:{]', line[1:]):
                # Check if explicit is added nearby
                for j in range(max(0, i-3), min(i+3, len(lines))):
                    if lines[j].startswith('+') and 'explicit' in lines[j]:
                        return True
    
    return False

def extract_explicit_changes(input_file, output_file):
    with open(input_file, 'r') as f:
        lines = f.readlines()
    
    output_lines = []
    i = 0
    files_with_changes = []
    
    while i < len(lines):
        line = lines[i]
        
        if line.startswith('diff --git'):
            file_start = i
            current_file = line.split()[2]
            file_has_explicit = False
            
            # Skip non-header files
            if not current_file.endswith(('.h', '.hpp')):
                j = i + 1
                while j < len(lines) and not lines[j].startswith('diff --git'):
                    j += 1
                i = j
                continue
            
            # Skip documentation files
            if any(x in current_file for x in ['docs/', 'README', 'CHANGELOG', '.md']):
                j = i + 1
                while j < len(lines) and not lines[j].startswith('diff --git'):
                    j += 1
                i = j
                continue
            
            # Collect the entire file diff
            j = i + 1
            while j < len(lines) and not lines[j].startswith('diff --git'):
                if lines[j].startswith('@@'):
                    if is_explicit_constructor_change(lines, j):
                        file_has_explicit = True
                j += 1
            
            if file_has_explicit:
                for k in range(file_start, j):
                    output_lines.append(lines[k])
                files_with_changes.append(current_file)
            
            i = j
        else:
            i += 1
    
    # Write the output
    with open(output_file, 'w') as f:
        f.writelines(output_lines)
    
    # Count actual explicit additions
    explicit_count = 0
    for line in output_lines:
        if line.startswith('+') and 'explicit' in line and '(' in line:
            explicit_count += 1
    
    return explicit_count, len(files_with_changes)

if __name__ == "__main__":
    input_file = 'patch/extracted/source_changes.patch'
    output_file = 'patch/extracted/patch_03_explicit_constructors.patch'
    
    print("Extracting explicit constructor changes...")
    explicit_count, file_count = extract_explicit_changes(input_file, output_file)
    
    print(f"Extracted explicit constructor changes:")
    print(f"  - Explicit additions: {explicit_count}")
    print(f"  - Files affected: {file_count}")
    print(f"  - Output written to: {output_file}")
    
    if file_count == 0:
        print("\nNo explicit constructor changes found in header files.")