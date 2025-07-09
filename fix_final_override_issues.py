#!/usr/bin/env python3
import os
import re

# Specific files and methods that still have override issues from CI
fixes_needed = [
    ('/Users/jerry/simulator/lpz/selforg/controller/controllernet.h', 'response'),
]

# Also fix remaining = 0 to nullptr for pointer parameters
def fix_file(filepath):
    try:
        with open(filepath, 'r') as f:
            content = f.read()
    except:
        return False
        
    original = content
    
    # Fix specific override issues
    content = re.sub(r'(virtual\s+const\s+matrix::Matrix&\s+response\(\)\s*const\s*)override(\s*;)', r'\1\2', content)
    
    # Fix = 0 to = nullptr in function parameters
    content = re.sub(r'(\w+\*\s+\w+\s*=\s*)0(\s*[,)])', r'\1nullptr\2', content)
    
    if content != original:
        with open(filepath, 'w') as f:
            f.write(content)
        return True
    return False

# Fix specific files
for filepath, method in fixes_needed:
    if os.path.exists(filepath):
        if fix_file(filepath):
            print(f"Fixed {os.path.basename(filepath)}")

# Also search for any remaining = 0 that should be nullptr
import glob
for file in glob.glob('/Users/jerry/simulator/lpz/**/*.h', recursive=True):
    if fix_file(file):
        print(f"Fixed pointer parameters in {os.path.basename(file)}")