#!/usr/bin/env python3
"""
Script to automatically add override keywords to virtual functions in derived classes.
Uses clang-tidy for safe AST-based refactoring.
"""

import subprocess
import os
import sys
import glob
import argparse

def run_clang_tidy_fix(file_path):
    """Run clang-tidy to add override keywords."""
    cmd = [
        'clang-tidy',
        '-fix',
        '-checks=modernize-use-override',
        file_path,
        '--',
        '-std=c++17',
        '-I./selforg',
        '-I./selforg/controller',
        '-I./selforg/wirings',
        '-I./selforg/utils',
        '-I./selforg/matrix',
        '-I./selforg/include',
        '-I./selforg/include/selforg',
        '-I/opt/homebrew/include',
        '-I/opt/local/include'
    ]
    
    try:
        result = subprocess.run(cmd, capture_output=True, text=True)
        if result.returncode == 0:
            print(f"✓ Fixed overrides in: {file_path}")
            return True
        else:
            print(f"✗ Error fixing {file_path}: {result.stderr}")
            return False
    except FileNotFoundError:
        print("Error: clang-tidy not found. Please install it first.")
        print("On macOS: brew install llvm")
        sys.exit(1)

def find_cpp_files(directory):
    """Find all C++ source and header files."""
    patterns = ['**/*.cpp', '**/*.h', '**/*.hpp']
    files = []
    for pattern in patterns:
        files.extend(glob.glob(os.path.join(directory, pattern), recursive=True))
    return files

def main():
    parser = argparse.ArgumentParser(description='Add override keywords to virtual functions')
    parser.add_argument('directory', help='Directory to process')
    parser.add_argument('--dry-run', action='store_true', help='Show what would be done without making changes')
    args = parser.parse_args()
    
    # Check if clang-tidy is available
    try:
        subprocess.run(['clang-tidy', '--version'], capture_output=True, check=True)
    except:
        print("Error: clang-tidy not found. Install it with: brew install llvm")
        sys.exit(1)
    
    # Find all C++ files
    cpp_files = find_cpp_files(args.directory)
    print(f"Found {len(cpp_files)} C++ files to process")
    
    if args.dry_run:
        print("Dry run - no files will be modified")
        for f in cpp_files:
            print(f"Would process: {f}")
        return
    
    # Process files
    success_count = 0
    for file_path in cpp_files:
        # Skip generated files and backups
        if any(skip in file_path for skip in ['.bak', 'build/', 'build_opt/', 'build_dbg/']):
            continue
            
        if run_clang_tidy_fix(file_path):
            success_count += 1
    
    print(f"\nProcessed {success_count}/{len(cpp_files)} files successfully")

if __name__ == '__main__':
    main()