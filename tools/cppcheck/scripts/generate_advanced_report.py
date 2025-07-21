#!/usr/bin/env python3
"""
Advanced Cppcheck Report Generator
Creates comprehensive, interactive HTML reports with advanced analytics
"""

import json
import sys
from pathlib import Path
from datetime import datetime
from typing import Dict, List, Tuple
from collections import defaultdict, Counter
import re
import hashlib

# Constants
SCRIPT_DIR = Path(__file__).parent
TOOLS_DIR = SCRIPT_DIR.parent
PROJECT_ROOT = TOOLS_DIR.parent.parent

class AdvancedReportGenerator:
    def __init__(self, report_data: Dict):
        self.data = report_data
        self.issues = report_data.get('issues', [])
        self.stats = report_data.get('stats', {})
        self.profile = report_data.get('profile', 'unknown')
        
    def generate_advanced_html(self, output_path: Path):
        """Generate advanced HTML report with comprehensive analytics"""
        # Perform advanced analysis
        analytics = self._perform_advanced_analytics()
        
        html_content = self._generate_html_structure(analytics)
        
        with open(output_path, 'w') as f:
            f.write(html_content)
            
        print(f"Advanced report generated: {output_path}")
        
    def _perform_advanced_analytics(self) -> Dict:
        """Perform comprehensive analysis on issues"""
        analytics = {
            'severity_distribution': self._analyze_severity_distribution(),
            'component_analysis': self._analyze_components(),
            'file_analysis': self._analyze_files(),
            'issue_patterns': self._analyze_patterns(),
            'fix_complexity': self._analyze_fix_complexity(),
            'hot_spots': self._identify_hot_spots(),
            'issue_categories': self._categorize_issues(),
            'recommendations': self._generate_recommendations(),
            'code_quality_score': self._calculate_quality_score(),
            'modernization_progress': self._analyze_modernization_progress()
        }
        
        return analytics
        
    def _analyze_severity_distribution(self) -> Dict:
        """Analyze distribution of issues by severity"""
        severity_counts = Counter(issue.get('severity', 'unknown') for issue in self.issues)
        
        # Calculate percentages
        total = len(self.issues)
        severity_percentages = {
            severity: (count / total * 100) if total > 0 else 0
            for severity, count in severity_counts.items()
        }
        
        return {
            'counts': dict(severity_counts),
            'percentages': severity_percentages,
            'total': total
        }
        
    def _analyze_components(self) -> Dict:
        """Deep analysis of issues by component"""
        component_data = defaultdict(lambda: {
            'total': 0,
            'by_severity': defaultdict(int),
            'by_type': defaultdict(int),
            'files': set(),
            'top_issues': []
        })
        
        for issue in self.issues:
            component = self._extract_component(issue.get('file', ''))
            severity = issue.get('severity', 'unknown')
            issue_id = issue.get('id', 'unknown')
            
            component_data[component]['total'] += 1
            component_data[component]['by_severity'][severity] += 1
            component_data[component]['by_type'][issue_id] += 1
            component_data[component]['files'].add(issue.get('file', ''))
            
        # Convert sets to lists and get top issues
        for component, data in component_data.items():
            data['files'] = len(data['files'])  # Just count files
            data['by_severity'] = dict(data['by_severity'])
            data['by_type'] = dict(data['by_type'])
            # Get top 5 issue types
            data['top_issues'] = sorted(
                data['by_type'].items(), 
                key=lambda x: x[1], 
                reverse=True
            )[:5]
            
        return dict(component_data)
        
    def _analyze_files(self) -> Dict:
        """Analyze issues by file"""
        file_issues = defaultdict(lambda: {
            'count': 0,
            'severities': defaultdict(int),
            'types': defaultdict(int)
        })
        
        for issue in self.issues:
            filepath = issue.get('file', '')
            if filepath:
                file_issues[filepath]['count'] += 1
                file_issues[filepath]['severities'][issue.get('severity', 'unknown')] += 1
                file_issues[filepath]['types'][issue.get('id', 'unknown')] += 1
                
        # Get top 20 problematic files
        top_files = sorted(
            file_issues.items(),
            key=lambda x: x[1]['count'],
            reverse=True
        )[:20]
        
        return {
            'top_files': [
                {
                    'path': self._shorten_path(path),
                    'count': data['count'],
                    'severities': dict(data['severities']),
                    'dominant_type': max(data['types'].items(), key=lambda x: x[1])[0] if data['types'] else 'unknown'
                }
                for path, data in top_files
            ],
            'total_files': len(file_issues)
        }
        
    def _analyze_patterns(self) -> Dict:
        """Identify common issue patterns"""
        pattern_groups = {
            'modernization': {
                'patterns': ['missingOverride', 'noExplicitConstructor', 'useAuto', 'passedByValue'],
                'count': 0,
                'issues': []
            },
            'memory_safety': {
                'patterns': ['uninitMemberVar', 'memleak', 'doubleFree', 'nullPointer'],
                'count': 0,
                'issues': []
            },
            'performance': {
                'patterns': ['passedByValue', 'useStlAlgorithm', 'postfixOperator', 'inefficientAlgorithm'],
                'count': 0,
                'issues': []
            },
            'best_practices': {
                'patterns': ['variableScope', 'unusedVariable', 'redundantAssignment', 'duplicateExpression'],
                'count': 0,
                'issues': []
            }
        }
        
        # Categorize issues
        for issue in self.issues:
            issue_id = issue.get('id', '')
            for category, data in pattern_groups.items():
                if any(pattern in issue_id for pattern in data['patterns']):
                    data['count'] += 1
                    if len(data['issues']) < 5:  # Keep sample issues
                        data['issues'].append({
                            'file': self._shorten_path(issue.get('file', '')),
                            'line': issue.get('line', ''),
                            'message': issue.get('message', '')[:100] + '...' if len(issue.get('message', '')) > 100 else issue.get('message', '')
                        })
                        
        return pattern_groups
        
    def _analyze_fix_complexity(self) -> Dict:
        """Estimate fix complexity for different issue types"""
        complexity_map = {
            'trivial': {
                'patterns': ['missingOverride', 'noExplicitConstructor', 'useAuto'],
                'count': 0,
                'estimated_time': '< 1 min',
                'automation': 'Fully automatable'
            },
            'simple': {
                'patterns': ['passedByValue', 'variableScope', 'redundantAssignment'],
                'count': 0,
                'estimated_time': '1-5 mins',
                'automation': 'Partially automatable'
            },
            'moderate': {
                'patterns': ['uninitMemberVar', 'useStlAlgorithm', 'inefficientAlgorithm'],
                'count': 0,
                'estimated_time': '5-15 mins',
                'automation': 'Manual review needed'
            },
            'complex': {
                'patterns': ['memleak', 'doubleFree', 'nullPointer', 'invalidContainer'],
                'count': 0,
                'estimated_time': '> 15 mins',
                'automation': 'Manual fix required'
            }
        }
        
        # Categorize by complexity
        for issue in self.issues:
            issue_id = issue.get('id', '')
            categorized = False
            for complexity, data in complexity_map.items():
                if any(pattern in issue_id for pattern in data['patterns']):
                    data['count'] += 1
                    categorized = True
                    break
            if not categorized:
                complexity_map['simple']['count'] += 1  # Default to simple
                
        # Calculate total estimated time
        time_estimates = {
            'trivial': 0.5,  # minutes
            'simple': 3,
            'moderate': 10,
            'complex': 20
        }
        
        total_time = sum(
            data['count'] * time_estimates[complexity]
            for complexity, data in complexity_map.items()
        )
        
        return {
            'categories': complexity_map,
            'total_estimated_hours': round(total_time / 60, 1),
            'total_estimated_days': round(total_time / 480, 1)  # 8 hour days
        }
        
    def _identify_hot_spots(self) -> List[Dict]:
        """Identify code hot spots that need immediate attention"""
        hot_spots = []
        
        # Group issues by file and function
        file_function_issues = defaultdict(lambda: defaultdict(list))
        
        for issue in self.issues:
            filepath = issue.get('file', '')
            # Try to extract function name from message
            message = issue.get('message', '')
            function_match = re.search(r"'(\w+)'\s*(?:function|method|constructor)", message)
            function = function_match.group(1) if function_match else 'global'
            
            file_function_issues[filepath][function].append(issue)
            
        # Find hot spots
        for filepath, functions in file_function_issues.items():
            for function, issues in functions.items():
                if len(issues) >= 5:  # Threshold for hot spot
                    severity_counts = Counter(issue.get('severity', 'unknown') for issue in issues)
                    hot_spots.append({
                        'file': self._shorten_path(filepath),
                        'function': function,
                        'issue_count': len(issues),
                        'severities': dict(severity_counts),
                        'priority': self._calculate_priority(severity_counts)
                    })
                    
        # Sort by priority
        hot_spots.sort(key=lambda x: x['priority'], reverse=True)
        
        return hot_spots[:10]  # Top 10 hot spots
        
    def _categorize_issues(self) -> Dict:
        """Categorize issues by C++17 modernization categories"""
        categories = {
            'Language Features': {
                'keywords': ['override', 'explicit', 'auto', 'nullptr', 'constexpr', 'noexcept'],
                'count': 0,
                'examples': []
            },
            'Smart Pointers': {
                'keywords': ['unique_ptr', 'shared_ptr', 'make_unique', 'make_shared', 'raw pointer'],
                'count': 0,
                'examples': []
            },
            'STL Usage': {
                'keywords': ['algorithm', 'container', 'iterator', 'stl', 'vector', 'map'],
                'count': 0,
                'examples': []
            },
            'Type Safety': {
                'keywords': ['cast', 'conversion', 'narrow', 'signed', 'unsigned'],
                'count': 0,
                'examples': []
            },
            'Resource Management': {
                'keywords': ['leak', 'uninit', 'destruct', 'raii', 'resource'],
                'count': 0,
                'examples': []
            },
            'Performance': {
                'keywords': ['copy', 'move', 'forward', 'emplace', 'reserve', 'efficiency'],
                'count': 0,
                'examples': []
            }
        }
        
        # Categorize each issue
        for issue in self.issues:
            message = issue.get('message', '').lower()
            issue_id = issue.get('id', '').lower()
            full_text = f"{message} {issue_id}"
            
            for category, data in categories.items():
                if any(keyword in full_text for keyword in data['keywords']):
                    data['count'] += 1
                    if len(data['examples']) < 3:
                        data['examples'].append({
                            'file': self._shorten_path(issue.get('file', '')),
                            'line': issue.get('line', ''),
                            'id': issue.get('id', ''),
                            'message': issue.get('message', '')[:80] + '...'
                        })
                        
        return categories
        
    def _generate_recommendations(self) -> List[Dict]:
        """Generate actionable recommendations based on analysis"""
        recommendations = []
        
        # Analyze patterns
        severity_dist = self._analyze_severity_distribution()
        components = self._analyze_components()
        patterns = self._analyze_patterns()
        
        # High error count
        if severity_dist['counts'].get('error', 0) > 100:
            recommendations.append({
                'priority': 'high',
                'title': 'High Error Count',
                'description': f"Found {severity_dist['counts']['error']} errors. Focus on fixing errors first as they may indicate bugs.",
                'action': 'Run `./tools/cppcheck/cppcheck autofix --fix nullptr --dry-run` to preview nullptr fixes'
            })
            
        # Modernization opportunities
        if patterns['modernization']['count'] > 500:
            recommendations.append({
                'priority': 'medium',
                'title': 'Significant Modernization Opportunities',
                'description': f"{patterns['modernization']['count']} C++17 modernization issues found",
                'action': 'Use the autofix tool to automatically apply override and explicit keywords'
            })
            
        # Component-specific recommendations
        sorted_components = sorted(
            components.items(),
            key=lambda x: x[1]['total'],
            reverse=True
        )
        
        if sorted_components and sorted_components[0][1]['total'] > 1000:
            worst_component = sorted_components[0]
            recommendations.append({
                'priority': 'high',
                'title': f'Focus on {worst_component[0]} Component',
                'description': f"This component has {worst_component[1]['total']} issues ({worst_component[1]['total'] / len(self.issues) * 100:.1f}% of all issues)",
                'action': f'Run targeted analysis: `./tools/cppcheck/cppcheck cpp17 --files {worst_component[0]}/*`'
            })
            
        # Memory safety
        if patterns['memory_safety']['count'] > 50:
            recommendations.append({
                'priority': 'critical',
                'title': 'Memory Safety Concerns',
                'description': f"Found {patterns['memory_safety']['count']} potential memory safety issues",
                'action': 'Review uninitialized members and potential memory leaks immediately'
            })
            
        return sorted(recommendations, key=lambda x: {'critical': 0, 'high': 1, 'medium': 2, 'low': 3}[x['priority']])
        
    def _calculate_quality_score(self) -> Dict:
        """Calculate overall code quality score"""
        total_issues = len(self.issues)
        
        # Estimate lines of code (rough approximation)
        files = set(issue.get('file', '') for issue in self.issues)
        estimated_loc = len(files) * 500  # Rough estimate
        
        # Weight different severity levels
        weights = {
            'error': 10,
            'warning': 5,
            'style': 1,
            'performance': 3,
            'information': 0.5
        }
        
        severity_counts = Counter(issue.get('severity', 'unknown') for issue in self.issues)
        weighted_score = sum(
            weights.get(severity, 1) * count 
            for severity, count in severity_counts.items()
        )
        
        # Calculate score (0-100, where 100 is best)
        issues_per_kloc = (weighted_score / estimated_loc) * 1000 if estimated_loc > 0 else 0
        quality_score = max(0, 100 - (issues_per_kloc * 10))
        
        # Grade assignment
        if quality_score >= 90:
            grade = 'A'
            grade_color = '#48bb78'
        elif quality_score >= 80:
            grade = 'B'
            grade_color = '#38a169'
        elif quality_score >= 70:
            grade = 'C'
            grade_color = '#ecc94b'
        elif quality_score >= 60:
            grade = 'D'
            grade_color = '#ed8936'
        else:
            grade = 'F'
            grade_color = '#f56565'
            
        return {
            'score': round(quality_score, 1),
            'grade': grade,
            'grade_color': grade_color,
            'issues_per_kloc': round(issues_per_kloc, 2),
            'total_weighted_issues': weighted_score,
            'interpretation': self._interpret_score(quality_score)
        }
        
    def _analyze_modernization_progress(self) -> Dict:
        """Analyze C++17 modernization progress"""
        modernization_issues = {
            'nullptr_conversion': 0,
            'override_missing': 0,
            'explicit_missing': 0,
            'auto_opportunity': 0,
            'using_vs_typedef': 0,
            'smart_pointer_opportunity': 0,
            'algorithm_opportunity': 0,
            'constexpr_opportunity': 0
        }
        
        # Count modernization opportunities
        for issue in self.issues:
            issue_id = issue.get('id', '')
            message = issue.get('message', '').lower()
            
            if 'nullptr' in message or 'null' in message:
                modernization_issues['nullptr_conversion'] += 1
            if 'override' in issue_id:
                modernization_issues['override_missing'] += 1
            if 'explicit' in issue_id:
                modernization_issues['explicit_missing'] += 1
            if 'auto' in issue_id or 'auto' in message:
                modernization_issues['auto_opportunity'] += 1
            if 'typedef' in message:
                modernization_issues['using_vs_typedef'] += 1
            if 'unique_ptr' in message or 'shared_ptr' in message:
                modernization_issues['smart_pointer_opportunity'] += 1
            if 'algorithm' in issue_id:
                modernization_issues['algorithm_opportunity'] += 1
            if 'constexpr' in message:
                modernization_issues['constexpr_opportunity'] += 1
                
        total_modernization = sum(modernization_issues.values())
        
        return {
            'issues': modernization_issues,
            'total': total_modernization,
            'percentage_of_all': (total_modernization / len(self.issues) * 100) if self.issues else 0,
            'easy_wins': modernization_issues['override_missing'] + modernization_issues['explicit_missing'] + modernization_issues['nullptr_conversion'],
            'estimated_auto_fixable': modernization_issues['override_missing'] + modernization_issues['explicit_missing'] + modernization_issues['nullptr_conversion']
        }
        
    def _generate_html_structure(self, analytics: Dict) -> str:
        """Generate the complete HTML structure"""
        return f"""
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>LPZRobots Advanced Cppcheck Report - {self.profile}</title>
    <script src="https://cdn.plot.ly/plotly-latest.min.js"></script>
    <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
    <link href="https://fonts.googleapis.com/css2?family=Inter:wght@400;500;600;700&display=swap" rel="stylesheet">
    <link href="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/6.0.0/css/all.min.css" rel="stylesheet">
    {self._generate_styles()}
</head>
<body>
    <div class="container">
        {self._generate_header(analytics)}
        {self._generate_executive_summary(analytics)}
        {self._generate_recommendations_section(analytics)}
        {self._generate_visualizations_section(analytics)}
        {self._generate_hot_spots_section(analytics)}
        {self._generate_modernization_section(analytics)}
        {self._generate_detailed_analysis_section(analytics)}
        {self._generate_footer()}
    </div>
    {self._generate_scripts(analytics)}
</body>
</html>
"""

    def _generate_styles(self) -> str:
        """Generate comprehensive CSS styles"""
        return """
    <style>
        * { box-sizing: border-box; margin: 0; padding: 0; }
        
        :root {
            --primary: #667eea;
            --primary-dark: #5a67d8;
            --secondary: #764ba2;
            --success: #48bb78;
            --warning: #ed8936;
            --danger: #f56565;
            --info: #4299e1;
            --dark: #1a202c;
            --light: #f7fafc;
            --gray: #718096;
            --border: #e2e8f0;
        }
        
        body {
            font-family: 'Inter', -apple-system, BlinkMacSystemFont, sans-serif;
            background: #f5f7fa;
            color: var(--dark);
            line-height: 1.6;
        }
        
        .container {
            max-width: 1600px;
            margin: 0 auto;
            padding: 20px;
        }
        
        /* Header Styles */
        .header {
            background: linear-gradient(135deg, var(--primary) 0%, var(--secondary) 100%);
            color: white;
            padding: 60px 40px;
            border-radius: 20px;
            margin-bottom: 40px;
            position: relative;
            overflow: hidden;
        }
        
        .header::before {
            content: '';
            position: absolute;
            top: -50%;
            right: -50%;
            width: 200%;
            height: 200%;
            background: radial-gradient(circle, rgba(255,255,255,0.1) 0%, transparent 70%);
            animation: pulse 4s ease-in-out infinite;
        }
        
        @keyframes pulse {
            0%, 100% { transform: scale(1); opacity: 0.5; }
            50% { transform: scale(1.1); opacity: 0.3; }
        }
        
        .header-content {
            position: relative;
            z-index: 1;
        }
        
        .header h1 {
            font-size: 3em;
            font-weight: 700;
            margin-bottom: 15px;
            text-shadow: 2px 2px 4px rgba(0,0,0,0.1);
        }
        
        .header-meta {
            display: flex;
            gap: 30px;
            margin-top: 20px;
            flex-wrap: wrap;
        }
        
        .header-meta-item {
            display: flex;
            align-items: center;
            gap: 10px;
            background: rgba(255,255,255,0.2);
            padding: 10px 20px;
            border-radius: 30px;
            backdrop-filter: blur(10px);
        }
        
        /* Executive Summary */
        .executive-summary {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(300px, 1fr));
            gap: 25px;
            margin-bottom: 40px;
        }
        
        .summary-card {
            background: white;
            padding: 30px;
            border-radius: 16px;
            box-shadow: 0 4px 6px rgba(0,0,0,0.07);
            transition: all 0.3s ease;
            position: relative;
            overflow: hidden;
        }
        
        .summary-card:hover {
            transform: translateY(-5px);
            box-shadow: 0 12px 24px rgba(0,0,0,0.15);
        }
        
        .summary-card.quality-score {
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            color: white;
        }
        
        .quality-grade {
            font-size: 4em;
            font-weight: 700;
            text-align: center;
            margin: 20px 0;
        }
        
        .quality-details {
            text-align: center;
            opacity: 0.9;
        }
        
        /* Stats Grid */
        .stats-grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(200px, 1fr));
            gap: 20px;
        }
        
        .stat-item {
            text-align: center;
            padding: 20px;
            background: rgba(0,0,0,0.05);
            border-radius: 12px;
        }
        
        .stat-value {
            font-size: 2.5em;
            font-weight: 700;
            margin: 10px 0;
        }
        
        .stat-label {
            font-size: 0.9em;
            color: var(--gray);
            text-transform: uppercase;
            letter-spacing: 0.05em;
        }
        
        /* Recommendations */
        .recommendations {
            background: white;
            padding: 40px;
            border-radius: 16px;
            margin-bottom: 40px;
            box-shadow: 0 4px 6px rgba(0,0,0,0.07);
        }
        
        .recommendation-item {
            display: flex;
            gap: 20px;
            padding: 20px;
            margin-bottom: 20px;
            border-radius: 12px;
            border: 2px solid var(--border);
            transition: all 0.3s ease;
        }
        
        .recommendation-item:hover {
            border-color: var(--primary);
            background: rgba(102, 126, 234, 0.05);
        }
        
        .recommendation-priority {
            display: flex;
            align-items: center;
            justify-content: center;
            width: 60px;
            height: 60px;
            border-radius: 50%;
            font-weight: 700;
            flex-shrink: 0;
        }
        
        .priority-critical {
            background: var(--danger);
            color: white;
        }
        
        .priority-high {
            background: var(--warning);
            color: white;
        }
        
        .priority-medium {
            background: var(--info);
            color: white;
        }
        
        .recommendation-content h4 {
            margin-bottom: 10px;
            color: var(--dark);
        }
        
        .recommendation-action {
            margin-top: 10px;
            padding: 10px;
            background: var(--light);
            border-radius: 8px;
            font-family: 'Monaco', 'Consolas', monospace;
            font-size: 0.9em;
        }
        
        /* Visualizations */
        .visualizations {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(500px, 1fr));
            gap: 30px;
            margin-bottom: 40px;
        }
        
        .viz-card {
            background: white;
            padding: 30px;
            border-radius: 16px;
            box-shadow: 0 4px 6px rgba(0,0,0,0.07);
        }
        
        .viz-card h3 {
            margin-bottom: 20px;
            color: var(--dark);
            display: flex;
            align-items: center;
            gap: 10px;
        }
        
        .chart-container {
            position: relative;
            height: 400px;
        }
        
        /* Hot Spots */
        .hot-spots {
            background: white;
            padding: 40px;
            border-radius: 16px;
            margin-bottom: 40px;
            box-shadow: 0 4px 6px rgba(0,0,0,0.07);
        }
        
        .hot-spot-item {
            display: grid;
            grid-template-columns: 1fr auto;
            gap: 20px;
            padding: 20px;
            margin-bottom: 15px;
            background: var(--light);
            border-radius: 12px;
            border-left: 4px solid var(--danger);
        }
        
        .hot-spot-info h4 {
            margin-bottom: 5px;
            font-family: 'Monaco', 'Consolas', monospace;
        }
        
        .severity-badges {
            display: flex;
            gap: 10px;
            margin-top: 10px;
        }
        
        .severity-badge {
            padding: 4px 12px;
            border-radius: 20px;
            font-size: 0.85em;
            font-weight: 500;
        }
        
        /* Modernization Progress */
        .modernization-progress {
            background: white;
            padding: 40px;
            border-radius: 16px;
            margin-bottom: 40px;
            box-shadow: 0 4px 6px rgba(0,0,0,0.07);
        }
        
        .progress-item {
            margin-bottom: 25px;
        }
        
        .progress-header {
            display: flex;
            justify-content: space-between;
            margin-bottom: 10px;
        }
        
        .progress-bar {
            width: 100%;
            height: 30px;
            background: var(--light);
            border-radius: 15px;
            overflow: hidden;
            position: relative;
        }
        
        .progress-fill {
            height: 100%;
            background: linear-gradient(90deg, var(--primary) 0%, var(--secondary) 100%);
            transition: width 0.5s ease;
            display: flex;
            align-items: center;
            justify-content: flex-end;
            padding-right: 10px;
            color: white;
            font-weight: 600;
        }
        
        /* Detailed Analysis */
        .detailed-section {
            background: white;
            padding: 40px;
            border-radius: 16px;
            margin-bottom: 40px;
            box-shadow: 0 4px 6px rgba(0,0,0,0.07);
        }
        
        .tabs {
            display: flex;
            gap: 10px;
            margin-bottom: 30px;
            border-bottom: 2px solid var(--border);
        }
        
        .tab {
            padding: 15px 30px;
            background: none;
            border: none;
            font-size: 1em;
            font-weight: 600;
            color: var(--gray);
            cursor: pointer;
            transition: all 0.3s ease;
            position: relative;
        }
        
        .tab:hover {
            color: var(--primary);
        }
        
        .tab.active {
            color: var(--primary);
        }
        
        .tab.active::after {
            content: '';
            position: absolute;
            bottom: -2px;
            left: 0;
            right: 0;
            height: 2px;
            background: var(--primary);
        }
        
        .tab-content {
            display: none;
        }
        
        .tab-content.active {
            display: block;
        }
        
        /* Tables */
        .data-table {
            width: 100%;
            border-collapse: collapse;
            margin-top: 20px;
        }
        
        .data-table th {
            background: var(--light);
            padding: 15px;
            text-align: left;
            font-weight: 600;
            color: var(--dark);
            border-bottom: 2px solid var(--border);
        }
        
        .data-table td {
            padding: 15px;
            border-bottom: 1px solid var(--border);
        }
        
        .data-table tr:hover {
            background: rgba(102, 126, 234, 0.05);
        }
        
        /* Footer */
        .footer {
            text-align: center;
            padding: 40px;
            color: var(--gray);
            font-size: 0.9em;
        }
        
        /* Utility Classes */
        .text-danger { color: var(--danger); }
        .text-warning { color: var(--warning); }
        .text-success { color: var(--success); }
        .text-info { color: var(--info); }
        
        .bg-danger { background: var(--danger); color: white; }
        .bg-warning { background: var(--warning); color: white; }
        .bg-success { background: var(--success); color: white; }
        .bg-info { background: var(--info); color: white; }
        
        .icon {
            width: 24px;
            height: 24px;
            display: inline-block;
            vertical-align: middle;
        }
        
        /* Animations */
        .fade-in {
            animation: fadeIn 0.5s ease-in;
        }
        
        @keyframes fadeIn {
            from { opacity: 0; transform: translateY(20px); }
            to { opacity: 1; transform: translateY(0); }
        }
        
        /* Responsive */
        @media (max-width: 768px) {
            .header h1 { font-size: 2em; }
            .visualizations { grid-template-columns: 1fr; }
            .executive-summary { grid-template-columns: 1fr; }
            .container { padding: 10px; }
        }
    </style>
"""

    def _generate_header(self, analytics: Dict) -> str:
        """Generate header section"""
        quality = analytics['code_quality_score']
        severity = analytics['severity_distribution']
        
        return f"""
        <div class="header">
            <div class="header-content">
                <h1>Advanced Cppcheck Analysis Report</h1>
                <p style="font-size: 1.2em; opacity: 0.9;">Comprehensive code quality analysis for LPZRobots</p>
                <div class="header-meta">
                    <div class="header-meta-item">
                        <i class="fas fa-code-branch"></i>
                        <span>Profile: {self.profile}</span>
                    </div>
                    <div class="header-meta-item">
                        <i class="fas fa-calendar"></i>
                        <span>{datetime.now().strftime('%B %d, %Y at %I:%M %p')}</span>
                    </div>
                    <div class="header-meta-item">
                        <i class="fas fa-file-code"></i>
                        <span>{analytics['file_analysis']['total_files']} files analyzed</span>
                    </div>
                    <div class="header-meta-item">
                        <i class="fas fa-exclamation-triangle"></i>
                        <span>{severity['total']:,} total issues</span>
                    </div>
                </div>
            </div>
        </div>
"""

    def _generate_executive_summary(self, analytics: Dict) -> str:
        """Generate executive summary section"""
        quality = analytics['code_quality_score']
        severity = analytics['severity_distribution']
        fix_complexity = analytics['fix_complexity']
        modernization = analytics['modernization_progress']
        
        return f"""
        <div class="executive-summary">
            <div class="summary-card quality-score">
                <h3><i class="fas fa-trophy"></i> Code Quality Score</h3>
                <div class="quality-grade" style="color: {quality['grade_color']};">{quality['grade']}</div>
                <div class="quality-details">
                    <p style="font-size: 1.4em; margin-bottom: 10px;">{quality['score']}/100</p>
                    <p>{quality['interpretation']}</p>
                    <p style="margin-top: 10px; font-size: 0.9em;">{quality['issues_per_kloc']} issues per 1000 lines</p>
                </div>
            </div>
            
            <div class="summary-card">
                <h3><i class="fas fa-chart-pie"></i> Issue Distribution</h3>
                <div class="stats-grid">
                    <div class="stat-item">
                        <div class="stat-value text-danger">{severity['counts'].get('error', 0):,}</div>
                        <div class="stat-label">Errors</div>
                    </div>
                    <div class="stat-item">
                        <div class="stat-value text-warning">{severity['counts'].get('warning', 0):,}</div>
                        <div class="stat-label">Warnings</div>
                    </div>
                    <div class="stat-item">
                        <div class="stat-value text-info">{severity['counts'].get('style', 0):,}</div>
                        <div class="stat-label">Style</div>
                    </div>
                    <div class="stat-item">
                        <div class="stat-value text-success">{severity['counts'].get('performance', 0):,}</div>
                        <div class="stat-label">Performance</div>
                    </div>
                </div>
            </div>
            
            <div class="summary-card">
                <h3><i class="fas fa-clock"></i> Fix Effort Estimate</h3>
                <div style="text-align: center; margin-top: 20px;">
                    <div style="font-size: 3em; font-weight: 700; color: var(--primary);">
                        {fix_complexity['total_estimated_days']} days
                    </div>
                    <p style="color: var(--gray); margin-top: 10px;">
                        ({fix_complexity['total_estimated_hours']} hours total)
                    </p>
                    <div style="margin-top: 20px;">
                        <p><span class="severity-badge bg-success">Trivial: {fix_complexity['categories']['trivial']['count']}</span></p>
                        <p style="margin-top: 5px;"><span class="severity-badge bg-info">Simple: {fix_complexity['categories']['simple']['count']}</span></p>
                        <p style="margin-top: 5px;"><span class="severity-badge bg-warning">Moderate: {fix_complexity['categories']['moderate']['count']}</span></p>
                        <p style="margin-top: 5px;"><span class="severity-badge bg-danger">Complex: {fix_complexity['categories']['complex']['count']}</span></p>
                    </div>
                </div>
            </div>
            
            <div class="summary-card">
                <h3><i class="fas fa-rocket"></i> C++17 Modernization</h3>
                <div style="text-align: center; margin-top: 20px;">
                    <div style="font-size: 3em; font-weight: 700; color: var(--secondary);">
                        {modernization['total']:,}
                    </div>
                    <p style="color: var(--gray);">modernization opportunities</p>
                    <p style="margin-top: 20px; font-size: 1.2em;">
                        <strong>{modernization['easy_wins']:,}</strong> easy wins
                    </p>
                    <p style="margin-top: 10px; color: var(--success);">
                        <i class="fas fa-robot"></i> {modernization['estimated_auto_fixable']:,} auto-fixable
                    </p>
                </div>
            </div>
        </div>
"""

    def _generate_recommendations_section(self, analytics: Dict) -> str:
        """Generate recommendations section"""
        recommendations = analytics['recommendations']
        
        if not recommendations:
            return ""
            
        items_html = ""
        for rec in recommendations:
            priority_class = f"priority-{rec['priority']}"
            priority_icon = {
                'critical': 'fas fa-exclamation-circle',
                'high': 'fas fa-exclamation-triangle',
                'medium': 'fas fa-info-circle',
                'low': 'fas fa-check-circle'
            }.get(rec['priority'], 'fas fa-info-circle')
            
            items_html += f"""
            <div class="recommendation-item fade-in">
                <div class="recommendation-priority {priority_class}">
                    <i class="{priority_icon}"></i>
                </div>
                <div class="recommendation-content">
                    <h4>{rec['title']}</h4>
                    <p>{rec['description']}</p>
                    <div class="recommendation-action">
                        <i class="fas fa-terminal"></i> {rec['action']}
                    </div>
                </div>
            </div>
            """
            
        return f"""
        <div class="recommendations">
            <h2><i class="fas fa-lightbulb"></i> Actionable Recommendations</h2>
            {items_html}
        </div>
"""

    def _generate_visualizations_section(self, analytics: Dict) -> str:
        """Generate visualizations section"""
        return f"""
        <div class="visualizations">
            <div class="viz-card">
                <h3><i class="fas fa-chart-bar"></i> Component Analysis</h3>
                <div id="componentChart" class="chart-container"></div>
            </div>
            
            <div class="viz-card">
                <h3><i class="fas fa-chart-line"></i> Issue Categories</h3>
                <div id="categoryChart" class="chart-container"></div>
            </div>
            
            <div class="viz-card">
                <h3><i class="fas fa-code"></i> Pattern Distribution</h3>
                <div id="patternChart" class="chart-container"></div>
            </div>
            
            <div class="viz-card">
                <h3><i class="fas fa-layer-group"></i> Fix Complexity</h3>
                <div id="complexityChart" class="chart-container"></div>
            </div>
        </div>
"""

    def _generate_hot_spots_section(self, analytics: Dict) -> str:
        """Generate hot spots section"""
        hot_spots = analytics['hot_spots']
        
        if not hot_spots:
            return ""
            
        items_html = ""
        for spot in hot_spots[:5]:  # Top 5
            severity_badges = ""
            for sev, count in spot['severities'].items():
                color_class = {
                    'error': 'bg-danger',
                    'warning': 'bg-warning',
                    'style': 'bg-info',
                    'performance': 'bg-success'
                }.get(sev, '')
                severity_badges += f'<span class="severity-badge {color_class}">{sev}: {count}</span>'
                
            items_html += f"""
            <div class="hot-spot-item">
                <div class="hot-spot-info">
                    <h4>{spot['file']}</h4>
                    <p>Function: <strong>{spot['function']}</strong> • {spot['issue_count']} issues</p>
                    <div class="severity-badges">{severity_badges}</div>
                </div>
                <div style="text-align: right;">
                    <div style="font-size: 2em; font-weight: 700; color: var(--danger);">
                        P{spot['priority']}
                    </div>
                    <p style="color: var(--gray); font-size: 0.9em;">Priority</p>
                </div>
            </div>
            """
            
        return f"""
        <div class="hot-spots">
            <h2><i class="fas fa-fire"></i> Code Hot Spots</h2>
            <p style="color: var(--gray); margin-bottom: 20px;">Areas requiring immediate attention</p>
            {items_html}
        </div>
"""

    def _generate_modernization_section(self, analytics: Dict) -> str:
        """Generate modernization progress section"""
        modernization = analytics['modernization_progress']
        
        progress_items = [
            ('Missing override specifiers', modernization['issues']['override_missing'], '#4299e1'),
            ('Missing explicit constructors', modernization['issues']['explicit_missing'], '#667eea'),
            ('nullptr conversions needed', modernization['issues']['nullptr_conversion'], '#764ba2'),
            ('auto opportunities', modernization['issues']['auto_opportunity'], '#48bb78'),
            ('typedef → using migrations', modernization['issues']['using_vs_typedef'], '#ed8936'),
            ('Smart pointer opportunities', modernization['issues']['smart_pointer_opportunity'], '#f56565')
        ]
        
        items_html = ""
        for name, count, color in progress_items:
            if count > 0:
                percentage = (count / modernization['total'] * 100) if modernization['total'] > 0 else 0
                items_html += f"""
                <div class="progress-item">
                    <div class="progress-header">
                        <span>{name}</span>
                        <span style="font-weight: 600;">{count:,}</span>
                    </div>
                    <div class="progress-bar">
                        <div class="progress-fill" style="width: {percentage}%; background: {color};">
                            {percentage:.1f}%
                        </div>
                    </div>
                </div>
                """
                
        return f"""
        <div class="modernization-progress">
            <h2><i class="fas fa-sync-alt"></i> C++17 Modernization Progress</h2>
            <p style="color: var(--gray); margin-bottom: 30px;">
                Total modernization opportunities: <strong>{modernization['total']:,}</strong> 
                ({modernization['percentage_of_all']:.1f}% of all issues)
            </p>
            {items_html}
            <div style="margin-top: 30px; padding: 20px; background: var(--light); border-radius: 12px;">
                <p><i class="fas fa-magic"></i> <strong>Quick wins:</strong> {modernization['easy_wins']:,} issues can be fixed automatically</p>
                <p style="margin-top: 10px;"><i class="fas fa-robot"></i> Run autofix tool to apply {modernization['estimated_auto_fixable']:,} fixes instantly</p>
            </div>
        </div>
"""

    def _generate_detailed_analysis_section(self, analytics: Dict) -> str:
        """Generate detailed analysis tabs section"""
        return f"""
        <div class="detailed-section">
            <h2><i class="fas fa-microscope"></i> Detailed Analysis</h2>
            
            <div class="tabs">
                <button class="tab active" onclick="showTab('files')">Top Files</button>
                <button class="tab" onclick="showTab('patterns')">Issue Patterns</button>
                <button class="tab" onclick="showTab('categories')">Categories</button>
                <button class="tab" onclick="showTab('components')">Components</button>
            </div>
            
            <div id="files-tab" class="tab-content active">
                {self._generate_files_table(analytics['file_analysis'])}
            </div>
            
            <div id="patterns-tab" class="tab-content">
                {self._generate_patterns_content(analytics['issue_patterns'])}
            </div>
            
            <div id="categories-tab" class="tab-content">
                {self._generate_categories_content(analytics['issue_categories'])}
            </div>
            
            <div id="components-tab" class="tab-content">
                {self._generate_components_table(analytics['component_analysis'])}
            </div>
        </div>
"""

    def _generate_files_table(self, file_analysis: Dict) -> str:
        """Generate files analysis table"""
        rows = ""
        for file_data in file_analysis['top_files']:
            severity_summary = ", ".join(
                f"{sev}: {count}" 
                for sev, count in file_data['severities'].items()
            )
            rows += f"""
            <tr>
                <td style="font-family: monospace;">{file_data['path']}</td>
                <td style="text-align: center; font-weight: 600;">{file_data['count']}</td>
                <td>{severity_summary}</td>
                <td><code>{file_data['dominant_type']}</code></td>
            </tr>
            """
            
        return f"""
        <h3>Top 20 Files with Most Issues</h3>
        <table class="data-table">
            <thead>
                <tr>
                    <th>File Path</th>
                    <th>Issue Count</th>
                    <th>Severity Distribution</th>
                    <th>Dominant Issue Type</th>
                </tr>
            </thead>
            <tbody>
                {rows}
            </tbody>
        </table>
"""

    def _generate_patterns_content(self, patterns: Dict) -> str:
        """Generate patterns analysis content"""
        cards = ""
        for category, data in patterns.items():
            examples = ""
            for ex in data['issues']:
                examples += f"""
                <div style="margin: 10px 0; padding: 10px; background: var(--light); border-radius: 8px;">
                    <code>{ex['file']}:{ex['line']}</code>
                    <p style="margin-top: 5px;">{ex['message']}</p>
                </div>
                """
                
            cards += f"""
            <div style="margin-bottom: 30px; padding: 20px; border: 2px solid var(--border); border-radius: 12px;">
                <h4>{category.replace('_', ' ').title()}</h4>
                <p style="font-size: 2em; font-weight: 700; color: var(--primary); margin: 10px 0;">{data['count']:,}</p>
                <p style="color: var(--gray);">Example issues:</p>
                {examples}
            </div>
            """
            
        return f"""
        <h3>Issue Pattern Analysis</h3>
        {cards}
"""

    def _generate_categories_content(self, categories: Dict) -> str:
        """Generate categories analysis content"""
        cards = ""
        for category, data in categories.items():
            if data['count'] > 0:
                examples = ""
                for ex in data['examples']:
                    examples += f"""
                    <tr>
                        <td><code>{ex['file']}:{ex['line']}</code></td>
                        <td>{ex['id']}</td>
                        <td>{ex['message']}</td>
                    </tr>
                    """
                    
                cards += f"""
                <div style="margin-bottom: 30px;">
                    <h4>{category} ({data['count']} issues)</h4>
                    <table class="data-table" style="margin-top: 10px;">
                        <thead>
                            <tr>
                                <th>Location</th>
                                <th>Issue ID</th>
                                <th>Message</th>
                            </tr>
                        </thead>
                        <tbody>
                            {examples}
                        </tbody>
                    </table>
                </div>
                """
                
        return f"""
        <h3>C++17 Modernization Categories</h3>
        {cards}
"""

    def _generate_components_table(self, components: Dict) -> str:
        """Generate components analysis table"""
        rows = ""
        sorted_components = sorted(
            components.items(),
            key=lambda x: x[1]['total'],
            reverse=True
        )
        
        for comp, data in sorted_components:
            top_issues = ", ".join(
                f"{issue[0]} ({issue[1]})"
                for issue in data['top_issues'][:3]
            )
            rows += f"""
            <tr>
                <td><strong>{comp}</strong></td>
                <td style="text-align: center; font-weight: 600;">{data['total']:,}</td>
                <td>{data['files']} files</td>
                <td>E: {data['by_severity'].get('error', 0)}, W: {data['by_severity'].get('warning', 0)}, S: {data['by_severity'].get('style', 0)}</td>
                <td style="font-size: 0.9em;">{top_issues}</td>
            </tr>
            """
            
        return f"""
        <h3>Component Breakdown</h3>
        <table class="data-table">
            <thead>
                <tr>
                    <th>Component</th>
                    <th>Total Issues</th>
                    <th>Files Affected</th>
                    <th>Severity</th>
                    <th>Top Issue Types</th>
                </tr>
            </thead>
            <tbody>
                {rows}
            </tbody>
        </table>
"""

    def _generate_footer(self) -> str:
        """Generate footer"""
        return """
        <div class="footer">
            <p>
                <i class="fas fa-robot"></i> Generated by LPZRobots Advanced Cppcheck Infrastructure<br>
                <i class="fas fa-heart"></i> Built for C++17 modernization excellence
            </p>
        </div>
"""

    def _generate_scripts(self, analytics: Dict) -> str:
        """Generate JavaScript for interactive features"""
        # Prepare data for charts
        components = analytics['component_analysis']
        categories = analytics['issue_categories']
        patterns = analytics['issue_patterns']
        fix_complexity = analytics['fix_complexity']
        
        return f"""
    <script>
        // Component Analysis Chart
        const componentData = {{
            labels: {json.dumps([c for c in components.keys()])},
            datasets: [
                {{
                    label: 'Errors',
                    data: {json.dumps([c['by_severity'].get('error', 0) for c in components.values()])},
                    backgroundColor: '#f56565'
                }},
                {{
                    label: 'Warnings',
                    data: {json.dumps([c['by_severity'].get('warning', 0) for c in components.values()])},
                    backgroundColor: '#ed8936'
                }},
                {{
                    label: 'Style',
                    data: {json.dumps([c['by_severity'].get('style', 0) for c in components.values()])},
                    backgroundColor: '#4299e1'
                }},
                {{
                    label: 'Performance',
                    data: {json.dumps([c['by_severity'].get('performance', 0) for c in components.values()])},
                    backgroundColor: '#48bb78'
                }}
            ]
        }};
        
        // Category Chart
        const categoryData = {{
            labels: {json.dumps([cat for cat in categories.keys()])},
            datasets: [{{
                data: {json.dumps([data['count'] for data in categories.values()])},
                backgroundColor: ['#667eea', '#764ba2', '#f56565', '#ed8936', '#48bb78', '#4299e1']
            }}]
        }};
        
        // Pattern Chart
        const patternData = {{
            labels: {json.dumps([p.replace('_', ' ').title() for p in patterns.keys()])},
            datasets: [{{
                label: 'Issue Count',
                data: {json.dumps([data['count'] for data in patterns.values()])},
                backgroundColor: '#667eea',
                borderColor: '#5a67d8',
                borderWidth: 2
            }}]
        }};
        
        // Complexity Chart
        const complexityData = {{
            labels: ['Trivial', 'Simple', 'Moderate', 'Complex'],
            datasets: [{{
                data: {json.dumps([
                    fix_complexity['categories']['trivial']['count'],
                    fix_complexity['categories']['simple']['count'],
                    fix_complexity['categories']['moderate']['count'],
                    fix_complexity['categories']['complex']['count']
                ])},
                backgroundColor: ['#48bb78', '#4299e1', '#ed8936', '#f56565']
            }}]
        }};
        
        // Initialize charts
        window.onload = function() {{
            // Component Chart
            new Chart(document.getElementById('componentChart'), {{
                type: 'bar',
                data: componentData,
                options: {{
                    responsive: true,
                    maintainAspectRatio: false,
                    scales: {{
                        x: {{ stacked: true }},
                        y: {{ stacked: true }}
                    }},
                    plugins: {{
                        legend: {{
                            position: 'bottom'
                        }}
                    }}
                }}
            }});
            
            // Category Chart
            new Chart(document.getElementById('categoryChart'), {{
                type: 'doughnut',
                data: categoryData,
                options: {{
                    responsive: true,
                    maintainAspectRatio: false,
                    plugins: {{
                        legend: {{
                            position: 'right'
                        }}
                    }}
                }}
            }});
            
            // Pattern Chart
            new Chart(document.getElementById('patternChart'), {{
                type: 'radar',
                data: patternData,
                options: {{
                    responsive: true,
                    maintainAspectRatio: false,
                    scale: {{
                        beginAtZero: true
                    }}
                }}
            }});
            
            // Complexity Chart
            new Chart(document.getElementById('complexityChart'), {{
                type: 'polarArea',
                data: complexityData,
                options: {{
                    responsive: true,
                    maintainAspectRatio: false,
                    plugins: {{
                        legend: {{
                            position: 'bottom'
                        }}
                    }}
                }}
            }});
        }};
        
        // Tab functionality
        function showTab(tabName) {{
            // Hide all tabs
            document.querySelectorAll('.tab-content').forEach(tab => {{
                tab.classList.remove('active');
            }});
            document.querySelectorAll('.tab').forEach(tab => {{
                tab.classList.remove('active');
            }});
            
            // Show selected tab
            document.getElementById(tabName + '-tab').classList.add('active');
            event.target.classList.add('active');
        }}
        
        // Smooth animations
        const observer = new IntersectionObserver(entries => {{
            entries.forEach(entry => {{
                if (entry.isIntersecting) {{
                    entry.target.classList.add('fade-in');
                }}
            }});
        }});
        
        document.querySelectorAll('.recommendation-item, .hot-spot-item').forEach(el => {{
            observer.observe(el);
        }});
    </script>
"""

    # Helper methods
    def _extract_component(self, filepath: str) -> str:
        """Extract component name from file path"""
        if 'selforg' in filepath:
            return 'selforg'
        elif 'ode_robots' in filepath:
            return 'ode_robots'
        elif 'ga_tools' in filepath:
            return 'ga_tools'
        elif 'opende' in filepath:
            return 'opende'
        elif 'guilogger' in filepath:
            return 'guilogger'
        elif 'matrixviz' in filepath:
            return 'matrixviz'
        else:
            return 'other'
            
    def _shorten_path(self, path: str) -> str:
        """Shorten file path for display"""
        path_str = str(path)
        project_root_str = str(PROJECT_ROOT)
        if path_str.startswith(project_root_str):
            return path_str[len(project_root_str)+1:]
        return path_str
        
    def _calculate_priority(self, severity_counts: Counter) -> int:
        """Calculate priority score for hot spots"""
        weights = {
            'error': 10,
            'warning': 5,
            'style': 1,
            'performance': 3
        }
        return sum(weights.get(sev, 1) * count for sev, count in severity_counts.items())
        
    def _interpret_score(self, score: float) -> str:
        """Provide interpretation of quality score"""
        if score >= 90:
            return "Excellent code quality - minimal issues found"
        elif score >= 80:
            return "Good code quality - some improvements needed"
        elif score >= 70:
            return "Fair code quality - moderate refactoring recommended"
        elif score >= 60:
            return "Below average - significant improvements needed"
        else:
            return "Poor code quality - major refactoring required"


def main():
    if len(sys.argv) < 2:
        print("Usage: generate_advanced_report.py <report.json> [output.html]")
        sys.exit(1)
        
    input_path = Path(sys.argv[1])
    output_path = Path(sys.argv[2]) if len(sys.argv) > 2 else input_path.with_suffix('.advanced.html')
    
    # Load report data
    with open(input_path) as f:
        report_data = json.load(f)
        
    # Generate advanced report
    generator = AdvancedReportGenerator(report_data)
    generator.generate_advanced_html(output_path)
    
    print(f"Advanced report generated: {output_path}")


if __name__ == '__main__':
    main()