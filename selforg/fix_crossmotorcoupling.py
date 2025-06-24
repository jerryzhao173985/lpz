#!/usr/bin/env python3
"""Fix issues in crossmotorcoupling.cpp"""

import re

def fix_crossmotorcoupling():
    file_path = "/Users/jerry/lpzrobot_mac/selforg/controller/crossmotorcoupling.cpp"
    
    with open(file_path, 'r') as f:
        content = f.read()
    
    # Remove override used as statement terminator
    content = re.sub(r'(\w+(?:\([^)]*\))?)\s+override;', r'\1;', content)
    
    # Fix explicit for
    content = re.sub(r'explicit\s+for\s*\(', 'for (', content)
    
    # Fix specific patterns
    content = re.sub(r'(getLastMotorValues\(\))\s+override;', r'\1;', content)
    content = re.sub(r'(setMotorTeaching\([^)]+\))\s+override;', r'\1;', content)
    content = re.sub(r'(->step\([^)]+\))\s+override;', r'\1;', content)
    
    with open(file_path, 'w') as f:
        f.write(content)
    
    print(f"Fixed {file_path}")

fix_crossmotorcoupling()