#!/usr/bin/env python3
"""Fix incorrect override keywords in configurable.h"""

import re
import sys

def fix_override(file_path):
    with open(file_path, 'r') as f:
        content = f.read()
    
    # Fix the typo "co overridenst" -> "const"
    content = content.replace("co overridenst", "const")
    
    # Since Configurable is a base class (only inherits from BackCaller),
    # most of its virtual methods should not have override
    # We'll remove override from methods that are not overriding BackCaller methods
    
    # Pattern to find virtual method declarations with override
    pattern = r'(virtual\s+\w+(?:\s*<[^>]+>)?\s+\w+\s*\([^)]*\)(?:\s+const)?(?:\s+noexcept)?)\s+override(\s*[;{])'
    
    # Replace with the same but without override
    content = re.sub(pattern, r'\1\2', content)
    
    # Also fix method definitions with override at the end
    pattern2 = r'(\)\s*const)\s+override(\s*;)'
    content = re.sub(pattern2, r'\1\2', content)
    
    # Fix inline method with override
    pattern3 = r'(virtual\s+void\s+setRevision\s*\([^)]+\))\s*override\s*(\{)'
    content = re.sub(pattern3, r'\1 \2', content)
    
    # Fix other patterns
    pattern4 = r'(virtual\s+\w+(?:\s*<[^>]+>)?\s+\w+\s*\([^)]*\))\s+override(\s*\{)'
    content = re.sub(pattern4, r'\1\2', content)
    
    # Fix print method
    pattern5 = r'(void\s+print\s*\([^)]+\)\s+const)\s+override(\s*;)'
    content = re.sub(pattern5, r'\1\2', content)
    
    with open(file_path, 'w') as f:
        f.write(content)
    
    print(f"Fixed {file_path}")

if __name__ == "__main__":
    fix_override("/Users/jerry/lpzrobot_mac/selforg/utils/configurable.h")