#!/usr/bin/env python3

import os
import re
import subprocess
from pathlib import Path
from collections import defaultdict

class UninitMemberFixer:
    def __init__(self):
        self.member_pattern = re.compile(r'Member variable \'([^:]+):([^\']+)\' is not initialized')
        self.constructor_pattern = re.compile(r'^(\s*)(\w+)::\2\s*\([^)]*\)\s*(?::\s*(.+?))?\s*{', re.MULTILINE)
        self.issues_by_class = defaultdict(list)
        
    def parse_cppcheck_log(self, log_file):
        """Parse cppcheck log to extract uninitialized member issues"""
        issues = []
        with open(log_file, 'r') as f:
            for line in f:
                if 'uninitMemberVar' in line:
                    parts = line.split(':')
                    if len(parts) >= 4:
                        file_path = parts[0]
                        line_num = parts[1]
                        # Extract class and member from warning message
                        match = self.member_pattern.search(line)
                        if match:
                            class_name = match.group(1)
                            member_name = match.group(2)
                            issues.append({
                                'file': file_path,
                                'line': line_num,
                                'class': class_name,
                                'member': member_name
                            })
                            self.issues_by_class[class_name].append(member_name)
        return issues
    
    def find_member_type(self, header_file, class_name, member_name):
        """Try to determine the type of a member variable from header file"""
        try:
            with open(header_file, 'r', encoding='utf-8', errors='ignore') as f:
                content = f.read()
                
            # Look for class definition
            class_pattern = re.compile(rf'class\s+{class_name}\s*[:\{{].*?^}};', re.MULTILINE | re.DOTALL)
            class_match = class_pattern.search(content)
            
            if class_match:
                class_body = class_match.group(0)
                # Look for member declaration
                # Common patterns: type member; or type* member; or type member[size];
                member_patterns = [
                    rf'(\w+(?:\s*[*&])?)\s+{member_name}\s*(?:\[[^\]]*\])?\s*;',
                    rf'(\w+(?:::\w+)*(?:\s*[*&])?)\s+{member_name}\s*(?:\[[^\]]*\])?\s*;',
                    rf'(matrix::Matrix|Matrix)\s+{member_name}\s*;'
                ]
                
                for pattern in member_patterns:
                    match = re.search(pattern, class_body)
                    if match:
                        return match.group(1).strip()
            
            return None
        except Exception as e:
            print(f"Error reading {header_file}: {e}")
            return None
    
    def get_default_initializer(self, member_type, member_name):
        """Get appropriate default initializer for a type"""
        if not member_type:
            return f"{member_name}()"  # Default construct
            
        # Clean up type
        type_clean = member_type.replace('*', '').replace('&', '').strip()
        
        # Pointer types
        if '*' in member_type:
            return f"{member_name}(nullptr)"
        
        # Numeric types
        if type_clean in ['int', 'unsigned', 'unsigned int', 'long', 'unsigned long', 
                          'short', 'unsigned short', 'size_t', 'I']:
            return f"{member_name}(0)"
        
        if type_clean in ['double', 'float', 'D']:
            return f"{member_name}(0.0)"
            
        if type_clean in ['bool']:
            return f"{member_name}(false)"
            
        # Default construct everything else
        return f"{member_name}()"
    
    def fix_constructor(self, cpp_file, class_name, members_to_init):
        """Fix constructor in cpp file to initialize members"""
        try:
            with open(cpp_file, 'r', encoding='utf-8', errors='ignore') as f:
                content = f.read()
                
            # Find the header file
            header_file = cpp_file.replace('.cpp', '.h')
            if not os.path.exists(header_file):
                header_file = cpp_file.replace('.cpp', '.hpp')
                
            # Get member types
            member_inits = []
            for member in members_to_init:
                member_type = None
                if os.path.exists(header_file):
                    member_type = self.find_member_type(header_file, class_name, member)
                init = self.get_default_initializer(member_type, member)
                member_inits.append(init)
            
            # Find constructor
            pattern = rf'^(\s*)({class_name}::{class_name}\s*\([^)]*\))\s*(:\s*(.+?))?\s*{{' 
            
            def replace_constructor(match):
                indent = match.group(1)
                constructor_decl = match.group(2)
                existing_init = match.group(4) if match.group(4) else ""
                
                # Build new initializer list
                if existing_init:
                    # Add to existing list
                    new_inits = existing_init + ",\n" + indent + "    " + f",\n{indent}    ".join(member_inits)
                else:
                    # Create new list
                    new_inits = f",\n{indent}    ".join(member_inits)
                
                return f"{indent}{constructor_decl}\n{indent}  : {existing_init + ',' if existing_init else ''}\n{indent}    {new_inits} {{"
            
            # Apply fix
            new_content = re.sub(pattern, replace_constructor, content, flags=re.MULTILINE)
            
            if new_content != content:
                # Backup and write
                backup_file = cpp_file + '.bak'
                with open(backup_file, 'w', encoding='utf-8') as f:
                    f.write(content)
                    
                with open(cpp_file, 'w', encoding='utf-8') as f:
                    f.write(new_content)
                    
                return True
            return False
            
        except Exception as e:
            print(f"Error fixing {cpp_file}: {e}")
            return False
    
    def run(self, log_file):
        """Run the fixer"""
        print("=== Uninitialized Member Variable Fixer ===")
        print(f"Parsing {log_file}...")
        
        issues = self.parse_cppcheck_log(log_file)
        print(f"Found {len(issues)} uninitialized member warnings")
        
        # Group by class
        classes_to_fix = defaultdict(set)
        class_files = defaultdict(str)
        
        for issue in issues:
            classes_to_fix[issue['class']].add(issue['member'])
            # Try to find the cpp file
            cpp_file = issue['file'].replace('.h', '.cpp')
            if os.path.exists(cpp_file):
                class_files[issue['class']] = cpp_file
        
        # Sort by number of issues
        sorted_classes = sorted(classes_to_fix.items(), key=lambda x: len(x[1]), reverse=True)
        
        print("\nClasses with most issues:")
        for class_name, members in sorted_classes[:10]:
            print(f"  {class_name}: {len(members)} members")
            
        # Fix the worst offenders
        fixed_count = 0
        for class_name, members in sorted_classes[:5]:  # Fix top 5 classes
            if class_name in class_files:
                cpp_file = class_files[class_name]
                print(f"\nFixing {class_name} in {cpp_file}...")
                print(f"  Members to initialize: {', '.join(sorted(members))}")
                
                if self.fix_constructor(cpp_file, class_name, sorted(members)):
                    fixed_count += 1
                    print(f"  ✓ Fixed {class_name}")
                else:
                    print(f"  ✗ Could not fix {class_name}")
            else:
                print(f"\nSkipping {class_name} - no cpp file found")
        
        print(f"\n=== Summary ===")
        print(f"Fixed {fixed_count} classes")
        print(f"Backups created with .bak extension")

if __name__ == "__main__":
    fixer = UninitMemberFixer()
    fixer.run("cppcheck_selforg_clean.log")