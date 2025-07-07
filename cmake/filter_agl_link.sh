#!/bin/bash
# Filter out AGL framework from link command
# AGL (Apple Graphics Library) no longer exists on modern macOS

# Get all arguments
ARGS=("$@")

# Filter out AGL framework
FILTERED_ARGS=()
SKIP_NEXT=false

for arg in "${ARGS[@]}"; do
    if [ "$SKIP_NEXT" = true ]; then
        SKIP_NEXT=false
        if [ "$arg" != "AGL" ]; then
            # If it's not AGL, we need to keep both the -framework and this arg
            FILTERED_ARGS+=("-framework")
            FILTERED_ARGS+=("$arg")
        fi
    elif [ "$arg" = "-framework" ]; then
        # Don't add -framework yet, check the next argument
        SKIP_NEXT=true
    else
        FILTERED_ARGS+=("$arg")
    fi
done

# Execute the filtered command
exec "${FILTERED_ARGS[@]}"