#!/bin/bash

# Script to fix ControllerNet override issues

echo "Fixing ControllerNet override issues..."

# Remove override from methods that don't exist in AbstractModel
sed -i.bak 's/processX (const matrix::Matrix& input,$/processX (const matrix::Matrix& input,/' selforg/controller/controllernet.h
sed -i.bak 's/unsigned int injectInLayer) override;/unsigned int injectInLayer);/' selforg/controller/controllernet.h

sed -i.bak 's/response() const override;/response() const;/' selforg/controller/controllernet.h
sed -i.bak 's/responsePart(int from, int to) const override;/responsePart(int from, int to) const;/' selforg/controller/controllernet.h
sed -i.bak 's/responseLinear() const override;/responseLinear() const;/' selforg/controller/controllernet.h

sed -i.bak 's/matrix::Matrices\* zetas = 0) const override;/matrix::Matrices* zetas = 0) const;/' selforg/controller/controllernet.h
sed -i.bak 's/int startWithLayer = -1) const override;/int startWithLayer = -1) const;/' selforg/controller/controllernet.h

sed -i.bak 's/calcResponseIntern() override;/calcResponseIntern();/' selforg/controller/controllernet.h

# Add override to methods that do exist in AbstractModel
sed -i.bak 's/unsigned int getInputDim() const {/unsigned int getInputDim() const override {/' selforg/controller/controllernet.h
sed -i.bak 's/unsigned int getOutputDim() const {/unsigned int getOutputDim() const override {/' selforg/controller/controllernet.h

# Also need to fix the init method - AbstractModel doesn't have 5 parameters
# Remove the extra parameters from the override
sed -i.bak '/virtual void init(unsigned int inputDim, unsigned  int outputDim,/,/RandGen\* randGen = 0) override;/ {
s/override;//
}' selforg/controller/controllernet.h

# The AbstractModel learn method needs to be implemented
echo "Note: ControllerNet needs to implement the learn() method from AbstractModel"

echo "Done fixing ControllerNet."