#!/usr/bin/env python3
"""
Comprehensive warning analysis tool for LPZRobots codebase.
Identifies and categorizes common C++ warning patterns.
"""

import os
import re
import sys
from collections import defaultdict
from pathlib import Path

class WarningAnalyzer:
    def __init__(self):
        self.warning_patterns = {
            'missing_override': {
                'pattern': re.compile(r'virtual\s+[^;{]+\([^)]*\)\s*(?:const\s*)?(?:;|(?:\s*=\s*0\s*;)|\s*\{)'),
                'description': 'Virtual functions without override keyword',
                'count': 0,
                'files': []
            },
            'c_style_cast': {
                'pattern': re.compile(r'(?<![a-zA-Z_])\(\s*(int|double|float|char\s*\*|void\s*\*|unsigned|long|short)\s*\)'),
                'description': 'C-style casts instead of C++ casts',
                'count': 0,
                'files': []
            },
            'uninitialized_member': {
                'pattern': re.compile(r'^\s*((?:int|double|float|bool|char|unsigned|long|short)\s+\w+|[a-zA-Z_]\w*\s*\*\s*\w+)\s*;', re.MULTILINE),
                'description': 'Potentially uninitialized member variables',
                'count': 0,
                'files': []
            },
            'non_const_getter': {
                'pattern': re.compile(r'get\w+\s*\(\s*\)\s*(?!const)\s*\{'),
                'description': 'Getter methods not marked const',
                'count': 0,
                'files': []
            },
            'auto_ref_loop': {
                'pattern': re.compile(r'for\s*\(\s*auto\s*&\s*\w+\s*:\s*[^)]+\)'),
                'description': 'Range-based for loops with auto& that might need const auto&',
                'count': 0,
                'files': []
            },
            'shadow_variable': {
                'pattern': re.compile(r'^\s*(?:int|double|float|bool|char|unsigned|long|short|auto)\s+(\w+)', re.MULTILINE),
                'description': 'Potential variable shadowing',
                'count': 0,
                'files': []
            },
            'old_style_include': {
                'pattern': re.compile(r'#include\s*<(stdio\.h|stdlib\.h|string\.h|math\.h|ctype\.h)>'),
                'description': 'C headers instead of C++ headers',
                'count': 0,
                'files': []
            },
            'null_instead_nullptr': {
                'pattern': re.compile(r'(?<![a-zA-Z_])NULL(?![a-zA-Z_])'),
                'description': 'NULL instead of nullptr',
                'count': 0,
                'files': []
            }
        }
        
        self.file_issues = defaultdict(list)
        self.total_issues = 0
        
    def check_missing_override(self, content, filepath):
        """Check for virtual functions without override in derived classes"""
        matches = []
        
        # Skip if this looks like a base class (has pure virtual functions)
        if '= 0;' in content and content.count('= 0;') > 2:
            return matches
            
        for match in self.warning_patterns['missing_override']['pattern'].finditer(content):
            line = match.group(0)
            # Skip if already has override
            if 'override' in line:
                continue
            # Skip destructors
            if '~' in line:
                continue
            # Skip if it's likely in a base class
            if '= 0' in line:
                continue
                
            # Get line number
            line_num = content[:match.start()].count('\n') + 1
            matches.append((line_num, line.strip()))
            
        return matches
    
    def check_c_style_casts(self, content, filepath):
        """Check for C-style casts"""
        matches = []
        
        for match in self.warning_patterns['c_style_cast']['pattern'].finditer(content):
            # Get surrounding context
            start = max(0, match.start() - 20)
            end = min(len(content), match.end() + 20)
            context = content[start:end]
            
            # Skip function declarations/definitions
            if '(' in content[start:match.start()] and ')' in content[match.end():end]:
                continue
                
            line_num = content[:match.start()].count('\n') + 1
            matches.append((line_num, match.group(0)))
            
        return matches
    
    def check_uninitialized_members(self, content, filepath):
        """Check for potentially uninitialized member variables"""
        matches = []
        
        # Only check in class/struct definitions
        class_pattern = re.compile(r'(class|struct)\s+\w+[^{]*\{([^}]*)\}', re.DOTALL)
        
        for class_match in class_pattern.finditer(content):
            class_body = class_match.group(2)
            class_start = class_match.start(2)
            
            for match in self.warning_patterns['uninitialized_member']['pattern'].finditer(class_body):
                var_decl = match.group(1)
                # Skip if it has an initializer
                if '=' in var_decl or '{' in var_decl:
                    continue
                # Skip static members
                if 'static' in var_decl:
                    continue
                    
                line_num = content[:class_start + match.start()].count('\n') + 1
                matches.append((line_num, var_decl.strip()))
                
        return matches
    
    def analyze_file(self, filepath):
        """Analyze a single file for all warning patterns"""
        try:
            with open(filepath, 'r', encoding='utf-8', errors='ignore') as f:
                content = f.read()
        except Exception as e:
            print(f"Error reading {filepath}: {e}")
            return
            
        relative_path = os.path.relpath(filepath)
        
        # Check for missing override
        override_issues = self.check_missing_override(content, filepath)
        if override_issues:
            self.warning_patterns['missing_override']['count'] += len(override_issues)
            self.warning_patterns['missing_override']['files'].append(relative_path)
            self.file_issues[relative_path].extend([
                f"Line {line}: Missing override - {issue}"
                for line, issue in override_issues
            ])
            
        # Check for C-style casts
        cast_issues = self.check_c_style_casts(content, filepath)
        if cast_issues:
            self.warning_patterns['c_style_cast']['count'] += len(cast_issues)
            self.warning_patterns['c_style_cast']['files'].append(relative_path)
            self.file_issues[relative_path].extend([
                f"Line {line}: C-style cast - {issue}"
                for line, issue in cast_issues
            ])
            
        # Check for uninitialized members
        init_issues = self.check_uninitialized_members(content, filepath)
        if init_issues:
            self.warning_patterns['uninitialized_member']['count'] += len(init_issues)
            self.warning_patterns['uninitialized_member']['files'].append(relative_path)
            self.file_issues[relative_path].extend([
                f"Line {line}: Potentially uninitialized - {issue}"
                for line, issue in init_issues
            ])
            
        # Check for other patterns
        for pattern_name, pattern_info in self.warning_patterns.items():
            if pattern_name in ['missing_override', 'c_style_cast', 'uninitialized_member']:
                continue
                
            matches = list(pattern_info['pattern'].finditer(content))
            if matches:
                pattern_info['count'] += len(matches)
                pattern_info['files'].append(relative_path)
                
        self.total_issues += len(self.file_issues[relative_path])
        
    def analyze_directory(self, directory):
        """Analyze all C++ files in a directory recursively"""
        for root, dirs, files in os.walk(directory):
            # Skip build directories and hidden directories
            dirs[:] = [d for d in dirs if not d.startswith('.') and 
                      d not in ['build', 'build_opt', 'build_dbg', 'CMakeFiles']]
            
            for file in files:
                if file.endswith(('.cpp', '.h', '.hpp', '.cc')):
                    filepath = os.path.join(root, file)
                    self.analyze_file(filepath)
                    
    def generate_report(self):
        """Generate a comprehensive report of all warnings found"""
        report = []
        report.append("=" * 80)
        report.append("LPZRobots Warning Analysis Report")
        report.append("=" * 80)
        report.append(f"\nTotal issues found: {self.total_issues}\n")
        
        # Summary by warning type
        report.append("Summary by Warning Type:")
        report.append("-" * 40)
        for pattern_name, pattern_info in sorted(self.warning_patterns.items(), 
                                                key=lambda x: x[1]['count'], 
                                                reverse=True):
            if pattern_info['count'] > 0:
                report.append(f"{pattern_info['description']}: {pattern_info['count']} issues")
                report.append(f"  Affected files: {len(set(pattern_info['files']))}")
                
        # Most problematic files
        report.append("\n\nMost Problematic Files:")
        report.append("-" * 40)
        file_issue_counts = [(f, len(issues)) for f, issues in self.file_issues.items()]
        file_issue_counts.sort(key=lambda x: x[1], reverse=True)
        
        for filepath, count in file_issue_counts[:20]:
            report.append(f"{filepath}: {count} issues")
            
        # Detailed issues by file
        report.append("\n\nDetailed Issues by File:")
        report.append("-" * 40)
        for filepath in sorted(self.file_issues.keys()):
            issues = self.file_issues[filepath]
            if issues:
                report.append(f"\n{filepath}:")
                for issue in issues[:10]:  # Limit to first 10 issues per file
                    report.append(f"  {issue}")
                if len(issues) > 10:
                    report.append(f"  ... and {len(issues) - 10} more issues")
                    
        return "\n".join(report)
        
    def save_report(self, filename="warning_analysis_report.txt"):
        """Save the report to a file"""
        report = self.generate_report()
        with open(filename, 'w') as f:
            f.write(report)
        print(f"Report saved to {filename}")
        
def main():
    if len(sys.argv) < 2:
        print("Usage: python3 analyze_warnings.py <directory>")
        print("Example: python3 analyze_warnings.py /Users/jerry/lpzrobot_mac")
        sys.exit(1)
        
    directory = sys.argv[1]
    if not os.path.isdir(directory):
        print(f"Error: {directory} is not a valid directory")
        sys.exit(1)
        
    analyzer = WarningAnalyzer()
    print(f"Analyzing {directory}...")
    
    # Analyze specific subdirectories
    for subdir in ['selforg', 'ode_robots']:
        full_path = os.path.join(directory, subdir)
        if os.path.exists(full_path):
            print(f"  Analyzing {subdir}...")
            analyzer.analyze_directory(full_path)
            
    # Generate and save report
    print("\nGenerating report...")
    analyzer.save_report()
    
    # Print summary
    print(f"\nAnalysis complete!")
    print(f"Total issues found: {analyzer.total_issues}")
    print("\nTop warning types:")
    for pattern_name, pattern_info in sorted(analyzer.warning_patterns.items(), 
                                            key=lambda x: x[1]['count'], 
                                            reverse=True)[:5]:
        if pattern_info['count'] > 0:
            print(f"  {pattern_info['description']}: {pattern_info['count']}")

if __name__ == "__main__":
    main()