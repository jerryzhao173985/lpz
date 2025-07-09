#!/usr/bin/env python3
import re
import os
import sys

def fix_override_issues(filename):
    with open(filename, 'r') as f:
        content = f.read()
    
    original_content = content
    
    # Fix override on non-function declarations
    patterns = [
        # Fix override on comments
        (r'//(.*)(\s+override);', r'//\1;'),
        # Fix override on macro calls (any macro)
        (r'(\b[A-Z_]+\([^)]+\))\s+override;', r'\1;'),
        # Fix override on delete statements
        (r'(delete\[\]\s+\w+)\s+override;', r'\1;'),
        # Fix override on member access and assignments
        (r'(\.\w+\s*[=+\-*/]?=\s*[^;]+)\s+override;', r'\1;'),
        # Fix override on function calls
        (r'([\w>]+\([^)]*\))\s+override;', r'\1;'),
        # Fix override on array access
        (r'(\[[^\]]+\])\s+override;', r'\1;'),
        # Fix override on continue/break statements
        (r'(continue|break)\s+override;', r'\1;'),
        # Fix override on return statements
        (r'(return\s+[^;]+)\s+override;', r'\1;'),
        # Fix override on variable declarations/assignments
        (r'(\w+\s*[=+\-*/]?=\s*[^;]+)\s+override;', r'\1;'),
        # Fix override on static_cast
        (r'(static_cast<[^>]+>\([^)]+\))\s+override;', r'\1;'),
        # Fix override on const casts
        (r'(const_cast<[^>]+>\([^)]+\))\s+override;', r'\1;'),
        # Fix override on reinterpret_cast
        (r'(reinterpret_cast<[^>]+>\([^)]+\))\s+override;', r'\1;'),
        # Fix override on dynamic_cast
        (r'(dynamic_cast<[^>]+>\([^)]+\))\s+override;', r'\1;'),
    ]
    
    for pattern, replacement in patterns:
        content = re.sub(pattern, replacement, content, flags=re.MULTILINE)
    
    # Special cases for dxConvex destructor and constructor declarations
    content = re.sub(r'(\bdxConvex\s*\([^)]+\))\s+override;', r'\1;', content)
    content = re.sub(r'(~dxConvex\s*:\s*\w+\(\))\s*\{', r'~dxConvex() {', content)
    
    # Fix override on assertions and other macros more broadly
    content = re.sub(r'(\b[a-zA-Z_]+ASSERT[a-zA-Z_]*\s*\([^)]+\))\s+override;', r'\1;', content)
    
    # Count changes
    changes = content.count(' override;') - original_content.count(' override;')
    
    with open(filename, 'w') as f:
        f.write(content)
    
    return abs(changes)

if __name__ == "__main__":
    files_to_fix = [
        "/Users/jerry/simulator/lpz/opende/ode/src/convex.cpp",
        "/Users/jerry/simulator/lpz/opende/ode/src/collision_std.h"
    ]
    
    total_changes = 0
    for filename in files_to_fix:
        if os.path.exists(filename):
            changes = fix_override_issues(filename)
            print(f"Fixed {changes} override issues in {filename}")
            total_changes += changes
    
    print(f"Total fixes: {total_changes}")