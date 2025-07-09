#!/usr/bin/env python3
"""Extract remaining valid C++ modernization fixes from big.patch"""

import re
import sys
from collections import defaultdict

def extract_fixes_from_patch(patch_file):
    """Extract specific patterns of fixes that still need to be applied"""
    
    # Categories of fixes to extract
    fixes = {
        'explicit_misplaced': [],
        'override_corrupted': [],
        'nullptr_numeric': [],
        'static_cast_string': [],
        'virtual_destructor_override': []
    }
    
    # Patterns to match and fix
    patterns = {
        # Misplaced explicit keywords
        'explicit_if': (r'explicit\s+if\s*\(', r'if('),
        'explicit_switch': (r'explicit\s+switch\s*\(', r'switch('),
        'explicit_for': (r'explicit\s+for\s*\(', r'for('),
        'explicit_while': (r'explicit\s+while\s*\(', r'while('),
        'explicit_return': (r'explicit\s+return\b', r'return'),
        
        # Corrupted override keywords
        'semicolon_override': (r';\s*override\s*;', r';'),
        'paren_override_semicolon': (r'\)\s*override\s*;(?!\s*\{)', r');'),
        'override_brace': (r'override\s*\{', r'{'),
        'inside_call_override': (r'([,\)])\s*override\s*([,;])', r'\1\2'),
        
        # Virtual destructor with override
        'virtual_destructor': (r'virtual\s+~(\w+)\s*\(\s*\)\s*override\s*;', r'virtual ~\1();'),
        
        # nullptr in numeric contexts
        'for_nullptr': (r'for\s*\([^=]+=\s*nullptr\s*;', None),  # Need context
        'nullptr_numeric': (r'([!=<>]+)\s*nullptr(?=\s*[;,\)])', None),  # Need context
        
        # Unnecessary static_cast on strings
        'static_cast_string': (r'static_cast<char\s*\*>\s*string\s*\("([^"]*)"\)', r'"\1"'),
        'static_cast_literal': (r'static_cast<const\s+char\s*\*>\s*\("([^"]*)"\)', r'"\1"'),
    }
    
    current_file = None
    in_diff = False
    context_lines = []
    line_num = 0
    
    # Skip patterns - files/dirs to ignore
    skip_patterns = [
        r'\.cache/clangd/',
        r'dist/',
        r'backup_',
        r'\.github/',
        r'CLAUDE\.md',
        r'README\.md',
        r'MODERNIZATION.*\.md',
        r'\.gitignore',
        r'\.clang-',
        r'patch/',
    ]
    
    with open(patch_file, 'r', encoding='utf-8', errors='ignore') as f:
        for line in f:
            line_num += 1
            
            # Track current file
            if line.startswith('diff --git'):
                match = re.search(r'b/(.+?)(?:\s|$)', line)
                if match:
                    current_file = match.group(1)
                    # Check if we should skip this file
                    if any(re.search(pattern, current_file) for pattern in skip_patterns):
                        current_file = None
                        in_diff = False
                        continue
                    in_diff = True
                    context_lines = []
            
            # Skip if not in a relevant file
            if not current_file or not in_diff:
                continue
            
            # Only process source files
            if not (current_file.endswith('.cpp') or current_file.endswith('.h') or 
                    current_file.endswith('.hpp') or current_file.endswith('.cc')):
                continue
            
            # Look for fixes in added lines
            if line.startswith('+') and not line.startswith('+++'):
                content = line[1:]
                
                # Check each pattern
                for fix_type, (pattern, replacement) in patterns.items():
                    if re.search(pattern, content):
                        # Store the fix with context
                        fix_entry = {
                            'file': current_file,
                            'line_num': line_num,
                            'original': content.rstrip(),
                            'pattern': pattern,
                            'fix_type': fix_type
                        }
                        
                        # Categorize the fix
                        if 'explicit' in fix_type:
                            fixes['explicit_misplaced'].append(fix_entry)
                        elif 'override' in fix_type or 'destructor' in fix_type:
                            fixes['override_corrupted'].append(fix_entry)
                        elif 'nullptr' in fix_type:
                            fixes['nullptr_numeric'].append(fix_entry)
                        elif 'static_cast' in fix_type:
                            fixes['static_cast_string'].append(fix_entry)
                        
                        break
    
    return fixes

def create_fix_patches(fixes):
    """Create separate patch files for each category of fixes"""
    
    # Create patches by category
    for category, fix_list in fixes.items():
        if not fix_list:
            continue
            
        patch_name = f"patch/extracted/patch_remaining_{category}.patch"
        print(f"\nCreating {patch_name} with {len(fix_list)} fixes...")
        
        # Group fixes by file
        by_file = defaultdict(list)
        for fix in fix_list:
            by_file[fix['file']].append(fix)
        
        with open(patch_name, 'w') as f:
            f.write(f"# Extracted {category} fixes\n")
            f.write(f"# Total fixes: {len(fix_list)}\n")
            f.write(f"# Files affected: {len(by_file)}\n\n")
            
            for file, file_fixes in sorted(by_file.items()):
                f.write(f"\n# File: {file} ({len(file_fixes)} fixes)\n")
                for fix in file_fixes:
                    f.write(f"# Line {fix['line_num']}: {fix['fix_type']}\n")
                    f.write(f"# Pattern: {fix['pattern']}\n")
                    f.write(f"# {fix['original']}\n")

def main():
    print("Extracting remaining fixes from big.patch...")
    
    fixes = extract_fixes_from_patch('patch/big.patch')
    
    # Print summary
    total_fixes = sum(len(fix_list) for fix_list in fixes.values())
    print(f"\nTotal remaining fixes found: {total_fixes}")
    
    for category, fix_list in fixes.items():
        if fix_list:
            print(f"  {category}: {len(fix_list)} fixes")
            # Show sample files
            files = list(set(fix['file'] for fix in fix_list[:10]))
            print(f"    Sample files: {', '.join(files[:5])}")
    
    # Create the patch files
    create_fix_patches(fixes)
    
    print("\nExtraction complete!")

if __name__ == "__main__":
    main()