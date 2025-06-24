#\!/bin/bash
# Fix more static_cast syntax errors

echo "Fixing additional static_cast syntax errors..."

# Fix patterns
sed -i '' 's/static_cast<int>(n)umber_motors/static_cast<int>(number_motors)/g' selforg/controller/soml.cpp
sed -i '' 's/static_cast<int>(c)trl\.size()/static_cast<int>(ctrl.size())/g' selforg/controller/onecontrollerperchannel.cpp
sed -i '' 's/static_cast<int>(p)arams\.size()/static_cast<int>(params.size())/g' selforg/controller/pimax.cpp
sed -i '' 's/static_cast<int>(p)arams\.size()/static_cast<int>(params.size())/g' selforg/controller/sox.cpp
sed -i '' 's/static_cast<int>(c)onf\.numberContextSensors/static_cast<int>(conf.numberContextSensors)/g' selforg/controller/semox.cpp

echo "Done fixing additional static_cast syntax errors."
