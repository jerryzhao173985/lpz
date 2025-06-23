#!/bin/bash

# Script to fix incorrect override keywords in base classes

echo "Fixing incorrect override keywords in base classes..."

# Fix AbstractModel
if [ -f "selforg/controller/abstractmodel.h" ]; then
    echo "Fixing abstractmodel.h..."
    sed -i.bak 's/override = 0;/= 0;/g' selforg/controller/abstractmodel.h
fi

# Fix ControllerNet - it should inherit from AbstractModel
if [ -f "selforg/controller/controllernet.h" ]; then
    echo "Fixing controllernet.h to inherit from AbstractModel..."
    sed -i.bak 's/class ControllerNet : public Configurable {/class ControllerNet : public AbstractModel {/' selforg/controller/controllernet.h
    # Add include if not present
    if ! grep -q "#include \"abstractmodel.h\"" selforg/controller/controllernet.h; then
        sed -i.bak '/#include <selforg\/configurable.h>/a\
#include "abstractmodel.h"' selforg/controller/controllernet.h
    fi
fi

# Fix QlearningIn other base classes
echo "Fixing other base class pure virtual functions..."

# Find all files with "override = 0" pattern (pure virtual with incorrect override)
find selforg -name "*.h" -exec grep -l "override = 0" {} \; | while read file; do
    echo "Fixing $file..."
    sed -i.bak 's/override = 0;/= 0;/g' "$file"
done

# Fix agent.cpp if it has cast issues
if [ -f "selforg/agent.cpp" ]; then
    echo "Fixing agent.cpp casts..."
    sed -i.bak 's/(sensor\*) malloc(sizeof(sensor)/static_cast<sensor*>(malloc(sizeof(sensor)/g' selforg/agent.cpp
    sed -i.bak 's/(motor\*) malloc(sizeof(motor)/static_cast<motor*>(malloc(sizeof(motor)/g' selforg/agent.cpp
fi

# Fix controller_misc.h template casts
if [ -f "selforg/utils/controller_misc.h" ]; then
    echo "Fixing controller_misc.h template casts..."
    sed -i.bak 's/(T)0/static_cast<T>(0)/g' selforg/utils/controller_misc.h
    sed -i.bak 's/(T)-1/static_cast<T>(-1)/g' selforg/utils/controller_misc.h  
    sed -i.bak 's/(T)1/static_cast<T>(1)/g' selforg/utils/controller_misc.h
fi

echo "Done fixing base class issues."