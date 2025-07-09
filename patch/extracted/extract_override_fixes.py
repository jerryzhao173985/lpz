#!/usr/bin/env python3
"""
Extract override keyword fixes from the patch.
Focus on removing misplaced override keywords and adding them where needed.
"""

import re

def analyze_override_changes(input_file, output_file):
    with open(input_file, 'r') as f:
        lines = f.readlines()
    
    output_lines = []
    i = 0
    files_with_changes = []
    
    # Statistics
    stats = {
        'override_removed': 0,
        'override_added': 0,
        'misplaced_override': 0,
        'override_after_semicolon': 0,
        'override_in_expression': 0
    }
    
    examples = {
        'misplaced': [],
        'correct_addition': [],
        'removed_from_expression': []
    }
    
    while i < len(lines):
        line = lines[i]
        
        if line.startswith('diff --git'):
            file_start = i
            current_file = line.split()[2]
            file_has_override = False
            
            # Skip non-C++ files
            if not any(current_file.endswith(ext) for ext in ['.cpp', '.h', '.hpp', '.cc']):
                j = i + 1
                while j < len(lines) and not lines[j].startswith('diff --git'):
                    j += 1
                i = j
                continue
            
            # Skip documentation
            if any(x in current_file for x in ['docs/', 'README', '.md']):
                j = i + 1
                while j < len(lines) and not lines[j].startswith('diff --git'):
                    j += 1
                i = j
                continue
            
            # Analyze the file diff
            j = i + 1
            while j < len(lines) and not lines[j].startswith('diff --git'):
                if lines[j].startswith('-') and 'override' in lines[j]:
                    stats['override_removed'] += 1
                    
                    # Check for misplaced override
                    if re.search(r'\) override;', lines[j]):
                        stats['override_after_semicolon'] += 1
                        stats['misplaced_override'] += 1
                        file_has_override = True
                        if len(examples['misplaced']) < 5:
                            examples['misplaced'].append((current_file, lines[j].strip()))
                    
                    # Check for override in expressions
                    elif re.search(r'[=+\-*/] override|override[+\-*/]|\w+\s+override\s*;', lines[j]):
                        stats['override_in_expression'] += 1
                        stats['misplaced_override'] += 1
                        file_has_override = True
                        if len(examples['removed_from_expression']) < 5:
                            examples['removed_from_expression'].append((current_file, lines[j].strip()))
                
                elif lines[j].startswith('+') and 'override' in lines[j]:
                    # Check if it's a proper override addition
                    if re.search(r'\)\s*(const)?\s*override\s*(;|{)', lines[j]):
                        stats['override_added'] += 1
                        if len(examples['correct_addition']) < 5:
                            examples['correct_addition'].append((current_file, lines[j].strip()))
                
                j += 1
            
            # Include file if it has override changes
            if file_has_override:
                for k in range(file_start, j):
                    output_lines.append(lines[k])
                files_with_changes.append(current_file)
            
            i = j
        else:
            i += 1
    
    # Write the output patch
    with open(output_file, 'w') as f:
        f.writelines(output_lines)
    
    # Write analysis report
    with open('patch/extracted/override_analysis_report.txt', 'w') as f:
        f.write("OVERRIDE KEYWORD ANALYSIS REPORT\n")
        f.write("=" * 50 + "\n\n")
        
        f.write("Statistics:\n")
        f.write("-" * 30 + "\n")
        for key, value in stats.items():
            f.write(f"{key:25}: {value:6d}\n")
        
        f.write(f"\nTotal files with changes: {len(files_with_changes)}\n")
        
        f.write("\n\nExamples of Misplaced Override (after semicolon):\n")
        f.write("-" * 50 + "\n")
        for file, line in examples['misplaced']:
            f.write(f"{file}:\n  {line}\n\n")
        
        f.write("\n\nExamples of Override in Expressions:\n")
        f.write("-" * 50 + "\n")
        for file, line in examples['removed_from_expression']:
            f.write(f"{file}:\n  {line}\n\n")
        
        f.write("\n\nExamples of Correct Override Additions:\n")
        f.write("-" * 50 + "\n")
        for file, line in examples['correct_addition']:
            f.write(f"{file}:\n  {line}\n\n")
    
    return stats, len(files_with_changes)

if __name__ == "__main__":
    input_file = 'patch/extracted/source_changes.patch'
    output_file = 'patch/extracted/patch_05_override_fixes.patch'
    
    print("Analyzing override keyword changes...")
    stats, file_count = analyze_override_changes(input_file, output_file)
    
    print(f"\nOverride Analysis Results:")
    print(f"  - Override removals: {stats['override_removed']}")
    print(f"  - Override additions: {stats['override_added']}")
    print(f"  - Misplaced overrides: {stats['misplaced_override']}")
    print(f"  - Files affected: {file_count}")
    print(f"  - Output written to: {output_file}")
    print(f"  - Analysis report: override_analysis_report.txt")