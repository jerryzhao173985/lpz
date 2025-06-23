#!/bin/bash
# Set Qt5 environment for LPZRobots build

export PATH="/opt/homebrew/opt/qt@5/bin:$PATH"
export QT_SELECT=qt5
export LDFLAGS="-L/opt/homebrew/opt/qt@5/lib"
export CPPFLAGS="-I/opt/homebrew/opt/qt@5/include"
export PKG_CONFIG_PATH="/opt/homebrew/opt/qt@5/lib/pkgconfig"

echo "Qt5 environment configured:"
echo "PATH includes: /opt/homebrew/opt/qt@5/bin"
which qmake
qmake --version