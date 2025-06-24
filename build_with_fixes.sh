#!/bin/bash
# Build with automatic fixes applied

set -e

echo "=== Applying critical fixes before build ==="

# Fix Matrix explicit constructor
echo "Fixing Matrix constructor..."
sed -i '' 's/explicit Matrix(const Matrix& c);/Matrix(const Matrix& c);/g' selforg/matrix/matrix.h

# Remove incorrect override from destructors
echo "Removing incorrect override specifiers..."
find . -name "*.h" -exec sed -i '' 's/virtual ~BackCaller() override;/virtual ~BackCaller();/g' {} \;
find . -name "*.h" -exec sed -i '' 's/virtual ~Inspectable() override;/virtual ~Inspectable();/g' {} \;
find . -name "*.h" -exec sed -i '' 's/virtual ~PlotOptionEngine() override;/virtual ~PlotOptionEngine();/g' {} \;
find . -name "*.h" -exec sed -i '' 's/virtual ~ClassicReinforce() override;/virtual ~ClassicReinforce();/g' {} \;

echo "=== Building selforg ==="
cd selforg
make clean > /dev/null 2>&1 || true
if make -j16; then
    echo "=== selforg built successfully ==="
    cd ..
    
    # Continue with the full build
    echo "=== Running full build ==="
    ./build_lpzrobots.sh ~/lpzrobots USER
else
    echo "=== selforg build failed ==="
    exit 1
fi