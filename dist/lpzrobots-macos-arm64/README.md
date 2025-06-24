# LPZRobots macOS ARM64 Distribution

This is a binary distribution of LPZRobots compiled for macOS ARM64 (Apple Silicon).

## Contents

- `bin/guilogger` - GUI logger tool for visualization
- `bin/matrixviz` - Matrix visualization tool
- `lib/libselforg.a` - Self-organizing controller library
- `lib/libconfigurator.a` - Configuration GUI library

## Requirements

- macOS 14.0 or later (ARM64)
- Qt 6.9.0 (installed via Homebrew)
- OpenSceneGraph (installed via Homebrew)
- GNU Scientific Library (GSL)

## Installation

1. Install dependencies:
   ```bash
   brew install qt@6 open-scene-graph gsl readline
   ```

2. Set Qt path:
   ```bash
   export PATH="/opt/homebrew/opt/qt@6/bin:$PATH"
   ```

3. Copy binaries to your preferred location or add the bin directory to your PATH.

## Build Information

- Compiler: Apple clang (Xcode beta)
- C++ Standard: C++17
- Architecture: arm64
- Optimization: -O2 with ARM64 SIMD support

## Known Issues

- OpenGL deprecation warnings on macOS (framework still functional)
- Shadow rendering may need to be disabled with `-noshadow` flag

## Version

Built from LPZRobots repository commit f07ed69 (2025-06-23)