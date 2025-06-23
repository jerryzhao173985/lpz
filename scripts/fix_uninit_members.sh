#!/bin/bash

# Script to find and show uninitialized member variables in selforg

cd /Users/jerry/lpzrobot_mac

echo "=== Finding Uninitialized Member Variables ==="
echo ""

# Extract the uninitialized member warnings from cppcheck log
grep "uninitMemberVar" cppcheck_selforg_clean.log | while read line; do
    # Extract file, line, class, and member name
    file=$(echo "$line" | cut -d':' -f1)
    line_num=$(echo "$line" | cut -d':' -f2)
    warning=$(echo "$line" | cut -d':' -f4- | sed 's/^ //')
    
    # Extract member name from warning message
    member=$(echo "$warning" | sed "s/.*Member variable '\([^']*\)'.*/\1/")
    class_member=$(echo "$member" | cut -d':' -f2)
    class_name=$(echo "$member" | cut -d':' -f1)
    
    echo "File: $file:$line_num"
    echo "  Class: $class_name"
    echo "  Member: $class_member"
    echo "  Warning: $warning"
    echo ""
done | tee uninit_members_report.txt

echo ""
echo "Total uninitialized member warnings: $(grep -c "uninitMemberVar" cppcheck_selforg_clean.log)"
echo ""
echo "Most common patterns:"
grep "uninitMemberVar" cppcheck_selforg_clean.log | sed "s/.*Member variable '\([^']*\)'.*/\1/" | cut -d':' -f2 | sort | uniq -c | sort -nr | head -20

echo ""
echo "Files with most issues:"
grep "uninitMemberVar" cppcheck_selforg_clean.log | cut -d':' -f1 | sort | uniq -c | sort -nr | head -10