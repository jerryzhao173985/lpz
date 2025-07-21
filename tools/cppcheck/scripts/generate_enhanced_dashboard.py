#!/usr/bin/env python3
"""
Enhanced Cppcheck Dashboard Generator with Interactive Code Preview and Fix Suggestions
"""

import json
import sys
from pathlib import Path
from typing import Dict, List, Optional
from datetime import datetime
from html import escape as html_escape
import base64

# Import our new modules
sys.path.insert(0, str(Path(__file__).parent))
from code_context_extractor import CodeContextExtractor
from fix_generator import FixGenerator

class EnhancedDashboardGenerator:
    def __init__(self, analysis_results: Dict):
        self.results = analysis_results
        self.issues = analysis_results.get('issues', [])
        self.stats = analysis_results.get('stats', {})
        self.profile = analysis_results.get('profile', 'unknown')
        
        # Initialize helpers
        self.project_root = Path(__file__).parent.parent.parent
        self.context_extractor = CodeContextExtractor(self.project_root)
        self.fix_generator = FixGenerator()
        
        # Pre-extract code contexts for efficiency
        self._extract_code_contexts()
        
    def _extract_code_contexts(self):
        """Pre-extract code context for each issue"""
        print("Extracting code contexts for issues...")
        for i, issue in enumerate(self.issues):
            if i % 100 == 0:
                print(f"  Processing issue {i}/{len(self.issues)}...")
                
            filepath = issue.get('file', '')
            line_num = int(issue.get('line', 0))
            
            if filepath and line_num:
                context = self.context_extractor.extract_context(filepath, line_num)
                issue['code_context'] = context
                
                # Generate fix suggestion
                if context.get('success'):
                    fix = self.fix_generator.generate_fix(issue, context)
                    issue['fix_suggestion'] = fix
                    
    def generate_enhanced_dashboard(self, output_path: Path):
        """Generate the enhanced dashboard with all features"""
        html_content = self._generate_html()
        
        with open(output_path, 'w', encoding='utf-8') as f:
            f.write(html_content)
            
    def _generate_html(self) -> str:
        """Generate complete HTML dashboard"""
        # Encode issue data for JavaScript
        issues_json = json.dumps(self.issues)
        issues_b64 = base64.b64encode(issues_json.encode()).decode()
        
        html = f"""<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Enhanced Cppcheck Dashboard - {self.profile}</title>
    
    <!-- External Dependencies -->
    <link href="https://fonts.googleapis.com/css2?family=Inter:wght@400;500;600;700;800&display=swap" rel="stylesheet">
    <link href="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/6.0.0/css/all.min.css" rel="stylesheet">
    
    <!-- Highlight.js for syntax highlighting -->
    <link href="https://cdnjs.cloudflare.com/ajax/libs/highlight.js/11.9.0/styles/github-dark.min.css" rel="stylesheet">
    
    <!-- diff2html for diff viewing -->
    <link href="https://cdn.jsdelivr.net/npm/diff2html@3.4.35/bundles/css/diff2html.min.css" rel="stylesheet">
    
    <!-- Scripts -->
    <script src="https://cdnjs.cloudflare.com/ajax/libs/highlight.js/11.9.0/highlight.min.js"></script>
    <script src="https://cdnjs.cloudflare.com/ajax/libs/highlight.js/11.9.0/languages/cpp.min.js"></script>
    <script src="https://cdn.plot.ly/plotly-latest.min.js"></script>
    <script src="https://cdn.jsdelivr.net/npm/diff2html@3.4.35/bundles/js/diff2html-ui.min.js"></script>
    
    <style>
        {self._generate_styles()}
    </style>
</head>
<body>
    <div class="app-container">
        {self._generate_header()}
        {self._generate_summary_cards()}
        {self._generate_issue_explorer()}
        {self._generate_code_modal()}
        {self._generate_keyboard_shortcuts_modal()}
    </div>
    
    <script>
        // Decode issues data
        const issuesData = JSON.parse(atob('{issues_b64}'));
        
        {self._generate_javascript()}
    </script>
</body>
</html>"""
        
        return html
        
    def _generate_styles(self) -> str:
        """Generate comprehensive styles"""
        return """
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
        }
        
        .app-container {
            min-height: 100vh;
            padding-bottom: 50px;
        }
        
        /* Header */
        .header {
            background: linear-gradient(135deg, var(--primary) 0%, var(--secondary) 100%);
            color: white;
            padding: 40px 0;
            box-shadow: var(--shadow-lg);
        }
        
        .header-content {
            max-width: 1400px;
            margin: 0 auto;
            padding: 0 20px;
        }
        
        .header h1 {
            font-size: 2.5em;
            font-weight: 800;
            margin-bottom: 10px;
        }
        
        /* Summary Cards */
        .summary-cards {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(250px, 1fr));
            gap: 20px;
            max-width: 1400px;
            margin: -30px auto 30px;
            padding: 0 20px;
        }
        
        .summary-card {
            background: white;
            border-radius: 12px;
            padding: 25px;
            box-shadow: var(--shadow);
            position: relative;
            overflow: hidden;
            transition: transform 0.2s, box-shadow 0.2s;
        }
        
        .summary-card:hover {
            transform: translateY(-2px);
            box-shadow: var(--shadow-xl);
        }
        
        .summary-card-icon {
            width: 50px;
            height: 50px;
            border-radius: 10px;
            display: flex;
            align-items: center;
            justify-content: center;
            font-size: 24px;
            margin-bottom: 15px;
        }
        
        .summary-card h3 {
            font-size: 0.9em;
            color: var(--gray-600);
            text-transform: uppercase;
            letter-spacing: 0.05em;
            margin-bottom: 5px;
        }
        
        .summary-card .value {
            font-size: 2.5em;
            font-weight: 700;
            margin-bottom: 5px;
        }
        
        /* Issue Explorer */
        .issue-explorer {
            max-width: 1400px;
            margin: 0 auto;
            padding: 0 20px;
        }
        
        .explorer-header {
            background: white;
            border-radius: 12px 12px 0 0;
            padding: 20px;
            border-bottom: 1px solid var(--gray-300);
        }
        
        .explorer-controls {
            display: flex;
            gap: 15px;
            flex-wrap: wrap;
            align-items: center;
        }
        
        .search-box {
            flex: 1;
            min-width: 300px;
            position: relative;
        }
        
        .search-box input {
            width: 100%;
            padding: 10px 40px 10px 15px;
            border: 1px solid var(--gray-300);
            border-radius: 8px;
            font-size: 0.95em;
            transition: border-color 0.2s;
        }
        
        .search-box input:focus {
            outline: none;
            border-color: var(--primary);
        }
        
        .search-box i {
            position: absolute;
            right: 15px;
            top: 50%;
            transform: translateY(-50%);
            color: var(--gray-500);
        }
        
        .filter-btn {
            padding: 10px 20px;
            border: 1px solid var(--gray-300);
            background: white;
            border-radius: 8px;
            cursor: pointer;
            font-size: 0.95em;
            transition: all 0.2s;
        }
        
        .filter-btn:hover {
            background: var(--gray-100);
            border-color: var(--gray-400);
        }
        
        .filter-btn.active {
            background: var(--primary);
            color: white;
            border-color: var(--primary);
        }
        
        /* Issue List */
        .issue-list {
            background: white;
            border-radius: 0 0 12px 12px;
            overflow: hidden;
        }
        
        .issue-row {
            padding: 20px;
            border-bottom: 1px solid var(--gray-200);
            cursor: pointer;
            transition: background 0.2s;
            position: relative;
        }
        
        .issue-row:hover {
            background: var(--gray-50);
        }
        
        .issue-row.expanded {
            background: var(--gray-100);
        }
        
        .issue-main {
            display: flex;
            justify-content: space-between;
            align-items: start;
        }
        
        .issue-info {
            flex: 1;
        }
        
        .issue-location {
            font-family: 'Monaco', 'Consolas', monospace;
            font-size: 0.9em;
            color: var(--gray-700);
            margin-bottom: 5px;
        }
        
        .issue-message {
            color: var(--gray-800);
            margin-bottom: 10px;
        }
        
        .issue-meta {
            display: flex;
            gap: 10px;
            align-items: center;
        }
        
        .issue-badge {
            padding: 4px 10px;
            border-radius: 12px;
            font-size: 0.85em;
            font-weight: 500;
        }
        
        .issue-actions {
            display: flex;
            gap: 10px;
        }
        
        .action-btn {
            padding: 8px 16px;
            border: 1px solid var(--gray-300);
            background: white;
            border-radius: 6px;
            cursor: pointer;
            font-size: 0.9em;
            transition: all 0.2s;
            display: flex;
            align-items: center;
            gap: 5px;
        }
        
        .action-btn:hover {
            background: var(--primary);
            color: white;
            border-color: var(--primary);
        }
        
        /* Code Context Expansion */
        .code-context {
            margin-top: 20px;
            background: var(--gray-900);
            border-radius: 8px;
            overflow: hidden;
            animation: slideDown 0.3s ease-out;
        }
        
        @keyframes slideDown {
            from {
                opacity: 0;
                transform: translateY(-10px);
            }
            to {
                opacity: 1;
                transform: translateY(0);
            }
        }
        
        .code-header {
            background: var(--gray-800);
            padding: 10px 15px;
            display: flex;
            justify-content: space-between;
            align-items: center;
        }
        
        .code-breadcrumb {
            color: var(--gray-300);
            font-size: 0.9em;
            font-family: monospace;
        }
        
        .code-actions {
            display: flex;
            gap: 10px;
        }
        
        .code-action {
            padding: 5px 10px;
            background: var(--gray-700);
            border: none;
            border-radius: 4px;
            color: var(--gray-300);
            cursor: pointer;
            font-size: 0.85em;
            transition: background 0.2s;
        }
        
        .code-action:hover {
            background: var(--gray-600);
        }
        
        .code-content {
            position: relative;
            overflow-x: auto;
            font-family: 'Monaco', 'Consolas', monospace;
            font-size: 0.9em;
        }
        
        .code-content pre {
            padding: 15px;
            margin: 0;
            background: transparent !important;
        }
        
        .code-content pre code {
            background: transparent !important;
            padding: 0 !important;
            line-height: 1.6;
        }
        
        /* Line numbers and indicators */
        .code-with-lines {
            display: flex;
            position: relative;
        }
        
        .line-numbers {
            background: var(--gray-800);
            padding: 15px 10px;
            text-align: right;
            user-select: none;
            color: var(--gray-600);
            font-size: 0.9em;
            line-height: 1.6;
            border-right: 1px solid var(--gray-700);
        }
        
        .line-number {
            display: block;
        }
        
        .line-number.target {
            color: var(--danger);
            font-weight: bold;
        }
        
        /* Line indicators overlay */
        .line-indicators {
            position: absolute;
            top: 0;
            left: 0;
            width: 100%;
            height: 100%;
            pointer-events: none;
        }
        
        .line-indicator {
            position: absolute;
            transition: all 0.2s;
        }
        
        .target-indicator {
            background: rgba(239, 68, 68, 0.15);
            border-left: 3px solid var(--danger);
        }
        
        /* Highlight.js overrides for dark theme */
        .hljs {
            background: transparent !important;
            color: #abb2bf;
        }
        
        .hljs-keyword { color: #c678dd; }
        .hljs-string { color: #98c379; }
        .hljs-function { color: #61afef; }
        .hljs-number { color: #d19a66; }
        .hljs-comment { color: #5c6370; }
        
        /* Fix Preview */
        .fix-preview {
            margin-top: 20px;
            background: white;
            border: 1px solid var(--gray-300);
            border-radius: 8px;
            overflow: hidden;
        }
        
        .fix-header {
            background: var(--gray-100);
            padding: 15px;
            border-bottom: 1px solid var(--gray-300);
            display: flex;
            justify-content: space-between;
            align-items: center;
        }
        
        .fix-title {
            font-weight: 600;
            display: flex;
            align-items: center;
            gap: 10px;
        }
        
        .fix-actions {
            display: flex;
            gap: 10px;
        }
        
        .confidence-badge {
            padding: 4px 10px;
            border-radius: 12px;
            font-size: 0.85em;
            font-weight: 500;
            background: var(--success);
            color: white;
        }
        
        .fix-content {
            padding: 20px;
        }
        
        .fix-description {
            margin-bottom: 15px;
            color: var(--gray-700);
        }
        
        .diff-container {
            margin-top: 15px;
            border: 1px solid var(--gray-300);
            border-radius: 6px;
            overflow: hidden;
            min-height: 100px;
        }
        
        /* diff2html custom styles */
        .d2h-wrapper {
            font-family: 'Monaco', 'Consolas', monospace;
            font-size: 0.9em;
        }
        
        .d2h-file-header {
            display: none;
        }
        
        .d2h-file-wrapper {
            border: none;
        }
        
        .d2h-diff-table {
            font-size: 0.9em;
        }
        
        .d2h-code-side-linenumber {
            background: var(--gray-800);
            color: var(--gray-500);
            border-right: 1px solid var(--gray-600);
        }
        
        .d2h-del {
            background: rgba(239, 68, 68, 0.2);
        }
        
        .d2h-ins {
            background: rgba(72, 187, 120, 0.2);
        }
        
        .d2h-code-side-line {
            padding: 0 10px;
        }
        
        /* Diff Styles */
        .d2h-wrapper {
            font-size: 0.9em;
        }
        
        /* Code Modal */
        .modal {
            display: none;
            position: fixed;
            top: 0;
            left: 0;
            width: 100%;
            height: 100%;
            background: rgba(0, 0, 0, 0.8);
            z-index: 1000;
            overflow-y: auto;
        }
        
        .modal-content {
            background: white;
            max-width: 1200px;
            margin: 50px auto;
            border-radius: 12px;
            box-shadow: var(--shadow-xl);
            max-height: 90vh;
            display: flex;
            flex-direction: column;
        }
        
        .modal-header {
            padding: 20px;
            border-bottom: 1px solid var(--gray-300);
            display: flex;
            justify-content: space-between;
            align-items: center;
        }
        
        .modal-body {
            padding: 20px;
            overflow-y: auto;
            flex: 1;
        }
        
        .close-btn {
            width: 36px;
            height: 36px;
            border-radius: 50%;
            border: none;
            background: var(--gray-200);
            cursor: pointer;
            display: flex;
            align-items: center;
            justify-content: center;
            transition: background 0.2s;
        }
        
        .close-btn:hover {
            background: var(--gray-300);
        }
        
        /* Keyboard shortcuts */
        .shortcuts-hint {
            position: fixed;
            bottom: 20px;
            right: 20px;
            background: var(--dark);
            color: white;
            padding: 10px 15px;
            border-radius: 8px;
            font-size: 0.9em;
            box-shadow: var(--shadow-lg);
        }
        
        /* Responsive */
        @media (max-width: 768px) {
            .summary-cards {
                grid-template-columns: 1fr;
            }
            
            .explorer-controls {
                flex-direction: column;
            }
            
            .search-box {
                min-width: 100%;
            }
        }
        """
        
    def _generate_header(self) -> str:
        """Generate header section"""
        total_issues = len(self.issues)
        quality_score = self._calculate_quality_score()
        
        return f"""
        <header class="header">
            <div class="header-content">
                <h1>Enhanced Cppcheck Analysis</h1>
                <p>Profile: {self.profile} | Generated: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}</p>
                <div style="margin-top: 20px;">
                    <span style="font-size: 1.2em;">Total Issues: {total_issues:,}</span>
                    <span style="margin-left: 30px;">Quality Score: {quality_score}</span>
                </div>
            </div>
        </header>
        """
        
    def _generate_summary_cards(self) -> str:
        """Generate summary statistics cards"""
        severity_counts = {}
        for issue in self.issues:
            severity = issue.get('severity', 'unknown')
            severity_counts[severity] = severity_counts.get(severity, 0) + 1
            
        cards_html = '<div class="summary-cards">'
        
        card_configs = [
            ('error', 'Errors', severity_counts.get('error', 0), '#f56565', 'fa-times-circle'),
            ('warning', 'Warnings', severity_counts.get('warning', 0), '#ed8936', 'fa-exclamation-triangle'),
            ('style', 'Style', severity_counts.get('style', 0), '#4299e1', 'fa-palette'),
            ('performance', 'Performance', severity_counts.get('performance', 0), '#48bb78', 'fa-tachometer-alt'),
        ]
        
        for key, label, count, color, icon in card_configs:
            cards_html += f"""
            <div class="summary-card">
                <div class="summary-card-icon" style="background: {color}20; color: {color};">
                    <i class="fas {icon}"></i>
                </div>
                <h3>{label}</h3>
                <div class="value" style="color: {color};">{count:,}</div>
                <div style="color: var(--gray-600); font-size: 0.9em;">
                    {count / len(self.issues) * 100:.1f}% of total
                </div>
            </div>
            """
            
        cards_html += '</div>'
        return cards_html
        
    def _generate_issue_explorer(self) -> str:
        """Generate the main issue explorer interface"""
        return """
        <div class="issue-explorer">
            <div class="explorer-header">
                <h2 style="margin-bottom: 20px;">Issue Explorer</h2>
                <div class="explorer-controls">
                    <div class="search-box">
                        <input type="text" id="searchInput" placeholder="Search issues..." onkeyup="filterIssues()">
                        <i class="fas fa-search"></i>
                    </div>
                    <button class="filter-btn" onclick="toggleFilter('error')" data-filter="error">
                        <i class="fas fa-times-circle"></i> Errors
                    </button>
                    <button class="filter-btn" onclick="toggleFilter('warning')" data-filter="warning">
                        <i class="fas fa-exclamation-triangle"></i> Warnings
                    </button>
                    <button class="filter-btn" onclick="toggleFilter('style')" data-filter="style">
                        <i class="fas fa-palette"></i> Style
                    </button>
                    <button class="filter-btn" onclick="toggleFilter('performance')" data-filter="performance">
                        <i class="fas fa-tachometer-alt"></i> Performance
                    </button>
                </div>
            </div>
            <div class="issue-list" id="issueList">
                <!-- Issues will be populated by JavaScript -->
            </div>
            <div style="padding: 20px; background: white; text-align: center;">
                <div id="pagination"></div>
            </div>
        </div>
        """
        
    def _generate_code_modal(self) -> str:
        """Generate code view modal"""
        return """
        <div id="codeModal" class="modal">
            <div class="modal-content">
                <div class="modal-header">
                    <h2 id="modalTitle">Code View</h2>
                    <button class="close-btn" onclick="closeModal()">
                        <i class="fas fa-times"></i>
                    </button>
                </div>
                <div class="modal-body">
                    <div id="codeContent"></div>
                </div>
            </div>
        </div>
        """
        
    def _generate_keyboard_shortcuts_modal(self) -> str:
        """Generate keyboard shortcuts hint"""
        return """
        <div class="shortcuts-hint">
            <strong>Keyboard Shortcuts:</strong> 
            <kbd>j</kbd>/<kbd>k</kbd> Navigate • 
            <kbd>Space</kbd> Expand • 
            <kbd>f</kbd> Show Fix • 
            <kbd>?</kbd> Help
        </div>
        """
        
    def _generate_javascript(self) -> str:
        """Generate all JavaScript functionality"""
        return """
        // Global state
        let currentPage = 1;
        const itemsPerPage = 50;
        let filteredIssues = [];  // Initialize empty, will be populated after issuesData is available
        let activeFilters = new Set();
        let expandedRows = new Set();
        let selectedRow = -1;
        
        // Helper function to shorten file paths
        function shortenPath(fullPath) {
            if (!fullPath) return 'unknown';
            // Remove common prefixes
            const prefixes = [
                '/Users/jerry/simulator/lpz/',
                '/home/jerry/lpz/',
                process.cwd() + '/'
            ];
            
            let shortPath = fullPath;
            for (const prefix of prefixes) {
                if (fullPath.startsWith(prefix)) {
                    shortPath = fullPath.substring(prefix.length);
                    break;
                }
            }
            return shortPath;
        }
        
        // Severity badges
        const severityBadges = {
            'error': 'background: #fee; color: #c53030;',
            'warning': 'background: #fef6e7; color: #c05621;',
            'style': 'background: #e0e7ff; color: #3730a3;',
            'performance': 'background: #d1fae5; color: #065f46;',
            'information': 'background: #f3f4f6; color: #374151;',
            'portability': 'background: #fef3c7; color: #92400e;',
            'unknown': 'background: #e5e7eb; color: #6b7280;'
        };
        
        // Initialize
        document.addEventListener('DOMContentLoaded', function() {
            console.log('DOMContentLoaded - initializing dashboard');
            console.log('issuesData length:', issuesData.length);
            console.log('First issue:', issuesData[0]);
            
            // Initialize filteredIssues after issuesData is available
            filteredIssues = [...issuesData];
            
            renderIssues();
            setupKeyboardShortcuts();
        });
        
        // Render issues
        function renderIssues() {
            console.log('renderIssues called');
            console.log('filteredIssues length:', filteredIssues.length);
            
            const start = (currentPage - 1) * itemsPerPage;
            const end = start + itemsPerPage;
            const pageIssues = filteredIssues.slice(start, end);
            
            console.log('Rendering issues', start, 'to', end, '- total on page:', pageIssues.length);
            
            const listElement = document.getElementById('issueList');
            if (!listElement) {
                console.error('issueList element not found!');
                return;
            }
            listElement.innerHTML = '';
            
            pageIssues.forEach((issue, index) => {
                const globalIndex = start + index;
                const row = createIssueRow(issue, globalIndex);
                listElement.appendChild(row);
            });
            
            renderPagination();
        }
        
        // Create issue row
        function createIssueRow(issue, index) {
            const row = document.createElement('div');
            row.className = 'issue-row';
            if (expandedRows.has(index)) {
                row.classList.add('expanded');
            }
            if (selectedRow === index) {
                row.style.outline = '2px solid var(--primary)';
            }
            
            row.innerHTML = `
                <div class="issue-main" onclick="toggleRow(${index})">
                    <div class="issue-info">
                        <div class="issue-location">
                            <i class="fas fa-file-code"></i> ${shortenPath(issue.file)}:${issue.line}
                        </div>
                        <div class="issue-message">${issue.message || 'No message'}</div>
                        <div class="issue-meta">
                            <span class="issue-badge" style="${severityBadges[issue.severity] || ''}">
                                ${issue.severity || 'unknown'}
                            </span>
                            ${issue.id ? `<span class="issue-badge" style="background: var(--gray-200); color: var(--gray-700);">${issue.id}</span>` : ''}
                        </div>
                    </div>
                    <div class="issue-actions" onclick="event.stopPropagation();">
                        ${issue.fix_suggestion ? `
                        <button class="action-btn" onclick="showFix(${index})">
                            <i class="fas fa-wrench"></i> Fix
                        </button>
                        ` : ''}
                        <button class="action-btn" onclick="viewFullCode(${index})">
                            <i class="fas fa-code"></i> View
                        </button>
                    </div>
                </div>
            `;
            
            // Add expanded content if row is expanded
            if (expandedRows.has(index)) {
                const expansion = createExpansion(issue);
                row.appendChild(expansion);
            }
            
            return row;
        }
        
        // Create expansion content
        function createExpansion(issue) {
            const expansion = document.createElement('div');
            expansion.className = 'code-context';
            
            if (issue.code_context && issue.code_context.success) {
                const context = issue.code_context;
                let codeHtml = `
                    <div class="code-header">
                        <div class="code-breadcrumb">
                            ${context.class ? `${context.class.name} › ` : ''}
                            ${context.function || 'Global scope'}
                        </div>
                        <div class="code-actions">
                            <button class="code-action" onclick="copyCode(${JSON.stringify(context.lines).replace(/"/g, '&quot;')})">
                                <i class="fas fa-copy"></i> Copy
                            </button>
                        </div>
                    </div>
                    <div class="code-content">
                        <pre style="margin: 0; background: transparent;"><code class="language-cpp">`;
                
                // Build the code as a single string for proper highlighting
                const codeText = context.lines.map(line => line.content).join('\n');
                codeHtml += escapeHtml(codeText);
                
                codeHtml += `</code></pre>
                        <div class="line-indicators">`;
                
                // Add line number indicators overlay
                context.lines.forEach(line => {
                    const isTarget = line.is_target ? 'target-indicator' : '';
                    codeHtml += `<div class="line-indicator ${isTarget}" data-line="${line.number}"></div>`;
                });
                
                codeHtml += '</div></div>';
                
                // Add fix preview if available
                if (issue.fix_suggestion && issue.fix_suggestion.success) {
                    codeHtml += createFixPreview(issue.fix_suggestion);
                }
                
                expansion.innerHTML = codeHtml;
                
                // Highlight syntax using Highlight.js
                expansion.querySelectorAll('pre code').forEach((block) => {
                    hljs.highlightElement(block);
                });
                
                // Position line indicators
                positionLineIndicators(expansion, context);
            } else {
                expansion.innerHTML = `
                    <div style="padding: 20px; color: var(--gray-600); text-align: center;">
                        <i class="fas fa-exclamation-circle"></i> 
                        Unable to load code context
                    </div>
                `;
            }
            
            return expansion;
        }
        
        // Create fix preview
        function createFixPreview(fix) {
            if (!fix.diff) return '';
            
            const confidence = fix.confidence || 50;
            const confidenceColor = confidence >= 80 ? 'var(--success)' : 
                                   confidence >= 60 ? 'var(--warning)' : 
                                   'var(--danger)';
            
            const diffId = 'diff-' + Date.now() + '-' + Math.random().toString(36).substr(2, 9);
            
            // Store diff data for later rendering
            if (!window.pendingDiffs) window.pendingDiffs = {};
            window.pendingDiffs[diffId] = fix.diff;
            
            return `
                <div class="fix-preview">
                    <div class="fix-header">
                        <div class="fix-title">
                            <i class="fas fa-magic"></i> Suggested Fix
                            <span class="confidence-badge" style="background: ${confidenceColor};">
                                ${confidence}% confidence
                            </span>
                        </div>
                        <div class="fix-actions">
                            <button class="action-btn" onclick="toggleDiffView('${diffId}')">
                                <i class="fas fa-columns"></i> Toggle View
                            </button>
                            <button class="action-btn" onclick="applyFix(this, '${diffId}')">
                                <i class="fas fa-check"></i> Apply
                            </button>
                        </div>
                    </div>
                    <div class="fix-content">
                        <p class="fix-description">${fix.description}</p>
                        ${fix.explanation ? `<p style="color: var(--gray-600); font-size: 0.9em; margin-top: 10px;">${fix.explanation}</p>` : ''}
                        <div class="diff-container" id="${diffId}" data-diff-mode="side-by-side"></div>
                    </div>
                </div>
            `;
        }
        
        // Toggle row expansion
        function toggleRow(index) {
            if (expandedRows.has(index)) {
                expandedRows.delete(index);
            } else {
                expandedRows.add(index);
            }
            selectedRow = index;
            renderIssues();
        }
        
        // Filter issues
        function filterIssues() {
            const searchTerm = document.getElementById('searchInput').value.toLowerCase();
            
            filteredIssues = issuesData.filter(issue => {
                // Check active severity filters
                if (activeFilters.size > 0 && !activeFilters.has(issue.severity)) {
                    return false;
                }
                
                // Check search term
                if (searchTerm) {
                    const searchableText = `
                        ${issue.file} 
                        ${issue.message} 
                        ${issue.id || ''} 
                        ${issue.severity}
                    `.toLowerCase();
                    
                    if (!searchableText.includes(searchTerm)) {
                        return false;
                    }
                }
                
                return true;
            });
            
            currentPage = 1;
            renderIssues();
        }
        
        // Toggle filter
        function toggleFilter(severity) {
            const btn = document.querySelector(`[data-filter="${severity}"]`);
            
            if (activeFilters.has(severity)) {
                activeFilters.delete(severity);
                btn.classList.remove('active');
            } else {
                activeFilters.add(severity);
                btn.classList.add('active');
            }
            
            filterIssues();
        }
        
        // Render pagination
        function renderPagination() {
            const totalPages = Math.ceil(filteredIssues.length / itemsPerPage);
            const pagination = document.getElementById('pagination');
            
            if (totalPages <= 1) {
                pagination.innerHTML = '';
                return;
            }
            
            let html = '';
            
            // Previous button
            if (currentPage > 1) {
                html += `<button onclick="changePage(${currentPage - 1})" style="margin: 0 5px; padding: 5px 10px;">Previous</button>`;
            }
            
            // Page numbers
            for (let i = 1; i <= Math.min(totalPages, 10); i++) {
                const active = i === currentPage ? 'background: var(--primary); color: white;' : '';
                html += `<button onclick="changePage(${i})" style="margin: 0 5px; padding: 5px 10px; ${active}">${i}</button>`;
            }
            
            // Next button
            if (currentPage < totalPages) {
                html += `<button onclick="changePage(${currentPage + 1})" style="margin: 0 5px; padding: 5px 10px;">Next</button>`;
            }
            
            pagination.innerHTML = html;
        }
        
        // Change page
        function changePage(page) {
            currentPage = page;
            renderIssues();
            window.scrollTo(0, 0);
        }
        
        // Keyboard shortcuts
        function setupKeyboardShortcuts() {
            document.addEventListener('keydown', function(e) {
                // Don't trigger shortcuts when typing in search
                if (e.target.tagName === 'INPUT') return;
                
                switch(e.key) {
                    case 'j':
                        navigateDown();
                        break;
                    case 'k':
                        navigateUp();
                        break;
                    case ' ':
                        e.preventDefault();
                        if (selectedRow >= 0) {
                            toggleRow(selectedRow);
                        }
                        break;
                    case 'f':
                        if (selectedRow >= 0) {
                            showFix(selectedRow);
                        }
                        break;
                    case '?':
                        showHelp();
                        break;
                    case 'Escape':
                        closeModal();
                        break;
                }
            });
        }
        
        // Navigation functions
        function navigateDown() {
            const start = (currentPage - 1) * itemsPerPage;
            const end = Math.min(start + itemsPerPage, filteredIssues.length);
            
            if (selectedRow < end - 1) {
                selectedRow++;
                renderIssues();
                scrollToRow(selectedRow - start);
            }
        }
        
        function navigateUp() {
            const start = (currentPage - 1) * itemsPerPage;
            
            if (selectedRow > start) {
                selectedRow--;
                renderIssues();
                scrollToRow(selectedRow - start);
            }
        }
        
        function scrollToRow(localIndex) {
            const rows = document.querySelectorAll('.issue-row');
            if (rows[localIndex]) {
                rows[localIndex].scrollIntoView({ behavior: 'smooth', block: 'center' });
            }
        }
        
        // Helper functions
        function escapeHtml(unsafe) {
            return unsafe
                .replace(/&/g, "&amp;")
                .replace(/</g, "&lt;")
                .replace(/>/g, "&gt;")
                .replace(/"/g, "&quot;")
                .replace(/'/g, "&#039;");
        }
        
        function copyCode(lines) {
            const text = lines.map(l => l.content).join('\\n');
            navigator.clipboard.writeText(text).then(() => {
                alert('Code copied to clipboard!');
            });
        }
        
        function closeModal() {
            document.getElementById('codeModal').style.display = 'none';
        }
        
        function showHelp() {
            alert(`Keyboard Shortcuts:
            
j/k - Navigate up/down
Space - Expand/collapse issue
f - Show fix suggestion
? - Show this help
            
Click on any issue to see code context and fix suggestions!`);
        }
        
        // Render diff using diff2html
        function renderDiff(diffId) {
            const diffData = window.pendingDiffs[diffId];
            if (!diffData) return;
            
            const targetElement = document.getElementById(diffId);
            const outputFormat = targetElement.getAttribute('data-diff-mode') || 'side-by-side';
            
            const configuration = {
                drawFileList: false,
                matching: 'lines',
                outputFormat: outputFormat,
                synchronisedScroll: true,
                highlight: true,
                renderNothingWhenEmpty: false
            };
            
            try {
                const diff2htmlUi = new Diff2HtmlUI(targetElement, diffData, configuration);
                diff2htmlUi.draw();
                diff2htmlUi.highlightCode();
                
                // Clean up
                delete window.pendingDiffs[diffId];
            } catch (e) {
                console.error('Error rendering diff:', e);
                targetElement.innerHTML = '<pre>' + escapeHtml(diffData) + '</pre>';
            }
        }
        
        // Toggle diff view between side-by-side and line-by-line
        function toggleDiffView(diffId) {
            const targetElement = document.getElementById(diffId);
            const currentMode = targetElement.getAttribute('data-diff-mode') || 'side-by-side';
            const newMode = currentMode === 'side-by-side' ? 'line-by-line' : 'side-by-side';
            
            targetElement.setAttribute('data-diff-mode', newMode);
            targetElement.innerHTML = ''; // Clear current content
            
            // Re-store the diff data and re-render
            const diffData = targetElement.parentElement.parentElement.querySelector('pre')?.textContent || window.lastDiffData[diffId];
            if (diffData) {
                window.pendingDiffs[diffId] = diffData;
                renderDiff(diffId);
            }
        }
        
        // Position line indicators for highlighted lines
        function positionLineIndicators(container, context) {
            const codeBlock = container.querySelector('pre code');
            if (!codeBlock) return;
            
            const lines = codeBlock.innerHTML.split('\\n');
            const lineHeight = parseFloat(getComputedStyle(codeBlock).lineHeight);
            const indicators = container.querySelectorAll('.line-indicator');
            
            indicators.forEach((indicator, index) => {
                const line = context.lines[index];
                if (line.is_target) {
                    indicator.style.position = 'absolute';
                    indicator.style.left = '0';
                    indicator.style.width = '100%';
                    indicator.style.height = lineHeight + 'px';
                    indicator.style.top = (index * lineHeight) + 'px';
                    indicator.style.background = 'rgba(239, 68, 68, 0.2)';
                    indicator.style.borderLeft = '3px solid var(--danger)';
                    indicator.style.pointerEvents = 'none';
                }
            });
        }
        
        // Show fix for selected row
        function showFix(index) {
            const issue = filteredIssues[index];
            if (!issue || !issue.fix_suggestion) {
                alert('No fix suggestion available for this issue');
                return;
            }
            
            // Expand the row if not already expanded
            if (!expandedRows.has(index)) {
                toggleRow(index);
            }
            
            // Scroll to the fix preview
            setTimeout(() => {
                const fixPreview = document.querySelector('.issue-row.expanded .fix-preview');
                if (fixPreview) {
                    fixPreview.scrollIntoView({ behavior: 'smooth', block: 'center' });
                }
            }, 300);
        }
        
        // View full code in modal
        function viewFullCode(index) {
            const issue = filteredIssues[index];
            if (!issue || !issue.code_context) {
                alert('No code context available for this issue');
                return;
            }
            
            const modal = document.getElementById('codeModal');
            const modalTitle = document.getElementById('modalTitle');
            const codeContent = document.getElementById('codeContent');
            
            // Set modal title
            modalTitle.textContent = `${issue.file}:${issue.line} - ${issue.message}`;
            
            // Build code content
            const context = issue.code_context;
            let html = `
                <div class="code-context">
                    <div class="code-header">
                        <div class="code-breadcrumb">
                            ${issue.file} • Line ${issue.line}
                            ${context.class ? ` • ${context.class.name}` : ''}
                            ${context.function ? ` • ${context.function}()` : ''}
                        </div>
                        <div class="code-actions">
                            <button class="code-action" onclick="copyFullCode()">
                                <i class="fas fa-copy"></i> Copy All
                            </button>
                        </div>
                    </div>
                    <div class="code-content">
                        <div class="code-with-lines">
                            <div class="line-numbers">`;
            
            // Add line numbers
            context.lines.forEach(line => {
                html += `<span class="line-number ${line.is_target ? 'target' : ''}">${line.number}</span>`;
            });
            
            html += `</div>
                            <pre><code class="language-cpp">`;
            
            // Add code
            const codeText = context.lines.map(line => line.content).join('\n');
            html += escapeHtml(codeText);
            
            html += `</code></pre>
                        </div>
                    </div>
                </div>`;
            
            // Add issue details
            html += `
                <div style="margin-top: 20px; padding: 15px; background: #f8f9fa; border-radius: 8px;">
                    <h3 style="margin-bottom: 10px;">Issue Details</h3>
                    <p><strong>Type:</strong> ${issue.id || 'Unknown'}</p>
                    <p><strong>Severity:</strong> <span class="issue-badge" style="${severityBadges[issue.severity] || ''}">${issue.severity}</span></p>
                    <p><strong>Message:</strong> ${issue.message}</p>
                </div>`;
            
            // Add fix if available
            if (issue.fix_suggestion && issue.fix_suggestion.success) {
                html += createFixPreview(issue.fix_suggestion);
            }
            
            codeContent.innerHTML = html;
            
            // Apply syntax highlighting
            codeContent.querySelectorAll('pre code').forEach(block => {
                hljs.highlightElement(block);
            });
            
            // Show modal
            modal.style.display = 'block';
            
            // Store current code for copy function
            window.currentModalCode = codeText;
        }
        
        // Copy full code from modal
        function copyFullCode() {
            if (window.currentModalCode) {
                navigator.clipboard.writeText(window.currentModalCode).then(() => {
                    // Show feedback
                    const btn = event.target.closest('button');
                    const originalHTML = btn.innerHTML;
                    btn.innerHTML = '<i class="fas fa-check"></i> Copied!';
                    setTimeout(() => {
                        btn.innerHTML = originalHTML;
                    }, 2000);
                });
            }
        }
        
        // Apply fix (placeholder for now)
        function applyFix(button, diffId) {
            // This would send an AJAX request to apply the fix
            button.disabled = true;
            button.innerHTML = '<i class="fas fa-spinner fa-spin"></i> Applying...';
            
            // Simulate applying
            setTimeout(() => {
                button.innerHTML = '<i class="fas fa-check-circle"></i> Applied';
                button.style.background = 'var(--success)';
                button.style.color = 'white';
            }, 1000);
        }
        
        // Modified toggle row to render diffs
        window.originalToggleRow = toggleRow;
        window.toggleRow = function(index) {
            originalToggleRow(index);
            
            // Render any pending diffs after expansion
            setTimeout(() => {
                Object.keys(window.pendingDiffs || {}).forEach(diffId => {
                    if (document.getElementById(diffId)) {
                        renderDiff(diffId);
                    }
                });
            }, 100);
        };
        """
        
    def _calculate_quality_score(self) -> str:
        """Calculate overall code quality score"""
        total = len(self.issues)
        if total == 0:
            return "A+"
            
        # Weight different severities
        weights = {
            'error': 10,
            'warning': 5,
            'performance': 3,
            'style': 1
        }
        
        score = 0
        for issue in self.issues:
            severity = issue.get('severity', 'style')
            score += weights.get(severity, 1)
            
        # Convert to grade
        if score < 10:
            return "A+"
        elif score < 50:
            return "A"
        elif score < 100:
            return "B"
        elif score < 200:
            return "C"
        elif score < 500:
            return "D"
        else:
            return "F"


def main():
    """Generate enhanced dashboard from analysis results"""
    import sys
    
    if len(sys.argv) < 2:
        print("Usage: generate_enhanced_dashboard.py <results.json> [output.html]")
        sys.exit(1)
        
    results_file = Path(sys.argv[1])
    output_file = Path(sys.argv[2] if len(sys.argv) > 2 else "enhanced_dashboard.html")
    
    with open(results_file) as f:
        results = json.load(f)
        
    generator = EnhancedDashboardGenerator(results)
    generator.generate_enhanced_dashboard(output_file)
    
    print(f"Enhanced dashboard generated: {output_file}")


if __name__ == '__main__':
    sys.exit(main())
