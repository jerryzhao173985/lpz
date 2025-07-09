#!/usr/bin/env python3
"""
Extract throw() to noexcept conversions.
"""

import re

def is_noexcept_change(lines, start_idx):
    """Check if a hunk contains throw() to noexcept conversions."""
    for i in range(start_idx, min(start_idx + 50, len(lines))):
        line = lines[i]
        
        # Skip documentation
        if any(marker in line for marker in ['```', '###', 'NOTE:', 'TODO:', '- ', '* ']):
            continue
            
        # Look for throw() removals
        if line.startswith('-') and 'throw()' in line:
            # Check if there's a corresponding noexcept addition
            for j in range(max(0, i-3), min(i+3, len(lines))):
                if lines[j].startswith('+') and ('noexcept' in lines[j] or 'throw()' not in lines[j]):
                    return True
                    
        # Look for noexcept additions
        if line.startswith('+') and 'noexcept' in line:
            return True
                        
    return False

def extract_noexcept_changes(input_file, output_file):
    with open(input_file, 'r') as f:
        lines = f.readlines()
    
    output_lines = []
    i = 0
    files_with_changes = []
    
    while i < len(lines):
        line = lines[i]
        
        # Found a new file diff
        if line.startswith('diff --git'):
            file_start = i
            file_has_noexcept = False
            current_file = line.split()[2]
            
            # Skip non-source files
            if not any(current_file.endswith(ext) for ext in ['.h', '.hpp', '.cpp', '.cc']):
                j = i + 1
                while j < len(lines) and not lines[j].startswith('diff --git'):
                    j += 1
                i = j
                continue
            
            # Collect the entire file diff
            j = i + 1
            while j < len(lines) and not lines[j].startswith('diff --git'):
                # Check each hunk in this file
                if lines[j].startswith('@@'):
                    if is_noexcept_change(lines, j):
                        file_has_noexcept = True
                j += 1
            
            # If this file has noexcept changes, include it
            if file_has_noexcept:
                # Add the entire file diff
                for k in range(file_start, j):
                    output_lines.append(lines[k])
                files_with_changes.append(current_file)
            
            i = j
        else:
            i += 1
    
    # Write the output
    with open(output_file, 'w') as f:
        f.writelines(output_lines)
    
    # Count the changes
    throw_count = sum(1 for line in output_lines if line.startswith('-') and 'throw()' in line)
    noexcept_count = sum(1 for line in output_lines if line.startswith('+') and 'noexcept' in line)
    
    return throw_count, noexcept_count, len(files_with_changes)

if __name__ == "__main__":
    input_file = 'source_changes.patch'
    output_file = 'patch_07_throw_to_noexcept_clean.patch'
    
    print("Extracting throw() to noexcept conversions...")
    throw_count, noexcept_count, file_count = extract_noexcept_changes(input_file, output_file)
    
    print(f"\nExtracted noexcept changes:")
    print(f"  - throw() removals: {throw_count}")
    print(f"  - noexcept additions: {noexcept_count}")
    print(f"  - Files affected: {file_count}")
    print(f"  - Output written to: {output_file}")