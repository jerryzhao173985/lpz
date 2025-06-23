#!/usr/bin/env python3

import os
import re
import shutil
from pathlib import Path

class FscanfVulnerabilityFixer:
    def __init__(self):
        self.vulnerable_pattern = re.compile(r'fscanf\s*\(\s*([^,]+)\s*,\s*"([^"]*%s[^"]*)"')
        self.buffer_size_pattern = re.compile(r'char\s+(\w+)\s*\[\s*(\d+)\s*\]')
        self.vulnerable_files = []
        
    def find_buffer_size(self, content, buffer_name):
        """Find the size of a buffer from its declaration"""
        matches = self.buffer_size_pattern.findall(content)
        for name, size in matches:
            if name == buffer_name:
                return int(size)
        return 128  # Default safe size if not found
    
    def fix_fscanf_line(self, line, content=None):
        """Fix a single line containing vulnerable fscanf"""
        match = self.vulnerable_pattern.search(line)
        if not match:
            return line
            
        format_str = match.group(2)
        
        # Skip if already has field width
        if re.search(r'%\d+s', format_str):
            return line
            
        # Extract buffer name from the arguments
        buffer_match = re.search(r'fscanf\s*\([^,]+,[^,]+,\s*(\w+)', line)
        if buffer_match and content:
            buffer_name = buffer_match.group(1)
            buffer_size = self.find_buffer_size(content, buffer_name)
            safe_size = buffer_size - 1
        else:
            safe_size = 127  # Default safe size
            
        # Replace %s with field-limited version
        new_format = format_str.replace('%s', f'%{safe_size}s')
        new_line = line.replace(f'"{format_str}"', f'"{new_format}"')
        
        return new_line
    
    def fix_file(self, filepath):
        """Fix all vulnerabilities in a single file"""
        try:
            with open(filepath, 'r', encoding='utf-8', errors='ignore') as f:
                content = f.read()
                
            lines = content.split('\n')
            fixed_lines = []
            changes_made = False
            
            for line in lines:
                if 'fscanf' in line and '%s' in line:
                    fixed_line = self.fix_fscanf_line(line, content)
                    if fixed_line != line:
                        changes_made = True
                        fixed_lines.append(fixed_line + "  // Security fix: added field width limit")
                    else:
                        fixed_lines.append(line)
                else:
                    fixed_lines.append(line)
            
            if changes_made:
                # Create backup
                backup_path = f"{filepath}.bak"
                shutil.copy2(filepath, backup_path)
                
                # Write fixed content
                with open(filepath, 'w', encoding='utf-8') as f:
                    f.write('\n'.join(fixed_lines))
                    
                print(f"✓ Fixed: {filepath}")
                return True
            return False
            
        except Exception as e:
            print(f"✗ Error processing {filepath}: {e}")
            return False
    
    def run(self, root_dir):
        """Run the fixer on all C/C++ files in the directory"""
        print("=== fscanf Vulnerability Fixer (Auto Mode) ===")
        print(f"Scanning directory: {root_dir}")
        
        fixed_count = 0
        total_files = 0
        
        for ext in ['*.c', '*.cpp', '*.h', '*.hpp']:
            for filepath in Path(root_dir).rglob(ext):
                total_files += 1
                try:
                    with open(filepath, 'r', encoding='utf-8', errors='ignore') as f:
                        content = f.read()
                    
                    if 'fscanf' in content and '%s' in content:
                        if self.fix_file(str(filepath)):
                            fixed_count += 1
                            
                except Exception as e:
                    print(f"✗ Error reading {filepath}: {e}")
        
        print(f"\n=== Summary ===")
        print(f"Total files scanned: {total_files}")
        print(f"Files fixed: {fixed_count}")
        print(f"Backups created with .bak extension")

if __name__ == "__main__":
    fixer = FscanfVulnerabilityFixer()
    fixer.run(".")