#!/usr/bin/env python3
"""
Analyze the patch to find different types of C++ modernization changes.
This helps identify what modernizations are in the patch beyond typedef.
"""

import re
from collections import defaultdict

def analyze_patch(patch_file):
    with open(patch_file, 'r') as f:
        lines = f.readlines()
    
    stats = defaultdict(int)
    examples = defaultdict(list)
    current_file = None
    
    for i, line in enumerate(lines):
        if line.startswith('diff --git'):
            current_file = line.split()[2]
            continue
            
        # Skip non-code changes
        if not (line.startswith('+') or line.startswith('-')):
            continue
            
        # Skip empty lines and comments
        stripped = line[1:].strip()
        if not stripped or stripped.startswith('//'):
            continue
        
        # Member initialization in constructor
        if re.search(r':\s*\w+\([^)]*\)', line):
            if line.startswith('+') and current_file and current_file.endswith(('.cpp', '.h')):
                stats['member_init'] += 1
                if len(examples['member_init']) < 3:
                    examples['member_init'].append((current_file, line.strip()))
        
        # explicit constructor
        if 'explicit' in line and ('(' in line or 'class' in line or 'struct' in line):
            stats['explicit'] += 1
            if len(examples['explicit']) < 3:
                examples['explicit'].append((current_file, line.strip()))
        
        # override specifier
        if 'override' in line and not 'override;' in line:
            if line.startswith('+'):
                stats['override_added'] += 1
            elif line.startswith('-'):
                stats['override_removed'] += 1
            if len(examples['override']) < 3:
                examples['override'].append((current_file, line.strip()))
        
        # static_cast
        if 'static_cast<' in line:
            stats['static_cast'] += 1
            if len(examples['static_cast']) < 3:
                examples['static_cast'].append((current_file, line.strip()))
        
        # const correctness
        if re.search(r'\bconst\b', line) and line.startswith('+'):
            stats['const_added'] += 1
        
        # noexcept
        if 'noexcept' in line:
            stats['noexcept'] += 1
            if len(examples['noexcept']) < 3:
                examples['noexcept'].append((current_file, line.strip()))
        
        # throw()
        if 'throw()' in line:
            stats['throw_spec'] += 1
            if len(examples['throw_spec']) < 3:
                examples['throw_spec'].append((current_file, line.strip()))
        
        # auto keyword
        if re.search(r'\bauto\b', line) and line.startswith('+'):
            stats['auto'] += 1
        
        # range-based for
        if re.search(r'for\s*\([^:]+:[^)]+\)', line):
            stats['range_for'] += 1
            if len(examples['range_for']) < 3:
                examples['range_for'].append((current_file, line.strip()))
    
    return stats, examples

def find_uninitialized_fixes(patch_file):
    """Look specifically for member initialization fixes."""
    with open(patch_file, 'r') as f:
        lines = f.readlines()
    
    init_patterns = []
    current_file = None
    current_class = None
    
    for i, line in enumerate(lines):
        if line.startswith('diff --git'):
            current_file = line.split()[2]
            continue
        
        # Look for constructor with initialization list
        if re.search(r'^\+.*\w+::\w+\([^)]*\)\s*:', line):
            # Found a constructor with init list
            j = i
            init_list = []
            while j < len(lines) and not lines[j].strip().endswith('{'):
                if lines[j].startswith('+'):
                    init_list.append(lines[j])
                j += 1
            
            if len(init_list) > 1:  # Has initialization
                init_patterns.append({
                    'file': current_file,
                    'line': i,
                    'init_list': init_list
                })
    
    return init_patterns

if __name__ == "__main__":
    patch_file = 'patch/extracted/source_changes.patch'
    
    print("Analyzing C++ modernizations in the patch...\n")
    
    stats, examples = analyze_patch(patch_file)
    
    print("Modernization Statistics:")
    print("-" * 40)
    for key, count in sorted(stats.items()):
        print(f"{key:20}: {count:6d}")
    
    print("\n\nExamples:")
    print("-" * 40)
    for key, example_list in examples.items():
        if example_list:
            print(f"\n{key}:")
            for file, line in example_list:
                print(f"  {file}: {line}")
    
    print("\n\nSearching for member initialization patterns...")
    init_patterns = find_uninitialized_fixes(patch_file)
    print(f"Found {len(init_patterns)} constructors with initialization lists")
    
    if init_patterns:
        print("\nFirst few examples:")
        for pattern in init_patterns[:3]:
            print(f"\nFile: {pattern['file']}")
            print("Init list:")
            for line in pattern['init_list'][:5]:
                print(f"  {line.strip()}")