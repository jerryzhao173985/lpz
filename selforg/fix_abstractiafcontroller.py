#!/usr/bin/env python3
"""Fix override statement terminators in abstractiafcontroller.cpp"""

import re

def fix_abstractiafcontroller():
    file_path = "/Users/jerry/lpzrobot_mac/selforg/controller/abstractiafcontroller.cpp"
    
    with open(file_path, 'r') as f:
        content = f.read()
    
    # Remove override used as statement terminator
    # Match patterns like: something() override;  or  something override;
    content = re.sub(r'(\w+(?:\([^)]*\))?)\s+override;', r'\1;', content)
    
    # Fix specific pattern: addParameter(...) override;
    content = re.sub(r'(addParameter\([^)]+\))\s+override;', r'\1;', content)
    
    # Fix specific pattern: addInspectableMatrix(...) override;
    content = re.sub(r'(addInspectableMatrix\([^)]+\))\s+override;', r'\1;', content)
    
    # Fix specific pattern: addInspectableValue(...) override;
    content = re.sub(r'(addInspectableValue\([^)]+\))\s+override;', r'\1;', content)
    
    with open(file_path, 'w') as f:
        f.write(content)
    
    print(f"Fixed {file_path}")

fix_abstractiafcontroller()