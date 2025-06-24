#\!/bin/bash
# Fix misplaced parentheses in static_cast expressions

echo "Fixing static_cast syntax errors..."

# Fix pattern static_cast<int>(x)variable => static_cast<int>(variable)
sed -i '' 's/static_cast<int>(u)pdate/static_cast<int>(update)/g' selforg/controller/elman.cpp
sed -i '' 's/static_cast<int>(n)umber_steps_for_averaging_/static_cast<int>(number_steps_for_averaging_)/g' selforg/controller/ffnncontroller.cpp
sed -i '' 's/static_cast<int>(k)wta/static_cast<int>(kwta)/g' selforg/controller/invertmotorbigmodel.cpp
sed -i '' 's/static_cast<int>(l)imitRF/static_cast<int>(limitRF)/g' selforg/controller/invertmotorbigmodel.cpp
sed -i '' 's/static_cast<int>(r)fSize/static_cast<int>(rfSize)/g' selforg/controller/invertmotorbigmodel.cpp
sed -i '' 's/static_cast<int>(k)wta/static_cast<int>(kwta)/g' selforg/controller/invertmotornstep.cpp
sed -i '' 's/static_cast<int>(l)imitRF/static_cast<int>(limitRF)/g' selforg/controller/invertmotornstep.cpp
sed -i '' 's/static_cast<int>(r)fSize/static_cast<int>(rfSize)/g' selforg/controller/invertmotornstep.cpp
sed -i '' 's/static_cast<int>(l)ayers/static_cast<int>(layers)/g' selforg/controller/multilayerffnn.cpp

echo "Fixed static_cast syntax errors."
