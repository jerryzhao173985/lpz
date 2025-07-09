#!/usr/bin/env python3
"""Fix incorrect nullptr usage in numeric contexts introduced by previous script"""

import re
import os

def fix_nullptr_numeric_errors(file_path):
    """Fix nullptr incorrectly used in numeric variable assignments"""
    
    with open(file_path, 'r', encoding='utf-8') as f:
        content = f.read()
    
    original = content
    
    # Fix patterns where nullptr was incorrectly used for numeric types
    patterns = [
        # dimension = nullptr -> dimension = 0
        (r'(\bdimension\s*=\s*)nullptr(\s*;)', r'\g<1>0\g<2>'),
        # tau = nullptr -> tau = 0
        (r'(\btau\s*=\s*)nullptr(\s*;)', r'\g<1>0\g<2>'),
        # sqrttau = nullptr -> sqrttau = 0
        (r'(\bsqrttau\s*=\s*)nullptr(\s*;)', r'\g<1>0\g<2>'),
        # mean1channel = nullptr -> mean1channel = 0
        (r'(\bmean1channel\s*=\s*)nullptr(\s*;)', r'\g<1>0\g<2>'),
        # factor = nullptr -> factor = 0
        (r'(\bfactor\s*=\s*)nullptr(\s*;)', r'\g<1>0\g<2>'),
        # omega = nullptr -> omega = 0
        (r'(\bomega\s*=\s*)nullptr(\s*;)', r'\g<1>0\g<2>'),
        # amplitude = nullptr -> amplitude = 0
        (r'(\bamplitude\s*=\s*)nullptr(\s*;)', r'\g<1>0\g<2>'),
        # channels = nullptr -> channels = 0
        (r'(\bchannels\s*=\s*)nullptr(\s*;)', r'\g<1>0\g<2>'),
        # phaseShift = nullptr -> phaseShift = 0
        (r'(\bphaseShift\s*=\s*)nullptr(\s*;)', r'\g<1>0\g<2>'),
        # t = nullptr -> t = 0
        (r'(\bt\s*=\s*)nullptr(\s*;)', r'\g<1>0\g<2>'),
        
        # Fix initializer list assignments
        (r'(double\s+\w+\s*=\s*)nullptr(\s*;)', r'\g<1>0\g<2>'),
        (r'(unsigned\s+int\s+\w+\s*=\s*)nullptr(\s*;)', r'\g<1>0\g<2>'),
        (r'(int\s+\w+\s*=\s*)nullptr(\s*;)', r'\g<1>0\g<2>'),
        
        # Fix for loops: for(type i = nullptr -> for(type i = 0
        (r'for\s*\(\s*(unsigned\s+int|int|size_t)\s+(\w+)\s*=\s*nullptr\s*;', r'for (\1 \2 = 0;'),
    ]
    
    for pattern, replacement in patterns:
        content = re.sub(pattern, replacement, content)
    
    if content != original:
        with open(file_path, 'w', encoding='utf-8') as f:
            f.write(content)
        return True
    return False

def main():
    # Files that need fixing
    files_to_fix = [
        'selforg/utils/noisegenerator.h',
        'selforg/controller/abstractcontroller.h',
        'include/selforg/noisegenerator.h',
        'include/selforg/abstractcontroller.h',
    ]
    
    # Also check for more files with the issue
    for root, dirs, files in os.walk('.'):
        # Skip certain directories
        skip_dirs = ['.git', '.cache', 'build', 'dist', 'backup_']
        dirs[:] = [d for d in dirs if not any(skip in d for skip in skip_dirs)]
        
        for file in files:
            if file.endswith(('.h', '.cpp')):
                file_path = os.path.join(root, file)
                # Quick check if file contains the problematic pattern
                try:
                    with open(file_path, 'r', encoding='utf-8') as f:
                        content = f.read()
                        if 'dimension = nullptr' in content or '= nullptr;' in content:
                            if file_path not in files_to_fix:
                                files_to_fix.append(file_path)
                except:
                    pass
    
    fixed_count = 0
    
    print("Fixing nullptr numeric errors...")
    for file_path in files_to_fix:
        if os.path.exists(file_path):
            if fix_nullptr_numeric_errors(file_path):
                print(f"Fixed: {file_path}")
                fixed_count += 1
    
    print(f"\nTotal files fixed: {fixed_count}")

if __name__ == "__main__":
    main()