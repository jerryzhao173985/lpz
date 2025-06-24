#!/bin/bash

# Fix for template_cycled_GA_Simulation syntax error
# The issue is similar to SingletonGenEngine.cpp - malformed cast with sqrt

echo "Fixing syntax error in template_cycled_GA_Simulation/main.cpp..."

FILE="ga_tools/simulations/template_cycled_GA_Simulation/main.cpp"

if [ -f "$FILE" ]; then
    # Create backup
    cp "$FILE" "$FILE.bak"
    
    # Fix line 411 - the issue is static_cast<int>(sqrt) should be static_cast<int>(sqrt(...))
    # The correct syntax should be: static_cast<int>(sqrt(numberIndividuals))
    sed -i '' '411s/static_cast<int>(sqrt)(numberIndividuals)/static_cast<int>(sqrt(numberIndividuals))/' "$FILE"
    
    # Fix line 412 as well (same issue)
    sed -i '' '412s/static_cast<int>(sqrt)(numberIndividuals)/static_cast<int>(sqrt(numberIndividuals))/' "$FILE"
    
    echo "Fixed syntax errors in $FILE"
    echo "Backup saved as $FILE.bak"
else
    echo "Error: File not found: $FILE"
fi

echo "Done!"