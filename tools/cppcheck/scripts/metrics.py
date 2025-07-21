#!/usr/bin/env python3
"""
LPZRobots Cppcheck Metrics Dashboard
Track and visualize static analysis trends over time
"""

import argparse
import json
import os
import sys
from pathlib import Path
from datetime import datetime, timedelta
from typing import Dict, List, Tuple
import sqlite3
from collections import defaultdict

# Colors for terminal output
class Colors:
    RED = '\033[0;31m'
    GREEN = '\033[0;32m'
    YELLOW = '\033[1;33m'
    BLUE = '\033[0;34m'
    MAGENTA = '\033[0;35m'
    CYAN = '\033[0;36m'
    NC = '\033[0m'

# Constants
SCRIPT_DIR = Path(__file__).parent
TOOLS_DIR = SCRIPT_DIR.parent
REPORTS_DIR = TOOLS_DIR / "reports"
METRICS_DB = TOOLS_DIR / "metrics.db"

class MetricsTracker:
    def __init__(self):
        self.conn = sqlite3.connect(METRICS_DB)
        self._init_database()
        
    def _init_database(self):
        """Initialize metrics database"""
        self.conn.executescript("""
            CREATE TABLE IF NOT EXISTS analyses (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,
                profile TEXT NOT NULL,
                total_issues INTEGER,
                errors INTEGER,
                warnings INTEGER,
                style INTEGER,
                performance INTEGER,
                files_analyzed INTEGER,
                analysis_time REAL
            );
            
            CREATE TABLE IF NOT EXISTS issue_trends (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,
                issue_type TEXT NOT NULL,
                count INTEGER,
                component TEXT
            );
            
            CREATE TABLE IF NOT EXISTS file_metrics (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,
                filepath TEXT NOT NULL,
                issues INTEGER,
                complexity INTEGER
            );
        """)
        self.conn.commit()
        
    def import_report(self, report_path: Path):
        """Import analysis report into metrics database"""
        # Try JSON format first
        json_report = report_path / "report.json"
        if json_report.exists():
            with open(json_report) as f:
                data = json.load(f)
                
            stats = data.get('stats', {})
            self.conn.execute("""
                INSERT INTO analyses 
                (profile, total_issues, errors, warnings, style, performance, 
                 files_analyzed, analysis_time)
                VALUES (?, ?, ?, ?, ?, ?, ?, ?)
            """, (
                data.get('profile', 'unknown'),
                len(data.get('issues', [])),
                stats.get('errors', 0),
                stats.get('warnings', 0),
                stats.get('style', 0),
                stats.get('performance', 0),
                stats.get('files_analyzed', 0),
                stats.get('analysis_time', 0)
            ))
            
            # Track issue types
            issue_counts = defaultdict(int)
            component_issues = defaultdict(lambda: defaultdict(int))
            
            for issue in data.get('issues', []):
                issue_type = issue.get('severity', 'unknown')
                issue_counts[issue_type] += 1
                
                # Extract component from filepath
                filepath = issue.get('file', '')
                # Handle absolute paths by finding the component name
                if 'selforg' in filepath:
                    component = 'selforg'
                elif 'ode_robots' in filepath:
                    component = 'ode_robots'
                elif 'ga_tools' in filepath:
                    component = 'ga_tools'
                elif 'opende' in filepath:
                    component = 'opende'
                elif 'guilogger' in filepath:
                    component = 'guilogger'
                elif 'matrixviz' in filepath:
                    component = 'matrixviz'
                else:
                    # Try to extract from relative path
                    if '/' in filepath and not filepath.startswith('/'):
                        component = filepath.split('/')[0]
                    else:
                        component = 'other'
                
                if component:
                    component_issues[component][issue_type] += 1
                    
            # Store issue trends
            for issue_type, count in issue_counts.items():
                self.conn.execute("""
                    INSERT INTO issue_trends (issue_type, count, component)
                    VALUES (?, ?, ?)
                """, (issue_type, count, 'all'))
                
            # Store component-specific trends
            for component, issues in component_issues.items():
                for issue_type, count in issues.items():
                    self.conn.execute("""
                        INSERT INTO issue_trends (issue_type, count, component)
                        VALUES (?, ?, ?)
                    """, (issue_type, count, component))
                    
            self.conn.commit()
            return True
            
        return False
        
    def get_trend_data(self, days: int = 30) -> Dict:
        """Get trend data for the specified period"""
        since = datetime.now() - timedelta(days=days)
        
        # Overall trends
        cursor = self.conn.execute("""
            SELECT DATE(timestamp) as date, 
                   SUM(errors) as errors,
                   SUM(warnings) as warnings,
                   SUM(style) as style,
                   SUM(performance) as performance
            FROM analyses
            WHERE timestamp >= ?
            GROUP BY DATE(timestamp)
            ORDER BY date
        """, (since.isoformat(),))
        
        trends = {
            'dates': [],
            'errors': [],
            'warnings': [],
            'style': [],
            'performance': []
        }
        
        for row in cursor:
            trends['dates'].append(row[0])
            trends['errors'].append(row[1] or 0)
            trends['warnings'].append(row[2] or 0)
            trends['style'].append(row[3] or 0)
            trends['performance'].append(row[4] or 0)
            
        return trends
        
    def get_component_stats(self) -> Dict:
        """Get statistics by component"""
        cursor = self.conn.execute("""
            SELECT component, issue_type, SUM(count) as total
            FROM issue_trends
            WHERE component != 'all'
              AND timestamp >= datetime('now', '-30 days')
            GROUP BY component, issue_type
            ORDER BY component, total DESC
        """)
        
        stats = defaultdict(dict)
        for component, issue_type, total in cursor:
            stats[component][issue_type] = total
            
        return dict(stats)
        
    def get_recent_analyses(self, limit: int = 10) -> List[Dict]:
        """Get recent analysis runs"""
        cursor = self.conn.execute("""
            SELECT timestamp, profile, total_issues, errors, warnings,
                   files_analyzed, analysis_time
            FROM analyses
            ORDER BY timestamp DESC
            LIMIT ?
        """, (limit,))
        
        analyses = []
        for row in cursor:
            analyses.append({
                'timestamp': row[0],
                'profile': row[1],
                'total_issues': row[2],
                'errors': row[3],
                'warnings': row[4],
                'files_analyzed': row[5],
                'analysis_time': row[6]
            })
            
        return analyses
        
    def print_dashboard(self):
        """Print metrics dashboard to console"""
        print(f"\n{Colors.BLUE}═══ LPZRobots Cppcheck Metrics Dashboard ═══{Colors.NC}\n")
        
        # Recent analyses
        recent = self.get_recent_analyses(5)
        if recent:
            print(f"{Colors.CYAN}Recent Analyses:{Colors.NC}")
            print(f"{'Date':<20} {'Profile':<15} {'Issues':<10} {'Errors':<8} {'Warnings':<10} {'Time':<8}")
            print("-" * 80)
            
            for analysis in recent:
                date = analysis['timestamp'][:19]
                print(f"{date:<20} {analysis['profile']:<15} "
                      f"{analysis['total_issues']:<10} "
                      f"{Colors.RED if analysis['errors'] > 0 else ''}{analysis['errors']:<8}{Colors.NC} "
                      f"{Colors.YELLOW if analysis['warnings'] > 0 else ''}{analysis['warnings']:<10}{Colors.NC} "
                      f"{analysis['analysis_time']:.1f}s")
                      
        # Trends
        trends = self.get_trend_data(7)
        if trends['dates']:
            print(f"\n{Colors.CYAN}7-Day Trend:{Colors.NC}")
            
            # Simple ASCII chart
            max_issues = max(
                max(trends['errors']) if trends['errors'] else 0,
                max(trends['warnings']) if trends['warnings'] else 0,
                max(trends['style']) if trends['style'] else 0
            )
            
            if max_issues > 0:
                scale = 20 / max_issues
                
                print("\nErrors (E), Warnings (W), Style (S)")
                for i, date in enumerate(trends['dates']):
                    e_bar = '█' * int(trends['errors'][i] * scale)
                    w_bar = '█' * int(trends['warnings'][i] * scale)
                    s_bar = '█' * int(trends['style'][i] * scale)
                    
                    print(f"{date}: {Colors.RED}E{e_bar}{Colors.NC} "
                          f"{Colors.YELLOW}W{w_bar}{Colors.NC} "
                          f"{Colors.BLUE}S{s_bar}{Colors.NC}")
                          
        # Component stats
        comp_stats = self.get_component_stats()
        if comp_stats:
            print(f"\n{Colors.CYAN}Component Issues (Last 30 Days):{Colors.NC}")
            print(f"{'Component':<20} {'Errors':<10} {'Warnings':<10} {'Style':<10} {'Total':<10}")
            print("-" * 60)
            
            for component, issues in sorted(comp_stats.items()):
                total = sum(issues.values())
                errors = issues.get('error', 0)
                warnings = issues.get('warning', 0)
                style = issues.get('style', 0)
                
                print(f"{component:<20} "
                      f"{Colors.RED if errors > 0 else ''}{errors:<10}{Colors.NC} "
                      f"{Colors.YELLOW if warnings > 0 else ''}{warnings:<10}{Colors.NC} "
                      f"{style:<10} "
                      f"{total:<10}")
                      
        # Summary statistics
        cursor = self.conn.execute("""
            SELECT 
                COUNT(*) as total_runs,
                AVG(analysis_time) as avg_time,
                SUM(files_analyzed) as total_files,
                SUM(errors) as total_errors,
                SUM(warnings) as total_warnings
            FROM analyses
            WHERE timestamp >= datetime('now', '-30 days')
        """)
        
        row = cursor.fetchone()
        if row and row[0] > 0:
            print(f"\n{Colors.CYAN}30-Day Summary:{Colors.NC}")
            print(f"  Total analysis runs: {row[0]}")
            print(f"  Average analysis time: {row[1]:.1f}s")
            print(f"  Total files analyzed: {row[2]}")
            print(f"  Total errors found: {Colors.RED}{row[3]}{Colors.NC}")
            print(f"  Total warnings found: {Colors.YELLOW}{row[4]}{Colors.NC}")
            
            # Calculate improvement
            cursor = self.conn.execute("""
                SELECT 
                    (SELECT SUM(errors) FROM analyses 
                     WHERE timestamp >= datetime('now', '-30 days') 
                       AND timestamp < datetime('now', '-15 days')) as old_errors,
                    (SELECT SUM(errors) FROM analyses 
                     WHERE timestamp >= datetime('now', '-15 days')) as new_errors
            """)
            
            old_errors, new_errors = cursor.fetchone()
            if old_errors and new_errors:
                improvement = ((old_errors - new_errors) / old_errors) * 100
                if improvement > 0:
                    print(f"  Error reduction: {Colors.GREEN}{improvement:.1f}%{Colors.NC}")
                elif improvement < 0:
                    print(f"  Error increase: {Colors.RED}{-improvement:.1f}%{Colors.NC}")
                    
    def export_html(self, output_path: Path):
        """Export enhanced dashboard as HTML"""
        # Get all data needed for dashboard
        trends = self.get_trend_data(30)
        component_stats = self.get_component_stats()
        recent_analyses = self.get_recent_analyses(10)
        
        # Calculate totals and summaries
        cursor = self.conn.execute("""
            SELECT 
                COUNT(DISTINCT DATE(timestamp)) as days_analyzed,
                COUNT(*) as total_runs,
                AVG(analysis_time) as avg_time,
                SUM(total_issues) as total_issues,
                SUM(errors) as total_errors,
                SUM(warnings) as total_warnings,
                SUM(style) as total_style,
                SUM(performance) as total_performance,
                SUM(files_analyzed) as total_files
            FROM analyses
            WHERE timestamp >= datetime('now', '-30 days')
        """)
        summary = cursor.fetchone()
        
        # Get issue counts from actual issues in database
        cursor = self.conn.execute("""
            SELECT issue_type, SUM(count) as total
            FROM issue_trends
            WHERE timestamp >= datetime('now', '-30 days')
              AND component != 'all'
            GROUP BY issue_type
        """)
        actual_counts = {row[0]: row[1] for row in cursor}
        
        html = """
<!DOCTYPE html>
<html>
<head>
    <title>LPZRobots Cppcheck Metrics Dashboard</title>
    <script src="https://cdn.plot.ly/plotly-latest.min.js"></script>
    <link href="https://fonts.googleapis.com/css2?family=Inter:wght@400;500;600;700&display=swap" rel="stylesheet">
    <style>
        * { box-sizing: border-box; margin: 0; padding: 0; }
        body { 
            font-family: 'Inter', -apple-system, BlinkMacSystemFont, sans-serif;
            background: #f5f7fa;
            color: #1a202c;
            line-height: 1.6;
        }
        .container {
            max-width: 1400px;
            margin: 0 auto;
            padding: 20px;
        }
        .header {
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            color: white;
            padding: 40px;
            border-radius: 16px;
            margin-bottom: 30px;
            box-shadow: 0 10px 30px rgba(0,0,0,0.1);
        }
        .header h1 {
            font-size: 2.5em;
            font-weight: 700;
            margin-bottom: 10px;
        }
        .header p {
            font-size: 1.1em;
            opacity: 0.9;
        }
        .stats-grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(250px, 1fr));
            gap: 20px;
            margin-bottom: 30px;
        }
        .stat-card {
            background: white;
            padding: 25px;
            border-radius: 12px;
            box-shadow: 0 4px 6px rgba(0,0,0,0.07);
            transition: transform 0.2s, box-shadow 0.2s;
            position: relative;
            overflow: hidden;
        }
        .stat-card:hover {
            transform: translateY(-2px);
            box-shadow: 0 8px 15px rgba(0,0,0,0.1);
        }
        .stat-card.error { border-left: 4px solid #f56565; }
        .stat-card.warning { border-left: 4px solid #ed8936; }
        .stat-card.style { border-left: 4px solid #4299e1; }
        .stat-card.performance { border-left: 4px solid #48bb78; }
        .stat-card.info { border-left: 4px solid #805ad5; }
        .stat-value {
            font-size: 2.5em;
            font-weight: 700;
            margin: 10px 0;
        }
        .stat-label {
            font-size: 0.9em;
            color: #718096;
            text-transform: uppercase;
            letter-spacing: 0.05em;
        }
        .stat-trend {
            position: absolute;
            top: 20px;
            right: 20px;
            font-size: 0.9em;
        }
        .trend-up { color: #f56565; }
        .trend-down { color: #48bb78; }
        .section {
            background: white;
            padding: 30px;
            border-radius: 12px;
            margin-bottom: 30px;
            box-shadow: 0 4px 6px rgba(0,0,0,0.07);
        }
        .section h2 {
            font-size: 1.8em;
            margin-bottom: 20px;
            color: #2d3748;
        }
        .chart {
            width: 100%;
            height: 400px;
            margin: 20px 0;
        }
        .table {
            width: 100%;
            border-collapse: collapse;
            margin-top: 20px;
        }
        .table th {
            background: #f7fafc;
            padding: 12px;
            text-align: left;
            font-weight: 600;
            color: #4a5568;
            border-bottom: 2px solid #e2e8f0;
        }
        .table td {
            padding: 12px;
            border-bottom: 1px solid #e2e8f0;
        }
        .table tr:hover {
            background: #f7fafc;
        }
        .badge {
            display: inline-block;
            padding: 4px 12px;
            border-radius: 20px;
            font-size: 0.85em;
            font-weight: 500;
        }
        .badge.error { background: #feb2b2; color: #c53030; }
        .badge.warning { background: #fbd38d; color: #c05621; }
        .badge.style { background: #bee3f8; color: #2c5282; }
        .badge.performance { background: #c6f6d5; color: #276749; }
        .progress-bar {
            width: 100%;
            height: 8px;
            background: #e2e8f0;
            border-radius: 4px;
            overflow: hidden;
            margin-top: 5px;
        }
        .progress-fill {
            height: 100%;
            background: linear-gradient(90deg, #48bb78 0%, #38a169 100%);
            transition: width 0.3s ease;
        }
        .component-card {
            background: #f7fafc;
            padding: 20px;
            border-radius: 8px;
            margin-bottom: 15px;
            display: flex;
            justify-content: space-between;
            align-items: center;
        }
        .component-name {
            font-weight: 600;
            font-size: 1.1em;
        }
        .component-stats {
            display: flex;
            gap: 15px;
        }
        .footer {
            text-align: center;
            padding: 30px;
            color: #718096;
            font-size: 0.9em;
        }
        @media (max-width: 768px) {
            .stats-grid {
                grid-template-columns: 1fr;
            }
            .header h1 {
                font-size: 2em;
            }
        }
    </style>
</head>
<body>
    <div class="container">
        <div class="header">
            <h1>LPZRobots Cppcheck Analysis Dashboard</h1>
            <p>Static Analysis Metrics and Trends • Last updated: """ + datetime.now().strftime('%Y-%m-%d %H:%M:%S') + """</p>
        </div>
        
        <div class="stats-grid">
"""
        
        # Add summary stat cards with actual issue counts
        total_errors = actual_counts.get('error', 0)
        total_warnings = actual_counts.get('warning', 0)
        total_style = actual_counts.get('style', 0)
        total_performance = actual_counts.get('performance', 0)
        total_all = total_errors + total_warnings + total_style + total_performance
        
        html += f"""
            <div class="stat-card error">
                <div class="stat-label">Total Errors</div>
                <div class="stat-value" style="color: #f56565;">{total_errors:,}</div>
                <div class="progress-bar">
                    <div class="progress-fill" style="width: {min(100, total_errors/10)}%; background: #f56565;"></div>
                </div>
            </div>
            <div class="stat-card warning">
                <div class="stat-label">Total Warnings</div>
                <div class="stat-value" style="color: #ed8936;">{total_warnings:,}</div>
                <div class="progress-bar">
                    <div class="progress-fill" style="width: {min(100, total_warnings/10)}%; background: #ed8936;"></div>
                </div>
            </div>
            <div class="stat-card style">
                <div class="stat-label">Style Issues</div>
                <div class="stat-value" style="color: #4299e1;">{total_style:,}</div>
                <div class="progress-bar">
                    <div class="progress-fill" style="width: {min(100, total_style/20)}%; background: #4299e1;"></div>
                </div>
            </div>
            <div class="stat-card performance">
                <div class="stat-label">Performance</div>
                <div class="stat-value" style="color: #48bb78;">{total_performance:,}</div>
                <div class="progress-bar">
                    <div class="progress-fill" style="width: {min(100, total_performance/10)}%; background: #48bb78;"></div>
                </div>
            </div>
            <div class="stat-card info">
                <div class="stat-label">Total Issues</div>
                <div class="stat-value" style="color: #805ad5;">{total_all:,}</div>
                <div class="progress-bar">
                    <div class="progress-fill" style="width: 100%; background: linear-gradient(90deg, #805ad5, #6b46c1);"></div>
                </div>
            </div>
            <div class="stat-card info">
                <div class="stat-label">Analysis Runs</div>
                <div class="stat-value" style="color: #667eea;">{summary[1] if summary else 0}</div>
                <p style="font-size: 0.9em; color: #718096; margin-top: 5px;">
                    Avg time: {summary[2]:.1f}s
                </p>
            </div>
        </div>
"""
        
        # Add trend charts section
        if trends['dates']:
            html += """
        <div class="section">
            <h2>📈 30-Day Issue Trends</h2>
            <div id="trendChart" class="chart"></div>
        </div>
        
        <div class="section">
            <h2>📊 Issue Distribution by Type</h2>
            <div id="pieChart" class="chart" style="height: 350px;"></div>
        </div>
"""
        
        # Add component breakdown
        if component_stats:
            html += """
        <div class="section">
            <h2>🔍 Component Breakdown</h2>
            <div id="componentChart" class="chart"></div>
            <div style="margin-top: 30px;">
"""
            
            # Sort components by total issues
            sorted_components = sorted(
                component_stats.items(),
                key=lambda x: sum(x[1].values()),
                reverse=True
            )
            
            for component, issues in sorted_components:
                total = sum(issues.values())
                if total > 0:
                    html += f"""
                <div class="component-card">
                    <div>
                        <div class="component-name">{component}</div>
                        <div style="color: #718096; font-size: 0.9em;">{total:,} total issues</div>
                    </div>
                    <div class="component-stats">
                        <span class="badge error">{issues.get('error', 0)} errors</span>
                        <span class="badge warning">{issues.get('warning', 0)} warnings</span>
                        <span class="badge style">{issues.get('style', 0)} style</span>
                        <span class="badge performance">{issues.get('performance', 0)} perf</span>
                    </div>
                </div>
"""
            
            html += """
            </div>
        </div>
"""
        
        # Add recent analyses table
        if recent_analyses:
            html += """
        <div class="section">
            <h2>📋 Recent Analysis Runs</h2>
            <table class="table">
                <thead>
                    <tr>
                        <th>Date & Time</th>
                        <th>Profile</th>
                        <th>Total Issues</th>
                        <th>Files Analyzed</th>
                        <th>Duration</th>
                    </tr>
                </thead>
                <tbody>
"""
            for analysis in recent_analyses:
                date = datetime.fromisoformat(analysis['timestamp']).strftime('%Y-%m-%d %H:%M')
                html += f"""
                    <tr>
                        <td>{date}</td>
                        <td><span class="badge" style="background: #e6fffa; color: #047481;">{analysis['profile']}</span></td>
                        <td>{analysis['total_issues']:,}</td>
                        <td>{analysis['files_analyzed']}</td>
                        <td>{analysis['analysis_time']:.1f}s</td>
                    </tr>
"""
            
            html += """
                </tbody>
            </table>
        </div>
"""
        
        # Add JavaScript for charts
        html += """
        <div class="footer">
            <p>LPZRobots Cppcheck Infrastructure &bull; Built with <i class="fas fa-heart" style="color: #f56565;"></i> for C++17 modernization</p>
        </div>
    </div>
    
    <script>
"""
        
        # Trend chart data
        if trends['dates']:
            html += """
        // Trend Chart
        var trendData = [
            {
                x: """ + json.dumps(trends['dates']) + """,
                y: """ + json.dumps(trends['errors']) + """,
                name: 'Errors',
                type: 'scatter',
                mode: 'lines+markers',
                line: {color: '#f56565', width: 3},
                marker: {size: 8}
            },
            {
                x: """ + json.dumps(trends['dates']) + """,
                y: """ + json.dumps(trends['warnings']) + """,
                name: 'Warnings',
                type: 'scatter',
                mode: 'lines+markers',
                line: {color: '#ed8936', width: 3},
                marker: {size: 8}
            },
            {
                x: """ + json.dumps(trends['dates']) + """,
                y: """ + json.dumps(trends['style']) + """,
                name: 'Style',
                type: 'scatter',
                mode: 'lines+markers',
                line: {color: '#4299e1', width: 3},
                marker: {size: 8}
            },
            {
                x: """ + json.dumps(trends['dates']) + """,
                y: """ + json.dumps(trends['performance']) + """,
                name: 'Performance',
                type: 'scatter',
                mode: 'lines+markers',
                line: {color: '#48bb78', width: 3},
                marker: {size: 8}
            }
        ];
        
        var trendLayout = {
            title: '',
            xaxis: { title: 'Date', showgrid: false },
            yaxis: { title: 'Number of Issues', showgrid: true, gridcolor: '#e2e8f0' },
            plot_bgcolor: 'rgba(0,0,0,0)',
            paper_bgcolor: 'rgba(0,0,0,0)',
            font: { family: 'Inter, sans-serif' },
            margin: { t: 20 },
            hovermode: 'x unified'
        };
        
        Plotly.newPlot('trendChart', trendData, trendLayout, {responsive: true});
        
        // Pie Chart
        var pieData = [{
            values: [""" + str(total_errors) + """, """ + str(total_warnings) + """, """ + str(total_style) + """, """ + str(total_performance) + """],
            labels: ['Errors', 'Warnings', 'Style', 'Performance'],
            type: 'pie',
            hole: .4,
            marker: {
                colors: ['#f56565', '#ed8936', '#4299e1', '#48bb78']
            },
            textinfo: 'label+percent',
            textposition: 'outside'
        }];
        
        var pieLayout = {
            title: '',
            showlegend: true,
            plot_bgcolor: 'rgba(0,0,0,0)',
            paper_bgcolor: 'rgba(0,0,0,0)',
            font: { family: 'Inter, sans-serif' },
            margin: { t: 20 }
        };
        
        Plotly.newPlot('pieChart', pieData, pieLayout, {responsive: true});
"""
        
        # Component chart
        if component_stats:
            components = list(component_stats.keys())
            errors = [component_stats[c].get('error', 0) for c in components]
            warnings = [component_stats[c].get('warning', 0) for c in components]
            style = [component_stats[c].get('style', 0) for c in components]
            performance = [component_stats[c].get('performance', 0) for c in components]
            
            html += """
        // Component Chart
        var componentData = [
            {
                x: """ + json.dumps(components) + """,
                y: """ + json.dumps(errors) + """,
                name: 'Errors',
                type: 'bar',
                marker: { color: '#f56565' }
            },
            {
                x: """ + json.dumps(components) + """,
                y: """ + json.dumps(warnings) + """,
                name: 'Warnings',
                type: 'bar',
                marker: { color: '#ed8936' }
            },
            {
                x: """ + json.dumps(components) + """,
                y: """ + json.dumps(style) + """,
                name: 'Style',
                type: 'bar',
                marker: { color: '#4299e1' }
            },
            {
                x: """ + json.dumps(components) + """,
                y: """ + json.dumps(performance) + """,
                name: 'Performance',
                type: 'bar',
                marker: { color: '#48bb78' }
            }
        ];
        
        var componentLayout = {
            title: '',
            barmode: 'stack',
            xaxis: { title: 'Component', showgrid: false },
            yaxis: { title: 'Number of Issues', showgrid: true, gridcolor: '#e2e8f0' },
            plot_bgcolor: 'rgba(0,0,0,0)',
            paper_bgcolor: 'rgba(0,0,0,0)',
            font: { family: 'Inter, sans-serif' },
            margin: { t: 20 }
        };
        
        Plotly.newPlot('componentChart', componentData, componentLayout, {responsive: true});
"""
        
        html += """
    </script>
</body>
</html>
"""
        
        with open(output_path, 'w') as f:
            f.write(html)
            
        print(f"{Colors.GREEN}Enhanced HTML dashboard exported to: {output_path}{Colors.NC}")

def main():
    parser = argparse.ArgumentParser(description='LPZRobots Cppcheck Metrics Dashboard')
    parser.add_argument('--import', dest='import_path', 
                        help='Import analysis results from directory')
    parser.add_argument('--trend', action='store_true',
                        help='Show trend analysis')
    parser.add_argument('--export-html', dest='export_html',
                        help='Export dashboard as HTML')
    parser.add_argument('--days', type=int, default=30,
                        help='Number of days to analyze (default: 30)')
    
    args = parser.parse_args()
    
    tracker = MetricsTracker()
    
    # Import results if specified
    if args.import_path:
        report_path = Path(args.import_path)
        if tracker.import_report(report_path):
            print(f"{Colors.GREEN}Imported results from {report_path}{Colors.NC}")
        else:
            print(f"{Colors.RED}Failed to import from {report_path}{Colors.NC}")
            return 1
            
    # Export HTML if requested
    if args.export_html:
        tracker.export_html(Path(args.export_html))
        return 0
        
    # Otherwise show dashboard
    tracker.print_dashboard()
    
    return 0

if __name__ == '__main__':
    sys.exit(main())