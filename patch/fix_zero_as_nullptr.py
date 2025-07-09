#!/usr/bin/env python3
"""Fix zero-as-null-pointer-constant warnings by replacing 0 with nullptr in pointer contexts"""

import re
import os

def fix_zero_as_nullptr(file_path):
    """Fix 0 used as null pointer constant"""
    
    with open(file_path, 'r', encoding='utf-8') as f:
        content = f.read()
    
    original = content
    
    # Common patterns where 0 should be nullptr
    patterns = [
        # Constructor initializer lists: : member(0)
        (r'(:\s*\w+\s*\()\s*0\s*\)', r'\1nullptr)'),
        # Assignment to pointer: ptr = 0;
        (r'(\w+\s*=\s*)0(\s*;)', r'\1nullptr\2'),
        # Comparison with pointer: ptr != 0
        (r'(\w+\s*[!=]=\s*)0(\s*[;&)])', r'\1nullptr\2'),
        # Function default parameters: func(Type* param = 0)
        (r'(\*\s*\w+\s*=\s*)0(\s*[,)])', r'\1nullptr\2'),
        # Return statements: return 0; (in pointer functions)
        (r'(return\s+)0(\s*;.*//.*pointer)', r'\1nullptr\2'),
        # data(0) in constructors
        (r'(\bdata\s*\()\s*0\s*\)', r'\1nullptr)'),
        # this->member = 0
        (r'(this->\w+\s*=\s*)0(\s*;)', r'\1nullptr\2'),
        # Array initialization: Type* arr = 0
        (r'(\*\s*\w+\s*=\s*)0(\s*;)', r'\1nullptr\2'),
    ]
    
    for pattern, replacement in patterns:
        content = re.sub(pattern, replacement, content, flags=re.MULTILINE)
    
    if content != original:
        with open(file_path, 'w', encoding='utf-8') as f:
            f.write(content)
        return True
    return False

def main():
    # Priority files from CI warnings
    priority_files = [
        'selforg/matrix/matrix.h',
        'selforg/utils/noisegenerator.h',
        'selforg/abstractwiring.h',
        'selforg/wiredcontroller.h',
        'selforg/trackrobots.h',
    ]
    
    # Additional files to check
    additional_patterns = [
        'selforg/**/*.h',
        'selforg/**/*.cpp',
        'ode_robots/**/*.h',
        'ode_robots/**/*.cpp',
    ]
    
    fixed_count = 0
    
    # Fix priority files first
    print("Fixing priority files from CI warnings...")
    for file_path in priority_files:
        if os.path.exists(file_path):
            if fix_zero_as_nullptr(file_path):
                print(f"Fixed: {file_path}")
                fixed_count += 1
        else:
            # Try with include/ prefix
            alt_path = f"include/{file_path}"
            if os.path.exists(alt_path):
                if fix_zero_as_nullptr(alt_path):
                    print(f"Fixed: {alt_path}")
                    fixed_count += 1
    
    print(f"\nTotal files fixed: {fixed_count}")

if __name__ == "__main__":
    main()