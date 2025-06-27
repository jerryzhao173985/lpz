#!/bin/bash
# Platform-specific shared library build script

if [ "$(uname)" = "Darwin" ]; then
    echo "Building shared library for macOS..."
    exec g++ -dynamiclib -o "$@"
else
    echo "Building shared library for Linux..."
    exec g++ -shared -Wl,-soname,$(basename "$1") -o "$@"
fi