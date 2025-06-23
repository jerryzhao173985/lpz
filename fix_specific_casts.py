#!/usr/bin/env python3
"""Fix specific C-style casts found by manual inspection."""

import os

def fix_file(filepath, replacements):
    """Apply specific replacements to a file."""
    if not os.path.exists(filepath):
        print(f"File not found: {filepath}")
        return False
    
    # Create backup
    backup_path = filepath + '.bak.casts3'
    if not os.path.exists(backup_path):
        os.system(f'cp "{filepath}" "{backup_path}"')
    
    with open(filepath, 'r', encoding='utf-8', errors='ignore') as f:
        content = f.read()
    
    modified = False
    for old, new in replacements:
        if old in content:
            content = content.replace(old, new)
            modified = True
            print(f"  Replaced: {old} → {new}")
    
    if modified:
        with open(filepath, 'w', encoding='utf-8') as f:
            f.write(content)
    
    return modified

def main():
    base_dir = '/Users/jerry/lpzrobot_mac/selforg/'
    
    # Specific replacements for each file
    fixes = {
        'controller/dercontroller.cpp': [
            ('assert((unsigned)number_sensors == this->number_sensors\n         && (unsigned)number_motors == this->number_motors);',
             'assert(static_cast<unsigned>(number_sensors) == this->number_sensors\n         && static_cast<unsigned>(number_motors) == this->number_motors);')
        ],
        'controller/oneactivemultipassivecontroller.cpp': [
            ('assert((unsigned)number_sensors == this->number_sensors &&\n         (unsigned)number_motors == this->number_motors);',
             'assert(static_cast<unsigned>(number_sensors) == this->number_sensors &&\n         static_cast<unsigned>(number_motors) == this->number_motors);')
        ],
        'controller/multireinforce.cpp': [
            ('assert((unsigned)number_sensors == this->number_sensors);',
             'assert(static_cast<unsigned>(number_sensors) == this->number_sensors);'),
            ('assert((unsigned)number_motors == this->number_motors);',
             'assert(static_cast<unsigned>(number_motors) == this->number_motors);'),
            ('action=clip(action_,0,conf.numSats-1);',
             'action=clip(action_,0,conf.numSats-1);')  # This one is fine as is
        ],
        'matrix/matrix.cpp': [
            ('for(int i=0; i < (int)m; i++){',
             'for(int i=0; i < static_cast<int>(m); i++){'),
            ('for(int j=0; j < (int)n; j++){',
             'for(int j=0; j < static_cast<int>(n); j++){')
        ],
        'utils/statistictools.cpp': [
            ('(double)cnt',
             'static_cast<double>(cnt)'),
            ('(double)(cnt*(cnt-1))',
             'static_cast<double>(cnt*(cnt-1))')
        ]
    }
    
    modified_count = 0
    for filepath, replacements in fixes.items():
        full_path = os.path.join(base_dir, filepath)
        print(f"\nProcessing {filepath}:")
        if fix_file(full_path, replacements):
            modified_count += 1
        else:
            print("  No changes needed")
    
    print(f"\nModified {modified_count} files")

if __name__ == '__main__':
    main()