#!/bin/bash
# Add missing override specifiers to virtual functions

echo "Adding missing override specifiers..."

# Function to add override to specific files
add_overrides() {
    local file=$1
    echo "Processing $file..."
    
    # Create backup
    cp "$file" "$file.bak"
    
    # Add override to common virtual functions
    # init function
    perl -i -pe 's/(virtual void init\(int sensornumber, int motornumber[^)]*\));/$1 override;/g unless /override/' "$file"
    
    # getSensorNumber/getMotorNumber
    perl -i -pe 's/(virtual int getSensorNumber\(\) const) \{/$1 override {/g unless /override/' "$file"
    perl -i -pe 's/(virtual int getMotorNumber\(\) const) \{/$1 override {/g unless /override/' "$file"
    
    # step functions
    perl -i -pe 's/(virtual void step\(const sensor[^)]*\));/$1 override;/g unless /override/' "$file"
    perl -i -pe 's/(virtual void stepNoLearning\(const sensor[^)]*\));/$1 override;/g unless /override/' "$file"
    
    # store/restore
    perl -i -pe 's/(virtual bool store\(FILE\* f\) const);/$1 override;/g unless /override/' "$file"
    perl -i -pe 's/(virtual bool restore\(FILE\* f\));/$1 override;/g unless /override/' "$file"
    
    # notifyOnChange
    perl -i -pe 's/(virtual void notifyOnChange\(const paramkey& key\));/$1 override;/g unless /override/' "$file"
    
    # Inspectable methods
    perl -i -pe 's/(virtual std::list<\w+> getInternal\w+\(\) const);/$1 override;/g unless /override/' "$file"
    perl -i -pe 's/(virtual std::list<\w+> getStructural\w+\(\) const);/$1 override;/g unless /override/' "$file"
    
    # motorBabblingStep
    perl -i -pe 's/(virtual void motorBabblingStep\([^{]*\));/$1 override;/g unless /override|{/' "$file"
    
    # Remove backup if no changes
    if cmp -s "$file" "$file.bak"; then
        rm "$file.bak"
    else
        echo "  Updated: $file"
        rm "$file.bak"
    fi
}

# Process header files with missing overrides
headers=(
    "selforg/controller/classicreinforce.h"
    "selforg/include/selforg/qlearning.h"
)

for header in "${headers[@]}"; do
    if [ -f "$header" ]; then
        add_overrides "$header"
    fi
done

echo "Override specifiers added."