#!/usr/bin/env python3
"""Fix override issues in controller files"""

import re
import os

def fix_override_issues(file_path):
    with open(file_path, 'r') as f:
        content = f.read()
    
    # Remove all instances of override used as statement terminator
    content = re.sub(r'([^;])\s+override;', r'\1;', content)
    
    # Fix for loops with override
    content = re.sub(r'for\s*\(([^)]+)\)\s*override\s*{', r'for (\1) {', content)
    
    # Fix "explicit if" -> "if"
    content = re.sub(r'explicit\s+if\s*\(', 'if (', content)
    
    # Fix "explicit switch" -> "switch"
    content = re.sub(r'explicit\s+switch\s*\(', 'switch (', content)
    
    # Fix "explicit for" -> "for"
    content = re.sub(r'explicit\s+for\s*\(', 'for (', content)
    
    # Fix static method with const
    content = re.sub(r'(static\s+\w+(?:\s*::\s*\w+)*\s+\w+\s*\([^)]*\))\s*const\s*{', r'\1 {', content)
    
    # Fix addParameter/addParameterDef with override
    content = re.sub(r'(addParameter(?:Def)?\([^)]+\))\s+override;', r'\1;', content)
    
    with open(file_path, 'w') as f:
        f.write(content)
    
    return file_path

# Files to fix based on the errors
files_to_fix = [
    "/Users/jerry/lpzrobot_mac/selforg/controller/derbigcontroller.cpp",
    "/Users/jerry/lpzrobot_mac/selforg/controller/dercontroller.cpp",
    "/Users/jerry/lpzrobot_mac/selforg/controller/dercontroller.h",
    "/Users/jerry/lpzrobot_mac/selforg/controller/derinf.h",
    "/Users/jerry/lpzrobot_mac/selforg/controller/multilayerffnn.h"
]

for file_path in files_to_fix:
    if os.path.exists(file_path):
        fixed = fix_override_issues(file_path)
        print(f"Fixed {fixed}")
    else:
        print(f"File not found: {file_path}")