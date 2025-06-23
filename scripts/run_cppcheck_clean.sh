#!/bin/bash

# Run cppcheck on clean codebase after fixing corrupted files

cd /Users/jerry/lpzrobot_mac

echo "Running cppcheck on selforg component..."
cppcheck --enable=all --error-exitcode=0 --std=c++17 --inline-suppr \
    --suppress=missingIncludeSystem \
    --suppress=unmatchedSuppression \
    --suppress=unusedFunction \
    --suppress=passedByValue \
    -I selforg -I selforg/controller -I selforg/utils -I selforg/matrix \
    selforg/ 2>&1 | tee cppcheck_selforg_clean.log

echo ""
echo "Analyzing results..."
echo "========================"
echo "Error Summary:"
grep -E "^\[.*\]: \(error\)" cppcheck_selforg_clean.log | sort | uniq -c | sort -nr

echo ""
echo "Warning Summary:"
grep -E "^\[.*\]: \(warning\)" cppcheck_selforg_clean.log | sort | uniq -c | sort -nr

echo ""
echo "Style Summary:"
grep -E "^\[.*\]: \(style\)" cppcheck_selforg_clean.log | sort | uniq -c | sort -nr | head -20

echo ""
echo "Performance Summary:"
grep -E "^\[.*\]: \(performance\)" cppcheck_selforg_clean.log | sort | uniq -c | sort -nr | head -10

echo ""
echo "Total issues:"
echo "Errors: $(grep -c "(error)" cppcheck_selforg_clean.log)"
echo "Warnings: $(grep -c "(warning)" cppcheck_selforg_clean.log)"
echo "Style: $(grep -c "(style)" cppcheck_selforg_clean.log)"
echo "Performance: $(grep -c "(performance)" cppcheck_selforg_clean.log)"