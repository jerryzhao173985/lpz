#!/usr/bin/env python3
import re

def fix_convex_override(filename):
    with open(filename, 'r') as f:
        content = f.read()
    
    # Fix override on non-function calls
    patterns = [
        # Fix override on comments
        (r'//(.*)(\s+override);', r'//\1;'),
        # Fix override on macro calls
        (r'(\bdMULTIPLY[0-9_]+\([^)]+\))\s+override;', r'\1;'),
        (r'(\bdMIN\([^)]+\))\s+override;', r'\1;'),
        (r'(\bdMAX\([^)]+\))\s+override;', r'\1;'),
        (r'(\bdVector3Copy\([^)]+\))\s+override;', r'\1;'),
        # Fix override on delete statements
        (r'(delete\[\]\s+\w+)\s+override;', r'\1;'),
        # Fix override on member access
        (r'(\.\w+\s*=\s*[^;]+)\s+override;', r'\1;'),
        # Fix override on memcpy
        (r'(memcpy\([^)]+\))\s+override;', r'\1;'),
        # Fix override on reserve calls
        (r'(\.\s*reserve\([^)]+\))\s+override;', r'\1;'),
        # Fix override on method calls
        (r'(this->Get\w+\([^)]+\))\s+override;', r'\1;'),
        (r'(this->tree\s*=\s*[^;]+)\s+override;', r'\1;'),
    ]
    
    for pattern, replacement in patterns:
        content = re.sub(pattern, replacement, content, flags=re.MULTILINE)
    
    # Count changes
    changes = 0
    for pattern, _ in patterns:
        changes += len(re.findall(pattern, content, flags=re.MULTILINE))
    
    with open(filename, 'w') as f:
        f.write(content)
    
    return changes

if __name__ == "__main__":
    import sys
    filename = sys.argv[1] if len(sys.argv) > 1 else "/Users/jerry/simulator/lpz/opende/ode/src/convex.cpp"
    changes = fix_convex_override(filename)
    print(f"Fixed {changes} override issues in {filename}")