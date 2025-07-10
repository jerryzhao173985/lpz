#!/usr/bin/env python3
import os
import re

# Fix the specific file with override issue
filepath = '/Users/jerry/simulator/lpz/selforg/controller/invertmotorbigmodel.h'

try:
    with open(filepath, 'r') as f:
        content = f.read()
except:
    print(f"Could not read {filepath}")
    exit(1)
    
original = content

# Remove override from calcCandHUpdates method
content = re.sub(
    r'(virtual\s+void\s+calcCandHUpdates\s*\([^)]*\)\s*)override(\s*;)',
    r'\1\2',
    content
)

if content != original:
    with open(filepath, 'w') as f:
        f.write(content)
    print(f"Fixed override issue in {os.path.basename(filepath)}")
else:
    print("No changes needed")