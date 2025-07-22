# Cppcheck Dashboard Deployment Guide

This guide explains how to set up and use the comprehensive Cppcheck dashboard deployment system for the LPZRobots project.

## 🚀 Overview

The enhanced Cppcheck CI system provides:
- **Automatic analysis** on every push, PR, and scheduled runs
- **Interactive HTML dashboards** with code preview and fix suggestions
- **Historical metrics tracking** with trend visualization
- **GitHub Pages deployment** for easy access to results
- **PR commenting** with analysis summaries

## 📋 Prerequisites

1. **GitHub Pages enabled** for your repository
   - Go to Settings → Pages
   - Source: Deploy from a branch
   - Branch: `gh-pages` (will be created automatically)

2. **Permissions** for GitHub Actions
   - Settings → Actions → General
   - Workflow permissions: Read and write permissions

## 🛠️ Setup Instructions

### 1. Add the GitHub Actions Workflow

Copy the workflow file to your repository:

```bash
# Create the workflows directory if it doesn't exist
mkdir -p .github/workflows

# Copy the workflow file
cp tools/cppcheck/workflows/cppcheck-dashboard.yml .github/workflows/
```

**Note**: Due to GitHub App permissions, you need to manually add workflow files to `.github/workflows/`.

### 2. Configure GitHub Pages

The workflow will automatically:
- Generate dashboards after analysis
- Deploy to GitHub Pages on push to main
- Update metrics database

Your dashboard will be available at:
```
https://<username>.github.io/<repository>/
```

### 3. Custom Domain (Optional)

To use a custom domain:

1. Add your domain to the workflow:
   ```yaml
   - name: Generate dashboards
     run: |
       python3 tools/cppcheck/scripts/deploy_dashboard.py \
         --deploy-dir _site \
         --update-metrics \
         --custom-domain your-domain.com
   ```

2. Configure DNS settings as per [GitHub's documentation](https://docs.github.com/en/pages/configuring-a-custom-domain-for-your-github-pages-site).

## 📊 Dashboard Features

### Main Dashboard (`/`)
- **Metrics Overview**: Total issues, files analyzed, C++17 opportunities
- **Analysis Dashboards**: Links to all analysis types
- **Historical Trends**: Link to trends visualization

### Analysis Dashboards
Each analysis type has its own dashboard:
- **Quick Analysis** (`/quick/`) - Fast development checks
- **Comprehensive** (`/comprehensive/`) - Full codebase analysis
- **C++17 Migration** (`/cpp17/`) - Modernization opportunities
- **Memory Safety** (`/memory/`) - Memory-related issues
- **Performance** (`/performance/`) - Performance optimizations

### Trends Dashboard (`/trends/`)
- Issue count over time
- Category distribution
- Performance metrics

### Dashboard Features
- **Interactive Code Preview**: Click issues to see context
- **Fix Suggestions**: One-click preview of fixes
- **Keyboard Navigation**: 
  - `j/k` - Navigate issues
  - `space` - Expand/collapse
  - `f` - Show fix suggestion
- **Search & Filter**: Real-time filtering
- **Export**: Download as CSV/JSON

## 🔧 Local Development

### Generate Dashboard Locally

```bash
# Run analysis
python3 tools/cppcheck/scripts/analyze.py --profile comprehensive

# Generate dashboard
python3 tools/cppcheck/scripts/deploy_dashboard.py --deploy-dir local_dashboard

# Open in browser
open local_dashboard/index.html
```

### Test Enhanced Features

```bash
# Generate enhanced dashboard with metrics
python3 tools/cppcheck/scripts/enhance_dashboard_metrics.py \
  --profile comprehensive \
  --output enhanced_dashboard.html
```

## 📈 Metrics Tracking

The system automatically tracks:
- Issue counts by severity
- Analysis performance
- Historical trends
- File coverage

Metrics are stored in SQLite database at:
```
tools/cppcheck/metrics/cppcheck_metrics.db
```

### View Metrics

```bash
# Show current metrics
python3 tools/cppcheck/scripts/metrics.py show

# Show trends
python3 tools/cppcheck/scripts/metrics.py trends --days 30
```

## 🔄 Workflow Triggers

The dashboard updates on:

1. **Push to main**: Full analysis + deployment
2. **Pull Request**: Quick analysis + PR comment
3. **Schedule**: Daily comprehensive analysis (2 AM UTC)
4. **Manual**: Via Actions tab with analysis type selection

## 🎯 Workflow Customization

### Change Schedule

Edit the cron expression in the workflow:
```yaml
schedule:
  - cron: '0 2 * * *'  # Daily at 2 AM UTC
```

### Add Analysis Profiles

1. Create new profile in `tools/cppcheck/profiles/`
2. Add to workflow:
   ```yaml
   - name: Run custom analysis
     run: |
       python3 tools/cppcheck/scripts/analyze.py \
         --profile custom_profile \
         --format json
   ```

### Customize PR Comments

Modify the `comment-pr` job to change comment format or add metrics.

## 🚨 Troubleshooting

### Dashboard Not Updating

1. Check Actions tab for workflow runs
2. Verify GitHub Pages is enabled
3. Check deployment logs

### Analysis Failing

1. Ensure cppcheck is installed
2. Check compile_commands.json generation
3. Review analysis logs

### Metrics Not Recording

1. Ensure metrics directory exists
2. Check write permissions
3. Verify SQLite is available

## 📝 Advanced Configuration

### Self-Contained Dashboards

For offline viewing without CDN dependencies:

```bash
python3 tools/cppcheck/scripts/deploy_dashboard.py \
  --deploy-dir _site \
  --standalone
```

### Multiple Branches

To analyze multiple branches:

```yaml
on:
  push:
    branches: [ main, develop, feature/* ]
```

### Slack/Email Notifications

Add notification step to workflow:

```yaml
- name: Notify Slack
  if: failure()
  uses: 8398a7/action-slack@v3
  with:
    status: ${{ job.status }}
    text: 'Cppcheck found issues!'
```

## 🎉 Best Practices

1. **Regular Analysis**: Use scheduled runs for comprehensive checks
2. **PR Checks**: Keep PR analysis fast with quick profile
3. **Fix Issues Promptly**: Address high-severity issues first
4. **Monitor Trends**: Check dashboard weekly for patterns
5. **Update Suppressions**: Keep suppressions file current

## 📚 Additional Resources

- [Cppcheck Documentation](http://cppcheck.sourceforge.net/)
- [GitHub Pages Documentation](https://docs.github.com/en/pages)
- [GitHub Actions Documentation](https://docs.github.com/en/actions)

## 🤝 Contributing

To improve the dashboard system:

1. Test changes locally first
2. Update documentation
3. Submit PR with examples

For issues or suggestions, please open a GitHub issue.