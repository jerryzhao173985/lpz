#!/usr/bin/env python3
"""Fix last override issues in for loops"""

import re
import os

def fix_file(file_path):
    with open(file_path, 'r') as f:
        content = f.read()
    
    # Fix for loops with override
    content = re.sub(r'for\s*\(([^)]+)\)\s*override\s*{', r'for (\1) {', content)
    
    # Also fix cases where override appears at the end of a for loop line
    content = re.sub(r'(\+\+[a-zA-Z_]\w*\))\s*override\s*{', r'\1 {', content)
    
    with open(file_path, 'w') as f:
        f.write(content)
    
    return file_path

# Files to fix
files_to_fix = [
    "/Users/jerry/lpzrobot_mac/selforg/controller/derbigcontroller.cpp",
    "/Users/jerry/lpzrobot_mac/selforg/controller/derlinunivers.cpp"
]

for file_path in files_to_fix:
    if os.path.exists(file_path):
        fixed = fix_file(file_path)
        print(f"Fixed {fixed}")
    else:
        print(f"File not found: {file_path}")