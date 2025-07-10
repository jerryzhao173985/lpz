#!/bin/bash

# Fix missing spaces between return type and function name
# These patterns were created by malformed sed replacements

echo "Fixing missing spaces between return types and function names..."

# Fix boolrestore -> bool restore
find . -name "*.h" -o -name "*.cpp" | xargs sed -i '' 's/boolrestore/bool restore/g'

# Fix voidset -> void set
find . -name "*.h" -o -name "*.cpp" | xargs sed -i '' 's/voidset/void set/g'

# Fix voidpush -> void push  
find . -name "*.h" -o -name "*.cpp" | xargs sed -i '' 's/voidpush/void push/g'

# Fix voidremove -> void remove
find . -name "*.h" -o -name "*.cpp" | xargs sed -i '' 's/voidremove/void remove/g'

# Fix other patterns
find . -name "*.h" -o -name "*.cpp" | xargs sed -i '' 's/voidSetSize/void setSize/g'
find . -name "*.h" -o -name "*.cpp" | xargs sed -i '' 's/constgetJumpPoint/const getJumpPoint/g'
find . -name "*.h" -o -name "*.cpp" | xargs sed -i '' 's/voidsignalHandler/void signalHandler/g'

echo "Done fixing missing spaces."