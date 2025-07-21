#!/bin/bash
# Demo script for Enhanced Cppcheck Dashboard Features

echo "🚀 Enhanced Cppcheck Dashboard Demo"
echo "=================================="
echo
echo "New Features:"
echo "✅ Click-to-expand code preview with full function/class context"
echo "✅ Interactive fix suggestions with diff visualization"
echo "✅ Keyboard shortcuts (j/k navigation, space to expand, f for fix)"
echo "✅ Smart search and filtering"
echo "✅ Confidence scoring for automated fixes"
echo "✅ Export capabilities"
echo
echo "Running analysis..."
echo

# Run quick analysis
./tools/cppcheck/cppcheck quick --format html

echo
echo "Dashboard generated! Features to try:"
echo
echo "1. Click on any issue row to see:"
echo "   - Full code context (not just the line)"
echo "   - Function/class breadcrumb navigation"
echo "   - Syntax-highlighted code"
echo "   - Fix suggestions with confidence scores"
echo
echo "2. Use keyboard shortcuts:"
echo "   - j/k to navigate up/down"
echo "   - Space to expand/collapse"
echo "   - f to show fix preview"
echo "   - ? for help"
echo
echo "3. Filter by severity:"
echo "   - Click the Error/Warning/Style/Performance buttons"
echo "   - Use search to find specific issues"
echo
echo "4. Fix preview shows:"
echo "   - Diff view of proposed changes"
echo "   - Confidence percentage"
echo "   - Explanation of the fix"
echo "   - Apply button (dry-run by default)"
echo
echo "Opening dashboard in browser..."

# Find the latest report
LATEST_REPORT=$(find tools/cppcheck/reports -name "report.html" -type f -exec ls -t {} + | head -1)
echo "Report: $LATEST_REPORT"
open "$LATEST_REPORT"