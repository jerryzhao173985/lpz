#!/usr/bin/env python3
"""Safely modernize C-style casts to C++ casts with better filtering."""

import re
import os
import sys

def is_valid_cast_context(content, match_start, match_end):
    """Check if the match is in a valid context for cast replacement."""
    # Skip if in a comment
    line_start = content.rfind('\n', 0, match_start) + 1
    line = content[line_start:content.find('\n', match_end)]
    if '//' in line[:match_start - line_start] or '/*' in content[max(0, match_start-50):match_start]:
        return False
    
    # Skip copyright notices
    if 'Copyright' in content[max(0, match_start-100):match_start+100]:
        return False
        
    # Skip if followed by delete/new/other keywords
    following = content[match_end:match_end+20].strip()
    if following.startswith(('delete', 'new', '2005', '2011', 'cannot', 'std::')):
        return False
    
    return True

def replace_c_style_casts(content):
    """Replace C-style casts with appropriate C++ casts."""
    changes = []
    modified = content
    
    # More specific patterns
    patterns = [
        # Casts to primitive types from variables
        (r'\b(?<![(])\((?:unsigned\s+)?(int|char|short|long|float|double|bool|size_t)\)\s*([a-zA-Z_]\w*(?:\[[^\]]+\])?(?:\.[a-zA-Z_]\w*)*)\b(?!\s*[({])', 
         r'static_cast<\1>(\2)'),
        
        # Casts to unsigned types
        (r'\b(?<![(])\((unsigned\s+(?:int|char|short|long))\)\s*([a-zA-Z_]\w*)\b(?!\s*[({])', 
         r'static_cast<\1>(\2)'),
        
        # Pointer casts (but not function pointers)
        (r'\b(?<![(])\((\w+)\s*\*\)\s*([a-zA-Z_]\w*)\b(?!\s*[({])', 
         r'static_cast<\1*>(\2)'),
        
        # Const pointer casts
        (r'\b(?<![(])\(const\s+(\w+)\s*\*\)\s*([a-zA-Z_]\w*)\b(?!\s*[({])', 
         r'static_cast<const \1*>(\2)'),
        
        # void* casts
        (r'\b(?<![(])\(void\s*\*\)\s*([a-zA-Z_]\w*)\b(?!\s*[({])', 
         r'static_cast<void*>(\1)'),
        
        # Numeric literal casts (but not years)
        (r'\b(?<![(])\((int|double|float)\)\s*(\d+(?:\.\d+)?)\b(?![)\d])', 
         r'static_cast<\1>(\2)'),
    ]
    
    for pattern, replacement in patterns:
        matches = list(re.finditer(pattern, modified))
        for match in reversed(matches):
            if is_valid_cast_context(content, match.start(), match.end()):
                old_text = match.group(0)
                new_text = re.sub(pattern, replacement, old_text)
                if old_text != new_text:
                    changes.append((old_text, new_text, match.start()))
                    modified = modified[:match.start()] + new_text + modified[match.end():]
    
    return modified, changes

def process_file(filepath):
    """Process a single file to modernize casts."""
    if not os.path.exists(filepath):
        print(f"File not found: {filepath}")
        return False
    
    # Skip certain files
    skip_patterns = ['configurator/', 'tests/', 'examples/']
    if any(pattern in filepath for pattern in skip_patterns):
        return False
    
    # Create backup
    backup_path = filepath + '.bak.casts2'
    if not os.path.exists(backup_path):
        os.system(f'cp "{filepath}" "{backup_path}"')
    
    with open(filepath, 'r', encoding='utf-8', errors='ignore') as f:
        content = f.read()
    
    modified_content, changes = replace_c_style_casts(content)
    
    if changes:
        print(f"\nProcessing {filepath}:")
        # Group changes by type
        for old, new, pos in changes[:5]:
            # Find line number
            line_num = content[:pos].count('\n') + 1
            print(f"  Line {line_num}: {old} → {new}")
        if len(changes) > 5:
            print(f"  ... and {len(changes) - 5} more changes")
        
        with open(filepath, 'w', encoding='utf-8') as f:
            f.write(modified_content)
        return True
    
    return False

def main():
    # Start with specific files known to have casts
    test_files = [
        'controller/dercontroller.cpp',
        'controller/invertmotorspace.cpp',
        'controller/multireinforce.cpp',
        'matrix/matrix.cpp',
        'utils/statistictools.cpp'
    ]
    
    base_dir = '/Users/jerry/lpzrobot_mac/selforg/'
    modified_count = 0
    
    for filepath in test_files:
        full_path = os.path.join(base_dir, filepath)
        if os.path.exists(full_path):
            if process_file(full_path):
                modified_count += 1
    
    print(f"\nModified {modified_count} files")
    
    if len(sys.argv) > 1 and sys.argv[1] == '--all':
        print("\nProcessing all C++ files...")
        total_changes = 0
        for root, dirs, files in os.walk(base_dir):
            # Skip certain directories
            dirs[:] = [d for d in dirs if d not in ['configurator', 'tests', 'examples']]
            
            for file in files:
                if file.endswith(('.cpp', '.h')):
                    full_path = os.path.join(root, file)
                    if process_file(full_path):
                        modified_count += 1
                        total_changes += 1
        
        print(f"\nTotal files modified: {modified_count}")
        print(f"Total changes made: {total_changes}")

if __name__ == '__main__':
    main()