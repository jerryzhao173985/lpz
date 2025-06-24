#!/usr/bin/env python3
"""Fix final compilation issues"""

import re
import os

def fix_for_override(content):
    """Fix for loops with override"""
    return re.sub(r'for\s*\(([^)]+)\)\s*override\s*{', r'for (\1) {', content)

def fix_static_const(content):
    """Fix static methods with const"""
    return re.sub(r'(static\s+\w+(?:\s*::\s*\w+)*\s+\w+\s*\([^)]*\))\s*const\s*{', r'\1 {', content)

def fix_override_semicolon(content):
    """Fix override used as statement terminator"""
    return re.sub(r'([^;])\s+override;', r'\1;', content)

def fix_const_matrix_ref(content):
    """Fix assignment to const matrix reference"""
    # For methods that modify the matrix, remove const
    content = re.sub(r'(\w+)\s*\(\s*const\s+(matrix::Matrix&)\s*(\w+)([^)]*)\)\s*{([^}]*\.val\([^)]+\)\s*[*+\-/]=)', 
                     r'\1(\2 \3\4) {\5', content, flags=re.DOTALL)
    return content

def fix_file(file_path):
    with open(file_path, 'r') as f:
        content = f.read()
    
    content = fix_for_override(content)
    content = fix_static_const(content) 
    content = fix_override_semicolon(content)
    content = fix_const_matrix_ref(content)
    
    with open(file_path, 'w') as f:
        f.write(content)
    
    return file_path

# Files to fix
files_to_fix = [
    "/Users/jerry/lpzrobot_mac/selforg/controller/derbigcontroller.cpp",
    "/Users/jerry/lpzrobot_mac/selforg/controller/derlininvert.h",
    "/Users/jerry/lpzrobot_mac/selforg/controller/derlinunivers.h",
    "/Users/jerry/lpzrobot_mac/selforg/controller/derlinunivers.cpp",
    "/Users/jerry/lpzrobot_mac/selforg/controller/derpseudosensor.cpp"
]

for file_path in files_to_fix:
    if os.path.exists(file_path):
        fixed = fix_file(file_path)
        print(f"Fixed {fixed}")
    else:
        print(f"File not found: {file_path}")