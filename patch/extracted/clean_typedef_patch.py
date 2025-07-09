#!/usr/bin/env python3
"""
Clean the typedef patch by removing incorrect changes and focusing only on valid typedef conversions.
"""

import re

def is_valid_typedef_change(removed_line, added_line):
    """Check if this is a valid typedef to using conversion."""
    # Must start with typedef
    if not removed_line.strip().startswith('typedef'):
        return False
    
    # Added line should have using
    if not added_line or 'using' not in added_line:
        return False
    
    # Should not contain 'explicit if' or 'explicit port' etc
    if 'explicit if' in removed_line or 'explicit port' in removed_line:
        return False
    
    return True

def clean_patch(input_file, output_file):
    """Clean the patch to only include valid typedef conversions."""
    with open(input_file, 'r') as f:
        content = f.read()
    
    # Split into file blocks
    file_blocks = content.split('diff --git')
    
    cleaned_blocks = []
    
    for block in file_blocks[1:]:  # Skip first empty block
        lines = block.split('\n')
        
        # Check if this block has any bad "explicit" changes
        has_bad_explicit = any('explicit if' in line or 'explicit port' in line 
                              or 'explicit const' in line for line in lines)
        
        # Check if this block has valid typedef changes
        has_typedef_change = False
        for i, line in enumerate(lines):
            if line.startswith('-') and 'typedef' in line and i + 1 < len(lines):
                next_line = lines[i + 1]
                if next_line.startswith('+') and 'using' in next_line:
                    has_typedef_change = True
                    break
        
        # Only include blocks with typedef changes and no bad explicit changes
        if has_typedef_change and not has_bad_explicit:
            cleaned_blocks.append('diff --git' + block)
    
    # Write cleaned patch
    with open(output_file, 'w') as f:
        f.write('\n'.join(cleaned_blocks))
    
    return len(file_blocks) - 1, len(cleaned_blocks)

if __name__ == "__main__":
    input_file = 'patch_01_typedef_to_using.patch'
    output_file = 'patch_01_typedef_to_using_clean.patch'
    
    print("Cleaning typedef patch...")
    total_blocks, cleaned_blocks = clean_patch(input_file, output_file)
    
    print(f"\nResults:")
    print(f"  Original file blocks: {total_blocks}")
    print(f"  Cleaned file blocks: {cleaned_blocks}")
    print(f"  Removed blocks with bad changes: {total_blocks - cleaned_blocks}")
    print(f"  Output written to: {output_file}")