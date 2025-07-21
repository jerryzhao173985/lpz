#!/usr/bin/env python3
"""
Fix Generator for Cppcheck Issues
Generates fix suggestions and diffs for various C++17 modernization issues
"""

import re
import difflib
from typing import Dict, List, Tuple, Optional
from pathlib import Path

class FixGenerator:
    def __init__(self):
        self.fix_patterns = {
            'noExplicitConstructor': self._fix_explicit_constructor,
            'missingOverride': self._fix_missing_override,
            'useNullptr': self._fix_use_nullptr,
            'passedByValue': self._fix_passed_by_value,
            'unusedVariable': self._fix_unused_variable,
            'uninitMemberVar': self._fix_uninit_member,
            'redundantAssignment': self._fix_redundant_assignment,
            'uselessCallsCompare': self._fix_useless_compare,
            'postfixOperator': self._fix_postfix_operator,
            'useStlAlgorithm': self._fix_use_stl_algorithm
        }
        
    def generate_fix(self, issue: Dict, code_context: Dict) -> Dict:
        """Generate a fix for a specific issue with diff preview"""
        issue_type = issue.get('id', 'unknown')
        
        if issue_type not in self.fix_patterns:
            return self._generic_fix_suggestion(issue, code_context)
            
        # Get the fix function for this issue type
        fix_func = self.fix_patterns[issue_type]
        
        # Generate the fix
        fix_result = fix_func(issue, code_context)
        
        # Generate diff if we have before/after code
        if fix_result.get('fixed_lines'):
            fix_result['diff'] = self._generate_diff(
                code_context['lines'],
                fix_result['fixed_lines'],
                code_context.get('file', 'unknown')
            )
            
        return fix_result
        
    def _fix_explicit_constructor(self, issue: Dict, context: Dict) -> Dict:
        """Add explicit keyword to single-parameter constructors"""
        target_line = None
        for line in context['lines']:
            if line['is_target']:
                target_line = line
                break
                
        if not target_line:
            return {'success': False, 'reason': 'Target line not found'}
            
        original = target_line['content']
        # Add explicit keyword before constructor
        fixed = re.sub(
            r'(\s*)((?:inline\s+)?)([\w:]+)\s*\(',
            r'\1\2explicit \3(',
            original
        )
        
        if fixed == original:
            # Try another pattern
            fixed = re.sub(
                r'^(\s*)([\w:]+)\s*\(',
                r'\1explicit \2(',
                original
            )
            
        fixed_lines = self._apply_line_fix(context['lines'], target_line['number'], fixed)
        
        return {
            'success': True,
            'description': 'Add explicit keyword to prevent implicit conversions',
            'confidence': 95,
            'fixed_lines': fixed_lines,
            'explanation': 'Single-parameter constructors should be marked explicit to prevent unintended implicit conversions.'
        }
        
    def _fix_missing_override(self, issue: Dict, context: Dict) -> Dict:
        """Add override specifier to virtual functions"""
        target_line = None
        for line in context['lines']:
            if line['is_target']:
                target_line = line
                break
                
        if not target_line:
            return {'success': False, 'reason': 'Target line not found'}
            
        original = target_line['content']
        
        # Add override before semicolon or opening brace
        if ';' in original:
            fixed = re.sub(r'(\s*)(;)', r' override\2', original)
        elif '{' in original:
            fixed = re.sub(r'(\s*)({)', r' override \2', original)
        else:
            # Multi-line declaration, add at end
            fixed = original.rstrip() + ' override'
            
        # Clean up multiple spaces
        fixed = re.sub(r'\s+', ' ', fixed)
        
        fixed_lines = self._apply_line_fix(context['lines'], target_line['number'], fixed)
        
        return {
            'success': True,
            'description': 'Add override specifier to virtual function',
            'confidence': 98,
            'fixed_lines': fixed_lines,
            'explanation': 'Virtual functions that override base class methods should be marked with override for better type safety.'
        }
        
    def _fix_use_nullptr(self, issue: Dict, context: Dict) -> Dict:
        """Replace NULL or 0 with nullptr"""
        target_line = None
        for line in context['lines']:
            if line['is_target']:
                target_line = line
                break
                
        if not target_line:
            return {'success': False, 'reason': 'Target line not found'}
            
        original = target_line['content']
        fixed = original
        
        # Replace NULL with nullptr
        fixed = re.sub(r'\bNULL\b', 'nullptr', fixed)
        
        # Replace pointer assignments of 0 with nullptr (careful pattern)
        # Look for patterns like: ptr = 0, ptr(0), return 0 (in pointer context)
        fixed = re.sub(r'(\w+\s*=\s*)0(\s*[;,)])', r'\1nullptr\2', fixed)
        fixed = re.sub(r'(\w+\()0(\))', r'\1nullptr\2', fixed)
        fixed = re.sub(r'(return\s+)0(\s*;)', r'\1nullptr\2', fixed)
        
        fixed_lines = self._apply_line_fix(context['lines'], target_line['number'], fixed)
        
        return {
            'success': True,
            'description': 'Replace NULL/0 with nullptr',
            'confidence': 90,
            'fixed_lines': fixed_lines,
            'explanation': 'C++11 introduced nullptr as a type-safe null pointer constant. It should be used instead of NULL or 0.'
        }
        
    def _fix_passed_by_value(self, issue: Dict, context: Dict) -> Dict:
        """Convert pass-by-value to pass-by-const-reference for efficiency"""
        target_line = None
        for line in context['lines']:
            if line['is_target']:
                target_line = line
                break
                
        if not target_line:
            return {'success': False, 'reason': 'Target line not found'}
            
        original = target_line['content']
        
        # Extract the parameter that should be passed by reference
        # Look for common patterns: function(Type param) -> function(const Type& param)
        fixed = re.sub(
            r'(\w+)\s+(\w+)(\s*[,)])',
            r'const \1& \2\3',
            original
        )
        
        fixed_lines = self._apply_line_fix(context['lines'], target_line['number'], fixed)
        
        return {
            'success': True,
            'description': 'Pass by const reference instead of value',
            'confidence': 85,
            'fixed_lines': fixed_lines,
            'explanation': 'Large objects should be passed by const reference to avoid unnecessary copying.'
        }
        
    def _fix_unused_variable(self, issue: Dict, context: Dict) -> Dict:
        """Comment out or remove unused variables"""
        target_line = None
        for line in context['lines']:
            if line['is_target']:
                target_line = line
                break
                
        if not target_line:
            return {'success': False, 'reason': 'Target line not found'}
            
        original = target_line['content']
        indent = ' ' * target_line['indent']
        
        # Comment out the line
        fixed = f"{indent}// {original.lstrip()} // UNUSED - TODO: Remove if not needed"
        
        fixed_lines = self._apply_line_fix(context['lines'], target_line['number'], fixed)
        
        return {
            'success': True,
            'description': 'Comment out unused variable',
            'confidence': 70,
            'fixed_lines': fixed_lines,
            'explanation': 'Unused variables should be removed to keep code clean. Commented out for review.',
            'manual_review': True
        }
        
    def _fix_uninit_member(self, issue: Dict, context: Dict) -> Dict:
        """Initialize member variables in constructor"""
        # This is more complex and would need to modify constructor
        return {
            'success': True,
            'description': 'Initialize member variable in constructor initializer list',
            'confidence': 60,
            'manual_fix': True,
            'suggestion': 'Add member initialization in constructor:\n' +
                         'MyClass() : member(0) { ... }',
            'explanation': 'Member variables should be initialized in the constructor initializer list.'
        }
        
    def _fix_redundant_assignment(self, issue: Dict, context: Dict) -> Dict:
        """Remove redundant assignments"""
        target_line = None
        for line in context['lines']:
            if line['is_target']:
                target_line = line
                break
                
        if not target_line:
            return {'success': False, 'reason': 'Target line not found'}
            
        # Comment out redundant assignment
        original = target_line['content']
        indent = ' ' * target_line['indent']
        fixed = f"{indent}// {original.lstrip()} // REDUNDANT - Removed"
        
        fixed_lines = self._apply_line_fix(context['lines'], target_line['number'], fixed)
        
        return {
            'success': True,
            'description': 'Remove redundant assignment',
            'confidence': 80,
            'fixed_lines': fixed_lines,
            'explanation': 'This assignment is redundant and can be safely removed.'
        }
        
    def _fix_useless_compare(self, issue: Dict, context: Dict) -> Dict:
        """Fix useless string comparisons"""
        return {
            'success': True,
            'description': 'Fix string comparison',
            'confidence': 75,
            'manual_fix': True,
            'suggestion': 'Use proper string comparison:\n' +
                         'if (str == "value") instead of if ("value")',
            'explanation': 'String literal comparisons should use proper comparison operators.'
        }
        
    def _fix_postfix_operator(self, issue: Dict, context: Dict) -> Dict:
        """Replace postfix with prefix increment/decrement"""
        target_line = None
        for line in context['lines']:
            if line['is_target']:
                target_line = line
                break
                
        if not target_line:
            return {'success': False, 'reason': 'Target line not found'}
            
        original = target_line['content']
        # Replace i++ with ++i, j-- with --j
        fixed = re.sub(r'(\w+)\+\+', r'++\1', original)
        fixed = re.sub(r'(\w+)--', r'--\1', fixed)
        
        fixed_lines = self._apply_line_fix(context['lines'], target_line['number'], fixed)
        
        return {
            'success': True,
            'description': 'Use prefix increment for better performance',
            'confidence': 90,
            'fixed_lines': fixed_lines,
            'explanation': 'Prefix increment/decrement is more efficient for non-primitive types.'
        }
        
    def _fix_use_stl_algorithm(self, issue: Dict, context: Dict) -> Dict:
        """Suggest STL algorithm usage"""
        return {
            'success': True,
            'description': 'Use STL algorithm',
            'confidence': 70,
            'manual_fix': True,
            'suggestion': 'Consider using STL algorithms:\n' +
                         'std::find, std::transform, std::copy_if, etc.',
            'explanation': 'STL algorithms are often more efficient and expressive than manual loops.'
        }
        
    def _generic_fix_suggestion(self, issue: Dict, context: Dict) -> Dict:
        """Generic fix suggestion for unknown issue types"""
        return {
            'success': True,
            'description': f'Fix {issue.get("id", "issue")}',
            'confidence': 50,
            'manual_fix': True,
            'suggestion': f'Review and fix: {issue.get("message", "See issue description")}',
            'explanation': 'This issue requires manual review and fixing.'
        }
        
    def _apply_line_fix(self, lines: List[Dict], line_number: int, fixed_content: str) -> List[Dict]:
        """Apply a fix to a specific line in the context"""
        fixed_lines = []
        for line in lines:
            if line['number'] == line_number:
                fixed_lines.append({
                    **line,
                    'content': fixed_content,
                    'modified': True
                })
            else:
                fixed_lines.append(line.copy())
        return fixed_lines
        
    def _generate_diff(self, original_lines: List[Dict], fixed_lines: List[Dict], filename: str) -> str:
        """Generate unified diff between original and fixed code"""
        original = [line['content'] for line in original_lines]
        fixed = [line['content'] for line in fixed_lines]
        
        # Add line numbers to make the diff more realistic
        original_with_nums = [f"{i+1}: {line}" for i, line in enumerate(original)]
        fixed_with_nums = [f"{i+1}: {line}" for i, line in enumerate(fixed)]
        
        diff = difflib.unified_diff(
            original,
            fixed,
            fromfile=f'a/{filename}',
            tofile=f'b/{filename}',
            lineterm='',
            n=5  # More context lines
        )
        
        return '\n'.join(diff)
        
    def generate_fix_script(self, issues: List[Dict], dry_run: bool = True) -> str:
        """Generate a script to apply multiple fixes"""
        script = f"""#!/bin/bash
# Generated fix script for {len(issues)} issues
# Run with --apply to actually apply fixes

DRY_RUN={'true' if dry_run else 'false'}

if [ "$1" == "--apply" ]; then
    DRY_RUN=false
    echo "Applying fixes..."
else
    echo "DRY RUN - No files will be modified"
fi

"""
        
        fixes_by_file = {}
        for issue in issues:
            filepath = issue.get('file', '')
            if filepath not in fixes_by_file:
                fixes_by_file[filepath] = []
            fixes_by_file[filepath].append(issue)
            
        for filepath, file_issues in fixes_by_file.items():
            script += f"\n# Fixes for {filepath}\n"
            script += f"echo 'Processing {filepath}...'\n"
            
            for issue in file_issues:
                script += f"# Line {issue.get('line', '?')}: {issue.get('id', 'unknown')}\n"
                
        script += "\necho 'Fix script complete!'\n"
        
        return script


def main():
    """Test the fix generator"""
    generator = FixGenerator()
    
    # Example issue
    issue = {
        'id': 'noExplicitConstructor',
        'file': 'test.cpp',
        'line': 10,
        'message': 'Single-parameter constructor should be explicit'
    }
    
    # Example context
    context = {
        'lines': [
            {'number': 8, 'content': 'class Test {', 'is_target': False, 'indent': 0},
            {'number': 9, 'content': 'public:', 'is_target': False, 'indent': 0},
            {'number': 10, 'content': '    Test(int value);', 'is_target': True, 'indent': 4},
            {'number': 11, 'content': '    ~Test();', 'is_target': False, 'indent': 4},
            {'number': 12, 'content': '};', 'is_target': False, 'indent': 0}
        ]
    }
    
    fix = generator.generate_fix(issue, context)
    print(f"Fix generated: {fix['success']}")
    print(f"Description: {fix['description']}")
    print(f"Confidence: {fix['confidence']}%")
    
    if fix.get('diff'):
        print("\nDiff:")
        print(fix['diff'])


if __name__ == '__main__':
    main()