#!/usr/bin/env python3
"""Apply final C++17 modernization patterns."""

import re
import os

def modernize_cpp_file(filepath):
    """Apply various C++17 modernization patterns."""
    if not os.path.exists(filepath):
        return False
    
    # Skip backup files
    if '.bak' in filepath:
        return False
    
    # Create backup
    backup_path = filepath + '.bak.modern'
    if not os.path.exists(backup_path):
        os.system(f'cp "{filepath}" "{backup_path}"')
    
    with open(filepath, 'r', encoding='utf-8', errors='ignore') as f:
        content = f.read()
    
    changes = []
    modified = content
    
    # 1. Replace push_back with emplace_back where appropriate
    emplace_pattern = r'(\w+)\.push_back\((\w+)\((.*?)\)\)'
    emplace_matches = list(re.finditer(emplace_pattern, modified))
    for match in reversed(emplace_matches):
        container = match.group(1)
        constructor = match.group(2)
        args = match.group(3)
        new_text = f'{container}.emplace_back({args})'
        modified = modified[:match.start()] + new_text + modified[match.end():]
        changes.append(('push_back → emplace_back', match.start()))
    
    # 2. Use std::make_unique for unique_ptr (C++14 but widely adopted)
    unique_pattern = r'std::unique_ptr<([^>]+)>\s*\(\s*new\s+\1\((.*?)\)\)'
    unique_matches = list(re.finditer(unique_pattern, modified))
    for match in reversed(unique_matches):
        type_name = match.group(1)
        args = match.group(2)
        new_text = f'std::make_unique<{type_name}>({args})'
        modified = modified[:match.start()] + new_text + modified[match.end():]
        changes.append(('unique_ptr → make_unique', match.start()))
    
    # 3. Replace NULL with nullptr (if any remaining)
    null_pattern = r'\bNULL\b'
    null_matches = list(re.finditer(null_pattern, modified))
    for match in reversed(null_matches):
        modified = modified[:match.start()] + 'nullptr' + modified[match.end():]
        changes.append(('NULL → nullptr', match.start()))
    
    # 4. Use structured bindings for pairs/tuples (C++17)
    # Example: pair<int, string> p = func(); int x = p.first; string s = p.second;
    # Becomes: auto [x, s] = func();
    # This is complex to do safely with regex, so skip
    
    # 5. Replace std::bind with lambdas (more readable)
    # This is also complex to do safely with regex
    
    # 6. Use std::optional instead of pointers for optional values
    # This requires semantic understanding, skip for automatic conversion
    
    # 7. Replace 0 with nullptr for pointer comparisons
    ptr_cmp_pattern = r'(\w+\s*[!=]=\s*)0\b(?!\d)'
    ptr_cmp_matches = list(re.finditer(ptr_cmp_pattern, modified))
    for match in reversed(ptr_cmp_matches):
        # Check if this looks like a pointer comparison
        prefix = modified[max(0, match.start()-50):match.start()]
        if '*' in prefix or 'ptr' in prefix.lower() or 'pointer' in prefix.lower():
            modified = modified[:match.end()-1] + 'nullptr' + modified[match.end():]
            changes.append(('0 → nullptr (comparison)', match.start()))
    
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
    # Focus on core library files
    target_dirs = [
        '/Users/jerry/lpzrobot_mac/selforg/controller',
        '/Users/jerry/lpzrobot_mac/selforg/utils',
        '/Users/jerry/lpzrobot_mac/selforg/wirings',
        '/Users/jerry/lpzrobot_mac/selforg/matrix'
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
                    if modernize_cpp_file(filepath):
                        print(f"  Modernized: {file}")
                        modified_count += 1
                        if modified_count >= 10:  # Limit for testing
                            break
            if modified_count >= 10:
                break
        if modified_count >= 10:
            break
    
    print(f"\nTotal files modernized: {modified_count}")

if __name__ == '__main__':
    main()