#!/usr/bin/env python3
"""
Code Context Extractor for Cppcheck Dashboard
Extracts code context around issues with function/class boundaries
"""

import re
from pathlib import Path
from typing import Dict, List, Tuple, Optional
import json

class CodeContextExtractor:
    def __init__(self, project_root: Path):
        self.project_root = project_root
        
    def extract_context(self, filepath: str, line_number: int, context_size: int = 15) -> Dict:
        """Extract code context around a specific line with smart boundaries"""
        try:
            # Handle both absolute and relative paths
            if filepath.startswith('/'):
                full_path = Path(filepath)
            else:
                full_path = self.project_root / filepath
                
            if not full_path.exists():
                return self._error_context(f"File not found: {filepath} (tried: {full_path})")
                
            with open(full_path, 'r', encoding='utf-8') as f:
                lines = f.readlines()
                
            if line_number < 1 or line_number > len(lines):
                return self._error_context(f"Invalid line number: {line_number}")
                
            # Find function/class boundaries
            func_start, func_end = self._find_function_boundaries(lines, line_number - 1)
            class_info = self._find_class_info(lines, line_number - 1)
            
            # Determine context range
            start_line = max(0, line_number - context_size - 1)
            end_line = min(len(lines), line_number + context_size)
            
            # Expand to include full function if possible
            if func_start is not None:
                start_line = min(start_line, func_start)
            if func_end is not None:
                end_line = max(end_line, func_end + 1)
                
            # Extract lines with metadata
            context_lines = []
            for i in range(start_line, end_line):
                context_lines.append({
                    'number': i + 1,
                    'content': lines[i].rstrip('\n'),
                    'is_target': i == line_number - 1,
                    'indent': len(lines[i]) - len(lines[i].lstrip())
                })
                
            return {
                'success': True,
                'file': filepath,
                'target_line': line_number,
                'lines': context_lines,
                'function': self._extract_function_signature(lines, func_start) if func_start else None,
                'class': class_info,
                'language': 'cpp'
            }
            
        except Exception as e:
            return self._error_context(f"Error reading file: {str(e)}")
            
    def _find_function_boundaries(self, lines: List[str], target_line: int) -> Tuple[Optional[int], Optional[int]]:
        """Find the start and end of the function containing the target line"""
        # Simple heuristic: look for { and } with proper nesting
        brace_count = 0
        func_start = None
        in_function = False
        
        # Search backwards for function start
        for i in range(target_line, -1, -1):
            line = lines[i].strip()
            
            # Skip comments and preprocessor directives
            if line.startswith('//') or line.startswith('#'):
                continue
                
            # Count braces
            brace_count += line.count('}') - line.count('{')
            
            # Function signature patterns
            if (re.match(r'^\s*(\w+\s+)*\w+\s*\(.*\)\s*(const)?\s*(override)?\s*{?\s*$', line) or
                re.match(r'^\s*(\w+::)*\w+\s*\(.*\)\s*(const)?\s*{?\s*$', line)):
                if brace_count <= 0:
                    func_start = i
                    break
                    
        # Search forward for function end
        if func_start is not None:
            brace_count = 0
            for i in range(func_start, len(lines)):
                line = lines[i]
                brace_count += line.count('{') - line.count('}')
                if brace_count == 0 and '{' in lines[func_start:i+1]:
                    return func_start, i
                    
        return func_start, None
        
    def _find_class_info(self, lines: List[str], target_line: int) -> Optional[Dict]:
        """Find class information for the target line"""
        # Search backwards for class declaration
        for i in range(target_line, -1, -1):
            line = lines[i].strip()
            match = re.match(r'^(class|struct)\s+(\w+)', line)
            if match:
                return {
                    'type': match.group(1),
                    'name': match.group(2),
                    'line': i + 1
                }
        return None
        
    def _extract_function_signature(self, lines: List[str], func_start: int) -> str:
        """Extract clean function signature"""
        signature = []
        i = func_start
        
        # Collect lines until we find the opening brace or semicolon
        while i < len(lines):
            line = lines[i].strip()
            signature.append(line)
            if '{' in line or ';' in line:
                break
            i += 1
            
        full_sig = ' '.join(signature)
        # Clean up the signature
        full_sig = re.sub(r'\s+', ' ', full_sig)
        full_sig = re.sub(r'\s*{\s*$', '', full_sig)
        
        return full_sig
        
    def _error_context(self, error_msg: str) -> Dict:
        """Return error context"""
        return {
            'success': False,
            'error': error_msg,
            'lines': []
        }
        
    def extract_fix_context(self, filepath: str, line_number: int, issue_type: str) -> Dict:
        """Extract context specifically for generating fixes"""
        context = self.extract_context(filepath, line_number, context_size=5)
        
        if not context['success']:
            return context
            
        # Add fix-specific information
        context['fix_info'] = self._get_fix_info(issue_type, context['lines'], line_number)
        
        return context
        
    def _get_fix_info(self, issue_type: str, lines: List[Dict], target_line: int) -> Dict:
        """Get information needed to generate fixes"""
        target_content = None
        for line in lines:
            if line['number'] == target_line:
                target_content = line['content']
                break
                
        if not target_content:
            return {}
            
        fix_info = {
            'issue_type': issue_type,
            'original_line': target_content
        }
        
        # Analyze based on issue type
        if issue_type == 'noExplicitConstructor':
            # Find constructor declaration
            match = re.search(r'(\w+)\s*\((.*?)\)', target_content)
            if match:
                fix_info['constructor_name'] = match.group(1)
                fix_info['parameters'] = match.group(2)
                
        elif issue_type == 'missingOverride':
            # Find virtual function
            match = re.search(r'virtual\s+(.+?)(\s+const)?\s*;', target_content)
            if match:
                fix_info['function_signature'] = match.group(1)
                fix_info['is_const'] = bool(match.group(2))
                
        elif issue_type == 'useNullptr':
            # Find NULL usage
            fix_info['has_null'] = 'NULL' in target_content
            fix_info['has_zero'] = re.search(r'\b0\b', target_content) is not None
            
        return fix_info


def main():
    """Test the extractor"""
    import sys
    
    if len(sys.argv) < 3:
        print("Usage: code_context_extractor.py <filepath> <line_number>")
        sys.exit(1)
        
    project_root = Path(__file__).parent.parent.parent
    extractor = CodeContextExtractor(project_root)
    
    filepath = sys.argv[1]
    line_number = int(sys.argv[2])
    
    context = extractor.extract_context(filepath, line_number)
    
    if context['success']:
        print(f"Code context for {filepath}:{line_number}")
        print(f"Function: {context.get('function', 'N/A')}")
        if context.get('class'):
            print(f"Class: {context['class']['name']}")
        print("-" * 80)
        
        for line in context['lines']:
            marker = '>>>' if line['is_target'] else '   '
            print(f"{marker} {line['number']:4d} | {line['content']}")
    else:
        print(f"Error: {context['error']}")


if __name__ == '__main__':
    main()