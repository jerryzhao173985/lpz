#!/bin/bash
# LPZRobots Installation Script

INSTALL_PREFIX="${1:-/usr/local}"

echo "Installing LPZRobots to ${INSTALL_PREFIX}..."

# Check permissions
if [ ! -w "${INSTALL_PREFIX}" ]; then
    echo "Error: No write permission to ${INSTALL_PREFIX}"
    echo "Try: sudo $0 ${INSTALL_PREFIX}"
    exit 1
fi

# Install files
cp -r lib/* "${INSTALL_PREFIX}/lib/"
cp -r bin/* "${INSTALL_PREFIX}/bin/"
cp -r include/* "${INSTALL_PREFIX}/include/"
mkdir -p "${INSTALL_PREFIX}/share/lpzrobots"
cp -r share/lpzrobots/* "${INSTALL_PREFIX}/share/lpzrobots/"

echo "Installation complete!"
echo "Add ${INSTALL_PREFIX}/bin to your PATH if not already included."
