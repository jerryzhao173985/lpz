#!/bin/bash

echo "Fixing remaining syntax errors from malformed sed replacements..."

# 1. Fix missing spaces between return types and function names
echo "1. Fixing missing spaces between types and function names..."
find . -name "*.h" -o -name "*.cpp" | xargs sed -i '' 's/boolrestore/bool restore/g'
find . -name "*.h" -o -name "*.cpp" | xargs sed -i '' 's/voidset/void set/g'
find . -name "*.h" -o -name "*.cpp" | xargs sed -i '' 's/voidpush/void push/g'
find . -name "*.h" -o -name "*.cpp" | xargs sed -i '' 's/voidremove/void remove/g'
find . -name "*.h" -o -name "*.cpp" | xargs sed -i '' 's/voidSetSize/void setSize/g'
find . -name "*.h" -o -name "*.cpp" | xargs sed -i '' 's/voidsignalHandler/void signalHandler/g'
find . -name "*.h" -o -name "*.cpp" | xargs sed -i '' 's/voidcreate/void create/g'
find . -name "*.h" -o -name "*.cpp" | xargs sed -i '' 's/doubleget/double get/g'
find . -name "*.h" -o -name "*.cpp" | xargs sed -i '' 's/boolisInside/bool isInside/g'

# 2. Fix "inline_ type explicit" patterns (should be "inline type")
echo "2. Fixing inline_ explicit patterns..."
find . -name "*.h" -o -name "*.cpp" | xargs sed -i '' 's/inline_ \([a-zA-Z_]*\) explicit/inline \1/g'

# 3. Fix "template <class T{" (missing closing >)
echo "3. Fixing template syntax..."
find . -name "*.h" -o -name "*.cpp" | xargs sed -i '' 's/template <class T{/template <class T> {/g'

# 4. Fix misplaced const in expressions like "(const Flags&"
echo "4. Fixing const reference patterns..."
find . -name "*.h" -o -name "*.cpp" | xargs sed -i '' 's/(const Flags&/(Flags const\&/g'

# 5. Fix destructors with misplaced constructors
echo "5. Fixing destructor syntax..."
find . -name "*.h" -o -name "*.cpp" | xargs sed -i '' 's/~\([A-Za-z_][A-Za-z0-9_]*\) : \([^{]*\) {/~\1() { \/\/ TODO: Check initialization list/g'

# 6. Fix "static void" patterns where static is in wrong place
echo "6. Fixing static placement..."
find . -name "*.h" -o -name "*.cpp" | xargs sed -i '' 's/\tstatic void/\tstatic void/g'

# 7. Fix malformed class declarations
echo "7. Fixing class declarations..."
find . -name "*.h" -o -name "*.cpp" | xargs sed -i '' 's/class static_cast{/class /g'
find . -name "*.h" -o -name "*.cpp" | xargs sed -i '' 's/Abstract class static_cast{/class /g'

# 8. Fix special inline patterns
echo "8. Fixing special inline patterns..."
find . -name "*.h" -o -name "*.cpp" | xargs sed -i '' 's/inline dReal explicit/inline dReal/g'

# 9. Remove trailing " override" from statements (not declarations)
echo "9. Fixing override in wrong contexts..."
# This is more complex and needs careful handling - best done manually

echo "Done! Please review the changes with 'git diff' and compile to verify."
echo ""
echo "Remaining manual fixes needed:"
echo "1. Check destructors with initialization lists"
echo "2. Review any 'override' keywords that appear outside method declarations"
echo "3. Verify template syntax is correct"
echo "4. Check for any remaining 'explicit' keywords in wrong contexts"