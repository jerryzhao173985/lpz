#!/usr/bin/env python3
"""Apply comprehensive C++17 modernization patterns."""

import re
import os
import subprocess

def modernize_comprehensive(filepath):
    """Apply comprehensive C++17 modernization patterns."""
    if not os.path.exists(filepath):
        return False
    
    # Skip backup files
    if '.bak' in filepath:
        return False
    
    # Create backup
    backup_path = filepath + '.bak.modern2'
    if not os.path.exists(backup_path):
        subprocess.run(['cp', filepath, backup_path], check=True)
    
    with open(filepath, 'r', encoding='utf-8', errors='ignore') as f:
        content = f.read()
    
    changes = []
    modified = content
    
    # 1. Replace remaining push_back with emplace_back for constructor calls
    emplace_pattern = r'(\w+)\.push_back\((\w+)\((.*?)\)\)'
    emplace_matches = list(re.finditer(emplace_pattern, modified))
    for match in reversed(emplace_matches):
        container = match.group(1)
        constructor = match.group(2)
        args = match.group(3)
        new_text = f'{container}.emplace_back({args})'
        modified = modified[:match.start()] + new_text + modified[match.end():]
        changes.append(('push_back → emplace_back', match.start()))
    
    # 2. Replace typedef with using (C++11 style)
    typedef_pattern = r'typedef\s+([^;]+?)\s+(\w+);'
    typedef_matches = list(re.finditer(typedef_pattern, modified))
    for match in reversed(typedef_matches):
        type_spec = match.group(1).strip()
        alias_name = match.group(2)
        new_text = f'using {alias_name} = {type_spec};'
        modified = modified[:match.start()] + new_text + modified[match.end():]
        changes.append(('typedef → using', match.start()))
    
    # 3. Replace iterator loops with range-based for where possible
    # This is complex and risky, so we'll skip it for now
    
    # 4. Replace NULL with nullptr (if any still remaining)
    null_pattern = r'\bNULL\b'
    null_matches = list(re.finditer(null_pattern, modified))
    for match in reversed(null_matches):
        modified = modified[:match.start()] + 'nullptr' + modified[match.end():]
        changes.append(('NULL → nullptr', match.start()))
    
    # 5. Add final keyword to classes that shouldn't be inherited from
    # This requires semantic understanding, skip for automatic conversion
    
    # 6. Replace 0 with nullptr for pointer initialization
    ptr_init_pattern = r'(\*\s*\w+\s*=\s*)0\b(?!\d)'
    ptr_init_matches = list(re.finditer(ptr_init_pattern, modified))
    for match in reversed(ptr_init_matches):
        modified = modified[:match.end()-1] + 'nullptr' + modified[match.end():]
        changes.append(('0 → nullptr (init)', match.start()))
    
    # 7. Use auto for obvious type deductions (conservative approach)
    # Only for iterators as they're verbose and safe to replace
    auto_iter_pattern = r'(std::\w+<[^>]+>::(?:const_)?iterator)\s+(\w+)\s*='
    auto_iter_matches = list(re.finditer(auto_iter_pattern, modified))
    for match in reversed(auto_iter_matches):
        var_name = match.group(2)
        new_text = f'auto {var_name} ='
        modified = modified[:match.start()] + new_text + modified[match.end():]
        changes.append(('iterator → auto', match.start()))
    
    if changes:
        # Count changes by type
        change_types = {}
        for change_type, _ in changes:
            change_types[change_type] = change_types.get(change_type, 0) + 1
        
        for change_type, count in change_types.items():
            print(f"  {change_type}: {count} occurrences")
        
        with open(filepath, 'w', encoding='utf-8') as f:
            f.write(modified)
        return True
    
    return False

def main():
    # Focus on all selforg directories
    target_dirs = [
        '/Users/jerry/lpzrobot_mac/selforg/controller',
        '/Users/jerry/lpzrobot_mac/selforg/utils',
        '/Users/jerry/lpzrobot_mac/selforg/wirings',
        '/Users/jerry/lpzrobot_mac/selforg/matrix',
        '/Users/jerry/lpzrobot_mac/selforg/statistictools',
        '/Users/jerry/lpzrobot_mac/selforg/include/selforg',
        '/Users/jerry/lpzrobot_mac/selforg/examples',
        '/Users/jerry/lpzrobot_mac/selforg/simulations'
    ]
    
    modified_count = 0
    for target_dir in target_dirs:
        if not os.path.exists(target_dir):
            continue
            
        print(f"\nProcessing directory: {target_dir}")
        for root, dirs, files in os.walk(target_dir):
            for file in files:
                if file.endswith(('.cpp', '.h')) and not file.endswith('.bak'):
                    filepath = os.path.join(root, file)
                    if modernize_comprehensive(filepath):
                        print(f"  Modernized: {file}")
                        modified_count += 1
                        if modified_count >= 20:  # Limit for testing
                            break
            if modified_count >= 20:
                break
        if modified_count >= 20:
            break
    
    print(f"\nTotal files modernized: {modified_count}")

if __name__ == '__main__':
    main()