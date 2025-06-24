#!/usr/bin/env python3
"""Fix override issues in dep.cpp"""

import re

def fix_dep():
    file_path = "/Users/jerry/lpzrobot_mac/selforg/controller/dep.cpp"
    
    with open(file_path, 'r') as f:
        content = f.read()
    
    # Remove override used as statement terminator
    content = re.sub(r'(\w+(?:\([^)]*\))?)\s+override;', r'\1;', content)
    
    # Fix specific patterns
    content = re.sub(r'(addParameterDef\([^)]+\))\s+override;', r'\1;', content)
    
    # Fix return statement with override
    content = re.sub(r'(return\s+[^;]+)\s+override;', r'\1;', content)
    
    with open(file_path, 'w') as f:
        f.write(content)
    
    print(f"Fixed {file_path}")

fix_dep()