#!/usr/bin/env python3
"""Fix all remaining override issues in controllernet.cpp"""

import re

def fix_all_overrides():
    file_path = "/Users/jerry/lpzrobot_mac/selforg/controller/controllernet.cpp"
    
    with open(file_path, 'r') as f:
        content = f.read()
    
    # Fix for loops with override
    content = re.sub(r'for\s*\(([^)]+)\)\s*override\s*{', r'for (\1) {', content)
    
    # Fix "explicit if" -> "if"
    content = re.sub(r'explicit\s+if\s*\(', 'if (', content)
    
    # Fix "explicit for" -> "for"
    content = re.sub(r'explicit\s+for\s*\(', 'for (', content)
    
    with open(file_path, 'w') as f:
        f.write(content)
    
    print(f"Fixed all overrides in {file_path}")

fix_all_overrides()