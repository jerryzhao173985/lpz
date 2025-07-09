#!/usr/bin/env python3
"""Fix remaining C-style casts to use modern C++ casts"""

import re
import os

def fix_cstyle_casts(file_path):
    """Convert C-style casts to appropriate C++ casts"""
    
    with open(file_path, 'r', encoding='utf-8') as f:
        content = f.read()
    
    original = content
    
    # Patterns for C-style casts
    patterns = [
        # (int)expr -> static_cast<int>(expr)
        (r'\(int\)\s*([a-zA-Z_][\w.()>-]*(?:\[[^\]]+\])?)', r'static_cast<int>(\1)'),
        # (double)expr -> static_cast<double>(expr)
        (r'\(double\)\s*([a-zA-Z_][\w.()>-]*(?:\[[^\]]+\])?)', r'static_cast<double>(\1)'),
        # (float)expr -> static_cast<float>(expr)
        (r'\(float\)\s*([a-zA-Z_][\w.()>-]*(?:\[[^\]]+\])?)', r'static_cast<float>(\1)'),
        # (unsigned)expr -> static_cast<unsigned>(expr)
        (r'\(unsigned\)\s*([a-zA-Z_][\w.()>-]*(?:\[[^\]]+\])?)', r'static_cast<unsigned>(\1)'),
        # (size_t)expr -> static_cast<size_t>(expr)
        (r'\(size_t\)\s*([a-zA-Z_][\w.()>-]*(?:\[[^\]]+\])?)', r'static_cast<size_t>(\1)'),
        # (bool)expr -> static_cast<bool>(expr)
        (r'\(bool\)\s*([a-zA-Z_][\w.()>-]*(?:\[[^\]]+\])?)', r'static_cast<bool>(\1)'),
        # (char)expr -> static_cast<char>(expr)
        (r'\(char\)\s*([a-zA-Z_][\w.()>-]*(?:\[[^\]]+\])?)', r'static_cast<char>(\1)'),
        # (unsigned int)expr -> static_cast<unsigned int>(expr)
        (r'\(unsigned\s+int\)\s*([a-zA-Z_][\w.()>-]*(?:\[[^\]]+\])?)', r'static_cast<unsigned int>(\1)'),
        # (long)expr -> static_cast<long>(expr)
        (r'\(long\)\s*([a-zA-Z_][\w.()>-]*(?:\[[^\]]+\])?)', r'static_cast<long>(\1)'),
        # (void*)expr -> static_cast<void*>(expr) - be careful with this one
        (r'\(void\s*\*\)\s*([a-zA-Z_][\w.()>-]*)', r'static_cast<void*>(\1)'),
    ]
    
    for pattern, replacement in patterns:
        content = re.sub(pattern, replacement, content)
    
    # Special case: (Type*)expr usually needs reinterpret_cast or static_cast
    # Only for simple pointer casts, not function pointers
    content = re.sub(r'\((\w+)\s*\*\)\s*([a-zA-Z_][\w.()>-]*)', r'reinterpret_cast<\1*>(\2)', content)
    
    if content != original:
        with open(file_path, 'w', encoding='utf-8') as f:
            f.write(content)
        return True
    return False

def main():
    # Files identified with C-style casts
    target_files = [
        'selforg/controller/ahsox.cpp',
        'selforg/controller/casox.cpp',
        'selforg/controller/depcontroller.cpp',
        'selforg/controller/derbigcontroller.cpp',
        'selforg/controller/dercontroller.cpp',
        'selforg/controller/derpseudosensor.cpp',
        'selforg/controller/dinvert3channelcontroller.cpp',
        'selforg/controller/fourwheeled_controller.cpp',
        'selforg/controller/homeokinbase.cpp',
        'selforg/controller/invertcontroller.cpp',
        'selforg/controller/invertmotorbigmodel.cpp',
        'selforg/controller/invertmotornstep.cpp',
        'selforg/controller/invertmotorspace.cpp',
        'selforg/controller/multilayerffnn.cpp',
        'selforg/controller/onecontroller.cpp',
        'selforg/controller/pimax.cpp',
        'selforg/controller/replaycontroller.cpp',
        'selforg/controller/semox.cpp',
        'selforg/controller/sinecontroller.cpp',
        'selforg/controller/soml.cpp',
        'selforg/controller/sos.cpp',
        'selforg/controller/sox.cpp',
    ]
    
    fixed_count = 0
    
    print("Fixing C-style casts in controller files...")
    for file_path in target_files:
        if os.path.exists(file_path):
            if fix_cstyle_casts(file_path):
                print(f"Fixed: {file_path}")
                fixed_count += 1
    
    # Also check matrix operations
    matrix_files = [
        'selforg/matrix/matrix.cpp',
        'selforg/matrix/sparsematrix.cpp',
    ]
    
    for file_path in matrix_files:
        if os.path.exists(file_path):
            if fix_cstyle_casts(file_path):
                print(f"Fixed: {file_path}")
                fixed_count += 1
    
    print(f"\nTotal files fixed: {fixed_count}")

if __name__ == "__main__":
    main()