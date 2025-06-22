#!/bin/bash
# Script to fix permissions and signing for GUI tools

echo "Fixing GUI tool permissions and signing..."

# Remove quarantine attributes if any
echo "Removing quarantine attributes..."
xattr -cr /Users/jerry/lpzrobots_mac/guilogger/src/bin/guilogger.app 2>/dev/null
xattr -cr /Users/jerry/lpzrobots_mac/matrixviz/bin/matrixviz.app 2>/dev/null

# Re-sign the applications
echo "Re-signing applications..."
codesign --force --deep --sign - /Users/jerry/lpzrobots_mac/guilogger/src/bin/guilogger.app
codesign --force --deep --sign - /Users/jerry/lpzrobots_mac/matrixviz/bin/matrixviz.app

echo "Done! The applications should now work properly."
echo ""
echo "To run guilogger:"
echo "  /Users/jerry/lpzrobots_mac/guilogger/guilogger-wrapper.sh [options]"
echo "  or"
echo "  open -a /Users/jerry/lpzrobots_mac/guilogger/src/bin/guilogger.app --args [options]"
echo ""
echo "To run matrixviz:"
echo "  /Users/jerry/lpzrobots_mac/matrixviz/matrixviz-wrapper.sh [options]"
echo "  or"
echo "  yourprogram | /Users/jerry/lpzrobots_mac/matrixviz/bin/matrixviz [options]"