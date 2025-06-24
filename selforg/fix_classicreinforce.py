#!/usr/bin/env python3
"""Fix override issues in classicreinforce.cpp"""

import re

def fix_classicreinforce():
    file_path = "/Users/jerry/lpzrobot_mac/selforg/controller/classicreinforce.cpp"
    
    with open(file_path, 'r') as f:
        content = f.read()
    
    # Remove override used as statement terminator
    content = re.sub(r'(\w+(?:\([^)]*\))?)\s+override;', r'\1;', content)
    
    # Fix specific patterns
    content = re.sub(r'(assert\([^)]+\))\s+override;', r'\1;', content)
    content = re.sub(r'(addParameterDef\([^)]+\))\s+override;', r'\1;', content)
    content = re.sub(r'(addParameter\([^)]+\))\s+override;', r'\1;', content)
    content = re.sub(r'(std::make_unique<[^>]+>\([^)]*\))\s+override;', r'\1;', content)
    content = re.sub(r'(\.set\([^)]+\))\s+override;', r'\1;', content)
    
    with open(file_path, 'w') as f:
        f.write(content)
    
    print(f"Fixed {file_path}")

fix_classicreinforce()