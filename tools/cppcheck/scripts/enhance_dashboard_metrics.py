#!/usr/bin/env python3
"""
Enhance existing dashboards with metrics integration
Adds historical trends, comparative analysis, and performance metrics
"""

import os
import sys
import json
import sqlite3
from datetime import datetime, timedelta
from pathlib import Path

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

class DashboardEnhancer:
    """Enhances cppcheck dashboards with metrics and trends"""
    
    def __init__(self):
        self.repo_root = Path(__file__).parent.parent.parent.parent
        self.cppcheck_dir = self.repo_root / "tools" / "cppcheck"
        self.metrics_db = self.cppcheck_dir / "metrics" / "cppcheck_metrics.db"
        self.reports_dir = self.cppcheck_dir / "reports"
        
    def enhance_dashboard(self, profile, output_path=None):
        """Enhance a dashboard with metrics data"""
        print(f"🎨 Enhancing {profile} dashboard with metrics...")
        
        # Load existing report data
        report_path = self.reports_dir / profile / "latest" / "report.json"
        if not report_path.exists():
            print(f"  ✗ No report found for {profile}")
            return False
            
        with open(report_path) as f:
            report_data = json.load(f)
        
        # Get historical metrics
        metrics = self.get_historical_metrics(profile)
        
        # Get comparative data
        comparison = self.get_profile_comparison()
        
        # Enhance report data
        enhanced_data = {
            **report_data,
            "metrics": metrics,
            "comparison": comparison,
            "generated_at": datetime.now().isoformat(),
            "trends": self.calculate_trends(metrics)
        }
        
        # Generate enhanced dashboard HTML
        if output_path:
            self.generate_enhanced_html(enhanced_data, output_path)
        else:
            # Save enhanced data
            enhanced_path = report_path.parent / "report_enhanced.json"
            with open(enhanced_path, 'w') as f:
                json.dump(enhanced_data, f, indent=2)
            print(f"  ✓ Saved enhanced data to {enhanced_path}")
        
        return True
    
    def get_historical_metrics(self, profile):
        """Get historical metrics from database"""
        metrics = {
            "history": [],
            "summary": {}
        }
        
        if not self.metrics_db.exists():
            return metrics
        
        try:
            conn = sqlite3.connect(self.metrics_db)
            cursor = conn.cursor()
            
            # Get last 30 days of data
            thirty_days_ago = (datetime.now() - timedelta(days=30)).isoformat()
            
            query = """
            SELECT timestamp, total_issues, error_count, warning_count, 
                   style_count, performance_count, portability_count,
                   files_analyzed, analysis_time
            FROM analysis_runs
            WHERE profile = ? AND timestamp > ?
            ORDER BY timestamp DESC
            """
            
            cursor.execute(query, (profile, thirty_days_ago))
            
            for row in cursor.fetchall():
                metrics["history"].append({
                    "timestamp": row[0],
                    "total_issues": row[1],
                    "error_count": row[2],
                    "warning_count": row[3],
                    "style_count": row[4],
                    "performance_count": row[5],
                    "portability_count": row[6],
                    "files_analyzed": row[7],
                    "analysis_time": row[8]
                })
            
            # Calculate summary statistics
            if metrics["history"]:
                latest = metrics["history"][0]
                oldest = metrics["history"][-1]
                
                metrics["summary"] = {
                    "trend_direction": "down" if latest["total_issues"] < oldest["total_issues"] else "up",
                    "trend_percentage": abs((latest["total_issues"] - oldest["total_issues"]) / max(oldest["total_issues"], 1) * 100),
                    "avg_analysis_time": sum(h["analysis_time"] for h in metrics["history"]) / len(metrics["history"]),
                    "total_runs": len(metrics["history"])
                }
            
            conn.close()
            
        except Exception as e:
            print(f"  ⚠️  Error reading metrics: {e}")
        
        return metrics
    
    def get_profile_comparison(self):
        """Get comparison data across different profiles"""
        comparison = {}
        
        profiles = ["quick_check", "comprehensive", "cpp17_migration", "memory_safety", "performance"]
        
        for profile in profiles:
            report_path = self.reports_dir / profile / "latest" / "report.json"
            if report_path.exists():
                try:
                    with open(report_path) as f:
                        data = json.load(f)
                        comparison[profile] = {
                            "total_issues": len(data.get("issues", [])),
                            "files_analyzed": data.get("files_analyzed", 0),
                            "timestamp": data.get("timestamp", "")
                        }
                except (json.JSONDecodeError, IOError) as e:
                    print(f"  Warning: Failed to load report for {profile}: {e}")
        
        return comparison
    
    def calculate_trends(self, metrics):
        """Calculate trend data for visualization"""
        trends = {
            "issue_trend": [],
            "category_trend": {},
            "performance_trend": []
        }
        
        if not metrics["history"]:
            return trends
        
        # Issue count trend
        for entry in metrics["history"]:
            trends["issue_trend"].append({
                "date": entry["timestamp"][:10],
                "count": entry["total_issues"]
            })
        
        # Category breakdown trend
        categories = ["error", "warning", "style", "performance", "portability"]
        for category in categories:
            trends["category_trend"][category] = []
            for entry in metrics["history"]:
                trends["category_trend"][category].append({
                    "date": entry["timestamp"][:10],
                    "count": entry[f"{category}_count"]
                })
        
        # Performance trend
        for entry in metrics["history"]:
            if entry["files_analyzed"] > 0:
                trends["performance_trend"].append({
                    "date": entry["timestamp"][:10],
                    "files_per_second": entry["files_analyzed"] / max(entry["analysis_time"], 1)
                })
        
        return trends
    
    def generate_enhanced_html(self, data, output_path):
        """Generate enhanced HTML dashboard with metrics"""
        html_content = f"""<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Enhanced Cppcheck Dashboard - {data.get('profile', 'Analysis')}</title>
    <link href="https://fonts.googleapis.com/css2?family=Inter:wght@400;500;600;700&display=swap" rel="stylesheet">
    <script src="https://cdn.plot.ly/plotly-latest.min.js"></script>
    <style>
        /* Enhanced dashboard styles */
        body {{
            font-family: 'Inter', -apple-system, BlinkMacSystemFont, sans-serif;
            background: #f0f2f5;
            margin: 0;
            padding: 0;
        }}
        
        .dashboard-container {{
            max-width: 1600px;
            margin: 0 auto;
            padding: 2rem;
        }}
        
        .metrics-header {{
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            color: white;
            padding: 3rem;
            border-radius: 16px;
            margin-bottom: 2rem;
            box-shadow: 0 4px 20px rgba(102, 126, 234, 0.3);
        }}
        
        .metrics-grid {{
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(280px, 1fr));
            gap: 1.5rem;
            margin-bottom: 2rem;
        }}
        
        .metric-card {{
            background: white;
            padding: 1.5rem;
            border-radius: 12px;
            box-shadow: 0 2px 8px rgba(0,0,0,0.08);
            transition: transform 0.2s, box-shadow 0.2s;
        }}
        
        .metric-card:hover {{
            transform: translateY(-2px);
            box-shadow: 0 4px 16px rgba(0,0,0,0.12);
        }}
        
        .metric-value {{
            font-size: 2.5rem;
            font-weight: 700;
            color: #333;
            margin-bottom: 0.5rem;
        }}
        
        .metric-label {{
            color: #666;
            font-size: 0.9rem;
            text-transform: uppercase;
            letter-spacing: 0.5px;
        }}
        
        .metric-trend {{
            display: inline-flex;
            align-items: center;
            gap: 0.25rem;
            font-size: 0.85rem;
            margin-top: 0.5rem;
        }}
        
        .trend-up {{ color: #ef4444; }}
        .trend-down {{ color: #10b981; }}
        
        .chart-section {{
            background: white;
            padding: 2rem;
            border-radius: 12px;
            box-shadow: 0 2px 8px rgba(0,0,0,0.08);
            margin-bottom: 2rem;
        }}
        
        .chart-title {{
            font-size: 1.5rem;
            font-weight: 600;
            margin-bottom: 1rem;
            color: #333;
        }}
        
        .issues-section {{
            background: white;
            padding: 2rem;
            border-radius: 12px;
            box-shadow: 0 2px 8px rgba(0,0,0,0.08);
        }}
    </style>
</head>
<body>
    <div class="dashboard-container">
        <div class="metrics-header">
            <h1>Enhanced Cppcheck Dashboard</h1>
            <p>Profile: {data.get('profile', 'Analysis').replace('_', ' ').title()}</p>
            <p>Generated: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}</p>
        </div>
        
        <div class="metrics-grid">
            <div class="metric-card">
                <div class="metric-value">{len(data.get('issues', []))}</div>
                <div class="metric-label">Total Issues</div>
                {self._generate_trend_indicator(data.get('metrics', {}).get('summary', {}))}
            </div>
            
            <div class="metric-card">
                <div class="metric-value">{data.get('files_analyzed', 0)}</div>
                <div class="metric-label">Files Analyzed</div>
            </div>
            
            <div class="metric-card">
                <div class="metric-value">{self._get_error_count(data.get('issues', []))}</div>
                <div class="metric-label">Errors</div>
            </div>
            
            <div class="metric-card">
                <div class="metric-value">{self._get_warning_count(data.get('issues', []))}</div>
                <div class="metric-label">Warnings</div>
            </div>
        </div>
        
        <div class="chart-section">
            <h2 class="chart-title">Issue Trends</h2>
            <div id="trends-chart" style="height: 400px;"></div>
        </div>
        
        <div class="chart-section">
            <h2 class="chart-title">Category Distribution</h2>
            <div id="category-chart" style="height: 400px;"></div>
        </div>
        
        <div class="issues-section">
            <h2 class="chart-title">Issues Details</h2>
            <div id="issues-content">
                <!-- Issues will be loaded here by the original dashboard script -->
            </div>
        </div>
    </div>
    
    <script>
        // Embed data safely
        const dashboardDataElement = document.createElement('script');
        dashboardDataElement.type = 'application/json';
        dashboardDataElement.id = 'dashboard-data';
        dashboardDataElement.textContent = {json.dumps(json.dumps(data))};
        document.head.appendChild(dashboardDataElement);
        
        // Parse the data safely
        const dashboardData = JSON.parse(document.getElementById('dashboard-data').textContent);
        
        // Plot trends
        if (dashboardData.trends && dashboardData.trends.issue_trend.length > 0) {{
            const trace = {{
                x: dashboardData.trends.issue_trend.map(d => d.date),
                y: dashboardData.trends.issue_trend.map(d => d.count),
                type: 'scatter',
                mode: 'lines+markers',
                line: {{color: '#667eea', width: 3}},
                marker: {{size: 8}}
            }};
            
            const layout = {{
                margin: {{t: 20, r: 20, b: 40, l: 50}},
                xaxis: {{title: 'Date'}},
                yaxis: {{title: 'Issue Count'}},
                hovermode: 'x unified'
            }};
            
            Plotly.newPlot('trends-chart', [trace], layout);
        }}
        
        // Plot category distribution
        const categories = ['error', 'warning', 'style', 'performance', 'portability'];
        const categoryCounts = {{
            error: {self._get_error_count(data.get('issues', []))},
            warning: {self._get_warning_count(data.get('issues', []))},
            style: {self._get_style_count(data.get('issues', []))},
            performance: {self._get_performance_count(data.get('issues', []))},
            portability: {self._get_portability_count(data.get('issues', []))}
        }};
        
        const categoryTrace = {{
            x: categories,
            y: categories.map(c => categoryCounts[c]),
            type: 'bar',
            marker: {{
                color: ['#ef4444', '#f59e0b', '#3b82f6', '#10b981', '#8b5cf6']
            }}
        }};
        
        const categoryLayout = {{
            margin: {{t: 20, r: 20, b: 40, l: 50}},
            xaxis: {{title: 'Category'}},
            yaxis: {{title: 'Count'}},
            showlegend: false
        }};
        
        Plotly.newPlot('category-chart', [categoryTrace], categoryLayout);
    </script>
</body>
</html>"""
        
        # Write enhanced dashboard
        output_path = Path(output_path)
        output_path.parent.mkdir(parents=True, exist_ok=True)
        output_path.write_text(html_content)
        print(f"  ✓ Generated enhanced dashboard: {output_path}")
    
    def _generate_trend_indicator(self, summary):
        """Generate trend indicator HTML"""
        if not summary:
            return ""
        
        direction = summary.get("trend_direction", "")
        percentage = summary.get("trend_percentage", 0)
        
        if direction == "up":
            return f'<div class="metric-trend trend-up">↑ {percentage:.1f}%</div>'
        elif direction == "down":
            return f'<div class="metric-trend trend-down">↓ {percentage:.1f}%</div>'
        return ""
    
    def _get_error_count(self, issues):
        """Count error severity issues"""
        return sum(1 for i in issues if i.get("severity") == "error")
    
    def _get_warning_count(self, issues):
        """Count warning severity issues"""
        return sum(1 for i in issues if i.get("severity") == "warning")
    
    def _get_style_count(self, issues):
        """Count style severity issues"""
        return sum(1 for i in issues if i.get("severity") == "style")
    
    def _get_performance_count(self, issues):
        """Count performance severity issues"""
        return sum(1 for i in issues if i.get("severity") == "performance")
    
    def _get_portability_count(self, issues):
        """Count portability severity issues"""
        return sum(1 for i in issues if i.get("severity") == "portability")

def main():
    import argparse
    
    parser = argparse.ArgumentParser(description="Enhance cppcheck dashboards with metrics")
    parser.add_argument("--profile", default="comprehensive", 
                       help="Analysis profile to enhance")
    parser.add_argument("--output", help="Output path for enhanced dashboard")
    parser.add_argument("--all", action="store_true", 
                       help="Enhance all profile dashboards")
    
    args = parser.parse_args()
    
    enhancer = DashboardEnhancer()
    
    if args.all:
        profiles = ["quick_check", "comprehensive", "cpp17_migration", "memory_safety", "performance"]
        for profile in profiles:
            enhancer.enhance_dashboard(profile)
    else:
        enhancer.enhance_dashboard(args.profile, args.output)

if __name__ == "__main__":
    main()