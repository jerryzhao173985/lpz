#!/bin/bash

# Script to revert critical corrupted files that are blocking compilation

echo "Reverting critical corrupted files..."

# Critical header files that are causing compilation errors
CRITICAL_FILES=(
    "selforg/utils/controller_misc.h"
    "selforg/controller/abstractcontrolleradapter.h"
    "selforg/controller/classicreinforce.h"
    "selforg/controller/classicreinforce.cpp"
    "selforg/controller/qlearning.h"
    "selforg/controller/reinforceable.h"
    "selforg/utils/quickmp.h"
    "selforg/utils/noisegenerator.h"
    "selforg/controller/invertablemodel.h"
    "selforg/utils/inspectable.h"
    "selforg/utils/plotoption.h"
    "selforg/utils/plotoptionengine.h"
    "selforg/utils/configurable.h"
    "selforg/utils/stl_adds.h"
    "selforg/utils/storeable.h"
    "selforg/utils/sensormotorinfo.h"
)

# Revert each file
for file in "${CRITICAL_FILES[@]}"; do
    if [ -f "$file" ]; then
        echo "Reverting $file..."
        git checkout -- "$file"
    fi
done

echo "Critical files reverted. Now fixing legitimate issues..."

# Fix the legitimate C-style casts manually in critical files
# These are safe replacements that won't corrupt the code

# Fix agent.cpp malloc casts
if [ -f "selforg/agent.cpp" ]; then
    echo "Fixing agent.cpp..."
    sed -i.bak 's/(sensor\*) malloc(sizeof(sensor)/static_cast<sensor*>(malloc(sizeof(sensor)/g' selforg/agent.cpp
    sed -i.bak 's/(motor\*) malloc(sizeof(motor)/static_cast<motor*>(malloc(sizeof(motor)/g' selforg/agent.cpp
fi

# Fix controller_misc.h template functions
if [ -f "selforg/utils/controller_misc.h" ]; then
    echo "Fixing controller_misc.h..."
    sed -i.bak 's/(T)0/static_cast<T>(0)/g' selforg/utils/controller_misc.h
    sed -i.bak 's/(T)-1/static_cast<T>(-1)/g' selforg/utils/controller_misc.h
    sed -i.bak 's/(T)1/static_cast<T>(1)/g' selforg/utils/controller_misc.h
fi

echo "Done. Try building again."