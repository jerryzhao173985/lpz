#!/bin/bash

# Script to revert ALL corrupted header files

echo "Reverting all corrupted header files..."

# Find all header files that were corrupted by the cast script
CORRUPTED_HEADERS=(
    # Core headers
    "selforg/utils/controller_misc.h"
    "selforg/controller/abstractcontrolleradapter.h"
    "selforg/controller/abstractmodel.h"
    "selforg/controller/classicreinforce.h"
    "selforg/controller/qlearning.h"
    "selforg/controller/reinforceable.h"
    "selforg/controller/regularisation.h"
    "selforg/controller/feedforwardnn.h"
    "selforg/controller/controllernet.h"
    "selforg/controller/invertablemodel.h"
    "selforg/controller/layer.h"
    "selforg/utils/quickmp.h"
    "selforg/utils/noisegenerator.h"
    "selforg/utils/inspectable.h"
    "selforg/utils/plotoption.h"
    "selforg/utils/plotoptionengine.h"
    "selforg/utils/configurable.h"
    "selforg/utils/stl_adds.h"
    "selforg/utils/storeable.h"
    "selforg/utils/sensormotorinfo.h"
    
    # Additional headers with issues
    "selforg/controller/braitenberg.h"
    "selforg/controller/crossmotorcoupling.h"
    "selforg/controller/dep.h"
    "selforg/controller/derbigcontroller.h"
    "selforg/controller/dercontroller.h"
    "selforg/controller/derinf.h"
    "selforg/controller/derlininvert.h"
    "selforg/controller/derlinunivers.h"
    "selforg/controller/derpseudosensor.h"
    "selforg/controller/dinvert3channelcontroller.h"
    "selforg/controller/discretecontrolleradapter.h"
    "selforg/controller/discretesizable.h"
    "selforg/controller/elman.h"
    "selforg/controller/esn.h"
    "selforg/controller/ffnncontroller.h"
    "selforg/controller/homeokinbase.h"
    "selforg/controller/invert3channelcontroller.h"
    "selforg/controller/invertcontroller.h"
    "selforg/controller/invertmotorbigmodel.h"
    "selforg/controller/invertmotorcontroller.h"
    "selforg/controller/invertmotornstep.h"
    "selforg/controller/invertmotorspace.h"
    "selforg/controller/invertnchannelcontroller.h"
    "selforg/controller/measureadapter.h"
    "selforg/controller/modelwithmemoryadapter.h"
    "selforg/controller/motorbabbler.h"
    "selforg/controller/multilayerffnn.h"
    "selforg/controller/multireinforce.h"
    "selforg/controller/mutualinformationcontroller.h"
    "selforg/controller/neuralgas.h"
    "selforg/controller/oneactivemultipassivecontroller.h"
    "selforg/controller/onecontrollerperchannel.h"
    "selforg/controller/onelayerffnn.h"
    "selforg/controller/pimax.h"
    "selforg/controller/remotecontrolled.h"
    "selforg/controller/replaycontroller.h"
    "selforg/controller/semox.h"
    "selforg/controller/sinecontroller.h"
    "selforg/controller/som.h"
    "selforg/controller/soml.h"
    "selforg/controller/sos.h"
    "selforg/controller/sox.h"
    "selforg/controller/soxexpand.h"
    "selforg/controller/splitcontrol.h"
    "selforg/controller/switchcontroller.h"
    "selforg/controller/teachable.h"
    "selforg/controller/universalcontroller.h"
    "selforg/controller/use_java_controller.h"
)

# Revert each file
for file in "${CORRUPTED_HEADERS[@]}"; do
    if [ -f "$file" ]; then
        echo "Reverting $file..."
        git checkout -- "$file"
    fi
done

# Also revert corrupted cpp files
echo "Reverting corrupted cpp files..."
CORRUPTED_CPP=(
    "selforg/controller/classicreinforce.cpp"
    "selforg/controller/crossmotorcoupling.cpp"
    "selforg/controller/controllernet.cpp"
    "selforg/controller/dep.cpp"
    "selforg/controller/derbigcontroller.cpp"
    "selforg/controller/dercontroller.cpp"
    "selforg/controller/derinf.cpp"
    "selforg/controller/derlininvert.cpp"
    "selforg/controller/derlinunivers.cpp"
    "selforg/controller/derpseudosensor.cpp"
    "selforg/controller/discretecontrolleradapter.cpp"
    "selforg/controller/elman.cpp"
    "selforg/controller/esn.cpp"
    "selforg/controller/ffnncontroller.cpp"
    "selforg/controller/invertmotorbigmodel.cpp"
    "selforg/controller/invertmotornstep.cpp"
    "selforg/controller/invertmotorspace.cpp"
    "selforg/controller/invertnchannelcontroller.cpp"
    "selforg/controller/layer.cpp"
    "selforg/controller/measureadapter.cpp"
    "selforg/controller/modelwithmemoryadapter.cpp"
    "selforg/controller/motorbabbler.cpp"
    "selforg/controller/multilayerffnn.cpp"
    "selforg/controller/multireinforce.cpp"
    "selforg/controller/mutualinformationcontroller.cpp"
    "selforg/controller/neuralgas.cpp"
    "selforg/controller/oneactivemultipassivecontroller.cpp"
    "selforg/controller/onecontrollerperchannel.cpp"
    "selforg/controller/onelayerffnn.cpp"
    "selforg/controller/pimax.cpp"
    "selforg/controller/qlearning.cpp"
    "selforg/controller/semox.cpp"
    "selforg/controller/sinecontroller.cpp"
    "selforg/controller/som.cpp"
    "selforg/controller/soml.cpp"
    "selforg/controller/sos.cpp"
    "selforg/controller/sox.cpp"
    "selforg/controller/soxexpand.cpp"
    "selforg/controller/splitcontrol.cpp"
    "selforg/controller/switchcontroller.cpp"
    "selforg/controller/universalcontroller.cpp"
    "selforg/controller/use_java_controller.cpp"
)

for file in "${CORRUPTED_CPP[@]}"; do
    if [ -f "$file" ]; then
        echo "Reverting $file..."
        git checkout -- "$file"
    fi
done

echo "Done. All corrupted files have been reverted."