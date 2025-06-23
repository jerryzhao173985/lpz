#!/usr/bin/env python3
"""Apply C++17 modernization features to the codebase."""

import re
import os

def modernize_cpp17(content):
    """Apply various C++17 modernizations."""
    changes = []
    modified = content
    
    # 1. Replace NULL with nullptr
    null_pattern = r'\bNULL\b'
    null_matches = list(re.finditer(null_pattern, modified))
    for match in reversed(null_matches):
        modified = modified[:match.start()] + 'nullptr' + modified[match.end():]
        changes.append(('NULL → nullptr', match.start()))
    
    # 2. Use auto for obvious iterator declarations
    # for(vector<Type>::iterator it = container.begin(); it != container.end(); ++it)
    iterator_pattern = r'for\s*\(\s*(?:std::)?(?:vector|list|map|set)<[^>]+>::(?:const_)?iterator\s+(\w+)\s*=\s*(\w+)\.begin\(\)'
    iterator_matches = list(re.finditer(iterator_pattern, modified))
    for match in reversed(iterator_matches):
        var_name = match.group(1)
        container = match.group(2)
        new_text = f'for(auto {var_name} = {container}.begin()'
        modified = modified[:match.start()] + new_text + modified[match.end():]
        changes.append(('iterator → auto', match.start()))
    
    # 3. Replace typedef with using (simple cases)
    typedef_pattern = r'typedef\s+([^;]+)\s+(\w+);'
    typedef_matches = list(re.finditer(typedef_pattern, modified))
    for match in reversed(typedef_matches):
        type_spec = match.group(1).strip()
        alias = match.group(2)
        # Skip function pointers for now
        if '(' not in type_spec:
            new_text = f'using {alias} = {type_spec};'
            modified = modified[:match.start()] + new_text + modified[match.end():]
            changes.append(('typedef → using', match.start()))
    
    # 4. Use range-based for loops where appropriate (conservative)
    # for(int i=0; i<container.size(); i++) { ... container[i] ... }
    # This is too complex to do safely with regex, skip for now
    
    # 5. Replace 0 with nullptr for pointer initialization (very conservative)
    pointer_init_pattern = r'(\w+\s*\*\s*\w+)\s*=\s*0\s*;'
    pointer_matches = list(re.finditer(pointer_init_pattern, modified))
    for match in reversed(pointer_matches):
        declaration = match.group(1)
        new_text = f'{declaration} = nullptr;'
        modified = modified[:match.start()] + new_text + modified[match.end():]
        changes.append(('0 → nullptr', match.start()))
    
    return modified, changes

def process_file(filepath):
    """Process a single file for C++17 modernization."""
    if not os.path.exists(filepath):
        return False
    
    # Skip certain files/directories
    skip_patterns = ['configurator/', 'tests/', 'examples/', '.bak']
    if any(pattern in filepath for pattern in skip_patterns):
        return False
    
    # Create backup
    backup_path = filepath + '.bak.cpp17'
    if not os.path.exists(backup_path):
        os.system(f'cp "{filepath}" "{backup_path}"')
    
    with open(filepath, 'r', encoding='utf-8', errors='ignore') as f:
        content = f.read()
    
    modified_content, changes = modernize_cpp17(content)
    
    if changes:
        print(f"\nProcessing {filepath}:")
        # Count changes by type
        change_types = {}
        for change_type, _ in changes:
            change_types[change_type] = change_types.get(change_type, 0) + 1
        
        for change_type, count in change_types.items():
            print(f"  {change_type}: {count} occurrences")
        
        with open(filepath, 'w', encoding='utf-8') as f:
            f.write(modified_content)
        return True
    
    return False

def main():
    # Start with controller files
    base_dir = '/Users/jerry/lpzrobot_mac/selforg/'
    
    test_files = [
        'controller/abstractcontroller.cpp',
        'controller/abstractcontroller.h',
        'controller/abstractiafcontroller.cpp',
        'controller/dercontroller.cpp',
        'controller/dercontroller.h',
        'controller/invertmotorspace.cpp',
        'controller/invertmotorspace.h',
        'matrix/matrix.cpp',
        'matrix/matrix.h'
    ]
    
    modified_count = 0
    for filepath in test_files:
        full_path = os.path.join(base_dir, filepath)
        if process_file(full_path):
            modified_count += 1
    
    print(f"\nModified {modified_count} files")
    
    # Summary
    print("\nC++17 modernization features applied:")
    print("- NULL → nullptr")
    print("- typedef → using (for simple type aliases)")
    print("- Iterator declarations → auto")
    print("- Pointer initialization: 0 → nullptr")

if __name__ == '__main__':
    main()