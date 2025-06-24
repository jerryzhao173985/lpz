#!/usr/bin/env python3
"""Fix all issues in elman.cpp"""

import re

def fix_elman():
    file_path = "/Users/jerry/lpzrobot_mac/selforg/controller/elman.cpp"
    
    with open(file_path, 'r') as f:
        content = f.read()
    
    # Remove all instances of override used as statement terminator
    content = re.sub(r'\s+override;', ';', content)
    
    # Fix "explicit if" -> "if"
    content = re.sub(r'explicit\s+if\s*\(', 'if (', content)
    
    # Fix double semicolon
    content = re.sub(r';;', ';', content)
    
    # Fix statements ending with "override"
    content = re.sub(r'(\))\s*override\s*;', r'\1;', content)
    
    # Fix assert statements with override
    content = re.sub(r'(assert\([^)]+\))\s*override\s*;', r'\1;', content)
    
    # Fix variable declarations with override
    content = re.sub(r'(=\s*[^;]+)\s*override\s*;', r'\1;', content)
    
    with open(file_path, 'w') as f:
        f.write(content)
    
    print(f"Fixed {file_path}")

fix_elman()