#!/usr/bin/env python3
"""Fix issues in noisegenerator.h"""

import re

def fix_noisegenerator(file_path):
    with open(file_path, 'r') as f:
        content = f.read()
    
    # Fix incorrect use of "explicit if" -> "if"
    content = re.sub(r'explicit\s+if\s*\(', 'if (', content)
    
    # Fix incorrect override in loops
    content = re.sub(r'for\s*\([^)]+\)\s*override\s*{', r'for (\g<0>) {', content)
    content = re.sub(r'\)\s*override\s*{', ') {', content)
    
    # Since NoiseGenerator is a base class, remove override from its virtual methods
    # Pattern to match virtual methods in NoiseGenerator class
    content = re.sub(r'(class NoiseGenerator[^}]+})', fix_base_class_overrides, content, flags=re.DOTALL)
    
    # Fix subclasses that properly use override
    # But first check if they inherit from NoiseGenerator properly
    
    with open(file_path, 'w') as f:
        f.write(content)
    
    print(f"Fixed {file_path}")

def fix_base_class_overrides(match):
    """Remove override from base class methods"""
    class_content = match.group(1)
    # Remove override from virtual methods in base class
    class_content = re.sub(r'(virtual\s+\w+(?:\s*<[^>]+>)?\s+\w+\s*\([^)]*\)(?:\s+const)?)\s+override', r'\1', class_content)
    return class_content

# Fix the file
fix_noisegenerator("/Users/jerry/lpzrobot_mac/selforg/utils/noisegenerator.h")