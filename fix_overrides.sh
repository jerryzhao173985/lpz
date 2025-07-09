#!/bin/bash

# Script to remove incorrect override keywords from controller headers
# These methods don't exist in base classes but were incorrectly marked with override

echo "Fixing incorrect override keywords in controller headers..."

# Fix crossmotorcoupling.h
sed -i '' 's/virtual void setCMC(const matrix::Matrix& cmc) override;/virtual void setCMC(const matrix::Matrix& cmc);/' selforg/controller/crossmotorcoupling.h
sed -i '' 's/matrix::Matrix getCMC() const override;/matrix::Matrix getCMC() const;/' selforg/controller/crossmotorcoupling.h

# Fix controllernet.h
sed -i '' 's/virtual const matrix::Matrix response(const matrix::Matrix& input) const override;/virtual const matrix::Matrix response(const matrix::Matrix& input) const;/' selforg/controller/controllernet.h
sed -i '' 's/virtual matrix::Matrix responsePart(const matrix::Matrix& input, int from, int to) const override;/virtual matrix::Matrix responsePart(const matrix::Matrix& input, int from, int to) const;/' selforg/controller/controllernet.h
sed -i '' 's/virtual const matrix::Matrix responseLinear(const matrix::Matrix& input) const override;/virtual const matrix::Matrix responseLinear(const matrix::Matrix& input) const;/' selforg/controller/controllernet.h
sed -i '' 's/virtual void calcResponseIntern() const override;/virtual void calcResponseIntern() const;/' selforg/controller/controllernet.h

# Fix multilayerffnn.h
sed -i '' 's/virtual void setActivationFunction(ActivationFunction actfun) override;/virtual void setActivationFunction(ActivationFunction actfun);/' selforg/controller/multilayerffnn.h
sed -i '' 's/virtual void setActivationFunctions(std::vector<ActivationFunction> actfunList) override;/virtual void setActivationFunctions(std::vector<ActivationFunction> actfunList);/' selforg/controller/multilayerffnn.h

# Fix derbigcontroller.h
sed -i '' 's/virtual void setMotorTeachingSignal(const motor\* teaching, int len) override;/virtual void setMotorTeachingSignal(const motor* teaching, int len);/' selforg/controller/derbigcontroller.h
sed -i '' 's/virtual void setSensorTeachingSignal(const sensor\* teaching, int len) override;/virtual void setSensorTeachingSignal(const sensor* teaching, int len);/' selforg/controller/derbigcontroller.h

# Fix other common patterns
find selforg/controller -name "*.h" -exec sed -i '' \
  -e 's/virtual bool store(FILE\* f) const override;/virtual bool store(FILE* f) const;/g' \
  -e 's/virtual bool restore(FILE\* f) override;/virtual bool restore(FILE* f);/g' \
  {} \;

echo "Done fixing override keywords."