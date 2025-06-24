#!/bin/bash
# Comprehensive fix for all build issues

echo "=== Fixing all build issues ==="

# 1. Fix Matrix explicit constructor
echo "Fixing Matrix explicit constructor..."
sed -i '' 's/explicit Matrix(const Matrix& c);/Matrix(const Matrix& c);/g' selforg/matrix/matrix.h

# 2. Remove override from destructors that don't override
echo "Removing incorrect override specifiers from destructors..."
sed -i '' 's/virtual ~BackCaller() override;/virtual ~BackCaller();/g' selforg/include/selforg/backcaller.h
sed -i '' 's/virtual ~BackCaller() override;/virtual ~BackCaller();/g' selforg/utils/backcaller.h
sed -i '' 's/virtual ~Inspectable() override;/virtual ~Inspectable();/g' selforg/include/selforg/inspectable.h
sed -i '' 's/virtual ~Inspectable() override;/virtual ~Inspectable();/g' selforg/utils/inspectable.h
sed -i '' 's/virtual ~PlotOptionEngine() override;/virtual ~PlotOptionEngine();/g' selforg/utils/plotoptionengine.h

# 3. Fix Matrix override issue
echo "Fixing Matrix destructor override..."
sed -i '' 's/~Matrix() override {/~Matrix() {/g' selforg/matrix/matrix.h

# 4. Add override to qlearning methods
echo "Adding override specifiers to qlearning..."
sed -i '' 's/virtual bool store(FILE\* f) const;/virtual bool store(FILE* f) const override;/g' selforg/include/selforg/qlearning.h
sed -i '' 's/virtual bool restore(FILE\* f);/virtual bool restore(FILE* f) override;/g' selforg/include/selforg/qlearning.h

# 5. Fix ClassicReinforce destructor
echo "Fixing ClassicReinforce destructor..."
sed -i '' 's/virtual ~ClassicReinforce() override;/virtual ~ClassicReinforce();/g' selforg/controller/classicreinforce.h

echo "=== All fixes applied ==="