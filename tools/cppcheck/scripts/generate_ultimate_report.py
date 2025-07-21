#!/usr/bin/env python3
"""
Ultimate Cppcheck Developer Dashboard
The most comprehensive, feature-rich static analysis report ever created
"""

import json
import sys
from html import escape as html_escape
import re
import subprocess
from pathlib import Path
from datetime import datetime
from typing import Dict, List, Tuple, Optional
from collections import defaultdict, Counter
import hashlib

# Constants
SCRIPT_DIR = Path(__file__).parent
TOOLS_DIR = SCRIPT_DIR.parent
PROJECT_ROOT = TOOLS_DIR.parent.parent

class UltimateDashboardGenerator:
    def __init__(self, report_data: Dict):
        self.data = report_data
        self.issues = report_data.get('issues', [])
        self.stats = report_data.get('stats', {})
        self.profile = report_data.get('profile', 'unknown')
        
    def generate_ultimate_dashboard(self, output_path: Path):
        """Generate the ultimate developer dashboard"""
        # Perform comprehensive analysis
        analytics = self._perform_deep_analysis()
        
        # Generate HTML
        html_content = self._generate_html(analytics)
        
        with open(output_path, 'w', encoding='utf-8') as f:
            f.write(html_content)
            
        print(f"Ultimate dashboard generated: {output_path}")
        
    def _perform_deep_analysis(self) -> Dict:
        """Perform comprehensive analysis on all issues"""
        analytics = {
            'overview': self._analyze_overview(),
            'severity_analysis': self._analyze_severity_distribution(),
            'component_breakdown': self._analyze_components_deeply(),
            'file_hotspots': self._analyze_file_hotspots(),
            'pattern_insights': self._analyze_patterns_with_examples(),
            'fix_roadmap': self._generate_fix_roadmap(),
            'code_quality': self._calculate_comprehensive_quality_score(),
            'modernization': self._analyze_modernization_opportunities(),
            'quick_wins': self._identify_quick_wins(),
            'issue_clusters': self._cluster_similar_issues(),
            'fix_examples': self._generate_fix_examples()
        }
        
        return analytics
        
    def _analyze_overview(self) -> Dict:
        """Generate high-level overview statistics"""
        total = len(self.issues)
        by_severity = Counter(issue.get('severity', 'unknown') for issue in self.issues)
        
        # Calculate unique files and functions affected
        files = set()
        functions = set()
        for issue in self.issues:
            files.add(issue.get('file', ''))
            # Try to extract function from message
            msg = issue.get('message', '')
            func_match = re.search(r"'(\w+)'\s*(?:function|method|constructor)", msg)
            if func_match:
                functions.add(func_match.group(1))
                
        return {
            'total_issues': total,
            'by_severity': dict(by_severity),
            'unique_files': len(files),
            'unique_functions': len(functions),
            'issues_per_file': total / len(files) if files else 0
        }
        
    def _analyze_severity_distribution(self) -> Dict:
        """Detailed severity analysis with trends"""
        severity_data = defaultdict(lambda: {
            'count': 0,
            'percentage': 0,
            'top_types': Counter(),
            'affected_components': Counter(),
            'complexity': {'easy': 0, 'medium': 0, 'hard': 0}
        })
        
        total = len(self.issues)
        
        for issue in self.issues:
            severity = issue.get('severity', 'unknown')
            issue_id = issue.get('id', 'unknown')
            component = self._extract_component(issue.get('file', ''))
            
            severity_data[severity]['count'] += 1
            severity_data[severity]['top_types'][issue_id] += 1
            severity_data[severity]['affected_components'][component] += 1
            
            # Estimate complexity
            complexity = self._estimate_issue_complexity(issue)
            severity_data[severity]['complexity'][complexity] += 1
            
        # Calculate percentages
        for severity, data in severity_data.items():
            data['percentage'] = (data['count'] / total * 100) if total > 0 else 0
            data['top_types'] = dict(data['top_types'].most_common(5))
            data['affected_components'] = dict(data['affected_components'])
            
        return dict(severity_data)
        
    def _analyze_components_deeply(self) -> Dict:
        """Deep component analysis with actionable insights"""
        components = defaultdict(lambda: {
            'total': 0,
            'by_severity': Counter(),
            'by_type': Counter(),
            'files': set(),
            'functions': set(),
            'hot_files': Counter(),
            'priority_score': 0,
            'estimated_hours': 0,
            'top_patterns': []
        })
        
        for issue in self.issues:
            component = self._extract_component(issue.get('file', ''))
            filepath = issue.get('file', '')
            severity = issue.get('severity', 'unknown')
            issue_id = issue.get('id', 'unknown')
            
            components[component]['total'] += 1
            components[component]['by_severity'][severity] += 1
            components[component]['by_type'][issue_id] += 1
            components[component]['files'].add(filepath)
            components[component]['hot_files'][filepath] += 1
            
            # Extract function if possible
            msg = issue.get('message', '')
            func_match = re.search(r"'(\w+)'\s*(?:function|method|constructor)", msg)
            if func_match:
                components[component]['functions'].add(func_match.group(1))
                
            # Add to estimated time
            components[component]['estimated_hours'] += self._estimate_fix_time(issue)
            
        # Process component data
        for comp, data in components.items():
            # Calculate priority score
            data['priority_score'] = (
                data['by_severity'].get('error', 0) * 10 +
                data['by_severity'].get('warning', 0) * 5 +
                data['by_severity'].get('performance', 0) * 3 +
                data['by_severity'].get('style', 0) * 1
            )
            
            # Convert sets to counts
            data['file_count'] = len(data['files'])
            data['function_count'] = len(data['functions'])
            del data['files']
            del data['functions']
            
            # Get top hot files
            data['hot_files'] = dict(data['hot_files'].most_common(5))
            
            # Get top patterns
            data['top_patterns'] = list(data['by_type'].most_common(10))
            data['by_type'] = dict(data['by_type'])
            data['by_severity'] = dict(data['by_severity'])
            
        return dict(components)
        
    def _analyze_file_hotspots(self) -> List[Dict]:
        """Identify files that need immediate attention"""
        file_data = defaultdict(lambda: {
            'issues': [],
            'severity_counts': Counter(),
            'issue_types': Counter(),
            'functions': set(),
            'lines': set(),
            'complexity_score': 0
        })
        
        for issue in self.issues:
            filepath = issue.get('file', '')
            if filepath:
                file_data[filepath]['issues'].append(issue)
                file_data[filepath]['severity_counts'][issue.get('severity', 'unknown')] += 1
                file_data[filepath]['issue_types'][issue.get('id', 'unknown')] += 1
                file_data[filepath]['lines'].add(issue.get('line', 0))
                
                # Extract function
                msg = issue.get('message', '')
                func_match = re.search(r"'(\w+)'\s*(?:function|method|constructor)", msg)
                if func_match:
                    file_data[filepath]['functions'].add(func_match.group(1))
                    
        # Calculate complexity scores and prepare hotspots
        hotspots = []
        for filepath, data in file_data.items():
            # Complexity score based on issue density and severity
            score = (
                data['severity_counts'].get('error', 0) * 10 +
                data['severity_counts'].get('warning', 0) * 5 +
                data['severity_counts'].get('performance', 0) * 3 +
                data['severity_counts'].get('style', 0) * 1
            )
            
            # Boost score for high issue density
            line_count = len(data['lines'])
            if line_count > 0:
                issue_density = len(data['issues']) / line_count
                score *= (1 + issue_density)
                
            hotspots.append({
                'file': self._shorten_path(filepath),
                'full_path': filepath,
                'issue_count': len(data['issues']),
                'severity_breakdown': dict(data['severity_counts']),
                'affected_functions': len(data['functions']),
                'affected_lines': line_count,
                'top_issues': data['issue_types'].most_common(5),
                'complexity_score': round(score, 2),
                'priority': 'critical' if score > 100 else 'high' if score > 50 else 'medium'
            })
            
        # Sort by complexity score
        hotspots.sort(key=lambda x: x['complexity_score'], reverse=True)
        
        return hotspots[:20]  # Top 20 hotspots
        
    def _analyze_patterns_with_examples(self) -> Dict:
        """Analyze patterns and provide concrete examples"""
        patterns = {
            'modernization': {
                'name': 'C++17 Modernization',
                'icon': 'fa-sync-alt',
                'color': '#667eea',
                'patterns': ['missingOverride', 'noExplicitConstructor', 'useAuto', 'modernize-'],
                'issues': [],
                'fix_strategy': 'Use autofix tool for quick wins'
            },
            'memory_safety': {
                'name': 'Memory Safety',
                'icon': 'fa-shield-alt',
                'color': '#f56565',
                'patterns': ['uninitMemberVar', 'memleak', 'doubleFree', 'nullPointer', 'invalidContainer'],
                'issues': [],
                'fix_strategy': 'Manual review required - potential bugs'
            },
            'performance': {
                'name': 'Performance',
                'icon': 'fa-tachometer-alt',
                'color': '#48bb78',
                'patterns': ['passedByValue', 'useStlAlgorithm', 'postfixOperator', 'inefficient'],
                'issues': [],
                'fix_strategy': 'Profile first, then optimize hot paths'
            },
            'code_quality': {
                'name': 'Code Quality',
                'icon': 'fa-medal',
                'color': '#ed8936',
                'patterns': ['variableScope', 'unusedVariable', 'redundant', 'duplicate'],
                'issues': [],
                'fix_strategy': 'Refactor during feature work'
            },
            'api_usage': {
                'name': 'API Usage',
                'icon': 'fa-plug',
                'color': '#4299e1',
                'patterns': ['wrongAPI', 'deprecated', 'unsafeAPI'],
                'issues': [],
                'fix_strategy': 'Update to modern APIs'
            }
        }
        
        # Categorize issues
        for issue in self.issues:
            issue_id = issue.get('id', '')
            message = issue.get('message', '').lower()
            
            for category, data in patterns.items():
                if any(pattern in issue_id or pattern in message for pattern in data['patterns']):
                    if len(data['issues']) < 10:  # Keep examples manageable
                        data['issues'].append({
                            'file': self._shorten_path(issue.get('file', '')),
                            'line': issue.get('line', ''),
                            'message': issue.get('message', ''),
                            'id': issue_id,
                            'severity': issue.get('severity', 'unknown')
                        })
                    else:
                        data['count'] = data.get('count', 0) + 1
                        
        # Add counts
        for category, data in patterns.items():
            data['total_count'] = len(data['issues']) + data.get('count', 0)
            
        return patterns
        
    def _generate_fix_roadmap(self) -> List[Dict]:
        """Generate a prioritized fix roadmap"""
        roadmap = []
        
        # Phase 1: Critical Errors
        errors = [i for i in self.issues if i.get('severity') == 'error']
        if errors:
            roadmap.append({
                'phase': 1,
                'name': 'Critical Errors',
                'duration': f"{len(errors) * 0.5:.1f} hours",
                'issue_count': len(errors),
                'description': 'Fix all errors that could cause bugs or crashes',
                'automation': 'Limited - manual review required',
                'priority': 'critical'
            })
            
        # Phase 2: Easy Modernization Wins
        easy_modern = [i for i in self.issues if any(
            pattern in i.get('id', '') 
            for pattern in ['missingOverride', 'noExplicitConstructor', 'nullptr']
        )]
        if easy_modern:
            roadmap.append({
                'phase': 2,
                'name': 'Quick Modernization',
                'duration': f"{len(easy_modern) * 0.1:.1f} hours",
                'issue_count': len(easy_modern),
                'description': 'Apply easy C++17 fixes with autofix tool',
                'automation': 'Fully automated with review',
                'priority': 'high'
            })
            
        # Phase 3: Performance
        perf = [i for i in self.issues if i.get('severity') == 'performance']
        if perf:
            roadmap.append({
                'phase': 3,
                'name': 'Performance Optimization',
                'duration': f"{len(perf) * 0.3:.1f} hours",
                'issue_count': len(perf),
                'description': 'Optimize hot paths and algorithms',
                'automation': 'Semi-automated refactoring',
                'priority': 'medium'
            })
            
        # Phase 4: Code Quality
        quality = [i for i in self.issues if i.get('severity') == 'style']
        if quality:
            roadmap.append({
                'phase': 4,
                'name': 'Code Quality',
                'duration': f"{len(quality) * 0.2:.1f} hours",
                'issue_count': len(quality),
                'description': 'Improve code style and maintainability',
                'automation': 'Mostly automated',
                'priority': 'low'
            })
            
        return roadmap
        
    def _calculate_comprehensive_quality_score(self) -> Dict:
        """Calculate detailed quality metrics"""
        total = len(self.issues)
        files = len(set(issue.get('file', '') for issue in self.issues))
        
        # Weighted scoring
        weights = {
            'error': 10,
            'warning': 5,
            'performance': 3,
            'style': 1,
            'information': 0.5
        }
        
        severity_counts = Counter(issue.get('severity', 'unknown') for issue in self.issues)
        weighted_score = sum(
            weights.get(sev, 1) * count 
            for sev, count in severity_counts.items()
        )
        
        # Calculate metrics
        issues_per_file = total / files if files else 0
        estimated_loc = files * 500  # Rough estimate
        issues_per_kloc = (weighted_score / estimated_loc * 1000) if estimated_loc > 0 else 0
        
        # Score calculation (0-100)
        quality_score = max(0, 100 - (issues_per_kloc * 5))
        
        # Grade assignment
        if quality_score >= 90:
            grade = 'A'
            grade_color = '#48bb78'
            interpretation = 'Excellent - Industry leading quality'
        elif quality_score >= 80:
            grade = 'B'
            grade_color = '#38a169'
            interpretation = 'Good - Above average quality'
        elif quality_score >= 70:
            grade = 'C'
            grade_color = '#ecc94b'
            interpretation = 'Fair - Room for improvement'
        elif quality_score >= 60:
            grade = 'D'
            grade_color = '#ed8936'
            interpretation = 'Below Average - Needs attention'
        else:
            grade = 'F'
            grade_color = '#f56565'
            interpretation = 'Poor - Significant issues'
            
        return {
            'score': round(quality_score, 1),
            'grade': grade,
            'grade_color': grade_color,
            'interpretation': interpretation,
            'issues_per_file': round(issues_per_file, 2),
            'issues_per_kloc': round(issues_per_kloc, 2),
            'total_weighted_score': weighted_score,
            'metrics': {
                'maintainability': round(100 - (severity_counts.get('style', 0) / total * 100) if total else 100, 1),
                'reliability': round(100 - (severity_counts.get('error', 0) / total * 100) if total else 100, 1),
                'security': round(100 - (severity_counts.get('warning', 0) / total * 100) if total else 100, 1),
                'efficiency': round(100 - (severity_counts.get('performance', 0) / total * 100) if total else 100, 1)
            }
        }
        
    def _analyze_modernization_opportunities(self) -> Dict:
        """Detailed C++17 modernization analysis"""
        opportunities = {
            'auto_usage': {'count': 0, 'examples': [], 'effort': 'low'},
            'smart_pointers': {'count': 0, 'examples': [], 'effort': 'medium'},
            'algorithms': {'count': 0, 'examples': [], 'effort': 'medium'},
            'constexpr': {'count': 0, 'examples': [], 'effort': 'low'},
            'structured_bindings': {'count': 0, 'examples': [], 'effort': 'low'},
            'if_init': {'count': 0, 'examples': [], 'effort': 'low'},
            'string_view': {'count': 0, 'examples': [], 'effort': 'medium'}
        }
        
        # Analyze each issue
        for issue in self.issues:
            issue_id = issue.get('id', '')
            message = issue.get('message', '')
            
            # Categorize opportunities
            if 'auto' in issue_id or 'auto' in message:
                opportunities['auto_usage']['count'] += 1
                if len(opportunities['auto_usage']['examples']) < 3:
                    opportunities['auto_usage']['examples'].append(self._create_example(issue))
                    
            if 'pointer' in message or 'new' in message or 'delete' in message:
                opportunities['smart_pointers']['count'] += 1
                if len(opportunities['smart_pointers']['examples']) < 3:
                    opportunities['smart_pointers']['examples'].append(self._create_example(issue))
                    
        # Calculate totals
        total_opportunities = sum(cat['count'] for cat in opportunities.values())
        easy_wins = sum(cat['count'] for cat in opportunities.values() if cat['effort'] == 'low')
        
        return {
            'categories': opportunities,
            'total': total_opportunities,
            'easy_wins': easy_wins,
            'estimated_hours': total_opportunities * 0.2  # Average 12 mins per fix
        }
        
    def _identify_quick_wins(self) -> List[Dict]:
        """Identify issues that can be fixed quickly"""
        quick_wins = []
        
        # Define quick win patterns
        patterns = [
            {
                'id': 'missingOverride',
                'name': 'Add override specifiers',
                'command': './tools/cppcheck/cppcheck autofix --fix override',
                'time': '< 1 minute per file',
                'risk': 'None - compile-time check'
            },
            {
                'id': 'noExplicitConstructor',
                'name': 'Add explicit to constructors',
                'command': './tools/cppcheck/cppcheck autofix --fix explicit',
                'time': '< 1 minute per class',
                'risk': 'Low - may break implicit conversions'
            },
            {
                'id': 'useAuto',
                'name': 'Use auto for iterators',
                'command': './tools/cppcheck/cppcheck autofix --fix auto',
                'time': '< 30 seconds per instance',
                'risk': 'Low - improves readability'
            }
        ]
        
        # Count instances of each pattern
        for pattern in patterns:
            count = sum(1 for issue in self.issues if pattern['id'] in issue.get('id', ''))
            if count > 0:
                pattern['count'] = count
                pattern['total_time'] = f"{count * 0.5:.1f} minutes"
                quick_wins.append(pattern)
                
        # Sort by count
        quick_wins.sort(key=lambda x: x['count'], reverse=True)
        
        return quick_wins
        
    def _cluster_similar_issues(self) -> Dict:
        """Group similar issues for batch fixing"""
        clusters = defaultdict(list)
        
        # Group by issue ID and file
        for issue in self.issues:
            key = (issue.get('id', 'unknown'), self._extract_component(issue.get('file', '')))
            clusters[key].append(issue)
            
        # Process clusters
        processed_clusters = []
        for (issue_id, component), issues in clusters.items():
            if len(issues) >= 5:  # Only show clusters with 5+ issues
                processed_clusters.append({
                    'issue_type': issue_id,
                    'component': component,
                    'count': len(issues),
                    'files': len(set(i.get('file', '') for i in issues)),
                    'sample': issues[0],
                    'fix_strategy': self._get_fix_strategy(issue_id)
                })
                
        # Sort by count
        processed_clusters.sort(key=lambda x: x['count'], reverse=True)
        
        return {
            'clusters': processed_clusters[:20],
            'total_clusters': len(clusters)
        }
        
    def _generate_fix_examples(self) -> Dict:
        """Generate concrete fix examples for common issues"""
        examples = {
            'missingOverride': {
                'before': '''class Derived : public Base {
    virtual void process(); // Missing override
};''',
                'after': '''class Derived : public Base {
    void process() override; // Fixed: added override
};''',
                'explanation': 'Adding override helps catch errors when base class changes'
            },
            'noExplicitConstructor': {
                'before': '''class Widget {
    Widget(int value); // Allows implicit conversion
};''',
                'after': '''class Widget {
    explicit Widget(int value); // Fixed: prevents implicit conversion
};''',
                'explanation': 'Explicit constructors prevent unexpected implicit conversions'
            },
            'passedByValue': {
                'before': '''void processData(std::string data) { // Expensive copy
    // ...
}''',
                'after': '''void processData(const std::string& data) { // Fixed: pass by const ref
    // ...
}''',
                'explanation': 'Passing by const reference avoids unnecessary copies'
            },
            'useAuto': {
                'before': '''std::vector<int>::iterator it = vec.begin();
for (std::vector<int>::iterator i = vec.begin(); i != vec.end(); ++i) {''',
                'after': '''auto it = vec.begin();
for (auto i = vec.begin(); i != vec.end(); ++i) {''',
                'explanation': 'Auto makes code more maintainable and readable'
            }
        }
        
        return examples
        
    def _generate_html(self, analytics: Dict) -> str:
        """Generate the ultimate HTML dashboard"""
        return f"""<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Ultimate Cppcheck Dashboard - {html_escape(self.profile)}</title>
    <link href="https://fonts.googleapis.com/css2?family=Inter:wght@400;500;600;700;800&display=swap" rel="stylesheet">
    <link href="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/6.0.0/css/all.min.css" rel="stylesheet">
    <link href="https://cdnjs.cloudflare.com/ajax/libs/prism/1.29.0/themes/prism-tomorrow.min.css" rel="stylesheet">
    <script src="https://cdnjs.cloudflare.com/ajax/libs/prism/1.29.0/prism.min.js"></script>
    <script src="https://cdnjs.cloudflare.com/ajax/libs/prism/1.29.0/components/prism-cpp.min.js"></script>
    <script src="https://cdn.plot.ly/plotly-latest.min.js"></script>
    <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
    {self._generate_ultimate_styles()}
</head>
<body>
    <div class="app-container">
        {self._generate_header(analytics)}
        {self._generate_executive_dashboard(analytics)}
        {self._generate_actionable_insights(analytics)}
        {self._generate_issue_explorer(analytics)}
        {self._generate_code_viewer_modal()}
        {self._generate_fix_helper_modal()}
    </div>
    {self._generate_ultimate_scripts(analytics)}
</body>
</html>"""

    def _generate_ultimate_styles(self) -> str:
        """Generate comprehensive styles for the ultimate dashboard"""
        return """
    <style>
        * { box-sizing: border-box; margin: 0; padding: 0; }
        
        :root {
            --primary: #667eea;
            --primary-dark: #5a67d8;
            --primary-light: #7c3aed;
            --secondary: #764ba2;
            --success: #48bb78;
            --warning: #ed8936;
            --danger: #f56565;
            --info: #4299e1;
            --dark: #1a202c;
            --gray-900: #2d3748;
            --gray-800: #4a5568;
            --gray-700: #718096;
            --gray-600: #a0aec0;
            --gray-500: #cbd5e0;
            --gray-400: #e2e8f0;
            --gray-300: #edf2f7;
            --gray-200: #f7fafc;
            --white: #ffffff;
            
            --shadow-sm: 0 1px 2px 0 rgba(0, 0, 0, 0.05);
            --shadow: 0 4px 6px -1px rgba(0, 0, 0, 0.1);
            --shadow-lg: 0 10px 15px -3px rgba(0, 0, 0, 0.1);
            --shadow-xl: 0 20px 25px -5px rgba(0, 0, 0, 0.1);
        }
        
        body {
            font-family: 'Inter', -apple-system, sans-serif;
            background: #f0f2f5;
            color: var(--dark);
            line-height: 1.6;
            overflow-x: hidden;
        }
        
        .app-container {
            min-height: 100vh;
        }
        
        /* Header */
        .header {
            background: linear-gradient(135deg, var(--primary) 0%, var(--primary-light) 50%, var(--secondary) 100%);
            color: white;
            padding: 40px 0;
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
            animation: pulse 8s ease-in-out infinite;
        }
        
        @keyframes pulse {
            0%, 100% { transform: scale(1) rotate(0deg); }
            50% { transform: scale(1.1) rotate(180deg); }
        }
        
        .header-content {
            max-width: 1400px;
            margin: 0 auto;
            padding: 0 20px;
            position: relative;
            z-index: 1;
        }
        
        .header h1 {
            font-size: 3em;
            font-weight: 800;
            margin-bottom: 10px;
            text-shadow: 2px 2px 4px rgba(0,0,0,0.2);
        }
        
        .header-stats {
            display: flex;
            gap: 30px;
            margin-top: 20px;
            flex-wrap: wrap;
        }
        
        .header-stat {
            display: flex;
            align-items: center;
            gap: 10px;
            background: rgba(255,255,255,0.2);
            padding: 10px 20px;
            border-radius: 50px;
            backdrop-filter: blur(10px);
        }
        
        /* Executive Dashboard */
        .executive-dashboard {
            max-width: 1400px;
            margin: -40px auto 40px;
            padding: 0 20px;
            position: relative;
            z-index: 10;
        }
        
        .quality-score-card {
            background: white;
            border-radius: 20px;
            padding: 40px;
            box-shadow: var(--shadow-xl);
            text-align: center;
            margin-bottom: 30px;
            position: relative;
            overflow: hidden;
        }
        
        .quality-grade {
            font-size: 8em;
            font-weight: 800;
            margin: 20px 0;
            background: linear-gradient(45deg, var(--primary), var(--secondary));
            -webkit-background-clip: text;
            -webkit-text-fill-color: transparent;
            background-clip: text;
        }
        
        .quality-metrics {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(200px, 1fr));
            gap: 20px;
            margin-top: 30px;
        }
        
        .quality-metric {
            padding: 20px;
            background: var(--gray-200);
            border-radius: 12px;
            position: relative;
        }
        
        .metric-label {
            font-size: 0.9em;
            color: var(--gray-700);
            text-transform: uppercase;
            letter-spacing: 0.05em;
        }
        
        .metric-value {
            font-size: 2em;
            font-weight: 700;
            margin: 5px 0;
        }
        
        .metric-bar {
            height: 6px;
            background: var(--gray-400);
            border-radius: 3px;
            overflow: hidden;
            margin-top: 10px;
        }
        
        .metric-fill {
            height: 100%;
            background: var(--primary);
            transition: width 1s ease;
        }
        
        /* Cards Grid */
        .cards-grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(300px, 1fr));
            gap: 25px;
            margin-bottom: 40px;
        }
        
        .dashboard-card {
            background: white;
            border-radius: 16px;
            padding: 30px;
            box-shadow: var(--shadow);
            transition: all 0.3s ease;
            position: relative;
            overflow: hidden;
        }
        
        .dashboard-card:hover {
            transform: translateY(-5px);
            box-shadow: var(--shadow-xl);
        }
        
        .card-icon {
            width: 60px;
            height: 60px;
            border-radius: 12px;
            display: flex;
            align-items: center;
            justify-content: center;
            font-size: 1.5em;
            margin-bottom: 20px;
        }
        
        .card-title {
            font-size: 1.1em;
            font-weight: 600;
            color: var(--gray-800);
            margin-bottom: 10px;
        }
        
        .card-value {
            font-size: 2.5em;
            font-weight: 700;
            margin: 10px 0;
        }
        
        .card-subtitle {
            color: var(--gray-600);
            font-size: 0.9em;
        }
        
        /* Actionable Insights */
        .insights-section {
            max-width: 1400px;
            margin: 0 auto 40px;
            padding: 0 20px;
        }
        
        .insights-header {
            display: flex;
            justify-content: space-between;
            align-items: center;
            margin-bottom: 30px;
        }
        
        .insights-tabs {
            display: flex;
            gap: 10px;
            background: var(--gray-200);
            padding: 5px;
            border-radius: 12px;
        }
        
        .tab-btn {
            padding: 10px 20px;
            background: transparent;
            border: none;
            border-radius: 8px;
            font-weight: 600;
            color: var(--gray-700);
            cursor: pointer;
            transition: all 0.3s;
        }
        
        .tab-btn:hover {
            background: rgba(102, 126, 234, 0.1);
        }
        
        .tab-btn.active {
            background: white;
            color: var(--primary);
            box-shadow: var(--shadow-sm);
        }
        
        .insights-content {
            background: white;
            border-radius: 16px;
            padding: 30px;
            box-shadow: var(--shadow);
        }
        
        /* Issue Explorer */
        .issue-explorer {
            max-width: 1400px;
            margin: 0 auto 40px;
            padding: 0 20px;
        }
        
        .explorer-header {
            background: white;
            border-radius: 16px 16px 0 0;
            padding: 30px;
            box-shadow: var(--shadow);
            position: sticky;
            top: 0;
            z-index: 100;
        }
        
        .search-controls {
            display: flex;
            gap: 20px;
            margin-bottom: 20px;
            flex-wrap: wrap;
        }
        
        .search-input {
            flex: 1;
            min-width: 300px;
            padding: 12px 20px;
            border: 2px solid var(--gray-400);
            border-radius: 12px;
            font-size: 1em;
            transition: all 0.3s;
        }
        
        .search-input:focus {
            outline: none;
            border-color: var(--primary);
            box-shadow: 0 0 0 3px rgba(102, 126, 234, 0.1);
        }
        
        .filter-group {
            display: flex;
            gap: 10px;
            align-items: center;
        }
        
        .filter-btn {
            padding: 10px 20px;
            border: 2px solid var(--gray-400);
            background: white;
            border-radius: 10px;
            font-weight: 600;
            cursor: pointer;
            transition: all 0.3s;
            display: flex;
            align-items: center;
            gap: 8px;
        }
        
        .filter-btn:hover {
            border-color: var(--primary);
            background: rgba(102, 126, 234, 0.05);
        }
        
        .filter-btn.active {
            background: var(--primary);
            color: white;
            border-color: var(--primary);
        }
        
        /* Issue List */
        .issue-list {
            background: white;
            border-radius: 0 0 16px 16px;
            box-shadow: var(--shadow);
            overflow: hidden;
        }
        
        .issue-group {
            border-bottom: 1px solid var(--gray-300);
        }
        
        .issue-group:last-child {
            border-bottom: none;
        }
        
        .group-header {
            padding: 20px 30px;
            background: var(--gray-100);
            cursor: pointer;
            user-select: none;
            display: flex;
            justify-content: space-between;
            align-items: center;
            transition: background 0.3s;
        }
        
        .group-header:hover {
            background: var(--gray-200);
        }
        
        .group-title {
            display: flex;
            align-items: center;
            gap: 15px;
            font-weight: 600;
            font-size: 1.1em;
        }
        
        .group-count {
            background: var(--primary);
            color: white;
            padding: 4px 12px;
            border-radius: 20px;
            font-size: 0.9em;
            font-weight: 600;
        }
        
        .issue-item {
            padding: 20px 30px;
            border-bottom: 1px solid var(--gray-200);
            cursor: pointer;
            transition: all 0.3s;
            display: flex;
            justify-content: space-between;
            align-items: start;
            gap: 20px;
        }
        
        .issue-item:hover {
            background: rgba(102, 126, 234, 0.02);
            padding-left: 35px;
        }
        
        .issue-info {
            flex: 1;
        }
        
        .issue-location {
            font-family: 'Monaco', 'Consolas', monospace;
            font-size: 0.95em;
            color: var(--primary);
            margin-bottom: 8px;
            display: flex;
            align-items: center;
            gap: 10px;
        }
        
        .issue-message {
            color: var(--gray-800);
            line-height: 1.6;
            margin-bottom: 8px;
        }
        
        .issue-meta {
            display: flex;
            gap: 15px;
            align-items: center;
        }
        
        .issue-badge {
            padding: 4px 10px;
            border-radius: 6px;
            font-size: 0.85em;
            font-weight: 500;
        }
        
        .issue-actions {
            display: flex;
            gap: 10px;
        }
        
        .action-btn {
            padding: 8px 16px;
            border: 1px solid var(--gray-400);
            background: white;
            border-radius: 8px;
            font-size: 0.9em;
            cursor: pointer;
            transition: all 0.3s;
            display: flex;
            align-items: center;
            gap: 5px;
        }
        
        .action-btn:hover {
            border-color: var(--primary);
            background: rgba(102, 126, 234, 0.05);
            color: var(--primary);
        }
        
        /* Code Viewer Modal */
        .modal {
            display: none;
            position: fixed;
            top: 0;
            left: 0;
            width: 100%;
            height: 100%;
            background: rgba(0, 0, 0, 0.8);
            z-index: 1000;
            animation: fadeIn 0.3s ease;
        }
        
        @keyframes fadeIn {
            from { opacity: 0; }
            to { opacity: 1; }
        }
        
        .modal-content {
            background: white;
            margin: 50px auto;
            width: 90%;
            max-width: 1200px;
            border-radius: 20px;
            box-shadow: var(--shadow-xl);
            animation: slideUp 0.3s ease;
            max-height: 90vh;
            display: flex;
            flex-direction: column;
        }
        
        @keyframes slideUp {
            from { transform: translateY(50px); opacity: 0; }
            to { transform: translateY(0); opacity: 1; }
        }
        
        .modal-header {
            background: var(--gray-900);
            color: white;
            padding: 25px 30px;
            border-radius: 20px 20px 0 0;
            display: flex;
            justify-content: space-between;
            align-items: center;
        }
        
        .modal-title {
            font-size: 1.3em;
            font-weight: 600;
            font-family: 'Monaco', 'Consolas', monospace;
        }
        
        .modal-actions {
            display: flex;
            gap: 15px;
        }
        
        .modal-btn {
            background: rgba(255, 255, 255, 0.1);
            border: none;
            color: white;
            padding: 10px 20px;
            border-radius: 8px;
            cursor: pointer;
            transition: background 0.3s;
            display: flex;
            align-items: center;
            gap: 8px;
        }
        
        .modal-btn:hover {
            background: rgba(255, 255, 255, 0.2);
        }
        
        .modal-body {
            padding: 30px;
            overflow-y: auto;
            flex: 1;
        }
        
        .code-section {
            margin-bottom: 30px;
        }
        
        .code-section h3 {
            margin-bottom: 15px;
            color: var(--gray-800);
        }
        
        .code-viewer {
            background: #1e1e1e;
            border-radius: 12px;
            padding: 20px;
            overflow-x: auto;
            position: relative;
        }
        
        .code-header {
            display: flex;
            justify-content: space-between;
            align-items: center;
            margin-bottom: 15px;
            padding-bottom: 15px;
            border-bottom: 1px solid #333;
        }
        
        .code-filename {
            color: #fff;
            font-family: monospace;
            font-size: 0.9em;
        }
        
        .code-actions {
            display: flex;
            gap: 10px;
        }
        
        .code-action {
            background: rgba(255, 255, 255, 0.1);
            border: none;
            color: #fff;
            padding: 5px 10px;
            border-radius: 4px;
            font-size: 0.85em;
            cursor: pointer;
            transition: background 0.3s;
        }
        
        .code-action:hover {
            background: rgba(255, 255, 255, 0.2);
        }
        
        .code-line {
            display: flex;
            margin: 2px 0;
            font-family: 'Monaco', 'Consolas', monospace;
            font-size: 0.9em;
            line-height: 1.6;
        }
        
        .line-number {
            color: #666;
            width: 50px;
            text-align: right;
            padding-right: 20px;
            user-select: none;
        }
        
        .line-content {
            flex: 1;
            white-space: pre;
            color: #d4d4d4;
        }
        
        .line-highlight {
            background: rgba(237, 137, 54, 0.3);
            border-radius: 3px;
            position: relative;
        }
        
        .line-highlight::before {
            content: '>';
            position: absolute;
            left: -30px;
            color: #ed8936;
            font-weight: bold;
        }
        
        /* Fix Examples */
        .fix-examples {
            display: grid;
            grid-template-columns: 1fr 1fr;
            gap: 20px;
            margin-top: 20px;
        }
        
        .fix-example {
            background: var(--gray-100);
            border-radius: 12px;
            padding: 20px;
        }
        
        .fix-example h4 {
            margin-bottom: 10px;
            color: var(--gray-800);
        }
        
        .fix-example pre {
            background: #1e1e1e;
            color: #d4d4d4;
            padding: 15px;
            border-radius: 8px;
            overflow-x: auto;
            font-size: 0.85em;
        }
        
        /* Charts */
        .chart-container {
            position: relative;
            height: 400px;
            margin: 20px 0;
        }
        
        /* Responsive */
        @media (max-width: 768px) {
            .header h1 { font-size: 2em; }
            .cards-grid { grid-template-columns: 1fr; }
            .search-controls { flex-direction: column; }
            .modal-content { margin: 20px; width: calc(100% - 40px); }
            .fix-examples { grid-template-columns: 1fr; }
        }
        
        /* Utilities */
        .text-danger { color: var(--danger); }
        .text-warning { color: var(--warning); }
        .text-success { color: var(--success); }
        .text-info { color: var(--info); }
        
        .bg-danger-light { background: rgba(245, 101, 101, 0.1); color: var(--danger); }
        .bg-warning-light { background: rgba(237, 137, 54, 0.1); color: var(--warning); }
        .bg-success-light { background: rgba(72, 187, 120, 0.1); color: var(--success); }
        .bg-info-light { background: rgba(66, 153, 225, 0.1); color: var(--info); }
        
        /* Loading States */
        .skeleton {
            background: linear-gradient(90deg, var(--gray-300) 25%, var(--gray-200) 50%, var(--gray-300) 75%);
            background-size: 200% 100%;
            animation: loading 1.5s infinite;
        }
        
        @keyframes loading {
            0% { background-position: 200% 0; }
            100% { background-position: -200% 0; }
        }
        
        /* Tooltips */
        .tooltip {
            position: relative;
            cursor: help;
        }
        
        .tooltip::after {
            content: attr(data-tooltip);
            position: absolute;
            bottom: 100%;
            left: 50%;
            transform: translateX(-50%) translateY(-5px);
            background: var(--gray-900);
            color: white;
            padding: 8px 12px;
            border-radius: 6px;
            font-size: 0.85em;
            white-space: nowrap;
            opacity: 0;
            pointer-events: none;
            transition: opacity 0.3s, transform 0.3s;
        }
        
        .tooltip:hover::after {
            opacity: 1;
            transform: translateX(-50%) translateY(-10px);
        }
    </style>
"""

    def _generate_header(self, analytics: Dict) -> str:
        """Generate the header section"""
        overview = analytics['overview']
        quality = analytics['code_quality']
        
        return f"""
        <div class="header">
            <div class="header-content">
                <h1>Ultimate Cppcheck Analysis Dashboard</h1>
                <p style="font-size: 1.2em; opacity: 0.9;">Comprehensive code quality analysis for LPZRobots</p>
                <div class="header-stats">
                    <div class="header-stat">
                        <i class="fas fa-exclamation-triangle"></i>
                        <span><strong>{overview['total_issues']:,}</strong> total issues</span>
                    </div>
                    <div class="header-stat">
                        <i class="fas fa-file-code"></i>
                        <span><strong>{overview['unique_files']}</strong> files affected</span>
                    </div>
                    <div class="header-stat">
                        <i class="fas fa-function"></i>
                        <span><strong>{overview['unique_functions']}</strong> functions</span>
                    </div>
                    <div class="header-stat">
                        <i class="fas fa-chart-line"></i>
                        <span><strong>{overview['issues_per_file']:.1f}</strong> issues/file</span>
                    </div>
                    <div class="header-stat">
                        <i class="fas fa-trophy"></i>
                        <span>Grade: <strong>{quality['grade']}</strong></span>
                    </div>
                </div>
            </div>
        </div>
"""

    def _generate_executive_dashboard(self, analytics: Dict) -> str:
        """Generate the executive dashboard section"""
        quality = analytics['code_quality']
        overview = analytics['overview']
        modernization = analytics['modernization']
        roadmap = analytics['fix_roadmap']
        
        # Calculate total fix time
        total_hours = sum(float(phase['duration'].split()[0]) for phase in roadmap)
        total_days = total_hours / 8
        
        return f"""
        <div class="executive-dashboard">
            <!-- Quality Score Card -->
            <div class="quality-score-card">
                <h2>Code Quality Score</h2>
                <div class="quality-grade" style="color: {quality['grade_color']}">
                    {quality['grade']}
                </div>
                <div style="font-size: 2em; font-weight: 700; margin-bottom: 10px;">
                    {quality['score']}/100
                </div>
                <p style="font-size: 1.1em; color: var(--gray-700);">
                    {quality['interpretation']}
                </p>
                
                <div class="quality-metrics">
                    <div class="quality-metric">
                        <div class="metric-label">Maintainability</div>
                        <div class="metric-value">{quality['metrics']['maintainability']}%</div>
                        <div class="metric-bar">
                            <div class="metric-fill" style="width: {quality['metrics']['maintainability']}%; background: var(--info);"></div>
                        </div>
                    </div>
                    <div class="quality-metric">
                        <div class="metric-label">Reliability</div>
                        <div class="metric-value">{quality['metrics']['reliability']}%</div>
                        <div class="metric-bar">
                            <div class="metric-fill" style="width: {quality['metrics']['reliability']}%; background: var(--danger);"></div>
                        </div>
                    </div>
                    <div class="quality-metric">
                        <div class="metric-label">Security</div>
                        <div class="metric-value">{quality['metrics']['security']}%</div>
                        <div class="metric-bar">
                            <div class="metric-fill" style="width: {quality['metrics']['security']}%; background: var(--warning);"></div>
                        </div>
                    </div>
                    <div class="quality-metric">
                        <div class="metric-label">Efficiency</div>
                        <div class="metric-value">{quality['metrics']['efficiency']}%</div>
                        <div class="metric-bar">
                            <div class="metric-fill" style="width: {quality['metrics']['efficiency']}%; background: var(--success);"></div>
                        </div>
                    </div>
                </div>
            </div>
            
            <!-- Summary Cards -->
            <div class="cards-grid">
                <div class="dashboard-card">
                    <div class="card-icon" style="background: rgba(245, 101, 101, 0.1); color: var(--danger);">
                        <i class="fas fa-bug"></i>
                    </div>
                    <div class="card-title">Critical Errors</div>
                    <div class="card-value text-danger">{overview['by_severity'].get('error', 0):,}</div>
                    <div class="card-subtitle">Potential bugs and crashes</div>
                </div>
                
                <div class="dashboard-card">
                    <div class="card-icon" style="background: rgba(237, 137, 54, 0.1); color: var(--warning);">
                        <i class="fas fa-exclamation-triangle"></i>
                    </div>
                    <div class="card-title">Warnings</div>
                    <div class="card-value text-warning">{overview['by_severity'].get('warning', 0):,}</div>
                    <div class="card-subtitle">Issues requiring attention</div>
                </div>
                
                <div class="dashboard-card">
                    <div class="card-icon" style="background: rgba(102, 126, 234, 0.1); color: var(--primary);">
                        <i class="fas fa-sync-alt"></i>
                    </div>
                    <div class="card-title">Modernization</div>
                    <div class="card-value text-info">{modernization['total']:,}</div>
                    <div class="card-subtitle">{modernization['easy_wins']} easy wins</div>
                </div>
                
                <div class="dashboard-card">
                    <div class="card-icon" style="background: rgba(72, 187, 120, 0.1); color: var(--success);">
                        <i class="fas fa-clock"></i>
                    </div>
                    <div class="card-title">Fix Estimate</div>
                    <div class="card-value text-success">{total_days:.1f} days</div>
                    <div class="card-subtitle">{total_hours:.1f} total hours</div>
                </div>
                
                <div class="dashboard-card">
                    <div class="card-icon" style="background: rgba(118, 75, 162, 0.1); color: var(--secondary);">
                        <i class="fas fa-code"></i>
                    </div>
                    <div class="card-title">Code Density</div>
                    <div class="card-value" style="color: var(--secondary);">{quality['issues_per_kloc']:.1f}</div>
                    <div class="card-subtitle">Issues per 1000 lines</div>
                </div>
                
                <div class="dashboard-card">
                    <div class="card-icon" style="background: rgba(66, 153, 225, 0.1); color: var(--info);">
                        <i class="fas fa-robot"></i>
                    </div>
                    <div class="card-title">Auto-fixable</div>
                    <div class="card-value text-info">{self._count_autofixable()}</div>
                    <div class="card-subtitle">Can be fixed automatically</div>
                </div>
            </div>
        </div>
"""

    def _generate_actionable_insights(self, analytics: Dict) -> str:
        """Generate the actionable insights section"""
        return f"""
        <div class="insights-section">
            <div class="insights-header">
                <h2><i class="fas fa-lightbulb"></i> Actionable Insights</h2>
                <div class="insights-tabs">
                    <button class="tab-btn active" onclick="showInsightTab('roadmap')">
                        <i class="fas fa-road"></i> Fix Roadmap
                    </button>
                    <button class="tab-btn" onclick="showInsightTab('hotspots')">
                        <i class="fas fa-fire"></i> Hot Spots
                    </button>
                    <button class="tab-btn" onclick="showInsightTab('patterns')">
                        <i class="fas fa-layer-group"></i> Patterns
                    </button>
                    <button class="tab-btn" onclick="showInsightTab('quickwins')">
                        <i class="fas fa-bolt"></i> Quick Wins
                    </button>
                    <button class="tab-btn" onclick="showInsightTab('examples')">
                        <i class="fas fa-code"></i> Fix Examples
                    </button>
                </div>
            </div>
            
            <div class="insights-content">
                {self._generate_roadmap_content(analytics['fix_roadmap'])}
                {self._generate_hotspots_content(analytics['file_hotspots'])}
                {self._generate_patterns_content(analytics['pattern_insights'])}
                {self._generate_quickwins_content(analytics['quick_wins'])}
                {self._generate_examples_content(analytics['fix_examples'])}
            </div>
        </div>
"""

    def _generate_roadmap_content(self, roadmap: List[Dict]) -> str:
        """Generate fix roadmap content"""
        html = '<div id="roadmap-content" class="insight-tab active">'
        html += '<h3>Prioritized Fix Roadmap</h3>'
        html += '<p style="color: var(--gray-700); margin-bottom: 20px;">Follow this roadmap for systematic improvement</p>'
        
        for phase in roadmap:
            priority_color = {
                'critical': 'var(--danger)',
                'high': 'var(--warning)',
                'medium': 'var(--info)',
                'low': 'var(--success)'
            }.get(phase['priority'], 'var(--gray-700)')
            
            html += f"""
            <div style="display: flex; gap: 20px; margin-bottom: 25px; padding: 20px; background: var(--gray-100); border-radius: 12px; border-left: 4px solid {priority_color};">
                <div style="font-size: 2em; font-weight: 700; color: {priority_color};">
                    {phase['phase']}
                </div>
                <div style="flex: 1;">
                    <h4 style="margin-bottom: 10px;">{phase['name']}</h4>
                    <p style="color: var(--gray-700); margin-bottom: 10px;">{phase['description']}</p>
                    <div style="display: flex; gap: 20px; font-size: 0.9em;">
                        <span><i class="fas fa-tasks"></i> {phase['issue_count']} issues</span>
                        <span><i class="fas fa-clock"></i> {phase['duration']}</span>
                        <span><i class="fas fa-robot"></i> {phase['automation']}</span>
                    </div>
                </div>
            </div>
            """
            
        html += '</div>'
        return html
        
    def _generate_hotspots_content(self, hotspots: List[Dict]) -> str:
        """Generate hotspots content"""
        html = '<div id="hotspots-content" class="insight-tab" style="display: none;">'
        html += '<h3>Code Hot Spots</h3>'
        html += '<p style="color: var(--gray-700); margin-bottom: 20px;">Files requiring immediate attention</p>'
        
        html += '<div style="overflow-x: auto;">'
        html += '<table style="width: 100%; border-collapse: collapse;">'
        html += '<thead><tr style="background: var(--gray-100);">'
        html += '<th style="padding: 15px; text-align: left;">File</th>'
        html += '<th style="padding: 15px; text-align: center;">Issues</th>'
        html += '<th style="padding: 15px; text-align: center;">Functions</th>'
        html += '<th style="padding: 15px; text-align: center;">Priority</th>'
        html += '<th style="padding: 15px; text-align: center;">Score</th>'
        html += '<th style="padding: 15px; text-align: center;">Action</th>'
        html += '</tr></thead><tbody>'
        
        for spot in hotspots[:10]:
            priority_badge = {
                'critical': 'bg-danger-light',
                'high': 'bg-warning-light',
                'medium': 'bg-info-light'
            }.get(spot['priority'], 'bg-success-light')
            
            html += f"""
            <tr style="border-bottom: 1px solid var(--gray-300);">
                <td style="padding: 15px; font-family: monospace; font-size: 0.9em;">
                    <a href="#" onclick="viewFile('{spot['full_path']}')" style="color: var(--primary); text-decoration: none;">
                        {spot['file']}
                    </a>
                </td>
                <td style="padding: 15px; text-align: center; font-weight: 600;">{spot['issue_count']}</td>
                <td style="padding: 15px; text-align: center;">{spot['affected_functions']}</td>
                <td style="padding: 15px; text-align: center;">
                    <span class="issue-badge {priority_badge}">{spot['priority'].upper()}</span>
                </td>
                <td style="padding: 15px; text-align: center; font-weight: 600;">{spot['complexity_score']}</td>
                <td style="padding: 15px; text-align: center;">
                    <button class="action-btn" onclick="analyzeFile('{spot['full_path']}')">
                        <i class="fas fa-microscope"></i> Analyze
                    </button>
                </td>
            </tr>
            """
            
        html += '</tbody></table></div>'
        html += '</div>'
        return html
        
    def _generate_patterns_content(self, patterns: Dict) -> str:
        """Generate patterns content"""
        html = '<div id="patterns-content" class="insight-tab" style="display: none;">'
        html += '<h3>Issue Patterns Analysis</h3>'
        html += '<p style="color: var(--gray-700); margin-bottom: 20px;">Common patterns identified in your codebase</p>'
        
        html += '<div style="display: grid; grid-template-columns: repeat(auto-fit, minmax(300px, 1fr)); gap: 20px;">'
        
        for category, data in patterns.items():
            html += f"""
            <div style="background: var(--gray-100); border-radius: 12px; padding: 25px;">
                <div style="display: flex; align-items: center; gap: 15px; margin-bottom: 20px;">
                    <div style="width: 50px; height: 50px; background: {data['color']}22; color: {data['color']}; 
                               border-radius: 10px; display: flex; align-items: center; justify-content: center;">
                        <i class="fas {data['icon']}"></i>
                    </div>
                    <div>
                        <h4 style="margin: 0;">{data['name']}</h4>
                        <p style="margin: 5px 0 0 0; color: var(--gray-600); font-size: 0.9em;">
                            {data['total_count']} issues found
                        </p>
                    </div>
                </div>
                <p style="color: var(--gray-700); margin-bottom: 15px; font-size: 0.95em;">
                    <strong>Strategy:</strong> {data['fix_strategy']}
                </p>
                <div style="font-size: 0.9em;">
                    <strong>Examples:</strong>
                    <ul style="margin: 10px 0 0 20px; color: var(--gray-700);">
            """
            
            for ex in data['issues'][:3]:
                html += f"<li>{ex['file']}:{ex['line']} - {ex['id']}</li>"
                
            html += """
                    </ul>
                </div>
            </div>
            """
            
        html += '</div></div>'
        return html
        
    def _generate_quickwins_content(self, quick_wins: List[Dict]) -> str:
        """Generate quick wins content"""
        html = '<div id="quickwins-content" class="insight-tab" style="display: none;">'
        html += '<h3>Quick Wins</h3>'
        html += '<p style="color: var(--gray-700); margin-bottom: 20px;">Low-hanging fruit that can be fixed quickly</p>'
        
        for win in quick_wins:
            html += f"""
            <div style="background: var(--gray-100); border-radius: 12px; padding: 25px; margin-bottom: 20px;">
                <div style="display: flex; justify-content: space-between; align-items: start; margin-bottom: 15px;">
                    <h4 style="margin: 0;">{win['name']}</h4>
                    <span style="background: var(--success); color: white; padding: 5px 15px; border-radius: 20px; font-size: 0.9em;">
                        {win['count']} instances
                    </span>
                </div>
                <p style="color: var(--gray-700); margin-bottom: 15px;">
                    <i class="fas fa-clock"></i> Time: {win['time']} &bull; 
                    <i class="fas fa-shield-alt"></i> Risk: {win['risk']}
                </p>
                <div style="background: var(--gray-900); color: white; padding: 15px; border-radius: 8px; font-family: monospace; font-size: 0.9em;">
                    $ {win['command']}
                </div>
            </div>
            """
            
        html += '</div>'
        return html
        
    def _generate_examples_content(self, examples: Dict) -> str:
        """Generate fix examples content"""
        html = '<div id="examples-content" class="insight-tab" style="display: none;">'
        html += '<h3>Fix Examples</h3>'
        html += '<p style="color: var(--gray-700); margin-bottom: 20px;">Concrete examples of how to fix common issues</p>'
        
        for issue_type, example in examples.items():
            html += f"""
            <div style="margin-bottom: 30px;">
                <h4>{issue_type.replace('_', ' ').title()}</h4>
                <p style="color: var(--gray-700); margin-bottom: 15px;">{example['explanation']}</p>
                <div class="fix-examples">
                    <div class="fix-example">
                        <h4><i class="fas fa-times-circle text-danger"></i> Before</h4>
                        <pre><code class="language-cpp">{html_escape(example['before'])}</code></pre>
                    </div>
                    <div class="fix-example">
                        <h4><i class="fas fa-check-circle text-success"></i> After</h4>
                        <pre><code class="language-cpp">{html_escape(example['after'])}</code></pre>
                    </div>
                </div>
            </div>
            """
            
        html += '</div>'
        return html
        
    def _generate_issue_explorer(self, analytics: Dict) -> str:
        """Generate the issue explorer section"""
        return f"""
        <div class="issue-explorer">
            <div class="explorer-header">
                <h2><i class="fas fa-search"></i> Issue Explorer</h2>
                <div class="search-controls">
                    <input type="text" class="search-input" id="searchInput" 
                           placeholder="Search by file, function, message, or issue ID..." 
                           autocomplete="off">
                    <div class="filter-group">
                        <button class="filter-btn active" data-severity="all" onclick="filterBySeverity('all')">
                            All ({analytics['overview']['total_issues']})
                        </button>
                        <button class="filter-btn" data-severity="error" onclick="filterBySeverity('error')">
                            <i class="fas fa-bug"></i> Errors ({analytics['overview']['by_severity'].get('error', 0)})
                        </button>
                        <button class="filter-btn" data-severity="warning" onclick="filterBySeverity('warning')">
                            <i class="fas fa-exclamation-triangle"></i> Warnings ({analytics['overview']['by_severity'].get('warning', 0)})
                        </button>
                        <button class="filter-btn" data-severity="style" onclick="filterBySeverity('style')">
                            <i class="fas fa-paint-brush"></i> Style ({analytics['overview']['by_severity'].get('style', 0)})
                        </button>
                        <button class="filter-btn" data-severity="performance" onclick="filterBySeverity('performance')">
                            <i class="fas fa-tachometer-alt"></i> Performance ({analytics['overview']['by_severity'].get('performance', 0)})
                        </button>
                    </div>
                </div>
                
                <div style="display: flex; justify-content: space-between; align-items: center; margin-top: 20px;">
                    <div id="searchStatus" style="color: var(--gray-600);"></div>
                    <div class="filter-group">
                        <select id="sortBy" onchange="sortIssues()" style="padding: 8px 15px; border: 2px solid var(--gray-400); border-radius: 8px;">
                            <option value="severity">Sort by Severity</option>
                            <option value="file">Sort by File</option>
                            <option value="type">Sort by Type</option>
                            <option value="component">Sort by Component</option>
                        </select>
                        <button class="filter-btn" onclick="exportIssues()">
                            <i class="fas fa-download"></i> Export
                        </button>
                    </div>
                </div>
            </div>
            
            <div class="issue-list" id="issueList">
                <!-- Issues will be populated by JavaScript -->
            </div>
        </div>
"""

    def _generate_code_viewer_modal(self) -> str:
        """Generate the code viewer modal"""
        return """
        <div id="codeModal" class="modal">
            <div class="modal-content">
                <div class="modal-header">
                    <div class="modal-title" id="modalTitle">Code Context</div>
                    <div class="modal-actions">
                        <button class="modal-btn" onclick="copyCode()">
                            <i class="fas fa-copy"></i> Copy
                        </button>
                        <button class="modal-btn" onclick="openInEditor()">
                            <i class="fas fa-external-link-alt"></i> Open in Editor
                        </button>
                        <button class="modal-btn" onclick="showGitBlame()">
                            <i class="fas fa-code-branch"></i> Git Blame
                        </button>
                        <button class="modal-btn" onclick="closeModal()">
                            <i class="fas fa-times"></i> Close
                        </button>
                    </div>
                </div>
                <div class="modal-body">
                    <div class="code-section">
                        <h3>Code Context</h3>
                        <div class="code-viewer" id="codeViewer">
                            <!-- Code will be populated here -->
                        </div>
                    </div>
                    
                    <div class="code-section">
                        <h3>Issue Details</h3>
                        <div id="issueDetails" style="background: var(--gray-100); padding: 20px; border-radius: 12px;">
                            <!-- Issue details will be populated here -->
                        </div>
                    </div>
                    
                    <div class="code-section" id="fixSection">
                        <h3>Suggested Fix</h3>
                        <div id="suggestedFix">
                            <!-- Fix suggestion will be populated here -->
                        </div>
                    </div>
                </div>
            </div>
        </div>
"""

    def _generate_fix_helper_modal(self) -> str:
        """Generate the fix helper modal"""
        return """
        <div id="fixModal" class="modal">
            <div class="modal-content">
                <div class="modal-header">
                    <div class="modal-title">Fix Assistant</div>
                    <button class="modal-btn" onclick="closeFixModal()">
                        <i class="fas fa-times"></i> Close
                    </button>
                </div>
                <div class="modal-body">
                    <div id="fixAssistantContent">
                        <!-- Fix assistant content will be populated here -->
                    </div>
                </div>
            </div>
        </div>
"""

    def _generate_ultimate_scripts(self, analytics: Dict) -> str:
        """Generate JavaScript for the ultimate dashboard"""
        # Prepare issue data
        issues_json = json.dumps(self.issues)
        
        return f"""
    <script>
        // Global variables
        const allIssues = {issues_json};
        const analytics = {json.dumps(analytics)};
        let currentSeverity = 'all';
        let currentSort = 'severity';
        let searchTerm = '';
        let filteredIssues = [...allIssues];
        
        // Initialize on load
        window.onload = function() {{
            displayIssues();
            updateSearchStatus();
            initializeCharts();
            
            // Setup search with debounce
            let searchTimeout;
            document.getElementById('searchInput').addEventListener('input', function(e) {{
                clearTimeout(searchTimeout);
                searchTimeout = setTimeout(() => {{
                    searchTerm = e.target.value.toLowerCase();
                    filterAndDisplay();
                }}, 300);
            }});
            
            // Setup keyboard shortcuts
            document.addEventListener('keydown', function(e) {{
                if (e.key === '/' && !e.target.matches('input')) {{
                    e.preventDefault();
                    document.getElementById('searchInput').focus();
                }} else if (e.key === 'Escape') {{
                    closeModal();
                    closeFixModal();
                }}
            }});
        }};
        
        // Display issues
        function displayIssues() {{
            const container = document.getElementById('issueList');
            const grouped = groupIssuesBySeverity(filteredIssues);
            
            let html = '';
            for (const [severity, issues] of Object.entries(grouped)) {{
                if (issues.length === 0) continue;
                
                const severityColors = {{
                    error: '#f56565',
                    warning: '#ed8936',
                    style: '#4299e1',
                    performance: '#48bb78',
                    information: '#805ad5'
                }};
                
                html += `
                    <div class="issue-group">
                        <div class="group-header" onclick="toggleGroup('${{severity}}')">
                            <div class="group-title">
                                <i class="fas fa-chevron-down" id="icon-${{severity}}"></i>
                                <span style="color: ${{severityColors[severity] || '#718096'}}">
                                    ${{severity.charAt(0).toUpperCase() + severity.slice(1)}}
                                </span>
                                <span class="group-count">${{issues.length}}</span>
                            </div>
                            <button class="action-btn" onclick="event.stopPropagation(); batchFix('${{severity}}')">
                                <i class="fas fa-magic"></i> Batch Fix
                            </button>
                        </div>
                        <div id="group-${{severity}}" style="display: none;">
                            ${{issues.slice(0, 20).map(issue => createIssueHtml(issue)).join('')}}
                            ${{issues.length > 20 ? `
                                <div style="text-align: center; padding: 20px;">
                                    <button class="filter-btn" onclick="showAllInGroup('${{severity}}')">
                                        Show all ${{issues.length}} issues
                                    </button>
                                </div>
                            ` : ''}}
                        </div>
                    </div>
                `;
            }}
            
            container.innerHTML = html || '<div style="text-align: center; padding: 40px; color: var(--gray-600);">No issues found</div>';
        }}
        
        // Create issue HTML
        function createIssueHtml(issue) {{
            const component = extractComponent(issue.file);
            const shortPath = issue.file.replace('{str(PROJECT_ROOT)}/', '');
            const functionName = extractFunction(issue.message);
            
            const severityBadges = {{
                error: 'bg-danger-light',
                warning: 'bg-warning-light',
                style: 'bg-info-light',
                performance: 'bg-success-light'
            }};
            
            return `
                <div class="issue-item" onclick="viewCodeContext('${{escapeHtml(issue.file)}}', ${{issue.line || 0}}, '${{escapeHtml(JSON.stringify(issue))}}')">
                    <div class="issue-info">
                        <div class="issue-location">
                            <i class="fas fa-file-code"></i>
                            ${{escapeHtml(shortPath)}}:${{issue.line || 0}}
                            ${{functionName ? `<span style="color: var(--gray-600);">in <strong>${{functionName}}()</strong></span>` : ''}}
                        </div>
                        <div class="issue-message">
                            ${{escapeHtml(issue.message || '')}}
                        </div>
                        <div class="issue-meta">
                            <span class="issue-badge ${{severityBadges[issue.severity] || ''}}">
                                ${{issue.severity || 'unknown'}}
                            </span>
                            <span class="issue-badge" style="background: var(--gray-200); color: var(--gray-700);">
                                ${{issue.id || 'unknown'}}
                            </span>
                            <span style="color: var(--gray-600); font-size: 0.9em;">
                                <i class="fas fa-folder"></i> ${{component}}
                            </span>
                        </div>
                    </div>
                    <div class="issue-actions" onclick="event.stopPropagation();">
                        <button class="action-btn" onclick="quickFix('${{escapeHtml(JSON.stringify(issue))}}')">
                            <i class="fas fa-wrench"></i> Fix
                        </button>
                        <button class="action-btn" onclick="markResolved('${{escapeHtml(JSON.stringify(issue))}}')">
                            <i class="fas fa-check"></i> Resolve
                        </button>
                    </div>
                </div>
            `;
        }}
        
        // View code context
        function viewCodeContext(filepath, line, issueJson) {{
            const issue = JSON.parse(issueJson);
            const modal = document.getElementById('codeModal');
            const modalTitle = document.getElementById('modalTitle');
            const codeViewer = document.getElementById('codeViewer');
            
            modal.style.display = 'block';
            modalTitle.textContent = filepath.split('/').pop() + ':' + line;
            
            // Generate code context
            const context = generateCodeContext(filepath, line, issue);
            displayCodeContext(context, line, issue);
            
            // Show issue details
            displayIssueDetails(issue);
            
            // Show suggested fix if available
            const fix = generateSuggestedFix(issue);
            if (fix) {{
                document.getElementById('fixSection').style.display = 'block';
                document.getElementById('suggestedFix').innerHTML = fix;
            }} else {{
                document.getElementById('fixSection').style.display = 'none';
            }}
        }}
        
        // Generate code context (mock implementation)
        function generateCodeContext(filepath, line, issue) {{
            // In real implementation, this would fetch actual code
            const lines = [];
            const contextSize = 10;
            const startLine = Math.max(1, line - contextSize);
            const endLine = line + contextSize;
            
            // Mock code based on issue type
            let mockCode = getMockCode(issue);
            
            for (let i = startLine; i <= endLine; i++) {{
                lines.push({{
                    number: i,
                    content: mockCode[i - startLine] || '// ...',
                    isTarget: i === line
                }});
            }}
            
            return lines;
        }}
        
        // Get mock code based on issue
        function getMockCode(issue) {{
            if (issue.id === 'missingOverride') {{
                return [
                    'class DerivedController : public AbstractController {{',
                    'public:',
                    '    DerivedController(const std::string& name);',
                    '    ',
                    '    // Initialize the controller',
                    '    virtual void init(int sensornumber, int motornumber,',
                    '                     RandGen* randGen = nullptr);',
                    '    ',
                    '    // Perform one step',
                    '    virtual void step(const sensor* sensors, int sensornumber,',
                    '                     motor* motors, int motornumber);  // <- Missing override',
                    '    ',
                    '    // Get the number of sensors',
                    '    virtual int getSensorNumber() const;',
                    '    ',
                    'private:',
                    '    Matrix A, C, h;',
                    '    paramval eps;',
                    '}};'
                ];
            }}
            // Add more mock code patterns for different issue types
            return ['// Code context would appear here'];
        }}
        
        // Display code context
        function displayCodeContext(lines, targetLine, issue) {{
            let html = `
                <div class="code-header">
                    <div class="code-filename">${{issue.file}}</div>
                    <div class="code-actions">
                        <button class="code-action" onclick="copyCode()">
                            <i class="fas fa-copy"></i> Copy
                        </button>
                        <button class="code-action" onclick="downloadCode()">
                            <i class="fas fa-download"></i> Download
                        </button>
                    </div>
                </div>
                <pre style="margin: 0;"><code>`;
            
            lines.forEach(line => {{
                const highlightClass = line.isTarget ? 'line-highlight' : '';
                html += `
                    <div class="code-line ${{highlightClass}}">
                        <span class="line-number">${{line.number}}</span>
                        <span class="line-content">${{escapeHtml(line.content)}}</span>
                    </div>
                `;
            }});
            
            html += '</code></pre>';
            document.getElementById('codeViewer').innerHTML = html;
            
            // Highlight syntax
            Prism.highlightAllUnder(document.getElementById('codeViewer'));
        }}
        
        // Display issue details
        function displayIssueDetails(issue) {{
            const details = document.getElementById('issueDetails');
            const complexity = estimateComplexity(issue);
            const fixTime = estimateFixTime(issue);
            
            details.innerHTML = `
                <div style="display: grid; grid-template-columns: repeat(auto-fit, minmax(200px, 1fr)); gap: 20px;">
                    <div>
                        <strong>Issue Type:</strong><br>
                        <span style="font-family: monospace;">${{issue.id || 'unknown'}}</span>
                    </div>
                    <div>
                        <strong>Severity:</strong><br>
                        <span class="issue-badge ${{getSeverityBadge(issue.severity)}}">
                            ${{issue.severity || 'unknown'}}
                        </span>
                    </div>
                    <div>
                        <strong>Component:</strong><br>
                        ${{extractComponent(issue.file)}}
                    </div>
                    <div>
                        <strong>Fix Complexity:</strong><br>
                        <span style="color: ${{getComplexityColor(complexity)}};">
                            ${{complexity.toUpperCase()}}
                        </span>
                    </div>
                    <div>
                        <strong>Estimated Time:</strong><br>
                        ${{fixTime}} minutes
                    </div>
                    <div>
                        <strong>Auto-fixable:</strong><br>
                        ${{isAutoFixable(issue) ? '<i class="fas fa-check text-success"></i> Yes' : '<i class="fas fa-times text-danger"></i> No'}}
                    </div>
                </div>
                <div style="margin-top: 20px;">
                    <strong>Full Message:</strong><br>
                    <div style="margin-top: 10px; padding: 15px; background: var(--gray-200); border-radius: 8px; font-family: monospace; font-size: 0.9em;">
                        ${{escapeHtml(issue.message)}}
                    </div>
                </div>
            `;
        }}
        
        // Generate suggested fix
        function generateSuggestedFix(issue) {{
            const fixes = {{
                'missingOverride': `
                    <div class="fix-examples">
                        <div class="fix-example">
                            <h4><i class="fas fa-times-circle text-danger"></i> Current Code</h4>
                            <pre><code class="language-cpp">virtual void step(const sensor* sensors, int sensornumber,
                 motor* motors, int motornumber);</code></pre>
                        </div>
                        <div class="fix-example">
                            <h4><i class="fas fa-check-circle text-success"></i> Fixed Code</h4>
                            <pre><code class="language-cpp">void step(const sensor* sensors, int sensornumber,
         motor* motors, int motornumber) override;</code></pre>
                        </div>
                    </div>
                    <div style="margin-top: 20px; padding: 15px; background: var(--info); color: white; border-radius: 8px;">
                        <i class="fas fa-lightbulb"></i> <strong>Tip:</strong> Remove 'virtual' when using 'override' - it's redundant!
                    </div>
                `,
                'noExplicitConstructor': `
                    <div class="fix-examples">
                        <div class="fix-example">
                            <h4><i class="fas fa-times-circle text-danger"></i> Current Code</h4>
                            <pre><code class="language-cpp">class Widget {{
    Widget(int value);  // Implicit conversion allowed
}};</code></pre>
                        </div>
                        <div class="fix-example">
                            <h4><i class="fas fa-check-circle text-success"></i> Fixed Code</h4>
                            <pre><code class="language-cpp">class Widget {{
    explicit Widget(int value);  // Prevent implicit conversion
}};</code></pre>
                        </div>
                    </div>
                `
            }};
            
            return fixes[issue.id] || null;
        }}
        
        // Filter and display
        function filterAndDisplay() {{
            filteredIssues = allIssues.filter(issue => {{
                // Severity filter
                if (currentSeverity !== 'all' && issue.severity !== currentSeverity) {{
                    return false;
                }}
                
                // Search filter
                if (searchTerm) {{
                    const searchText = `${{issue.file}} ${{issue.message}} ${{issue.id}}`.toLowerCase();
                    if (!searchText.includes(searchTerm)) {{
                        return false;
                    }}
                }}
                
                return true;
            }});
            
            // Apply sorting
            sortIssues();
            
            // Update display
            displayIssues();
            updateSearchStatus();
        }}
        
        // Sort issues
        function sortIssues() {{
            const sortBy = document.getElementById('sortBy').value;
            
            filteredIssues.sort((a, b) => {{
                switch (sortBy) {{
                    case 'severity':
                        const severityOrder = {{error: 0, warning: 1, performance: 2, style: 3, information: 4}};
                        return (severityOrder[a.severity] || 5) - (severityOrder[b.severity] || 5);
                    case 'file':
                        return a.file.localeCompare(b.file);
                    case 'type':
                        return (a.id || '').localeCompare(b.id || '');
                    case 'component':
                        return extractComponent(a.file).localeCompare(extractComponent(b.file));
                    default:
                        return 0;
                }}
            }});
            
            displayIssues();
        }}
        
        // Filter by severity
        function filterBySeverity(severity) {{
            currentSeverity = severity;
            
            // Update button states
            document.querySelectorAll('.filter-btn[data-severity]').forEach(btn => {{
                btn.classList.toggle('active', btn.dataset.severity === severity);
            }});
            
            filterAndDisplay();
        }}
        
        // Toggle issue group
        function toggleGroup(severity) {{
            const group = document.getElementById('group-' + severity);
            const icon = document.getElementById('icon-' + severity);
            
            if (group.style.display === 'none') {{
                group.style.display = 'block';
                icon.className = 'fas fa-chevron-up';
            }} else {{
                group.style.display = 'none';
                icon.className = 'fas fa-chevron-down';
            }}
        }}
        
        // Show all issues in group
        function showAllInGroup(severity) {{
            const group = document.getElementById('group-' + severity);
            const issues = filteredIssues.filter(i => i.severity === severity);
            
            group.innerHTML = issues.map(issue => createIssueHtml(issue)).join('');
        }}
        
        // Show insight tab
        function showInsightTab(tab) {{
            // Hide all tabs
            document.querySelectorAll('.insight-tab').forEach(t => {{
                t.style.display = 'none';
            }});
            
            // Show selected tab
            document.getElementById(tab + '-content').style.display = 'block';
            
            // Update button states
            document.querySelectorAll('.tab-btn').forEach(btn => {{
                btn.classList.remove('active');
            }});
            event.target.classList.add('active');
        }}
        
        // Helper functions
        function extractComponent(filepath) {{
            if (filepath.includes('selforg')) return 'selforg';
            if (filepath.includes('ode_robots')) return 'ode_robots';
            if (filepath.includes('ga_tools')) return 'ga_tools';
            if (filepath.includes('opende')) return 'opende';
            return 'other';
        }}
        
        function extractFunction(message) {{
            const match = message.match(/'(\\w+)'\\s*(?:function|method|constructor)/);
            return match ? match[1] : null;
        }}
        
        function groupIssuesBySeverity(issues) {{
            const grouped = {{}};
            const order = ['error', 'warning', 'performance', 'style', 'information'];
            
            order.forEach(severity => {{
                grouped[severity] = issues.filter(i => i.severity === severity);
            }});
            
            return grouped;
        }}
        
        function estimateComplexity(issue) {{
            const complexityMap = {{
                'missingOverride': 'easy',
                'noExplicitConstructor': 'easy',
                'useAuto': 'easy',
                'passedByValue': 'medium',
                'uninitMemberVar': 'medium',
                'memleak': 'hard',
                'nullPointer': 'hard'
            }};
            
            return complexityMap[issue.id] || 'medium';
        }}
        
        function estimateFixTime(issue) {{
            const timeMap = {{
                'easy': 1,
                'medium': 5,
                'hard': 15
            }};
            
            const complexity = estimateComplexity(issue);
            return timeMap[complexity] || 5;
        }}
        
        function getComplexityColor(complexity) {{
            const colors = {{
                'easy': 'var(--success)',
                'medium': 'var(--warning)',
                'hard': 'var(--danger)'
            }};
            
            return colors[complexity] || 'var(--gray-600)';
        }}
        
        function getSeverityBadge(severity) {{
            const badges = {{
                'error': 'bg-danger-light',
                'warning': 'bg-warning-light',
                'style': 'bg-info-light',
                'performance': 'bg-success-light'
            }};
            
            return badges[severity] || '';
        }}
        
        function isAutoFixable(issue) {{
            const autoFixable = ['missingOverride', 'noExplicitConstructor', 'useAuto', 'nullptr'];
            return autoFixable.includes(issue.id);
        }}
        
        function updateSearchStatus() {{
            const status = document.getElementById('searchStatus');
            if (searchTerm) {{
                status.textContent = `Found ${{filteredIssues.length}} issues matching "${{searchTerm}}"`;
            }} else {{
                status.textContent = `Showing ${{filteredIssues.length}} of ${{allIssues.length}} issues`;
            }}
        }}
        
        function escapeHtml(text) {{
            const div = document.createElement('div');
            div.textContent = text;
            return div.innerHTML;
        }}
        
        function closeModal() {{
            document.getElementById('codeModal').style.display = 'none';
        }}
        
        function closeFixModal() {{
            document.getElementById('fixModal').style.display = 'none';
        }}
        
        // Initialize charts
        function initializeCharts() {{
            // Component breakdown chart
            const componentData = Object.entries(analytics.component_breakdown).map(([name, data]) => ({{
                name: name,
                total: data.total,
                priority: data.priority_score
            }})).sort((a, b) => b.total - a.total).slice(0, 5);
            
            // Add more charts as needed
        }}
        
        // Quick actions
        function quickFix(issueJson) {{
            const issue = JSON.parse(issueJson);
            alert('Quick fix for: ' + issue.id);
            // Implement quick fix logic
        }}
        
        function markResolved(issueJson) {{
            const issue = JSON.parse(issueJson);
            alert('Marked as resolved: ' + issue.id);
            // Implement mark resolved logic
        }}
        
        function batchFix(severity) {{
            alert('Batch fix for ' + severity + ' issues');
            // Implement batch fix logic
        }}
        
        // Export functions
        function exportIssues() {{
            const data = {{
                generated: new Date().toISOString(),
                profile: '{self.profile}',
                total_issues: filteredIssues.length,
                issues: filteredIssues
            }};
            
            const blob = new Blob([JSON.stringify(data, null, 2)], {{type: 'application/json'}});
            const url = URL.createObjectURL(blob);
            const a = document.createElement('a');
            a.href = url;
            a.download = 'cppcheck_issues_' + new Date().toISOString().split('T')[0] + '.json';
            a.click();
        }}
        
        // Count auto-fixable issues
        window.countAutoFixable = function() {{
            return {self._count_autofixable()};
        }};
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
        else:
            return 'other'
            
    def _shorten_path(self, path: str) -> str:
        """Shorten file path for display"""
        path_str = str(path)
        project_root_str = str(PROJECT_ROOT)
        if path_str.startswith(project_root_str):
            return path_str[len(project_root_str)+1:]
        return path_str
        
    def _estimate_issue_complexity(self, issue: Dict) -> str:
        """Estimate complexity of fixing an issue"""
        issue_id = issue.get('id', '')
        
        easy_patterns = ['missingOverride', 'noExplicitConstructor', 'useAuto', 'nullptr']
        hard_patterns = ['memleak', 'doubleFree', 'nullPointer', 'uninitMemberVar']
        
        if any(pattern in issue_id for pattern in easy_patterns):
            return 'easy'
        elif any(pattern in issue_id for pattern in hard_patterns):
            return 'hard'
        else:
            return 'medium'
            
    def _estimate_fix_time(self, issue: Dict) -> float:
        """Estimate time to fix an issue in hours"""
        complexity = self._estimate_issue_complexity(issue)
        time_map = {
            'easy': 0.1,  # 6 minutes
            'medium': 0.25,  # 15 minutes
            'hard': 0.5  # 30 minutes
        }
        return time_map.get(complexity, 0.25)
        
    def _get_fix_strategy(self, issue_id: str) -> str:
        """Get fix strategy for an issue type"""
        strategies = {
            'missingOverride': 'Add override specifier to virtual function',
            'noExplicitConstructor': 'Add explicit keyword to single-parameter constructor',
            'useAuto': 'Replace verbose type with auto',
            'passedByValue': 'Change to const reference parameter',
            'uninitMemberVar': 'Initialize in constructor initializer list',
            'memleak': 'Use RAII or smart pointers',
            'nullPointer': 'Add null check before dereference'
        }
        return strategies.get(issue_id, 'Manual review required')
        
    def _create_example(self, issue: Dict) -> Dict:
        """Create an example from an issue"""
        return {
            'file': self._shorten_path(issue.get('file', '')),
            'line': issue.get('line', ''),
            'message': issue.get('message', '')[:100] + '...' if len(issue.get('message', '')) > 100 else issue.get('message', '')
        }
        
    def _count_autofixable(self) -> int:
        """Count issues that can be automatically fixed"""
        autofixable_patterns = [
            'missingOverride', 'noExplicitConstructor', 'useAuto',
            'nullptr', 'passedByValue'
        ]
        
        count = 0
        for issue in self.issues:
            if any(pattern in issue.get('id', '') for pattern in autofixable_patterns):
                count += 1
                
        return count


def main():
    if len(sys.argv) < 2:
        print("Usage: generate_ultimate_report.py <report.json> [output.html]")
        sys.exit(1)
        
    input_path = Path(sys.argv[1])
    output_path = Path(sys.argv[2]) if len(sys.argv) > 2 else input_path.with_suffix('.ultimate.html')
    
    # Load report data
    with open(input_path, encoding='utf-8') as f:
        report_data = json.load(f)
        
    # Generate ultimate dashboard
    generator = UltimateDashboardGenerator(report_data)
    generator.generate_ultimate_dashboard(output_path)


if __name__ == '__main__':
    main()