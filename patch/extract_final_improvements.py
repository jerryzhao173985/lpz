#!/usr/bin/env python3
"""Extract final remaining improvements from big.patch"""

import re
import os
from collections import defaultdict

def analyze_remaining_changes(patch_file):
    """Thoroughly analyze big.patch for remaining valid improvements"""
    
    improvements = {
        'const_methods': [],
        'member_init': [],
        'nullptr_comparisons': [],
        'virtual_destructors': [],
        'unused_params': [],
        'includes_cleanup': [],
        'whitespace': [],
        'explicit_single_param': [],
        'static_members': [],
        'enum_class': [],
    }
    
    current_file = None
    in_diff = False
    line_num = 0
    
    # Files to skip
    skip_patterns = [
        r'\.cache/clangd/',
        r'\.git',
        r'backup_',
        r'dist/',
        r'\.github/',
        r'CLAUDE\.md',
        r'README\.md',
        r'\.clang-',
        r'patch/',
        r'\.idx$',
        r'\.png$',
        r'\.jpg$',
    ]
    
    with open(patch_file, 'r', encoding='utf-8', errors='ignore') as f:
        for line in f:
            line_num += 1
            
            # Skip first 30k lines (binary files)
            if line_num < 30000:
                continue
                
            # Track current file
            if line.startswith('diff --git'):
                match = re.search(r'b/(.+?)(?:\s|$)', line)
                if match:
                    current_file = match.group(1)
                    # Skip if matches skip patterns
                    if any(re.search(pattern, current_file) for pattern in skip_patterns):
                        current_file = None
                        in_diff = False
                        continue
                    in_diff = True
            
            if not current_file or not in_diff:
                continue
            
            # Only process C++ source files
            if not (current_file.endswith(('.cpp', '.h', '.hpp', '.cc'))):
                continue
            
            # Look for improvements in diff lines
            if line.startswith('+') and not line.startswith('+++'):
                content = line[1:]
                
                # 1. Const methods - methods that should be const
                if re.search(r'\)\s*{\s*return\s+\w+;?\s*}(?!\s*const)', content):
                    if 'get' in content or 'is' in content or 'has' in content:
                        improvements['const_methods'].append({
                            'file': current_file,
                            'line': line_num,
                            'content': content.rstrip()
                        })
                
                # 2. Member initialization - constructors using initializer lists
                if ': ' in content and re.search(r'^\s*\w+\s*\([^)]*\)\s*:', content):
                    improvements['member_init'].append({
                        'file': current_file,
                        'line': line_num,
                        'content': content.rstrip()
                    })
                
                # 3. Virtual destructors with override
                if re.search(r'virtual\s+~\w+\s*\(\s*\)\s*override', content):
                    improvements['virtual_destructors'].append({
                        'file': current_file,
                        'line': line_num,
                        'content': content.rstrip()
                    })
                
                # 4. Explicit single parameter constructors
                if re.search(r'explicit\s+\w+\s*\([^,)]+\)', content) and 'explicit if' not in content:
                    improvements['explicit_single_param'].append({
                        'file': current_file,
                        'line': line_num,
                        'content': content.rstrip()
                    })
                
                # 5. Static member functions
                if re.search(r'static\s+\w+\s+\w+\s*\(', content):
                    improvements['static_members'].append({
                        'file': current_file,
                        'line': line_num,
                        'content': content.rstrip()
                    })
                
                # 6. enum class instead of enum
                if re.search(r'enum\s+class\s+\w+', content):
                    improvements['enum_class'].append({
                        'file': current_file,
                        'line': line_num,
                        'content': content.rstrip()
                    })
                
                # 7. Unused parameter names
                if re.search(r'\(\s*\w+\s+/\*\s*\w+\s*\*/\s*[,)]', content):
                    improvements['unused_params'].append({
                        'file': current_file,
                        'line': line_num,
                        'content': content.rstrip()
                    })
    
    return improvements

def filter_valid_improvements(improvements):
    """Filter out already applied or invalid improvements"""
    
    filtered = defaultdict(list)
    
    # Check each category
    for category, items in improvements.items():
        for item in items:
            # Skip if it's a corrupted override pattern
            if 'override override' in item['content']:
                continue
            if 'override;' in item['content'] and ');' not in item['content']:
                continue
            
            # Skip nullptr comparisons we've already fixed
            if category == 'nullptr_comparisons' and 'for' in item['content']:
                continue
                
            # Skip virtual destructors with override (we removed these)
            if category == 'virtual_destructors':
                continue
                
            # Add valid improvements
            filtered[category].append(item)
    
    return filtered

def create_improvement_patches(improvements):
    """Create specific patches for each improvement category"""
    
    for category, items in improvements.items():
        if not items:
            continue
            
        # Group by file
        by_file = defaultdict(list)
        for item in items:
            by_file[item['file']].append(item)
        
        print(f"\n{category}: {len(items)} improvements in {len(by_file)} files")
        
        # Show samples
        for file, file_items in list(by_file.items())[:3]:
            print(f"  {file}:")
            for item in file_items[:2]:
                print(f"    Line {item['line']}: {item['content'][:80]}...")

def main():
    print("Analyzing big.patch for remaining improvements...")
    
    improvements = analyze_remaining_changes('patch/big.patch')
    
    # Filter out invalid/already applied changes
    valid_improvements = filter_valid_improvements(improvements)
    
    # Print summary
    total = sum(len(items) for items in valid_improvements.values())
    print(f"\nTotal remaining improvements: {total}")
    
    # Create patches
    create_improvement_patches(valid_improvements)
    
    # Detailed breakdown
    print("\nDetailed breakdown:")
    for category, items in valid_improvements.items():
        if items:
            print(f"\n{category}: {len(items)} changes")
            files = set(item['file'] for item in items)
            print(f"  Files affected: {len(files)}")
            print(f"  Sample files: {', '.join(list(files)[:5])}")

if __name__ == "__main__":
    main()