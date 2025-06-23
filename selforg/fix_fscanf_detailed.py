#!/usr/bin/env python3
"""
Fix fscanf vulnerabilities in the selforg component by adding field width limits.
This prevents buffer overflow attacks when reading string data.
"""

import os
import re
import shutil
import sys
from datetime import datetime
from pathlib import Path

# ANSI color codes
RED = '\033[0;31m'
GREEN = '\033[0;32m'
YELLOW = '\033[1;33m'
BLUE = '\033[0;34m'
NC = '\033[0m'  # No Color

class FscanfFixer:
    def __init__(self):
        self.backup_dir = f"fscanf_backup_{datetime.now().strftime('%Y%m%d_%H%M%S')}"
        self.fixes_applied = 0
        self.files_processed = 0
        
        # Pattern to find vulnerable fscanf calls
        self.fscanf_pattern = re.compile(r'fscanf\s*\([^,]+,\s*"([^"]*%s[^"]*)"')
        
        # Pattern to find buffer declarations
        self.buffer_pattern = re.compile(r'char\s+(\w+)\s*\[\s*(\d+)\s*\]')
        
    def find_buffer_size(self, lines, buffer_name, current_line_idx):
        """Find the size of a buffer by looking for its declaration."""
        # Search backwards from current line
        for i in range(current_line_idx, max(0, current_line_idx - 50), -1):
            match = self.buffer_pattern.search(lines[i])
            if match and match.group(1) == buffer_name:
                return int(match.group(2)) - 1  # Leave room for null terminator
        
        # Search forwards (in case declaration is after usage)
        for i in range(current_line_idx, min(len(lines), current_line_idx + 20)):
            match = self.buffer_pattern.search(lines[i])
            if match and match.group(1) == buffer_name:
                return int(match.group(2)) - 1
        
        # Default safe size if buffer not found
        return 127
    
    def fix_fscanf_line(self, line, lines, line_idx):
        """Fix a single line containing vulnerable fscanf."""
        # Find fscanf calls with %s
        match = self.fscanf_pattern.search(line)
        if not match or '%s' not in match.group(1):
            return line, False
        
        format_string = match.group(1)
        
        # Skip if already has field width
        if re.search(r'%\d+s', format_string):
            return line, False
        
        # Try to find the buffer name
        buffer_match = re.search(r'fscanf\s*\([^,]+,[^,]+,\s*(\w+)\s*\)', line)
        if buffer_match:
            buffer_name = buffer_match.group(1)
            buffer_size = self.find_buffer_size(lines, buffer_name, line_idx)
        else:
            buffer_size = 127  # Safe default
        
        # Replace %s with %NNNs
        new_format = format_string.replace('%s', f'%{buffer_size}s')
        new_line = line.replace(f'"{format_string}"', f'"{new_format}"')
        
        return new_line, True
    
    def process_file(self, filepath):
        """Process a single file to fix fscanf vulnerabilities."""
        with open(filepath, 'r') as f:
            lines = f.readlines()
        
        modified = False
        new_lines = []
        
        for i, line in enumerate(lines):
            if 'fscanf' in line and '%s' in line:
                fixed_line, was_fixed = self.fix_fscanf_line(line, lines, i)
                if was_fixed:
                    # Add comment before the fixed line
                    indent = len(line) - len(line.lstrip())
                    comment = ' ' * indent + '// SECURITY FIX: Added field width limit to prevent buffer overflow\n'
                    new_lines.append(comment)
                    new_lines.append(fixed_line)
                    modified = True
                    self.fixes_applied += 1
                    print(f"{YELLOW}  Line {i+1}: Fixed vulnerable fscanf{NC}")
                else:
                    new_lines.append(line)
            else:
                new_lines.append(line)
        
        return new_lines, modified
    
    def find_vulnerable_files(self, root_dir):
        """Find all files with vulnerable fscanf patterns."""
        vulnerable_files = []
        
        for root, dirs, files in os.walk(root_dir):
            for file in files:
                if file.endswith(('.c', '.cpp', '.cc', '.cxx', '.h', '.hpp')):
                    filepath = os.path.join(root, file)
                    try:
                        with open(filepath, 'r') as f:
                            content = f.read()
                            if 'fscanf' in content and '%s' in content:
                                # Check if it's actually vulnerable
                                if re.search(r'fscanf\s*\([^)]+"%s"', content):
                                    vulnerable_files.append(filepath)
                    except Exception as e:
                        print(f"{RED}Error reading {filepath}: {e}{NC}")
        
        return vulnerable_files
    
    def create_backup(self, filepath):
        """Create a backup of the file."""
        os.makedirs(self.backup_dir, exist_ok=True)
        backup_path = os.path.join(self.backup_dir, os.path.basename(filepath))
        # Add number suffix if file already exists
        if os.path.exists(backup_path):
            base, ext = os.path.splitext(backup_path)
            i = 1
            while os.path.exists(f"{base}_{i}{ext}"):
                i += 1
            backup_path = f"{base}_{i}{ext}"
        shutil.copy2(filepath, backup_path)
        return backup_path
    
    def show_vulnerable_lines(self, filepath):
        """Display vulnerable lines in a file."""
        with open(filepath, 'r') as f:
            lines = f.readlines()
        
        for i, line in enumerate(lines):
            if 'fscanf' in line and re.search(r'fscanf\s*\([^)]+"%s"', line):
                print(f"    Line {i+1}: {line.strip()}")
    
    def run(self, root_dir='.'):
        """Main execution function."""
        print(f"{GREEN}=== fscanf Vulnerability Fixer ==={NC}")
        print(f"{BLUE}Scanning directory: {root_dir}{NC}\n")
        
        # Find vulnerable files
        vulnerable_files = self.find_vulnerable_files(root_dir)
        
        if not vulnerable_files:
            print(f"{GREEN}No vulnerable fscanf patterns found!{NC}")
            return
        
        # Report findings
        print(f"{RED}Found {len(vulnerable_files)} files with potential vulnerabilities:{NC}")
        for filepath in vulnerable_files:
            rel_path = os.path.relpath(filepath, root_dir)
            print(f"\n{YELLOW}File: {rel_path}{NC}")
            self.show_vulnerable_lines(filepath)
        
        print(f"\n{YELLOW}Total vulnerable files: {len(vulnerable_files)}{NC}")
        
        # Ask for confirmation
        response = input("\nDo you want to fix these vulnerabilities? (y/n): ")
        if response.lower() != 'y':
            print("Aborted.")
            return
        
        # Process each file
        print(f"\n{GREEN}Processing files...{NC}")
        for filepath in vulnerable_files:
            rel_path = os.path.relpath(filepath, root_dir)
            print(f"\n{BLUE}Processing: {rel_path}{NC}")
            
            # Create backup
            backup_path = self.create_backup(filepath)
            
            # Process file
            new_lines, modified = self.process_file(filepath)
            
            if modified:
                # Write fixed file
                with open(filepath, 'w') as f:
                    f.writelines(new_lines)
                print(f"{GREEN}  ✓ Fixed and saved{NC}")
                self.files_processed += 1
            else:
                print(f"{YELLOW}  No changes needed{NC}")
        
        # Summary
        print(f"\n{GREEN}=== Summary ==={NC}")
        print(f"Files processed: {self.files_processed}")
        print(f"Fixes applied: {self.fixes_applied}")
        print(f"Backups saved in: {self.backup_dir}")
        
        # Verification
        print(f"\n{YELLOW}Verifying fixes...{NC}")
        remaining = self.find_vulnerable_files(root_dir)
        if not remaining:
            print(f"{GREEN}✓ All vulnerabilities have been fixed!{NC}")
        else:
            print(f"{RED}✗ {len(remaining)} files still have vulnerabilities{NC}")
        
        print(f"\n{YELLOW}Recommended next steps:{NC}")
        print("1. Review changes: git diff")
        print("2. Compile and test the code")
        print("3. Consider using safer alternatives like fgets() + sscanf()")
        print("4. Add input validation after parsing")

if __name__ == "__main__":
    fixer = FscanfFixer()
    root_dir = sys.argv[1] if len(sys.argv) > 1 else '.'
    fixer.run(root_dir)