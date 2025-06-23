#!/usr/bin/env python3
"""
Script to fix uninitialized member variable warnings
"""

import re
import os
import sys
from pathlib import Path

class UninitMemberFixer:
    def __init__(self):
        self.files_processed = 0
        self.fixes_applied = 0
        
    def find_class_members(self, content, class_name):
        """Find all member variables of a class"""
        # Find class definition
        class_pattern = re.compile(
            rf'class\s+{class_name}\s*(?::\s*[^{{]+)?\s*{{',
            re.MULTILINE | re.DOTALL
        )
        
        class_match = class_pattern.search(content)
        if not class_match:
            return []
        
        # Find the class body
        start = class_match.end()
        brace_count = 1
        end = start
        
        while brace_count > 0 and end < len(content):
            if content[end] == '{':
                brace_count += 1
            elif content[end] == '}':
                brace_count -= 1
            end += 1
        
        class_body = content[start:end-1]
        
        # Find member variables (basic types)
        members = []
        member_patterns = [
            r'(?:int|unsigned int|short|unsigned short|long|unsigned long|size_t)\s+(\w+)\s*;',
            r'(?:float|double)\s+(\w+)\s*;',
            r'(?:bool)\s+(\w+)\s*;',
            r'(?:\w+)\s*\*\s*(\w+)\s*;',  # pointers
        ]
        
        for pattern in member_patterns:
            for match in re.finditer(pattern, class_body):
                var_name = match.group(1)
                # Skip static members
                line_start = class_body.rfind('\n', 0, match.start()) + 1
                line = class_body[line_start:match.end()]
                if 'static' not in line:
                    members.append(var_name)
        
        return members
    
    def find_constructor(self, content, class_name):
        """Find constructor definition and its initializer list"""
        # Pattern for constructor
        ctor_pattern = re.compile(
            rf'{class_name}\s*::\s*{class_name}\s*\([^)]*\)\s*(?::\s*([^{{]+))?\s*{{',
            re.MULTILINE | re.DOTALL
        )
        
        match = ctor_pattern.search(content)
        if not match:
            # Try inline constructor
            ctor_pattern = re.compile(
                rf'{class_name}\s*\([^)]*\)\s*(?::\s*([^{{]+))?\s*{{',
                re.MULTILINE | re.DOTALL
            )
            match = ctor_pattern.search(content)
        
        return match
    
    def add_member_initializers(self, content, class_name, members_to_init):
        """Add member initializers to constructor"""
        ctor_match = self.find_constructor(content, class_name)
        if not ctor_match:
            return content
        
        # Check if there's already an initializer list
        init_list = ctor_match.group(1) if ctor_match.lastindex >= 1 else None
        
        # Parse existing initializers
        existing_inits = set()
        if init_list:
            # Simple parsing of existing initializers
            for init in re.findall(r'(\w+)\s*\([^)]*\)', init_list):
                existing_inits.add(init)
        
        # Find members that need initialization
        new_inits = []
        for member in members_to_init:
            if member not in existing_inits:
                # Determine default value based on type
                # For now, use simple defaults
                if member.endswith('number') or member.endswith('count') or member.endswith('size'):
                    new_inits.append(f'{member}(0)')
                elif member.endswith('ptr') or member.endswith('pointer'):
                    new_inits.append(f'{member}(nullptr)')
                else:
                    # Try to infer from member name
                    new_inits.append(f'{member}(0)')
        
        if not new_inits:
            return content
        
        # Add new initializers
        if init_list:
            # Append to existing list
            new_init_list = init_list.rstrip() + ',\n      ' + ',\n      '.join(new_inits)
        else:
            # Create new initializer list
            new_init_list = '\n    : ' + ',\n      '.join(new_inits)
        
        # Replace in content
        if ctor_match.lastindex >= 1 and init_list:
            # Replace existing initializer list
            content = content[:ctor_match.start(1)] + new_init_list + content[ctor_match.end(1):]
        else:
            # Insert new initializer list
            insert_pos = ctor_match.start() + len(ctor_match.group(0).split('{')[0])
            content = content[:insert_pos] + new_init_list + ' ' + content[insert_pos:]
        
        self.fixes_applied += 1
        return content
    
    def process_file(self, filepath):
        """Process a single file to fix uninitialized members"""
        try:
            with open(filepath, 'r', encoding='utf-8', errors='ignore') as f:
                content = f.read()
        except Exception as e:
            print(f"Error reading {filepath}: {e}")
            return False
        
        original_content = content
        modified = False
        
        # Find all class definitions
        class_pattern = re.compile(r'class\s+(\w+)\s*(?::|{)')
        
        for class_match in class_pattern.finditer(content):
            class_name = class_match.group(1)
            
            # Find members
            members = self.find_class_members(content, class_name)
            
            if members:
                # Check if constructor initializes them
                ctor_match = self.find_constructor(content, class_name)
                if ctor_match:
                    # Simple check - this could be improved
                    uninit_members = []
                    for member in members:
                        if member not in content[ctor_match.start():ctor_match.end()]:
                            uninit_members.append(member)
                    
                    if uninit_members:
                        print(f"  Found uninitialized members in {class_name}: {uninit_members}")
                        # For safety, only initialize specific known problematic ones
                        if class_name in ['AbstractIAFController', 'ClassicReinforce']:
                            content = self.add_member_initializers(content, class_name, uninit_members)
                            modified = True
        
        if modified and content != original_content:
            try:
                with open(filepath, 'w', encoding='utf-8') as f:
                    f.write(content)
                print(f"Fixed {filepath}")
                return True
            except Exception as e:
                print(f"Error writing {filepath}: {e}")
                return False
        
        return False
    
    def process_directory(self, directory):
        """Process all C++ files in directory"""
        for root, dirs, files in os.walk(directory):
            # Skip build directories
            dirs[:] = [d for d in dirs if d not in ['build', 'build_opt', 'build_dbg', '.git']]
            
            for file in files:
                if file.endswith(('.cpp', '.h', '.hpp')):
                    filepath = os.path.join(root, file)
                    self.files_processed += 1
                    self.process_file(filepath)

def main():
    if len(sys.argv) < 2:
        print("Usage: python3 fix_uninit_members.py <directory>")
        sys.exit(1)
    
    fixer = UninitMemberFixer()
    
    path = sys.argv[1]
    if os.path.isfile(path):
        fixer.files_processed = 1
        fixer.process_file(path)
    elif os.path.isdir(path):
        fixer.process_directory(path)
    else:
        print(f"Error: {path} is not a valid file or directory")
        sys.exit(1)
    
    print(f"\nProcessed {fixer.files_processed} files")
    print(f"Applied {fixer.fixes_applied} fixes")

if __name__ == "__main__":
    main()