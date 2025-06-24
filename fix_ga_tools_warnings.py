#!/usr/bin/env python3
"""
Fix all warnings in ga_tools component
"""

import os
import re
import sys
from pathlib import Path

def fix_missing_override(content):
    """Add override specifier to virtual functions"""
    # Fix destructors
    content = re.sub(
        r'(virtual\s+~\w+\s*\([^)]*\)\s*)(;|{)',
        r'\1 override\2',
        content
    )
    
    # Fix virtual functions that should have override
    # This is more complex - we need to identify which functions override base class methods
    virtual_funcs = [
        ('select', 'Individual'),
        ('mutate', 'Gen'),
        ('getMutationProbability', 'double'),
        ('calcMutationFactor', 'double'),
        ('getRandomValue', 'IValue'),
        ('operator*', 'IValue'),
        ('operator+', 'IValue'),
        ('store', 'bool'),
        ('restore', 'bool'),
    ]
    
    for func_name, return_type in virtual_funcs:
        # Match virtual function declarations
        pattern = rf'(virtual\s+{return_type}[*&]?\s+{func_name}\s*\([^)]*\)\s*(?:const\s*)?)(;|{{)'
        content = re.sub(pattern, r'\1 override\2', content)
    
    return content

def fix_explicit_constructors(content):
    """Add explicit to single-argument constructors"""
    # Find single-argument constructors
    content = re.sub(
        r'^(\s*)([A-Z]\w+)\s*\(\s*(?:const\s+)?(?!void)([^,)]+)\s*\)\s*;',
        r'\1explicit \2(\3);',
        content,
        flags=re.MULTILINE
    )
    return content

def fix_const_correctness(content):
    """Fix const correctness issues"""
    # Fix const pointer/reference parameters
    replacements = [
        # Function parameters that should be const
        (r'(\w+\*)\s+(\w+)\s*\)', r'const \1 \2)'),
        (r'std::string\s+(\w+)\)', r'const std::string& \1)'),
    ]
    
    for pattern, replacement in replacements:
        content = re.sub(pattern, replacement, content)
    
    return content

def fix_variable_scope(content):
    """Fix variable scope issues"""
    # This is complex and requires understanding the code flow
    # For now, we'll mark them with comments for manual review
    scope_vars = ['r1', 'r2', 'f1', 'f2', 'tValue']
    
    for var in scope_vars:
        pattern = rf'(\n\s*(?:int|double|float|auto)\s+{var}\s*[=;])'
        content = re.sub(
            pattern,
            r'// TODO: Review scope of variable\1',
            content
        )
    
    return content

def fix_postfix_operators(content):
    """Replace postfix ++ with prefix ++ for iterators"""
    # Fix iterator++
    content = re.sub(
        r'(\b(?:it|iter|iterator)\w*)\+\+(?!\s*[;\)])',
        r'++\1',
        content
    )
    
    # Fix cases like: for(...; ...; it++)
    content = re.sub(
        r';\s*(\w+(?:it|iter|iterator)\w*)\+\+\s*\)',
        r'; ++\1)',
        content
    )
    
    return content

def fix_unused_variables(content):
    """Comment out or fix unused variables"""
    # Add [[maybe_unused]] attribute for parameters that might be unused
    content = re.sub(
        r'(virtual\s+\w+[*&]?\s+\w+\s*\()([^)]+)\)',
        lambda m: m.group(1) + add_maybe_unused_to_params(m.group(2)) + ')',
        content
    )
    
    return content

def add_maybe_unused_to_params(params):
    """Add [[maybe_unused]] to parameters that might be unused"""
    # Split parameters and add attribute to each
    param_list = params.split(',')
    new_params = []
    
    for param in param_list:
        param = param.strip()
        if param and not param.startswith('[['):
            # Add maybe_unused attribute
            param = '[[maybe_unused]] ' + param
        new_params.append(param)
    
    return ', '.join(new_params)

def process_file(filepath):
    """Process a single file to fix warnings"""
    try:
        with open(filepath, 'r', encoding='utf-8') as f:
            content = f.read()
        
        original_content = content
        
        # Apply fixes
        content = fix_missing_override(content)
        content = fix_explicit_constructors(content)
        content = fix_const_correctness(content)
        content = fix_variable_scope(content)
        content = fix_postfix_operators(content)
        # content = fix_unused_variables(content)  # This might be too aggressive
        
        # Only write if changes were made
        if content != original_content:
            with open(filepath, 'w', encoding='utf-8') as f:
                f.write(content)
            print(f"Fixed: {filepath}")
            return True
        
        return False
    except Exception as e:
        print(f"Error processing {filepath}: {e}")
        return False

def main():
    # Find all header and source files in ga_tools
    ga_tools_dir = Path('ga_tools')
    files_fixed = 0
    
    for ext in ['*.h', '*.cpp']:
        for filepath in ga_tools_dir.rglob(ext):
            # Skip corrupted files
            if '.!' in str(filepath):
                continue
            
            if process_file(filepath):
                files_fixed += 1
    
    print(f"\nTotal files fixed: {files_fixed}")

if __name__ == '__main__':
    main()