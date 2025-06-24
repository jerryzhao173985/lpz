#!/usr/bin/env python3
"""Fix logaE and rootE syntax errors"""

import re
import os

def fix_file(file_path):
    with open(file_path, 'r') as f:
        content = f.read()
    
    # Fix (logaE & n) patterns - these should be comparing against n
    content = re.sub(r'\(logaE\s*&\s*(\d+)\)\s*!=\s*0', r'(logaE >= \1)', content)
    content = re.sub(r'\(rootE\s*&\s*(\d+)\)\s*!=\s*0', r'(rootE >= \1)', content)
    
    # Fix for loops with override
    content = re.sub(r'for\s*\(([^)]+)\)\s*override\s*{', r'for (\1) {', content)
    
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