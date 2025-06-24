#!/usr/bin/env python3
"""Fix override statement terminators in wiredcontroller.cpp"""

import re

def fix_wiredcontroller():
    file_path = "/Users/jerry/lpzrobot_mac/selforg/wiredcontroller.cpp"
    
    with open(file_path, 'r') as f:
        content = f.read()
    
    # Remove override used as statement terminator
    # Match patterns like: something() override;  or  something override;
    content = re.sub(r'(\w+(?:\([^)]*\))?)\s+override;', r'\1;', content)
    
    # Fix specific pattern: delete something override;
    content = re.sub(r'delete\s+(\w+)\s+override;', r'delete \1;', content)
    
    # Fix specific pattern: free(something) override;
    content = re.sub(r'free\(([^)]+)\)\s+override;', r'free(\1);', content)
    
    # Fix specific pattern: assert(...) override;
    content = re.sub(r'assert\(([^)]+)\)\s+override;', r'assert(\1);', content)
    
    # Fix specific pattern: new Something() override;
    content = re.sub(r'new\s+(\w+(?:\([^)]*\))?)\s+override;', r'new \1;', content)
    
    # Fix specific pattern: static_cast<type>(...) override;
    content = re.sub(r'(static_cast<[^>]+>\([^)]+\))\s+override;', r'\1;', content)
    
    # Fix specific pattern: dynamic_cast<type>(...) override;
    content = re.sub(r'(dynamic_cast<[^>]+>\([^)]+\))\s+override;', r'\1;', content)
    
    # Fix specific pattern: malloc(sizeof(...) * ...) override;
    content = re.sub(r'(malloc\(sizeof\([^)]+\)\s*\*\s*[^)]+\))\s+override;', r'\1;', content)
    
    # Fix cases where override is on the next line after a function call
    content = re.sub(r'\)\s*\n\s*override;', r');', content)
    
    with open(file_path, 'w') as f:
        f.write(content)
    
    print(f"Fixed {file_path}")

fix_wiredcontroller()