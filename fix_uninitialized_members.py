#!/usr/bin/env python3
"""
Fix uninitialized member variables in LPZRobots codebase.
Adds default member initializers for primitive types.
"""

import os
import re
import sys
from pathlib import Path

class UninitializedMemberFixer:
    def __init__(self):
        self.files_processed = 0
        self.members_fixed = 0
        self.files_modified = 0
        
    def get_default_value(self, type_name):
        """Get appropriate default value for a type"""
        type_name = type_name.strip()
        
        # Boolean types
        if type_name == 'bool':
            return 'false'
            
        # Floating point types
        if type_name in ['float', 'double', 'paramval', 'sensor', 'motor']:
            return '0.0'
            
        # Integer types
        if type_name in ['int', 'unsigned', 'short', 'long', 'size_t', 
                         'unsigned int', 'unsigned short', 'unsigned long',
                         'uint8_t', 'uint16_t', 'uint32_t', 'uint64_t',
                         'int8_t', 'int16_t', 'int32_t', 'int64_t']:
            return '0'
            
        # Character types
        if type_name in ['char', 'unsigned char']:
            return '0'
            
        # Pointer types
        if '*' in type_name:
            return 'nullptr'
            
        # Unknown type
        return None
        
    def fix_struct_members(self, content):
        """Fix uninitialized members in structs and classes"""
        # Pattern to match struct/class definitions
        struct_pattern = re.compile(
            r'((?:struct|class)\s+\w+(?:\s*:\s*[^{]+)?)\s*\{([^}]*)\}',
            re.DOTALL
        )
        
        modified = False
        
        for struct_match in struct_pattern.finditer(content):
            struct_header = struct_match.group(1)
            struct_body = struct_match.group(2)
            original_body = struct_body
            
            # Split body into lines for processing
            lines = struct_body.split('\n')
            modified_lines = []
            
            for line in lines:
                # Skip empty lines
                if not line.strip():
                    modified_lines.append(line)
                    continue
                    
                # Skip comments
                if line.strip().startswith('//') or line.strip().startswith('/*'):
                    modified_lines.append(line)
                    continue
                    
                # Pattern for member variable declarations
                # Matches: type name; or type* name; or type name[size];
                member_pattern = re.compile(
                    r'^(\s*)((?:(?:const\s+)?(?:static\s+)?)*)'
                    r'((?:unsigned\s+)?(?:short\s+|long\s+)?'
                    r'(?:int|char|bool|float|double|size_t|paramval|sensor|motor|'
                    r'uint\d+_t|int\d+_t)(?:\s*\*)?)\s+'
                    r'(\w+)(?:\[[^\]]*\])?\s*;'
                )
                
                match = member_pattern.match(line)
                if match:
                    indent = match.group(1)
                    modifiers = match.group(2)
                    type_name = match.group(3)
                    var_name = match.group(4)
                    
                    # Skip if already initialized
                    if '=' in line or '{' in line:
                        modified_lines.append(line)
                        continue
                        
                    # Skip static members
                    if 'static' in modifiers:
                        modified_lines.append(line)
                        continue
                        
                    # Skip arrays for now
                    if '[' in line:
                        modified_lines.append(line)
                        continue
                        
                    # Get default value
                    default_val = self.get_default_value(type_name)
                    if default_val:
                        # Create new line with initialization
                        new_line = f"{indent}{modifiers}{type_name} {var_name} = {default_val};"
                        modified_lines.append(new_line)
                        self.members_fixed += 1
                    else:
                        modified_lines.append(line)
                else:
                    modified_lines.append(line)
                    
            # Reconstruct body
            new_body = '\n'.join(modified_lines)
            
            if new_body != original_body:
                # Replace in content
                old_struct = struct_match.group(0)
                new_struct = f"{struct_header} {{{new_body}}}"
                content = content.replace(old_struct, new_struct)
                modified = True
                
        return content, modified
        
    def process_file(self, filepath):
        """Process a single file"""
        try:
            with open(filepath, 'r', encoding='utf-8', errors='ignore') as f:
                original_content = f.read()
        except Exception as e:
            print(f"Error reading {filepath}: {e}")
            return False
            
        content, modified = self.fix_struct_members(original_content)
        
        if modified:
            try:
                with open(filepath, 'w', encoding='utf-8') as f:
                    f.write(content)
                print(f"Modified {os.path.relpath(filepath)}: Fixed member initializations")
                self.files_modified += 1
                return True
            except Exception as e:
                print(f"Error writing {filepath}: {e}")
                return False
                
        return False
        
    def process_directory(self, directory):
        """Process all header files in directory"""
        for root, dirs, files in os.walk(directory):
            # Skip build directories
            dirs[:] = [d for d in dirs if not d.startswith('.') and 
                      d not in ['build', 'build_opt', 'build_dbg', 'CMakeFiles']]
            
            for file in files:
                # Focus on header files where members are declared
                if file.endswith(('.h', '.hpp')):
                    filepath = os.path.join(root, file)
                    self.files_processed += 1
                    self.process_file(filepath)
                    
    def process_critical_files(self, base_dir):
        """Process files known to have many uninitialized members"""
        critical_files = [
            'ode_robots/robots/hexabot.h',
            'ode_robots/robots/ashigaru.h',
            'ode_robots/robots/amos4legs.h',
            'ode_robots/robots/amosII.h',
            'ode_robots/robots/sphererobot3masses.h',
            'ode_robots/robots/defaultCaterpillar.h',
            'ode_robots/robots/hand.h',
            'ode_robots/obstacles/playground.h',
            'selforg/controller/multireinforce.h',
            'selforg/controller/derinf.h',
            'selforg/controller/sox.h',
        ]
        
        for file in critical_files:
            filepath = os.path.join(base_dir, file)
            if os.path.exists(filepath):
                print(f"Processing critical file: {file}")
                self.files_processed += 1
                self.process_file(filepath)
                
    def run(self, directory):
        """Run the fixer on the given directory"""
        if not os.path.isdir(directory):
            print(f"Error: {directory} is not a valid directory")
            return False
            
        print(f"Fixing uninitialized members in {directory}...")
        
        # First process known critical files
        self.process_critical_files(directory)
        
        # Then process entire directories
        for subdir in ['selforg', 'ode_robots']:
            full_path = os.path.join(directory, subdir)
            if os.path.exists(full_path):
                print(f"\nProcessing all files in {subdir}...")
                self.process_directory(full_path)
                
        print(f"\n=== Summary ===")
        print(f"Files processed: {self.files_processed}")
        print(f"Files modified: {self.files_modified}")
        print(f"Member variables fixed: {self.members_fixed}")
        
        return True
        
def main():
    if len(sys.argv) < 2:
        print("Usage: python3 fix_uninitialized_members.py <directory>")
        print("Example: python3 fix_uninitialized_members.py /Users/jerry/lpzrobot_mac")
        sys.exit(1)
        
    fixer = UninitializedMemberFixer()
    if fixer.run(sys.argv[1]):
        sys.exit(0)
    else:
        sys.exit(1)
        
if __name__ == "__main__":
    main()