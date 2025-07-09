#!/usr/bin/env python3
"""
Filter a patch to only include changes to files that exist in the repository.
"""

import os
import sys
import subprocess

def file_exists_in_repo(filepath):
    """Check if a file exists in the repository."""
    # Remove leading a/ or b/ from git diff paths
    if filepath.startswith('a/') or filepath.startswith('b/'):
        filepath = filepath[2:]
    
    # Check from repository root
    full_path = os.path.join('../../', filepath)
    return os.path.exists(full_path)

def filter_patch(input_file, output_file):
    """Filter patch to only include existing files."""
    with open(input_file, 'r') as f:
        lines = f.readlines()
    
    filtered_lines = []
    include_current_file = True
    current_file_block = []
    files_included = 0
    files_excluded = 0
    
    for i, line in enumerate(lines):
        if line.startswith('diff --git'):
            # Process previous file block
            if current_file_block and include_current_file:
                filtered_lines.extend(current_file_block)
                files_included += 1
            elif current_file_block:
                files_excluded += 1
            
            # Start new file block
            current_file_block = [line]
            
            # Extract file paths
            parts = line.split()
            if len(parts) >= 4:
                file_a = parts[2]
                file_b = parts[3]
                
                # Check if file exists
                include_current_file = file_exists_in_repo(file_b)
                
                if not include_current_file:
                    print(f"Excluding: {file_b} (does not exist)")
        else:
            current_file_block.append(line)
    
    # Process last file block
    if current_file_block and include_current_file:
        filtered_lines.extend(current_file_block)
        files_included += 1
    elif current_file_block:
        files_excluded += 1
    
    # Write filtered patch
    with open(output_file, 'w') as f:
        f.writelines(filtered_lines)
    
    return files_included, files_excluded

if __name__ == "__main__":
    input_file = 'patch_01_typedef_to_using.patch'
    output_file = 'patch_01_typedef_to_using_filtered.patch'
    
    print("Filtering patch to include only existing files...")
    included, excluded = filter_patch(input_file, output_file)
    
    print(f"\nResults:")
    print(f"  Files included: {included}")
    print(f"  Files excluded: {excluded}")
    print(f"  Output written to: {output_file}")