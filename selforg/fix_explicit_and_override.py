#!/usr/bin/env python3
"""Fix explicit and override issues"""

import re
import os

def fix_file(file_path):
    with open(file_path, 'r') as f:
        content = f.read()
    
    # Fix "explicit if" -> "if"
    content = re.sub(r'explicit\s+if\s*\(', 'if (', content)
    
    # Fix "explicit switch" -> "switch"
    content = re.sub(r'explicit\s+switch\s*\(', 'switch (', content)
    
    # Fix for loops with override
    content = re.sub(r'for\s*\(([^)]+)\)\s*override\s*{', r'for (\1) {', content)
    
    # Fix (const logaE& n) patterns - logaE and rootE are member variables
    content = re.sub(r'\(const\s+logaE&\s*(\d+)\)\s*!=\s*0', r'(logaE >= \1)', content)
    content = re.sub(r'\(const\s+rootE&\s*(\d+)\)\s*!=\s*0', r'(rootE >= \1)', content)
    
    # Fix const on Matrix parameters that are modified
    # For methods like kwtaInhibition that modify the matrix
    content = re.sub(r'void\s+kwtaInhibition\s*\(\s*const\s+(matrix::Matrix&)', 
                     r'void kwtaInhibition(\1', content)
    content = re.sub(r'void\s+limitC\s*\(\s*const\s+(matrix::Matrix&)', 
                     r'void limitC(\1', content)
    
    with open(file_path, 'w') as f:
        f.write(content)
    
    return file_path

# Files to fix
files_to_fix = [
    "/Users/jerry/lpzrobot_mac/selforg/controller/derbigcontroller.cpp",
    "/Users/jerry/lpzrobot_mac/selforg/controller/derlinunivers.cpp",
    "/Users/jerry/lpzrobot_mac/selforg/controller/derpseudosensor.cpp"
]

for file_path in files_to_fix:
    if os.path.exists(file_path):
        fixed = fix_file(file_path)
        print(f"Fixed {fixed}")
    else:
        print(f"File not found: {file_path}")