# Cppcheck Dashboard Guide

## Overview

The cppcheck infrastructure provides three dashboard tiers with progressive enhancement:

1. **Basic Dashboard** - Charts and statistics (fallback)
2. **Enhanced Dashboard** - Interactive code preview and fixes (default)
3. **Ultimate Dashboard** - Strategic analysis and insights

## Enhanced Dashboard Features

### Interactive Code Preview
- **Click any issue** to expand ±15 lines of context
- **Smart boundaries** include full function/class when possible
- **Syntax highlighting** with highlight.js
- **Line numbers** for easy reference

### Fix Suggestions
- **One-click preview** of suggested fixes
- **Diff visualization** using diff2html
- **Confidence scoring** (85-99%)
- **Apply/Reject buttons** for each fix

### Keyboard Navigation
| Key | Action |
|-----|--------|
| `j`/`k` | Navigate up/down |
| `Space` | Expand/collapse |
| `f` | Show fix |
| `/` | Focus search |
| `Esc` | Close modal |
| `?` | Help |

### Search & Filter
```
Syntax examples:
- error:nullPointer      # Specific error type
- file:matrix.cpp       # File name
- line:>100            # Line ranges
- severity:warning     # By severity
- id:missingOverride   # By issue ID
```

## Dashboard Generation

### Basic Usage
```bash
# Generate enhanced dashboard (default)
./cppcheck quick --format html

# Force specific dashboard type
DASHBOARD_TYPE=ultimate ./cppcheck full --format html
```

### Dashboard Selection Logic
```python
# Automatic fallback chain
try:
    UltimateDashboard    # Full features
except:
    try:
        EnhancedDashboard  # Code preview + fixes
    except:
        BasicDashboard     # Charts only
```

## Implementation Details

### Issue Processing
```javascript
// Each issue enhanced with:
{
    id: 'hash-of-file-line',
    severity: 'warning',
    file: 'src/matrix.cpp',
    line: 42,
    message: 'Missing override',
    context: {
        lines: [...],
        function: 'process()',
        class: 'Matrix'
    },
    fix: {
        confidence: 98,
        description: 'Add override',
        diff: '...'
    }
}
```

### Performance Optimization
- **Virtual scrolling** for large issue lists
- **Lazy loading** of code contexts
- **Progressive enhancement** pattern
- **Debounced search** (300ms)

## Customization

### CSS Variables
```css
:root {
    --primary: #667eea;
    --danger: #f56565;
    --warning: #ed8936;
    --success: #48bb78;
}
```

### JavaScript Hooks
```javascript
// Add custom functionality
DashboardController.addHook('issueClick', (issue) => {
    console.log('Issue clicked:', issue);
});
```

## Browser Requirements
- Chrome 60+
- Firefox 55+
- Safari 11+
- Edge 79+

## Troubleshooting

### Dashboard Not Loading
1. Check browser console for errors
2. Verify JSON report exists
3. Try basic dashboard mode
4. Check file permissions

### Fix Suggestions Missing
- Not all issues have automated fixes
- Check fix confidence threshold
- Some fixes require human judgment

### Performance Issues
- Limit initial display to 500 issues
- Use search/filter to reduce DOM
- Enable virtual scrolling