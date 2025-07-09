#!/usr/bin/env python3
"""
Extract ONLY member initialization changes from patch_06.
Skip all 'explicit' keyword changes.
"""

import re

def is_member_init_change(line):
    """Check if a line is a member initialization change."""
    # Look for member initialization in constructors
    # Pattern: variable{value} or variable(value) in initialization lists
    if ':' in line and ('{' in line or '(' in line):
        # Check for common patterns like : m_var(0), m_var{0}
        if re.search(r':\s*\w+[({]', line):
            return True
    
    # Look for in-class member initialization
    # Pattern: type variable = value; or type variable{value};
    if '=' in line and ';' in line and not 'explicit' in line:
        if re.search(r'\w+\s+\w+\s*=\s*[^;]+;', line):
            return True
    if '{' in line and '}' in line and ';' in line and not 'explicit' in line:
        if re.search(r'\w+\s+\w+\s*\{[^}]*\}\s*;', line):
            return True
            
    # Look for member initialization in member declarations
    if re.search(r'(int|double|float|bool|size_t|unsigned)\s+\w+\s*=\s*\d+', line):
        return True
        
    return False

def is_explicit_change(line):
    """Check if a line contains explicit keyword changes."""
    return 'explicit' in line.lower()

def extract_member_init_only(input_file, output_file):
    with open(input_file, 'r') as f:
        lines = f.readlines()
    
    output_lines = []
    i = 0
    files_with_changes = set()
    skipped_explicit_count = 0
    
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
            file_has_init = False
            file_lines = [line]
            
            while j < len(lines) and not lines[j].startswith('diff --git'):
                file_lines.append(lines[j])
                
                # Skip explicit changes
                if lines[j].startswith(('-', '+')) and is_explicit_change(lines[j]):
                    skipped_explicit_count += 1
                    j += 1
                    continue
                
                # Check if this line is a member init change
                if lines[j].startswith(('+', '-')) and is_member_init_change(lines[j]):
                    file_has_init = True
                    
                j += 1
            
            # If this file has member init changes, include relevant parts
            if file_has_init:
                # Filter the file lines to only include member init hunks
                filtered_lines = []
                k = 0
                while k < len(file_lines):
                    if file_lines[k].startswith(('diff --git', 'index ', '--- ', '+++ ')):
                        filtered_lines.append(file_lines[k])
                    elif file_lines[k].startswith('@@'):
                        # Check if this hunk contains member init changes
                        hunk_start = k
                        hunk_has_init = False
                        has_explicit = False
                        m = k + 1
                        
                        while m < len(file_lines) and not file_lines[m].startswith('@@'):
                            if file_lines[m].startswith(('+', '-')):
                                if is_explicit_change(file_lines[m]):
                                    has_explicit = True
                                elif is_member_init_change(file_lines[m]):
                                    hunk_has_init = True
                            m += 1
                        
                        # Include the hunk only if it has member init and no explicit changes
                        if hunk_has_init and not has_explicit:
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
    init_count = sum(1 for line in output_lines if line.startswith(('+', '-')) and is_member_init_change(line))
    
    return init_count, len(files_with_changes), skipped_explicit_count

if __name__ == "__main__":
    input_file = 'patch_06_member_init_only.patch'
    output_file = 'patch_06_member_init_clean.patch'
    
    print("Extracting ONLY member initialization changes...")
    init_count, file_count, skipped_count = extract_member_init_only(input_file, output_file)
    
    print(f"\nExtracted member initialization changes:")
    print(f"  - Member init changes: {init_count}")
    print(f"  - Files affected: {file_count}")
    print(f"  - Skipped explicit changes: {skipped_count}")
    print(f"  - Output written to: {output_file}")