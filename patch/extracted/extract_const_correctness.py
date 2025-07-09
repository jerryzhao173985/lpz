#!/usr/bin/env python3
"""
Extract const correctness improvements from the patch.
Focus on adding const to methods, parameters, and return types.
"""

import re

def is_const_improvement(lines, start_idx):
    """Check if a hunk contains const correctness improvements."""
    for i in range(start_idx, min(start_idx + 30, len(lines))):
        line = lines[i]
        
        # Look for const additions
        if line.startswith('+') and ' const ' in line:
            # Check if it's a real const improvement (not just in comments)
            if not any(marker in line for marker in ['//', '/*', '*/', '#']):
                # Look for method const, parameter const, etc.
                if re.search(r'\) const[;{]|\bconst\s+\w+[&*]?\s+\w+|const\s+auto\b', line):
                    return True
        
        # Look for const method additions
        if line.startswith('-') and line.startswith('+'):
            old_line = line[1:]
            # Check if const was added to a method
            if ') const' in lines[i] and ') const' not in old_line:
                return True
    
    return False

def extract_const_improvements(input_file, output_file):
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
            file_has_const = False
            
            # Skip non-C++ files
            if not current_file.endswith(('.cpp', '.h', '.hpp', '.cc')):
                j = i + 1
                while j < len(lines) and not lines[j].startswith('diff --git'):
                    j += 1
                i = j
                continue
            
            # Skip docs and tests
            if any(x in current_file for x in ['docs/', 'test', '.md', 'examples/']):
                j = i + 1
                while j < len(lines) and not lines[j].startswith('diff --git'):
                    j += 1
                i = j
                continue
            
            # Check for const improvements
            j = i + 1
            while j < len(lines) and not lines[j].startswith('diff --git'):
                if lines[j].startswith('@@'):
                    if is_const_improvement(lines, j):
                        file_has_const = True
                j += 1
            
            if file_has_const:
                for k in range(file_start, j):
                    output_lines.append(lines[k])
                files_with_changes.append(current_file)
            
            i = j
        else:
            i += 1
    
    # Write output
    with open(output_file, 'w') as f:
        f.writelines(output_lines)
    
    # Count const additions
    const_count = 0
    for line in output_lines:
        if line.startswith('+') and ' const ' in line:
            if not any(marker in line for marker in ['//', '/*', '#', 'NOTE']):
                const_count += 1
    
    return const_count, len(files_with_changes)

if __name__ == "__main__":
    input_file = 'patch/extracted/source_changes.patch'
    output_file = 'patch/extracted/patch_06_const_correctness.patch'
    
    print("Extracting const correctness improvements...")
    const_count, file_count = extract_const_improvements(input_file, output_file)
    
    print(f"Extracted const correctness changes:")
    print(f"  - Const additions: {const_count}")
    print(f"  - Files affected: {file_count}")
    print(f"  - Output written to: {output_file}")