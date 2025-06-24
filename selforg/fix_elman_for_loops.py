#!/usr/bin/env python3
"""Fix remaining for loops with override in elman.cpp"""

import re

def fix_elman():
    file_path = "/Users/jerry/lpzrobot_mac/selforg/controller/elman.cpp"
    
    with open(file_path, 'r') as f:
        content = f.read()
    
    # Fix for loops with override
    content = re.sub(r'for\s*\(([^)]+)\)\s*override\s*{', r'for (\1) {', content)
    
    with open(file_path, 'w') as f:
        f.write(content)
    
    print(f"Fixed {file_path}")

fix_elman()