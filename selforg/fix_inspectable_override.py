#!/usr/bin/env python3
"""Fix incorrect override keywords in inspectable.h"""

import re
import sys

def fix_override(file_path):
    with open(file_path, 'r') as f:
        content = f.read()
    
    # Fix constructor with override
    content = re.sub(r'(explicit\s+Inspectable\s*\([^)]*\))\s*override(\s*;)', r'\1\2', content)
    
    # Pattern to find virtual method declarations with override
    pattern = r'(virtual\s+\w+(?:\s*<[^>]+>)?\s+\w+\s*\([^)]*\)(?:\s+const)?(?:\s+noexcept)?)\s+override(\s*[;{])'
    
    # Replace with the same but without override
    content = re.sub(pattern, r'\1\2', content)
    
    # Also fix method definitions with override at the end
    pattern2 = r'(\)\s*const)\s+override(\s*;)'
    content = re.sub(pattern2, r'\1\2', content)
    
    with open(file_path, 'w') as f:
        f.write(content)
    
    print(f"Fixed {file_path}")

if __name__ == "__main__":
    fix_override("/Users/jerry/lpzrobot_mac/selforg/utils/inspectable.h")