#!/usr/bin/env python3
"""
Extract ONLY throw() to noexcept conversions, excluding other changes.
"""

import re

def is_throw_to_noexcept_change(line):
    """Check if a line is specifically a throw() to noexcept change."""
    # Look for lines that remove throw()
    if line.startswith('-') and 'throw()' in line and 'throw() {' not in line:
        return True
    # Look for lines that add noexcept in place of throw()
    if line.startswith('+') and 'noexcept' in line and ('~' in line or 'what()' in line):
        return True
    return False

def extract_throw_noexcept_only(input_file, output_file):
    with open(input_file, 'r') as f:
        lines = f.readlines()
    
    output_lines = []
    i = 0
    files_with_changes = set()
    
    while i < len(lines):
        line = lines[i]
        
        # Found a new file diff
        if line.startswith('diff --git'):
            file_start = i
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
            file_has_throw_changes = False
            file_lines = [line]
            
            while j < len(lines) and not lines[j].startswith('diff --git'):
                file_lines.append(lines[j])
                
                # Check if this line is a throw() to noexcept change
                if is_throw_to_noexcept_change(lines[j]):
                    file_has_throw_changes = True
                    
                j += 1
            
            # If this file has throw() changes, include it
            if file_has_throw_changes:
                # Filter the file lines to only include relevant hunks
                filtered_lines = []
                k = 0
                while k < len(file_lines):
                    if file_lines[k].startswith('diff --git') or \
                       file_lines[k].startswith('index ') or \
                       file_lines[k].startswith('--- ') or \
                       file_lines[k].startswith('+++ '):
                        filtered_lines.append(file_lines[k])
                    elif file_lines[k].startswith('@@'):
                        # Check if this hunk contains throw() changes
                        hunk_start = k
                        hunk_has_changes = False
                        m = k + 1
                        while m < len(file_lines) and not file_lines[m].startswith('@@'):
                            if is_throw_to_noexcept_change(file_lines[m]):
                                hunk_has_changes = True
                            m += 1
                        
                        # Include the hunk if it has throw() changes
                        if hunk_has_changes:
                            for n in range(hunk_start, m):
                                filtered_lines.append(file_lines[n])
                        k = m - 1
                    k += 1
                
                if len(filtered_lines) > 4:  # Has more than just headers
                    output_lines.extend(filtered_lines)
                    files_with_changes.add(current_file)
            
            i = j
        else:
            i += 1
    
    # Write the output
    with open(output_file, 'w') as f:
        f.writelines(output_lines)
    
    # Count the changes
    throw_count = sum(1 for line in output_lines if line.startswith('-') and 'throw()' in line)
    noexcept_count = sum(1 for line in output_lines if line.startswith('+') and 'noexcept' in line and ('~' in line or 'what()' in line))
    
    return throw_count, noexcept_count, len(files_with_changes)

if __name__ == "__main__":
    input_file = 'source_changes.patch'
    output_file = 'patch_07_throw_noexcept_focused.patch'
    
    print("Extracting ONLY throw() to noexcept conversions...")
    throw_count, noexcept_count, file_count = extract_throw_noexcept_only(input_file, output_file)
    
    print(f"\nExtracted focused throw() to noexcept changes:")
    print(f"  - throw() removals: {throw_count}")
    print(f"  - noexcept additions: {noexcept_count}")
    print(f"  - Files affected: {file_count}")
    print(f"  - Output written to: {output_file}")