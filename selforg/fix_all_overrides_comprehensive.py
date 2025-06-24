#!/usr/bin/env python3
"""Comprehensive fix for all override issues"""

import re
import os
import glob

def fix_override_in_file(file_path):
    """Fix override issues in a single file"""
    if not os.path.exists(file_path):
        print(f"File not found: {file_path}")
        return False
        
    with open(file_path, 'r') as f:
        content = f.read()
    
    original_content = content
    
    # Fix constructors with override (constructors can't be override)
    content = re.sub(r'(explicit\s+\w+\s*\([^)]*\))\s*override(\s*;)', r'\1\2', content)
    
    # Fix non-member functions with override
    content = re.sub(r'^(\s*\w+\s+\w+\s*\([^)]*\)(?:\s+const)?)\s+override(\s*;)', r'\1\2', content, flags=re.MULTILINE)
    
    # Check if file contains a base class definition
    # If it's a base class (not inheriting), remove all overrides
    base_class_patterns = [
        r'class\s+Storeable\s*{',
        r'class\s+PlotOptionEngine\s*{',
        r'class\s+AbstractController\s*:?\s*(?:public\s+)?(?:Configurable|Inspectable|Storeable)',
    ]
    
    is_base_or_limited_inheritance = False
    for pattern in base_class_patterns:
        if re.search(pattern, content):
            is_base_or_limited_inheritance = True
            break
    
    if is_base_or_limited_inheritance:
        # Remove override from virtual methods in base classes or classes with limited inheritance
        content = re.sub(r'(virtual\s+\w+(?:\s*<[^>]+>)?\s+\w+\s*\([^)]*\)(?:\s+const)?(?:\s+noexcept)?)\s+override(\s*[;{])', 
                        r'\1\2', content)
    
    # Fix any remaining syntax errors with override
    # Remove override after expressions
    content = re.sub(r'([^;{]+)\s+override\s*;', r'\1;', content)
    
    # Fix struct members with override
    content = re.sub(r'(\s+(?:std::)?(?:string|int|double|bool|enum\s+\w+)\s+\w+(?:\s*\[\s*\d+\s*\])?)\s+override\s*;', r'\1;', content)
    
    if content != original_content:
        with open(file_path, 'w') as f:
            f.write(content)
        print(f"Fixed: {file_path}")
        return True
    return False

# Find all header files that might have override issues
header_files = glob.glob("/Users/jerry/lpzrobot_mac/selforg/**/*.h", recursive=True)
header_files.extend(glob.glob("/Users/jerry/lpzrobot_mac/selforg/*.h"))

# Also check specific files mentioned in errors
specific_files = [
    "/Users/jerry/lpzrobot_mac/selforg/utils/storeable.h",
    "/Users/jerry/lpzrobot_mac/selforg/controller/abstractcontroller.h", 
    "/Users/jerry/lpzrobot_mac/selforg/utils/plotoptionengine.h"
]

all_files = list(set(header_files + specific_files))

fixed_count = 0
for file_path in all_files:
    if os.path.exists(file_path):
        if fix_override_in_file(file_path):
            fixed_count += 1

print(f"\nFixed {fixed_count} files with override issues")