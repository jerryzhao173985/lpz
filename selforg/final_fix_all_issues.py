#!/usr/bin/env python3
"""Final comprehensive fix for all remaining issues"""

import re
import os
import glob

def fix_all_issues_in_file(file_path):
    """Fix all remaining issues in a single file"""
    if not os.path.exists(file_path):
        print(f"File not found: {file_path}")
        return False
        
    with open(file_path, 'r') as f:
        content = f.read()
    
    original_content = content
    
    # Fix duplicate const declarations
    content = re.sub(r'const\s+const\s+', 'const ', content)
    content = re.sub(r'(\)\s*const)\s+const\s+', r'\1 ', content)
    
    # Fix override on methods that don't override anything 
    # For base classes that don't inherit from anything or have limited inheritance
    if 'WiredController' in file_path or 'Agent' in file_path:
        # These classes have specific override issues
        content = re.sub(r'(virtual\s+[^;{]+getMotorBabbler[^;{]+)\s+override', r'\1', content)
        content = re.sub(r'(virtual\s+[^;{]+stopMotorBabblingMode[^;{]+)\s+override', r'\1', content)
        content = re.sub(r'(virtual\s+[^;{]+getMotorBabblingMode[^;{]+)\s+override', r'\1', content)
        content = re.sub(r'(virtual\s+[^;{]+getController[^;{]+)\s+override', r'\1', content)
        content = re.sub(r'(virtual\s+[^;{]+getWiring[^;{]+)\s+override', r'\1', content)
        content = re.sub(r'(virtual\s+[^;{]+getRobot[^;{]+)\s+override', r'\1', content)
        content = re.sub(r'(virtual\s+[^;{]+getTrackOptions[^;{]+)\s+override', r'\1', content)
    
    # Fix any syntax issues with "(double)override" -> "(double)"
    content = re.sub(r'\(double\)\s*override', '(double)', content)
    content = re.sub(r'static_cast<([^>]+)>\(override\)', r'static_cast<\1>', content)
    
    # Remove duplicate method declarations
    # Match pattern like: virtual const const AbstractController* getController() const const
    content = re.sub(r'virtual\s+const\s+const\s+(\w+\*?\s+\w+\([^)]*\)\s*const)\s+const[^{;]*{[^}]*}', '', content)
    
    if content != original_content:
        with open(file_path, 'w') as f:
            f.write(content)
        print(f"Fixed: {file_path}")
        return True
    return False

# Find all header files that might need fixes
header_files = glob.glob("/Users/jerry/lpzrobot_mac/selforg/**/*.h", recursive=True)
header_files.extend(glob.glob("/Users/jerry/lpzrobot_mac/selforg/*.h"))

# Also check specific files mentioned in errors
specific_files = [
    "/Users/jerry/lpzrobot_mac/selforg/wiredcontroller.h",
    "/Users/jerry/lpzrobot_mac/selforg/agent.h",
    "/Users/jerry/lpzrobot_mac/selforg/controller/feedforwardnn.h"
]

all_files = list(set(header_files + specific_files))

fixed_count = 0
for file_path in all_files:
    if os.path.exists(file_path):
        if fix_all_issues_in_file(file_path):
            fixed_count += 1

print(f"\nFixed {fixed_count} files")