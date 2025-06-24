#!/usr/bin/env python3
"""Fix all remaining override issues in multiple files"""

import re
import os

def fix_override_in_file(file_path):
    """Fix override issues in a single file"""
    if not os.path.exists(file_path):
        print(f"File not found: {file_path}")
        return
        
    with open(file_path, 'r') as f:
        content = f.read()
    
    original_content = content
    
    # Fix functions with override at the end of declaration
    # For functions declared outside of class
    content = re.sub(r'(\w+\s+\w+\s*\([^)]*\))\s+override(\s*;)', r'\1\2', content)
    
    # Fix inline statements with override
    content = re.sub(r'([^;]+)\s+override\s*;', r'\1;', content)
    
    # Fix struct members with override
    content = re.sub(r'(\s+\w+(?:\s*\[\s*\d+\s*\])?)\s+override\s*;', r'\1;', content)
    
    # Fix type declarations with override
    content = re.sub(r'(std::string\s+\w+)\s+override\s*;', r'\1;', content)
    content = re.sub(r'(enum\s+\w+\s+\w+)\s+override\s*;', r'\1;', content)
    
    if content != original_content:
        with open(file_path, 'w') as f:
            f.write(content)
        print(f"Fixed: {file_path}")

# Files that need fixing based on the error messages
files_to_fix = [
    "/Users/jerry/lpzrobot_mac/selforg/utils/mac_drand48r.h",
    "/Users/jerry/lpzrobot_mac/selforg/utils/randomgenerator.h",
    "/Users/jerry/lpzrobot_mac/selforg/utils/sensormotorinfo.h"
]

for file_path in files_to_fix:
    fix_override_in_file(file_path)

print("Done fixing override issues")