#!/usr/bin/env python3
"""
Systematically add override keywords to all virtual functions in derived classes
"""

import os
import re
import sys
from pathlib import Path

class OverrideFixer:
    def __init__(self):
        self.files_processed = 0
        self.overrides_added = 0
        self.files_modified = 0
        
    def find_virtual_functions(self, content):
        """Find all virtual function declarations that need override"""
        # Pattern to match virtual functions without override
        # Matches: virtual <return_type> <function_name>(...) [const] [= 0];
        pattern = re.compile(
            r'(\s*)(virtual\s+[^;{]+\([^)]*\)\s*(?:const\s*)?)(;|(?:\s*=\s*0\s*;))',
            re.MULTILINE
        )
        
        matches = []
        for match in pattern.finditer(content):
            full_match = match.group(0)
            # Skip if already has override
            if 'override' in full_match:
                continue
            # Skip if it's a pure virtual function in base class (no context to determine)
            if '= 0' in full_match and 'class' in content[max(0, match.start()-200):match.start()]:
                # Check if this might be a base class definition
                continue
            matches.append(match)
        
        return matches
    
    def add_override_keyword(self, content, match):
        """Add override keyword to a virtual function"""
        indent = match.group(1)
        func_decl = match.group(2)
        ending = match.group(3)
        
        if '= 0' in ending:
            # Pure virtual function
            new_line = f"{indent}{func_decl} override = 0;"
        else:
            # Regular virtual function
            new_line = f"{indent}{func_decl} override;"
        
        return content[:match.start()] + new_line + content[match.end():]
    
    def process_file(self, filepath):
        """Process a single header file to add override keywords"""
        try:
            with open(filepath, 'r', encoding='utf-8') as f:
                original_content = f.read()
        except Exception as e:
            print(f"Error reading {filepath}: {e}")
            return False
        
        content = original_content
        matches = self.find_virtual_functions(content)
        
        if not matches:
            return False
        
        # Process matches in reverse order to maintain correct positions
        for match in reversed(matches):
            content = self.add_override_keyword(content, match)
            self.overrides_added += 1
        
        if content != original_content:
            try:
                with open(filepath, 'w', encoding='utf-8') as f:
                    f.write(content)
                print(f"Modified {filepath}: Added {len(matches)} override keywords")
                self.files_modified += 1
                return True
            except Exception as e:
                print(f"Error writing {filepath}: {e}")
                return False
        
        return False
    
    def process_directory(self, directory):
        """Process all header files in a directory"""
        for root, dirs, files in os.walk(directory):
            # Skip build directories
            dirs[:] = [d for d in dirs if d not in ['build', 'build_opt', 'build_dbg', '.git']]
            
            for file in files:
                if file.endswith(('.h', '.hpp')):
                    filepath = os.path.join(root, file)
                    self.files_processed += 1
                    self.process_file(filepath)
    
    def get_specific_files_with_warnings(self):
        """Return list of files known to have override warnings"""
        return [
            'selforg/controller/pimax.h',
            'selforg/controller/semox.h',
            'selforg/controller/derpseudosensor.h',
            'selforg/controller/depcontroller.h',
            'selforg/controller/invertmotorcontroller.h',
            'selforg/controller/invertmotornstep.h',
            'selforg/controller/invertmotorspace.h',
            'selforg/controller/sinecontroller.h',
            'selforg/controller/ffnncontroller.h',
            'selforg/controller/replaycontroller.h',
            'selforg/controller/qlearning.h'
        ]
    
    def run(self, path):
        """Run the override fixer on the given path"""
        if os.path.isfile(path):
            self.files_processed = 1
            self.process_file(path)
        elif os.path.isdir(path):
            # First process known problem files
            base_dir = path
            for file in self.get_specific_files_with_warnings():
                full_path = os.path.join(base_dir, file)
                if os.path.exists(full_path):
                    print(f"Processing known problem file: {file}")
                    self.files_processed += 1
                    self.process_file(full_path)
            
            # Then process entire directory
            print(f"\nProcessing all files in {path}")
            self.process_directory(path)
        else:
            print(f"Error: {path} is not a valid file or directory")
            return False
        
        print(f"\n=== Summary ===")
        print(f"Files processed: {self.files_processed}")
        print(f"Files modified: {self.files_modified}")
        print(f"Override keywords added: {self.overrides_added}")
        return True

def main():
    if len(sys.argv) < 2:
        print("Usage: python3 fix_override_warnings.py <directory_or_file>")
        print("Example: python3 fix_override_warnings.py /Users/jerry/lpzrobot_mac")
        sys.exit(1)
    
    fixer = OverrideFixer()
    if fixer.run(sys.argv[1]):
        sys.exit(0)
    else:
        sys.exit(1)

if __name__ == "__main__":
    main()