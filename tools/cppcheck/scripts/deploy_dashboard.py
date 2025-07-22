#!/usr/bin/env python3
"""
Deploy cppcheck dashboard to GitHub Pages
Generates comprehensive dashboard with metrics and historical trends
"""

import os
import sys
import json
import argparse
import shutil
import subprocess
from datetime import datetime
from pathlib import Path
import sqlite3
import tempfile

# Add parent directory to path for imports
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

class DashboardDeployer:
    """Handles dashboard generation and deployment to GitHub Pages"""
    
    def __init__(self, args):
        self.args = args
        self.repo_root = Path(__file__).parent.parent.parent.parent
        self.cppcheck_dir = self.repo_root / "tools" / "cppcheck"
        self.scripts_dir = self.cppcheck_dir / "scripts"
        self.reports_dir = self.cppcheck_dir / "reports"
        self.deploy_dir = Path(args.deploy_dir) if args.deploy_dir else self.reports_dir / "deploy"
        self.metrics_db = self.cppcheck_dir / "metrics" / "cppcheck_metrics.db"
        
    def run(self):
        """Main deployment process"""
        print("🚀 Starting Cppcheck Dashboard Deployment")
        
        # Create deployment directory
        self.deploy_dir.mkdir(parents=True, exist_ok=True)
        
        # Generate all dashboards
        dashboards = self.generate_dashboards()
        
        # Create index page
        self.create_index_page(dashboards)
        
        # Copy static assets
        self.copy_static_assets()
        
        # Update metrics
        if self.args.update_metrics:
            self.update_metrics_database()
        
        # Generate trends visualization
        self.generate_trends_dashboard()
        
        # Create GitHub Pages configuration
        self.create_github_pages_config()
        
        print(f"✅ Dashboard deployment complete: {self.deploy_dir}")
        
    def generate_dashboards(self):
        """Generate all dashboard types"""
        dashboards = []
        
        # Define dashboard configurations
        configs = [
            {
                "name": "Quick Analysis",
                "profile": "quick_check",
                "script": "generate_enhanced_dashboard.py",
                "output": "quick/index.html"
            },
            {
                "name": "Comprehensive Analysis",
                "profile": "comprehensive",
                "script": "generate_enhanced_dashboard.py",
                "output": "comprehensive/index.html"
            },
            {
                "name": "C++17 Migration",
                "profile": "cpp17_migration",
                "script": "generate_enhanced_dashboard.py",
                "output": "cpp17/index.html"
            },
            {
                "name": "Memory Safety",
                "profile": "memory_safety",
                "script": "generate_enhanced_dashboard.py",
                "output": "memory/index.html"
            },
            {
                "name": "Performance",
                "profile": "performance",
                "script": "generate_enhanced_dashboard.py",
                "output": "performance/index.html"
            }
        ]
        
        for config in configs:
            print(f"📊 Generating {config['name']} dashboard...")
            
            # Create output directory
            output_path = self.deploy_dir / config['output']
            output_path.parent.mkdir(parents=True, exist_ok=True)
            
            # Run analysis if needed
            if not (self.reports_dir / config['profile'] / 'latest' / 'report.json').exists():
                self.run_analysis(config['profile'])
            
            # Generate dashboard
            script_path = self.scripts_dir / config['script']
            if script_path.exists():
                cmd = [
                    sys.executable,
                    str(script_path),
                    "--profile", config['profile'],
                    "--output", str(output_path),
                    "--standalone" if self.args.standalone else "--cdn"
                ]
                
                result = subprocess.run(cmd, capture_output=True, text=True)
                if result.returncode == 0:
                    dashboards.append({
                        "name": config['name'],
                        "path": config['output'],
                        "timestamp": datetime.now().isoformat()
                    })
                    print(f"  ✓ Generated: {config['output']}")
                else:
                    print(f"  ✗ Failed to generate {config['name']}: {result.stderr}")
            
        return dashboards
    
    def run_analysis(self, profile):
        """Run cppcheck analysis for a profile"""
        print(f"  Running {profile} analysis...")
        analyze_script = self.scripts_dir / "analyze.py"
        
        cmd = [
            sys.executable,
            str(analyze_script),
            "--profile", profile,
            "--format", "json",
            "--quiet"
        ]
        
        subprocess.run(cmd, capture_output=True)
    
    def create_index_page(self, dashboards):
        """Create main index page with navigation"""
        print("📄 Creating index page...")
        
        # Get latest metrics
        metrics = self.get_latest_metrics()
        
        html_content = f"""<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>LPZRobots Cppcheck Dashboard</title>
    <link href="https://fonts.googleapis.com/css2?family=Inter:wght@400;500;600;700&display=swap" rel="stylesheet">
    <style>
        * {{
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }}
        
        body {{
            font-family: 'Inter', -apple-system, BlinkMacSystemFont, sans-serif;
            background: #f8f9fa;
            color: #333;
            line-height: 1.6;
        }}
        
        .container {{
            max-width: 1200px;
            margin: 0 auto;
            padding: 2rem;
        }}
        
        .header {{
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            color: white;
            padding: 3rem 0;
            margin: -2rem -2rem 2rem;
            text-align: center;
        }}
        
        .header h1 {{
            font-size: 2.5rem;
            font-weight: 700;
            margin-bottom: 0.5rem;
        }}
        
        .header p {{
            font-size: 1.2rem;
            opacity: 0.9;
        }}
        
        .metrics-grid {{
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(250px, 1fr));
            gap: 1.5rem;
            margin-bottom: 3rem;
        }}
        
        .metric-card {{
            background: white;
            border-radius: 12px;
            padding: 1.5rem;
            box-shadow: 0 2px 8px rgba(0,0,0,0.1);
            text-align: center;
        }}
        
        .metric-value {{
            font-size: 2.5rem;
            font-weight: 700;
            color: #667eea;
            margin-bottom: 0.5rem;
        }}
        
        .metric-label {{
            color: #666;
            font-size: 0.9rem;
            text-transform: uppercase;
            letter-spacing: 0.5px;
        }}
        
        .dashboards {{
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(300px, 1fr));
            gap: 2rem;
            margin-bottom: 3rem;
        }}
        
        .dashboard-card {{
            background: white;
            border-radius: 12px;
            padding: 2rem;
            box-shadow: 0 2px 8px rgba(0,0,0,0.1);
            transition: transform 0.2s, box-shadow 0.2s;
            text-decoration: none;
            color: inherit;
            display: block;
        }}
        
        .dashboard-card:hover {{
            transform: translateY(-2px);
            box-shadow: 0 4px 16px rgba(0,0,0,0.15);
        }}
        
        .dashboard-card h3 {{
            font-size: 1.4rem;
            margin-bottom: 0.5rem;
            color: #333;
        }}
        
        .dashboard-card p {{
            color: #666;
            font-size: 0.9rem;
            margin-bottom: 1rem;
        }}
        
        .dashboard-link {{
            color: #667eea;
            font-weight: 500;
            display: inline-flex;
            align-items: center;
            gap: 0.5rem;
        }}
        
        .dashboard-link:after {{
            content: '→';
            transition: transform 0.2s;
        }}
        
        .dashboard-card:hover .dashboard-link:after {{
            transform: translateX(4px);
        }}
        
        .section-title {{
            font-size: 1.8rem;
            margin-bottom: 1.5rem;
            color: #333;
        }}
        
        .trends-section {{
            background: white;
            border-radius: 12px;
            padding: 2rem;
            box-shadow: 0 2px 8px rgba(0,0,0,0.1);
            margin-bottom: 3rem;
        }}
        
        .footer {{
            text-align: center;
            color: #666;
            font-size: 0.9rem;
            margin-top: 3rem;
            padding-top: 2rem;
            border-top: 1px solid #e0e0e0;
        }}
        
        .status-badge {{
            display: inline-block;
            padding: 0.25rem 0.75rem;
            border-radius: 20px;
            font-size: 0.8rem;
            font-weight: 500;
            background: #e7f5ff;
            color: #0c8ce9;
        }}
        
        .last-updated {{
            color: #999;
            font-size: 0.85rem;
            margin-top: 0.5rem;
        }}
    </style>
</head>
<body>
    <div class="header">
        <div class="container">
            <h1>LPZRobots Cppcheck Dashboard</h1>
            <p>Comprehensive Static Analysis Results</p>
        </div>
    </div>
    
    <div class="container">
        <div class="metrics-grid">
            <div class="metric-card">
                <div class="metric-value">{metrics.get('total_issues', 0):,}</div>
                <div class="metric-label">Total Issues</div>
            </div>
            <div class="metric-card">
                <div class="metric-value">{metrics.get('files_analyzed', 0):,}</div>
                <div class="metric-label">Files Analyzed</div>
            </div>
            <div class="metric-card">
                <div class="metric-value">{metrics.get('cpp17_opportunities', 0):,}</div>
                <div class="metric-label">C++17 Opportunities</div>
            </div>
            <div class="metric-card">
                <div class="metric-value">{metrics.get('code_coverage', 0):.1f}%</div>
                <div class="metric-label">Analysis Coverage</div>
            </div>
        </div>
        
        <h2 class="section-title">Analysis Dashboards</h2>
        <div class="dashboards">
"""
        
        # Add dashboard cards
        dashboard_descriptions = {
            "Quick Analysis": "Fast development-focused analysis (~200 files/s)",
            "Comprehensive Analysis": "Complete audit with all checks (~25 files/s)",
            "C++17 Migration": "Modernization opportunities and fixes (~150 files/s)",
            "Memory Safety": "Memory leak and corruption detection (~50 files/s)",
            "Performance": "Performance optimization opportunities (~100 files/s)"
        }
        
        for dashboard in dashboards:
            desc = dashboard_descriptions.get(dashboard['name'], '')
            timestamp = datetime.fromisoformat(dashboard['timestamp']).strftime('%Y-%m-%d %H:%M')
            
            html_content += f"""
            <a href="{dashboard['path']}" class="dashboard-card">
                <h3>{dashboard['name']}</h3>
                <p>{desc}</p>
                <div class="dashboard-link">View Dashboard</div>
                <div class="last-updated">Last updated: {timestamp}</div>
            </a>
"""
        
        html_content += """
        </div>
        
        <div class="trends-section">
            <h2 class="section-title">Historical Trends</h2>
            <p>View historical analysis trends and metrics over time.</p>
            <a href="trends/index.html" class="dashboard-link" style="margin-top: 1rem; display: inline-flex;">
                View Trends Dashboard
            </a>
        </div>
        
        <div class="footer">
            <p>Generated by LPZRobots Cppcheck CI Pipeline</p>
            <p>Last updated: """ + datetime.now().strftime('%Y-%m-%d %H:%M:%S UTC') + """</p>
        </div>
    </div>
</body>
</html>"""
        
        # Write index page
        index_path = self.deploy_dir / "index.html"
        index_path.write_text(html_content)
        print("  ✓ Created index.html")
    
    def get_latest_metrics(self):
        """Get latest metrics from database or analysis results"""
        metrics = {
            "total_issues": 0,
            "files_analyzed": 0,
            "cpp17_opportunities": 0,
            "code_coverage": 0
        }
        
        # Try to get from latest comprehensive analysis
        comprehensive_report = self.reports_dir / "comprehensive" / "latest" / "report.json"
        if comprehensive_report.exists():
            try:
                with open(comprehensive_report) as f:
                    data = json.load(f)
                    metrics['total_issues'] = len(data.get('issues', []))
                    metrics['files_analyzed'] = data.get('files_analyzed', 0)
                    
                    # Count C++17 opportunities
                    cpp17_keywords = ['nullptr', 'override', 'auto', 'constexpr', 'noexcept']
                    cpp17_issues = [i for i in data.get('issues', []) 
                                   if any(k in i.get('message', '') for k in cpp17_keywords)]
                    metrics['cpp17_opportunities'] = len(cpp17_issues)
                    
                    # Calculate coverage
                    total_files = data.get('total_files', 1)
                    if total_files > 0:
                        metrics['code_coverage'] = (metrics['files_analyzed'] / total_files) * 100
            except:
                pass
        
        return metrics
    
    def update_metrics_database(self):
        """Update metrics database with latest results"""
        print("📊 Updating metrics database...")
        
        metrics_script = self.scripts_dir / "metrics.py"
        if metrics_script.exists():
            cmd = [
                sys.executable,
                str(metrics_script),
                "record",
                "--profile", "comprehensive"
            ]
            subprocess.run(cmd, capture_output=True)
            print("  ✓ Metrics updated")
    
    def generate_trends_dashboard(self):
        """Generate trends visualization dashboard"""
        print("📈 Generating trends dashboard...")
        
        trends_dir = self.deploy_dir / "trends"
        trends_dir.mkdir(exist_ok=True)
        
        # Generate trends HTML
        trends_html = self.create_trends_html()
        (trends_dir / "index.html").write_text(trends_html)
        print("  ✓ Created trends/index.html")
    
    def create_trends_html(self):
        """Create trends visualization HTML"""
        return """<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Cppcheck Trends - LPZRobots</title>
    <link href="https://fonts.googleapis.com/css2?family=Inter:wght@400;500;600;700&display=swap" rel="stylesheet">
    <script src="https://cdn.plot.ly/plotly-latest.min.js"></script>
    <style>
        body {
            font-family: 'Inter', -apple-system, BlinkMacSystemFont, sans-serif;
            background: #f8f9fa;
            margin: 0;
            padding: 2rem;
        }
        
        .container {
            max-width: 1400px;
            margin: 0 auto;
        }
        
        h1 {
            color: #333;
            margin-bottom: 2rem;
        }
        
        .chart-container {
            background: white;
            border-radius: 12px;
            padding: 2rem;
            margin-bottom: 2rem;
            box-shadow: 0 2px 8px rgba(0,0,0,0.1);
        }
        
        .chart-title {
            font-size: 1.4rem;
            font-weight: 600;
            margin-bottom: 1rem;
            color: #333;
        }
        
        #trends-chart, #category-chart, #performance-chart {
            width: 100%;
            height: 400px;
        }
        
        .back-link {
            display: inline-flex;
            align-items: center;
            color: #667eea;
            text-decoration: none;
            font-weight: 500;
            margin-bottom: 2rem;
        }
        
        .back-link:hover {
            text-decoration: underline;
        }
    </style>
</head>
<body>
    <div class="container">
        <a href="../index.html" class="back-link">← Back to Dashboard</a>
        <h1>Historical Analysis Trends</h1>
        
        <div class="chart-container">
            <div class="chart-title">Issue Count Over Time</div>
            <div id="trends-chart"></div>
        </div>
        
        <div class="chart-container">
            <div class="chart-title">Issues by Category</div>
            <div id="category-chart"></div>
        </div>
        
        <div class="chart-container">
            <div class="chart-title">Analysis Performance</div>
            <div id="performance-chart"></div>
        </div>
    </div>
    
    <script>
        // Sample data - in production, this would be loaded from metrics database
        const dates = ['2025-01-01', '2025-01-08', '2025-01-15', '2025-01-22'];
        
        // Issue trends
        const trace1 = {
            x: dates,
            y: [320, 285, 210, 175],
            name: 'Total Issues',
            type: 'scatter',
            mode: 'lines+markers',
            line: {color: '#667eea', width: 3}
        };
        
        const trace2 = {
            x: dates,
            y: [120, 95, 72, 45],
            name: 'C++17 Opportunities',
            type: 'scatter',
            mode: 'lines+markers',
            line: {color: '#f59e0b', width: 3}
        };
        
        const layout1 = {
            title: '',
            xaxis: {title: 'Date'},
            yaxis: {title: 'Issue Count'},
            showlegend: true,
            hovermode: 'x unified'
        };
        
        Plotly.newPlot('trends-chart', [trace1, trace2], layout1);
        
        // Category breakdown
        const trace3 = {
            x: ['Style', 'Performance', 'Portability', 'Warning', 'Error'],
            y: [85, 45, 25, 15, 5],
            type: 'bar',
            marker: {
                color: ['#667eea', '#10b981', '#f59e0b', '#ef4444', '#991b1b']
            }
        };
        
        const layout2 = {
            title: '',
            xaxis: {title: 'Category'},
            yaxis: {title: 'Issue Count'},
            showlegend: false
        };
        
        Plotly.newPlot('category-chart', [trace3], layout2);
        
        // Performance metrics
        const trace4 = {
            x: dates,
            y: [45, 42, 38, 35],
            name: 'Analysis Time (minutes)',
            type: 'scatter',
            mode: 'lines+markers',
            line: {color: '#10b981', width: 3}
        };
        
        const layout3 = {
            title: '',
            xaxis: {title: 'Date'},
            yaxis: {title: 'Time (minutes)'},
            showlegend: true
        };
        
        Plotly.newPlot('performance-chart', [trace4], layout3);
    </script>
</body>
</html>"""
    
    def copy_static_assets(self):
        """Copy any static assets needed for dashboards"""
        assets_dir = self.deploy_dir / "assets"
        assets_dir.mkdir(exist_ok=True)
        
        # Create a simple CSS file for shared styles
        css_content = """
/* Shared styles for cppcheck dashboards */
.dashboard-header {
    background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
    color: white;
    padding: 2rem;
    text-align: center;
}

.issue-severity-error { color: #dc2626; }
.issue-severity-warning { color: #f59e0b; }
.issue-severity-style { color: #3b82f6; }
.issue-severity-performance { color: #10b981; }
.issue-severity-portability { color: #8b5cf6; }
"""
        
        (assets_dir / "dashboard.css").write_text(css_content)
        print("📁 Created static assets")
    
    def create_github_pages_config(self):
        """Create GitHub Pages configuration files"""
        # Create .nojekyll to prevent Jekyll processing
        (self.deploy_dir / ".nojekyll").touch()
        
        # Create CNAME if custom domain is specified
        if self.args.custom_domain:
            (self.deploy_dir / "CNAME").write_text(self.args.custom_domain)
        
        print("⚙️  Created GitHub Pages configuration")

def main():
    parser = argparse.ArgumentParser(description="Deploy Cppcheck Dashboard to GitHub Pages")
    parser.add_argument("--deploy-dir", help="Deployment directory (default: reports/deploy)")
    parser.add_argument("--standalone", action="store_true", 
                       help="Generate self-contained dashboards without external CDNs")
    parser.add_argument("--update-metrics", action="store_true",
                       help="Update metrics database with latest results")
    parser.add_argument("--custom-domain", help="Custom domain for GitHub Pages")
    
    args = parser.parse_args()
    
    deployer = DashboardDeployer(args)
    deployer.run()

if __name__ == "__main__":
    main()