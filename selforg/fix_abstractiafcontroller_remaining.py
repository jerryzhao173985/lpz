#!/usr/bin/env python3
"""Fix remaining override issues in abstractiafcontroller.cpp"""

import re

def fix_remaining_overrides():
    file_path = "/Users/jerry/lpzrobot_mac/selforg/controller/abstractiafcontroller.cpp"
    
    with open(file_path, 'r') as f:
        content = f.read()
    
    # Fix all remaining cases of override as statement terminator
    # Pattern: anything followed by ) override;
    content = re.sub(r'\)\s+override;', r');', content)
    
    # Fix for loops with override
    content = re.sub(r'for\s*\(([^)]+)\)\s*override\s*{', r'for (\1) {', content)
    
    # Fix assert statements
    content = re.sub(r'assert\(([^)]+)\)\s+override;', r'assert(\1);', content)
    
    # Fix multiplication and other operations
    content = re.sub(r'(\*=.*?)\s+override;', r'\1;', content)
    
    # Fix .set() calls
    content = re.sub(r'(\.set\([^)]+\))\s+override;', r'\1;', content)
    
    # Fix .toSum() calls
    content = re.sub(r'(\.toSum\([^)]+\))\s+override;', r'\1;', content)
    
    # Fix .toZero() and chain calls
    content = re.sub(r'(\.to\w+\([^)]*\)(?:\.[^;]+)?)\s+override;', r'\1;', content)
    
    with open(file_path, 'w') as f:
        f.write(content)
    
    print(f"Fixed remaining overrides in {file_path}")

fix_remaining_overrides()