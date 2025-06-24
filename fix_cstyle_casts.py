#!/usr/bin/env python3
"""
Replace C-style casts with modern C++ casts.
This script converts (type)value to static_cast<type>(value) and similar.
"""

import os
import re
import sys

def fix_cstyle_casts_in_file(filepath):
    """Fix C-style casts in a single file."""
    try:
        with open(filepath, 'r') as f:
            content = f.read()
    except Exception as e:
        print(f"Error reading {filepath}: {e}")
        return 0
    
    original_content = content
    changes = 0
    
    # Common cast patterns to replace
    cast_patterns = [
        # (int)expr -> static_cast<int>(expr)
        (r'\(int\)\s*([a-zA-Z_]\w*(?:\[[^\]]+\])?(?:\.\w+)*(?:\([^)]*\))?)',
         r'static_cast<int>(\1)'),
        
        # (double)expr -> static_cast<double>(expr)
        (r'\(double\)\s*([a-zA-Z_]\w*(?:\[[^\]]+\])?(?:\.\w+)*(?:\([^)]*\))?)',
         r'static_cast<double>(\1)'),
        
        # (float)expr -> static_cast<float>(expr)
        (r'\(float\)\s*([a-zA-Z_]\w*(?:\[[^\]]+\])?(?:\.\w+)*(?:\([^)]*\))?)',
         r'static_cast<float>(\1)'),
        
        # (unsigned int)expr -> static_cast<unsigned int>(expr)
        (r'\(unsigned\s+int\)\s*([a-zA-Z_]\w*(?:\[[^\]]+\])?(?:\.\w+)*(?:\([^)]*\))?)',
         r'static_cast<unsigned int>(\1)'),
        
        # (size_t)expr -> static_cast<size_t>(expr)
        (r'\(size_t\)\s*([a-zA-Z_]\w*(?:\[[^\]]+\])?(?:\.\w+)*(?:\([^)]*\))?)',
         r'static_cast<size_t>(\1)'),
        
        # (bool)expr -> static_cast<bool>(expr)
        (r'\(bool\)\s*([a-zA-Z_]\w*(?:\[[^\]]+\])?(?:\.\w+)*(?:\([^)]*\))?)',
         r'static_cast<bool>(\1)'),
        
        # (char)expr -> static_cast<char>(expr)
        (r'\(char\)\s*([a-zA-Z_]\w*(?:\[[^\]]+\])?(?:\.\w+)*(?:\([^)]*\))?)',
         r'static_cast<char>(\1)'),
        
        # (unsigned char)expr -> static_cast<unsigned char>(expr)
        (r'\(unsigned\s+char\)\s*([a-zA-Z_]\w*(?:\[[^\]]+\])?(?:\.\w+)*(?:\([^)]*\))?)',
         r'static_cast<unsigned char>(\1)'),
        
        # (short)expr -> static_cast<short>(expr)
        (r'\(short\)\s*([a-zA-Z_]\w*(?:\[[^\]]+\])?(?:\.\w+)*(?:\([^)]*\))?)',
         r'static_cast<short>(\1)'),
        
        # (long)expr -> static_cast<long>(expr)
        (r'\(long\)\s*([a-zA-Z_]\w*(?:\[[^\]]+\])?(?:\.\w+)*(?:\([^)]*\))?)',
         r'static_cast<long>(\1)'),
        
        # (void*)expr -> static_cast<void*>(expr)
        (r'\(void\s*\*\)\s*([a-zA-Z_]\w*(?:\[[^\]]+\])?(?:\.\w+)*(?:\([^)]*\))?)',
         r'static_cast<void*>(\1)'),
        
        # (const char*)expr -> static_cast<const char*>(expr)
        (r'\(const\s+char\s*\*\)\s*([a-zA-Z_]\w*(?:\[[^\]]+\])?(?:\.\w+)*(?:\([^)]*\))?)',
         r'static_cast<const char*>(\1)'),
    ]
    
    # More complex pattern for pointer casts
    # (Type*)expr -> static_cast<Type*>(expr) or reinterpret_cast<Type*>(expr)
    pointer_pattern = re.compile(
        r'\(([a-zA-Z_]\w*(?:::[a-zA-Z_]\w*)*)\s*\*\)\s*([a-zA-Z_]\w*(?:\[[^\]]+\])?(?:\.\w+)*(?:\([^)]*\))?)'
    )
    
    # Apply simple cast patterns
    for pattern, replacement in cast_patterns:
        new_content = re.sub(pattern, replacement, content)
        if new_content != content:
            changes += len(re.findall(pattern, content))
            content = new_content
    
    # Handle pointer casts
    def replace_pointer_cast(match):
        nonlocal changes
        type_name = match.group(1)
        expr = match.group(2)
        changes += 1
        # Use static_cast for most pointer conversions
        # Could use reinterpret_cast for low-level conversions
        return f'static_cast<{type_name}*>({expr})'
    
    content = pointer_pattern.sub(replace_pointer_cast, content)
    
    # Special case: NULL casts
    null_cast_pattern = re.compile(r'\([^)]+\*\)\s*NULL\b')
    content = null_cast_pattern.sub('nullptr', content)
    
    # Count NULL replacements
    null_pattern = re.compile(r'\bNULL\b')
    null_replacements = len(null_pattern.findall(content))
    if null_replacements > 0:
        content = null_pattern.sub('nullptr', content)
        changes += null_replacements
    
    if content != original_content:
        try:
            with open(filepath, 'w') as f:
                f.write(content)
            print(f"Fixed {changes} C-style casts in {filepath}")
        except Exception as e:
            print(f"Error writing {filepath}: {e}")
            return 0
    
    return changes

def main():
    if len(sys.argv) > 1:
        # Process specific files
        total_changes = 0
        for filepath in sys.argv[1:]:
            if os.path.isfile(filepath) and (filepath.endswith('.h') or filepath.endswith('.cpp')):
                total_changes += fix_cstyle_casts_in_file(filepath)
        print(f"\nTotal C-style casts replaced: {total_changes}")
    else:
        # Process all .h and .cpp files in selforg and ode_robots
        total_changes = 0
        dirs_to_process = ['selforg', 'ode_robots']
        
        for dir_name in dirs_to_process:
            if os.path.exists(dir_name):
                for root, dirs, files in os.walk(dir_name):
                    # Skip .svn directories
                    if '.svn' in root:
                        continue
                    for file in files:
                        if file.endswith(('.h', '.cpp')):
                            filepath = os.path.join(root, file)
                            total_changes += fix_cstyle_casts_in_file(filepath)
        
        print(f"\nTotal C-style casts replaced: {total_changes}")

if __name__ == '__main__':
    main()