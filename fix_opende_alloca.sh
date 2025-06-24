#!/bin/bash
# Fix alloca usage in opende

cd /Users/jerry/lpzrobot_mac/opende/ode/src

# Add necessary includes at the top of files that will use std::vector
for file in collision_space.cpp lcp.cpp; do
    if grep -q "alloca" "$file" && ! grep -q "#include <vector>" "$file"; then
        # Add vector include after the first include
        sed -i '' '/#include/a\
#include <vector>
' "$file"
    fi
done

# Replace alloca calls with std::vector or stack arrays
# Pattern 1: Simple array allocations
sed -i '' 's/dContactGeom \*contact_base = (dContactGeom\*) alloca (\([^)]*\))/std::vector<dContactGeom> contact_vec(\1 \/ sizeof(dContactGeom)); dContactGeom *contact_base = contact_vec.data()/' collision_space.cpp

sed -i '' 's/dContactGeom \*pcontact = (dContactGeom \*) alloca (\([^)]*\))/std::vector<dContactGeom> pcontact_vec(\1 \/ sizeof(dContactGeom)); dContactGeom *pcontact = pcontact_vec.data()/' collision_space.cpp

sed -i '' 's/int \*cg_idx = (int\*) alloca (\([^)]*\))/std::vector<int> cg_idx_vec(\1 \/ sizeof(int)); int *cg_idx = cg_idx_vec.data()/' collision_space.cpp

sed -i '' 's/int \*idx = (int\*) alloca (\([^)]*\))/std::vector<int> idx_vec(\1 \/ sizeof(int)); int *idx = idx_vec.data()/' collision_space.cpp

# Fix lcp.cpp alloca usage - these are more complex
# Most are of the form: type *var = (type*) alloca (n * sizeof(type))
# We'll use a more general approach

echo "Fixing alloca calls in lcp.cpp..."
# Create a backup first
cp lcp.cpp lcp.cpp.bak

# Process lcp.cpp with a Python script for more complex replacements
cat > fix_lcp_alloca.py << 'EOF'
import re
import sys

with open('lcp.cpp', 'r') as f:
    content = f.read()

# Track which types need vector includes
types_used = set()

def replace_alloca(match):
    full_match = match.group(0)
    type_name = match.group(1)
    var_name = match.group(2)
    size_expr = match.group(3)
    
    # Extract the count from size expressions like "n * sizeof(type)"
    size_match = re.search(r'(\w+)\s*\*\s*sizeof\s*\([^)]+\)', size_expr)
    if size_match:
        count = size_match.group(1)
        types_used.add(type_name)
        return f"std::vector<{type_name}> {var_name}_vec({count}); {type_name} *{var_name} = {var_name}_vec.data()"
    
    # For other patterns, try to extract a simple count
    # e.g., (n*sizeof(dReal)) -> n
    size_match2 = re.search(r'\(([^)]+)\)', size_expr)
    if size_match2:
        inner = size_match2.group(1)
        if '*sizeof' in inner:
            count = inner.split('*sizeof')[0].strip()
            types_used.add(type_name)
            return f"std::vector<{type_name}> {var_name}_vec({count}); {type_name} *{var_name} = {var_name}_vec.data()"
    
    # Default case - unchanged
    return full_match

# Pattern to match: type *var = (type*) alloca(size_expr)
pattern = r'(\w+)\s*\*\s*(\w+)\s*=\s*\([^)]+\)\s*alloca\s*\(([^;]+)\)'
content = re.sub(pattern, replace_alloca, content)

# Also handle ALLOCA macro if present
pattern2 = r'(\w+)\s*\*\s*(\w+)\s*=\s*\([^)]+\)\s*ALLOCA\s*\(([^;]+)\)'
content = re.sub(pattern2, replace_alloca, content)

with open('lcp.cpp', 'w') as f:
    f.write(content)

print(f"Replaced alloca calls. Types used: {types_used}")
EOF

python3 fix_lcp_alloca.py
rm fix_lcp_alloca.py

echo "All alloca replacements completed"