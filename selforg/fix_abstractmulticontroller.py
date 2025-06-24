#!/usr/bin/env python3
"""Fix override issues in abstractmulticontroller.cpp"""

import re

def fix_abstractmulticontroller():
    file_path = "/Users/jerry/lpzrobot_mac/selforg/controller/abstractmulticontroller.cpp"
    
    with open(file_path, 'r') as f:
        content = f.read()
    
    # Remove override used as statement terminator
    content = re.sub(r'(\w+(?:\([^)]*\))?)\s+override;', r'\1;', content)
    
    # Fix specific patterns
    content = re.sub(r'(\.push_back\([^)]+\))\s+override;', r'\1;', content)
    content = re.sub(r'(addConfigurable\([^)]+\))\s+override;', r'\1;', content)
    content = re.sub(r'(addInspectable\([^)]+\))\s+override;', r'\1;', content)
    
    # Fix function calls with override
    content = re.sub(r'([A-Za-z_:]+::init\([^)]+\))\s+override;', r'\1;', content)
    
    with open(file_path, 'w') as f:
        f.write(content)
    
    print(f"Fixed {file_path}")

fix_abstractmulticontroller()