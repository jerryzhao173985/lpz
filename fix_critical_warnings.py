#!/usr/bin/env python3
"""
Fix critical warnings in LPZRobots codebase.
Focuses on the most impactful and safe-to-fix issues.
"""

import os
import re
import sys
from pathlib import Path

class CriticalWarningFixer:
    def __init__(self):
        self.files_processed = 0
        self.fixes_applied = 0
        self.files_modified = 0
        
    def fix_null_to_nullptr(self, content):
        """Replace NULL with nullptr"""
        # Pattern to match NULL but not in comments or strings
        pattern = re.compile(r'\bNULL\b')
        
        lines = content.split('\n')
        modified_lines = []
        changes = 0
        
        for line in lines:
            # Skip comment lines
            if line.strip().startswith('//') or line.strip().startswith('/*'):
                modified_lines.append(line)
                continue
                
            # Skip lines with NULL in strings
            if '"' in line and 'NULL' in line:
                # Check if NULL is inside quotes
                in_string = False
                new_line = []
                i = 0
                while i < len(line):
                    if line[i] == '"' and (i == 0 or line[i-1] != '\\'):
                        in_string = not in_string
                    
                    if not in_string and line[i:i+4] == 'NULL' and (i+4 >= len(line) or not line[i+4].isalnum()):
                        new_line.append('nullptr')
                        i += 4
                        changes += 1
                    else:
                        new_line.append(line[i])
                        i += 1
                        
                modified_lines.append(''.join(new_line))
            else:
                # Simple replacement when no strings
                new_line = pattern.sub('nullptr', line)
                if new_line != line:
                    changes += 1
                modified_lines.append(new_line)
                
        if changes > 0:
            self.fixes_applied += changes
            return '\n'.join(modified_lines)
        return content
        
    def fix_c_headers(self, content):
        """Replace C headers with C++ headers"""
        header_map = {
            'stdio.h': 'cstdio',
            'stdlib.h': 'cstdlib',
            'string.h': 'cstring',
            'math.h': 'cmath',
            'ctype.h': 'cctype',
            'assert.h': 'cassert',
            'time.h': 'ctime',
            'errno.h': 'cerrno',
            'float.h': 'cfloat',
            'limits.h': 'climits',
            'locale.h': 'clocale',
            'setjmp.h': 'csetjmp',
            'signal.h': 'csignal',
            'stdarg.h': 'cstdarg',
            'stddef.h': 'cstddef'
        }
        
        changes = 0
        for old_header, new_header in header_map.items():
            pattern = f'#include\\s*<{old_header}>'
            replacement = f'#include <{new_header}>'
            new_content = re.sub(pattern, replacement, content)
            if new_content != content:
                changes += 1
                content = new_content
                
        if changes > 0:
            self.fixes_applied += changes
            
        return content
        
    def fix_virtual_override(self, content):
        """Add override to virtual functions in derived classes"""
        # This is a simplified version - only fixes obvious cases
        
        # Pattern: virtual <return_type> <function_name>(...) [const];
        pattern = re.compile(
            r'(\s*)(virtual\s+[^;{]+\([^)]*\)\s*(?:const\s*)?)(;)',
            re.MULTILINE
        )
        
        changes = 0
        for match in pattern.finditer(content):
            full_match = match.group(0)
            # Skip if already has override
            if 'override' in full_match:
                continue
            # Skip pure virtual (= 0)
            if '= 0' in full_match:
                continue
            # Skip destructors
            if '~' in full_match:
                continue
                
            # Add override
            indent = match.group(1)
            func_decl = match.group(2)
            new_line = f"{indent}{func_decl} override;"
            
            content = content[:match.start()] + new_line + content[match.end():]
            changes += 1
            
        if changes > 0:
            self.fixes_applied += changes
            
        return content
        
    def fix_member_initialization(self, content):
        """Add default initialization to member variables"""
        # Only fix simple cases - primitive types in structs/classes
        
        class_pattern = re.compile(r'(class|struct)\s+(\w+)[^{]*\{([^}]*)\}', re.DOTALL)
        
        for class_match in class_pattern.finditer(content):
            class_type = class_match.group(1)
            class_name = class_match.group(2)
            class_body = class_match.group(3)
            
            # Pattern for uninitialized primitive members
            member_pattern = re.compile(
                r'(\s*)((?:int|unsigned|short|long|float|double|bool|char)\s+)(\w+)\s*;'
            )
            
            modified_body = class_body
            for member_match in member_pattern.finditer(class_body):
                indent = member_match.group(1)
                type_name = member_match.group(2).strip()
                var_name = member_match.group(3)
                
                # Determine default value based on type
                if type_name == 'bool':
                    default_val = 'false'
                elif type_name in ['float', 'double']:
                    default_val = '0.0'
                else:
                    default_val = '0'
                    
                # Replace with initialized version
                new_decl = f"{indent}{type_name} {var_name} = {default_val};"
                modified_body = modified_body.replace(member_match.group(0), new_decl)
                
            if modified_body != class_body:
                content = content.replace(class_match.group(0), 
                                        f"{class_match.group(1)} {class_name}{class_match.group(0)[len(class_match.group(1)) + len(class_name):-len(class_body)-1]}{{{modified_body}}}")
                self.fixes_applied += 1
                
        return content
        
    def process_file(self, filepath):
        """Process a single file"""
        try:
            with open(filepath, 'r', encoding='utf-8', errors='ignore') as f:
                original_content = f.read()
        except Exception as e:
            print(f"Error reading {filepath}: {e}")
            return False
            
        content = original_content
        
        # Apply fixes
        content = self.fix_null_to_nullptr(content)
        content = self.fix_c_headers(content)
        
        # Only apply these fixes to header files
        if filepath.endswith('.h'):
            content = self.fix_virtual_override(content)
            content = self.fix_member_initialization(content)
            
        if content != original_content:
            try:
                with open(filepath, 'w', encoding='utf-8') as f:
                    f.write(content)
                print(f"Modified {os.path.relpath(filepath)}")
                self.files_modified += 1
                return True
            except Exception as e:
                print(f"Error writing {filepath}: {e}")
                return False
                
        return False
        
    def process_directory(self, directory):
        """Process all C++ files in directory"""
        for root, dirs, files in os.walk(directory):
            # Skip build directories
            dirs[:] = [d for d in dirs if not d.startswith('.') and 
                      d not in ['build', 'build_opt', 'build_dbg', 'CMakeFiles']]
            
            for file in files:
                if file.endswith(('.cpp', '.h', '.hpp', '.cc')):
                    filepath = os.path.join(root, file)
                    self.files_processed += 1
                    self.process_file(filepath)
                    
    def process_specific_files(self, base_dir):
        """Process files known to have critical issues"""
        critical_files = [
            # Files with NULL usage
            'selforg/controller/abstractcontroller.h',
            'selforg/controller/abstractmodel.h',
            'selforg/utils/configurable.h',
            'ode_robots/robots/oderobot.h',
            'ode_robots/osg/primitive.h',
            
            # Files with C headers
            'selforg/matrix/matrix.cpp',
            'selforg/utils/controller_misc.cpp',
            'ode_robots/utils/mathutils.cpp',
            
            # Files with uninitialized members
            'ode_robots/robots/hexabot.h',
            'ode_robots/robots/ashigaru.h',
            'ode_robots/robots/amos4legs.h',
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
            
        print(f"Fixing critical warnings in {directory}...")
        
        # First process known critical files
        self.process_specific_files(directory)
        
        # Then process entire directories
        for subdir in ['selforg', 'ode_robots']:
            full_path = os.path.join(directory, subdir)
            if os.path.exists(full_path):
                print(f"\nProcessing {subdir}...")
                self.process_directory(full_path)
                
        print(f"\n=== Summary ===")
        print(f"Files processed: {self.files_processed}")
        print(f"Files modified: {self.files_modified}")
        print(f"Fixes applied: {self.fixes_applied}")
        
        return True
        
def main():
    if len(sys.argv) < 2:
        print("Usage: python3 fix_critical_warnings.py <directory>")
        print("Example: python3 fix_critical_warnings.py /Users/jerry/lpzrobot_mac")
        sys.exit(1)
        
    fixer = CriticalWarningFixer()
    if fixer.run(sys.argv[1]):
        sys.exit(0)
    else:
        sys.exit(1)
        
if __name__ == "__main__":
    main()