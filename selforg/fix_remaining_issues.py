#!/usr/bin/env python3
"""Fix remaining issues in controller files"""

import re
import os

def fix_file(file_path):
    with open(file_path, 'r') as f:
        content = f.read()
    
    # Fix explicit outside class definition
    content = re.sub(r'^explicit\s+', '', content, flags=re.MULTILINE)
    
    # Fix (const logaE& n) and (const rootE& n) patterns
    content = re.sub(r'\(const\s+logaE&\s*(\d+)\)', r'(logaE & \1)', content)
    content = re.sub(r'\(const\s+rootE&\s*(\d+)\)', r'(rootE & \1)', content)
    
    # Fix for loops with override
    content = re.sub(r'for\s*\(([^)]+)\)\s*override\s*{', r'for (\1) {', content)
    
    # Fix matrix::const Matrix& -> const matrix::Matrix&
    content = re.sub(r'matrix::const\s+Matrix&', r'const matrix::Matrix&', content)
    
    # Fix void ... matrix::const Matrix& patterns in function declarations
    content = re.sub(r'(\w+)\s+(\w+)\s*\(matrix::const\s+Matrix&', r'\1 \2(const matrix::Matrix&', content)
    
    with open(file_path, 'w') as f:
        f.write(content)
    
    return file_path

# Files to fix
files_to_fix = [
    "/Users/jerry/lpzrobot_mac/selforg/controller/dercontroller.cpp",
    "/Users/jerry/lpzrobot_mac/selforg/controller/derbigcontroller.cpp"
]

for file_path in files_to_fix:
    if os.path.exists(file_path):
        fixed = fix_file(file_path)
        print(f"Fixed {fixed}")
    else:
        print(f"File not found: {file_path}")