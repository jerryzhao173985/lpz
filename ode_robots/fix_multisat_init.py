#!/usr/bin/env python3
import re
import os
import sys

def fix_multisat_constructor(filepath):
    """Fix MultiSat constructor to initialize all member variables"""
    
    with open(filepath, 'r') as f:
        content = f.read()
    
    # Pattern to find MultiSat constructor
    pattern = r'(MultiSat::MultiSat\s*\([^)]*\)\s*:\s*AbstractController[^{]+)\{([^}]*)\}'
    
    def replace_constructor(match):
        header = match.group(1)
        body = match.group(2)
        
        # Check if already fixed (has number_sensors initialization)
        if 'number_sensors(' in header:
            return match.group(0)
        
        # Extract existing initializations from header
        if 'buffersize(' in header and 'conf(' in header:
            # Remove the existing partial initializations
            header = re.sub(r',\s*buffersize\([^)]+\)', '', header)
            header = re.sub(r',\s*conf\([^)]+\)', '', header)
        
        # Add proper initialization list
        new_header = header.rstrip() + """, 
    number_sensors(0),
    number_motors(0),
    buffersize(_conf.buffersize), 
    x_buffer(nullptr),
    xp_buffer(nullptr),
    y_buffer(nullptr),
    x_context_buffer(nullptr),
    winner(0),
    satControl(false),
    runcompetefirsttime(true),
    conf(_conf),
    initialised(false),
    t(0),
    managementInterval(100)
{"""
        
        # Remove redundant assignments from body
        new_body = body
        redundant_assignments = [
            'runcompetefirsttime\s*=\s*true;',
            'managementInterval\s*=\s*100;',
            'winner\s*=\s*0;',
            'satControl\s*=\s*false;',
            'initialised\s*=\s*false;'
        ]
        for assignment in redundant_assignments:
            new_body = re.sub(assignment + r'\s*', '', new_body)
        
        # If body is now empty or just whitespace, return just the header with closing brace
        if not new_body.strip():
            return new_header + '\n}'
        else:
            return new_header + new_body + '}'
    
    # Replace constructor
    new_content = re.sub(pattern, replace_constructor, content, flags=re.DOTALL)
    
    if new_content != content:
        # Create backup
        backup_path = filepath + '.bak_multisat'
        with open(backup_path, 'w') as f:
            f.write(content)
        
        # Write fixed content
        with open(filepath, 'w') as f:
            f.write(new_content)
        
        print(f"✓ Fixed: {filepath}")
        return True
    
    return False

if __name__ == "__main__":
    # Find all multisat files
    multisat_files = []
    for root, dirs, files in os.walk('.'):
        for file in files:
            if 'multisat' in file.lower() and file.endswith('.cpp'):
                multisat_files.append(os.path.join(root, file))
    
    print(f"Found {len(multisat_files)} multisat files")
    
    fixed_count = 0
    for filepath in multisat_files:
        if fix_multisat_constructor(filepath):
            fixed_count += 1
    
    print(f"\n=== Summary ===")
    print(f"Files fixed: {fixed_count}")
    print(f"Backups created with .bak_multisat extension")