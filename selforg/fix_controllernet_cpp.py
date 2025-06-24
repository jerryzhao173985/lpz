#!/usr/bin/env python3
"""Fix override issues in controllernet.cpp"""

import re

def fix_controllernet_cpp():
    file_path = "/Users/jerry/lpzrobot_mac/selforg/controller/controllernet.cpp"
    
    with open(file_path, 'r') as f:
        content = f.read()
    
    # Remove all instances of override used as statement terminator
    content = re.sub(r'\s+override;', ';', content)
    
    # Fix specific patterns
    content = re.sub(r'(addParameterDef\([^)]+\))\s+override;', r'\1;', content)
    content = re.sub(r'(assert\([^)]+\))\s+override;', r'\1;', content)
    
    # Fix variable declarations with override
    content = re.sub(r'(\w+\s+\w+\s*=\s*[^;]+)\s+override;', r'\1;', content)
    
    # Fix more complex expressions 
    content = re.sub(r'(int\s+\w+\s*=\s*[^;]+)\s+override;', r'\1;', content)
    content = re.sub(r'(unsigned\s+int\s+\w+\s*=\s*[^;]+)\s+override;', r'\1;', content)
    
    with open(file_path, 'w') as f:
        f.write(content)
    
    print(f"Fixed {file_path}")

fix_controllernet_cpp()