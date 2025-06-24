#!/usr/bin/env python3
"""Fix override issues in derinf.cpp"""

import re

def fix_derinf():
    file_path = "/Users/jerry/lpzrobot_mac/selforg/controller/derinf.cpp"
    
    with open(file_path, 'r') as f:
        content = f.read()
    
    # Remove all instances of override used as statement terminator
    content = re.sub(r'\s+override;', ';', content)
    
    # Fix for loops with override
    content = re.sub(r'for\s*\(([^)]+)\)\s*override\s*{', r'for (\1) {', content)
    
    # Fix "explicit if" -> "if"
    content = re.sub(r'explicit\s+if\s*\(', 'if (', content)
    
    # Fix specific patterns
    content = re.sub(r'(addParameterDef\([^)]+\))\s+override;', r'\1;', content)
    
    with open(file_path, 'w') as f:
        f.write(content)
    
    print(f"Fixed {file_path}")

fix_derinf()