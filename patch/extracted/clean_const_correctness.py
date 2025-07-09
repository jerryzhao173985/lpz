#!/usr/bin/env python3
"""
Clean const correctness patch by removing misplaced overrides.
"""

import re

def clean_const_patch(input_file, output_file):
    with open(input_file, 'r') as f:
        content = f.read()
    
    # Remove lines that have misplaced override after semicolon
    # Pattern: lines ending with " override;"
    cleaned = re.sub(r'(\+.*) override;', r'\1;', content)
    
    # Remove lines that have override in wrong places
    # Pattern: getFitness() override or similar
    cleaned = re.sub(r'(\(\)) override([^{])', r'\1\2', cleaned)
    
    # Fix explicit keyword misplacements
    cleaned = re.sub(r'explicit if\s*\(', r'if (', cleaned)
    cleaned = re.sub(r'void explicit ', r'void ', cleaned)
    
    # Fix nullptr comparisons with numbers
    cleaned = re.sub(r'== nullptr\)', r'== 0)', cleaned)
    cleaned = re.sub(r'!= nullptr\)', r'!= 0)', cleaned)
    
    with open(output_file, 'w') as f:
        f.write(cleaned)
    
    # Count what's left
    remaining_override = len(re.findall(r' override;', cleaned))
    const_additions = len(re.findall(r'\+.*\bconst\b', cleaned))
    
    return remaining_override, const_additions

if __name__ == "__main__":
    input_file = 'patch_06_const_correctness.patch'
    output_file = 'patch_06_const_correctness_clean.patch'
    
    print("Cleaning const correctness patch...")
    remaining, const_count = clean_const_patch(input_file, output_file)
    
    print(f"\nCleaned patch statistics:")
    print(f"  - Remaining misplaced overrides: {remaining}")
    print(f"  - Const additions: ~{const_count}")
    print(f"  - Output written to: {output_file}")