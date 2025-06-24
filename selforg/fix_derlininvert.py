#!/usr/bin/env python3
"""Fix override issues in derlininvert.cpp"""

import re

def fix_derlininvert():
    file_path = "/Users/jerry/lpzrobot_mac/selforg/controller/derlininvert.cpp"
    
    with open(file_path, 'r') as f:
        content = f.read()
    
    # Remove all instances of override used as statement terminator
    content = re.sub(r'\s+override;', ';', content)
    
    # Fix specific patterns
    content = re.sub(r'(addParameter\([^)]+\))\s+override;', r'\1;', content)
    content = re.sub(r'(addParameterDef\([^)]+\))\s+override;', r'\1;', content)
    
    with open(file_path, 'w') as f:
        f.write(content)
    
    print(f"Fixed {file_path}")

fix_derlininvert()