#!/usr/bin/env python3
"""Add override keywords to remaining virtual functions."""

import re
import os
import sys

# Extract unique method names and files from build warnings
override_warnings = """
controller/abstractcontrolleradapter.h:99:16: warning: 'step' overrides a member function but is not marked 'override' [-Winconsistent-missing-override]
controller/abstractcontrolleradapter.h:107:16: warning: 'stepNoLearning' overrides a member function but is not marked 'override' [-Winconsistent-missing-override]
controller/abstractcontrolleradapter.h:142:16: warning: 'store' overrides a member function but is not marked 'override' [-Winconsistent-missing-override]
controller/abstractcontrolleradapter.h:147:16: warning: 'restore' overrides a member function but is not marked 'override' [-Winconsistent-missing-override]
controller/abstractiafcontroller.h:101:16: warning: 'step' overrides a member function but is not marked 'override' [-Winconsistent-missing-override]
controller/abstractiafcontroller.h:103:16: warning: 'stepNoLearning' overrides a member function but is not marked 'override' [-Winconsistent-missing-override]
controller/abstractiafcontroller.h:107:16: warning: 'store' overrides a member function but is not marked 'override' [-Winconsistent-missing-override]
controller/abstractiafcontroller.h:109:16: warning: 'restore' overrides a member function but is not marked 'override' [-Winconsistent-missing-override]
controller/abstractiafcontroller.h:112:16: warning: 'notifyOnChange' overrides a member function but is not marked 'override' [-Winconsistent-missing-override]
controller/abstractmulticontroller.h:86:22: warning: 'step' overrides a member function but is not marked 'override' [-Winconsistent-missing-override]
controller/abstractmulticontroller.h:92:22: warning: 'stepNoLearning' overrides a member function but is not marked 'override' [-Winconsistent-missing-override]
controller/derbigcontroller.h:69:16: warning: 'step' overrides a member function but is not marked 'override' [-Winconsistent-missing-override]
controller/derbigcontroller.h:72:16: warning: 'stepNoLearning' overrides a member function but is not marked 'override' [-Winconsistent-missing-override]
controller/derbigcontroller.h:78:16: warning: 'store' overrides a member function but is not marked 'override' [-Winconsistent-missing-override]
controller/derbigcontroller.h:80:16: warning: 'restore' overrides a member function but is not marked 'override' [-Winconsistent-missing-override]
controller/derbigcontroller.h:83:25: warning: 'getInternalParamNames' overrides a member function but is not marked 'override' [-Winconsistent-missing-override]
controller/derbigcontroller.h:84:25: warning: 'getInternalParams' overrides a member function but is not marked 'override' [-Winconsistent-missing-override]
controller/derbigcontroller.h:85:22: warning: 'getStructuralLayers' overrides a member function but is not marked 'override' [-Winconsistent-missing-override]
controller/derbigcontroller.h:86:27: warning: 'getStructuralConnections' overrides a member function but is not marked 'override' [-Winconsistent-missing-override]
controller/derbigcontroller.h:89:16: warning: 'notifyOnChange' overrides a member function but is not marked 'override' [-Winconsistent-missing-override]
controller/invertmotornstep.h:71:16: warning: 'step' overrides a member function but is not marked 'override' [-Winconsistent-missing-override]
controller/invertmotornstep.h:74:16: warning: 'stepNoLearning' overrides a member function but is not marked 'override' [-Winconsistent-missing-override]
controller/invertmotornstep.h:79:16: warning: 'store' overrides a member function but is not marked 'override' [-Winconsistent-missing-override]
controller/invertmotornstep.h:81:16: warning: 'restore' overrides a member function but is not marked 'override' [-Winconsistent-missing-override]
controller/invertmotornstep.h:84:29: warning: 'getStructuralLayers' overrides a member function but is not marked 'override' [-Winconsistent-missing-override]
controller/invertmotornstep.h:85:34: warning: 'getStructuralConnections' overrides a member function but is not marked 'override' [-Winconsistent-missing-override]
controller/invertmotorspace.h:51:16: warning: 'step' overrides a member function but is not marked 'override' [-Winconsistent-missing-override]
controller/invertmotorspace.h:54:16: warning: 'stepNoLearning' overrides a member function but is not marked 'override' [-Winconsistent-missing-override]
controller/invertmotorspace.h:59:16: warning: 'store' overrides a member function but is not marked 'override' [-Winconsistent-missing-override]
controller/invertmotorspace.h:61:16: warning: 'restore' overrides a member function but is not marked 'override' [-Winconsistent-missing-override]
controller/invertmotorspace.h:64:22: warning: 'getStructuralLayers' overrides a member function but is not marked 'override' [-Winconsistent-missing-override]
controller/invertmotorspace.h:65:27: warning: 'getStructuralConnections' overrides a member function but is not marked 'override' [-Winconsistent-missing-override]
controller/invertnchannelcontroller.h:47:19: warning: 'getName' overrides a member function but is not marked 'override' [-Winconsistent-missing-override]
controller/invertnchannelcontroller.h:55:16: warning: 'step' overrides a member function but is not marked 'override' [-Winconsistent-missing-override]
controller/invertnchannelcontroller.h:59:16: warning: 'stepNoLearning' overrides a member function but is not marked 'override' [-Winconsistent-missing-override]
controller/invertnchannelcontroller.h:65:16: warning: 'store' overrides a member function but is not marked 'override' [-Winconsistent-missing-override]
controller/invertnchannelcontroller.h:67:16: warning: 'restore' overrides a member function but is not marked 'override' [-Winconsistent-missing-override]
"""

def process_file(filepath):
    """Process a single file to add override keywords."""
    if not os.path.exists(filepath):
        print(f"File not found: {filepath}")
        return False
    
    # Create backup
    backup_path = filepath + '.bak.override2'
    if not os.path.exists(backup_path):
        os.system(f'cp "{filepath}" "{backup_path}"')
    
    with open(filepath, 'r', encoding='utf-8', errors='ignore') as f:
        content = f.read()
    
    # Parse warnings for this file
    file_warnings = []
    for line in override_warnings.strip().split('\n'):
        if filepath in line:
            match = re.match(r'([^:]+):(\d+):(\d+): warning: \'(\w+)\' overrides', line)
            if match:
                file_warnings.append({
                    'line': int(match.group(2)),
                    'col': int(match.group(3)),
                    'method': match.group(4)
                })
    
    if not file_warnings:
        return False
    
    lines = content.split('\n')
    modified = False
    
    for warning in file_warnings:
        line_idx = warning['line'] - 1
        if line_idx < len(lines):
            line = lines[line_idx]
            method_name = warning['method']
            
            # Check if override already exists
            if 'override' in line:
                continue
            
            # Pattern to match virtual function declarations
            # Look for method name followed by ( and ending with ; or {
            pattern = rf'\b{method_name}\s*\([^)]*\)(?:\s*const)?(?:\s*=\s*0)?(\s*[;{{])'
            
            match = re.search(pattern, line)
            if match:
                # Insert override before the semicolon or opening brace
                insert_pos = match.start(1)
                new_line = line[:insert_pos] + ' override' + line[insert_pos:]
                lines[line_idx] = new_line
                modified = True
                print(f"Added override to {method_name} at line {warning['line']} in {filepath}")
    
    if modified:
        with open(filepath, 'w', encoding='utf-8') as f:
            f.write('\n'.join(lines))
        return True
    
    return False

def main():
    files = [
        'controller/abstractcontrolleradapter.h',
        'controller/abstractiafcontroller.h',
        'controller/abstractmulticontroller.h',
        'controller/derbigcontroller.h',
        'controller/invertmotornstep.h',
        'controller/invertmotorspace.h',
        'controller/invertnchannelcontroller.h'
    ]
    
    modified_count = 0
    for filepath in files:
        full_path = f'/Users/jerry/lpzrobot_mac/selforg/{filepath}'
        if process_file(full_path):
            modified_count += 1
    
    print(f"\nModified {modified_count} files")

if __name__ == '__main__':
    main()