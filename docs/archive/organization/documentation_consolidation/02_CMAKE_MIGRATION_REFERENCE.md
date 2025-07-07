# CMake Migration Reference

## Overview

This document provides a comprehensive reference for the CMake migration of LPZRobots, including architectural decisions, implementation details, and migration status.

## Migration Status Summary

### ✅ Completed (95%)
- **Core Libraries**: selforg, ode_robots, ga_tools - fully migrated
- **GUI Tools**: guilogger (Qt6), matrixviz (Qt6), configurator - fully migrated
- **Config Scripts**: All *-config scripts generated via CMake templates
- **Build Variants**: Debug and optimized (_opt) library versions
- **Platform Support**: macOS ARM64, Linux x86_64
- **Modern Features**: CMake presets, export sets, target properties

### 🔄 Using Legacy System (5%)
- **OpenDE**: Still uses autotools (built automatically)
- **Java Components**: Optional soundman and javacontroller
- **Individual Simulations**: Can use either CMake or Make

## Architecture Overview

### CMake Module System

```
cmake/
├── LPZRobotsCore.cmake          # Main configuration
├── LPZRobotsCompiler.cmake      # Compiler settings
├── LPZRobotsDependencies.cmake  # External dependencies
├── LPZRobotsLibrary.cmake       # Library creation functions
├── LPZRobotsPlatform.cmake      # Platform detection
├── LPZRobotsTargets.cmake       # Helper functions
├── SimulationBuild.cmake        # Simulation support
├── LPZRobotsTesting.cmake       # Test infrastructure
└── QtMacOSConfig.cmake          # Qt/macOS fixes
```

### Key Design Decisions

1. **Modular Architecture**
   - Each aspect in separate CMake module
   - Clean separation of concerns
   - Easy to maintain and extend

2. **Target-Based Design**
   - Modern CMake with target properties
   - Automatic dependency propagation
   - IDE-friendly structure

3. **Backward Compatibility**
   - Generate identical config scripts
   - Support same installation layout
   - Preserve all functionality

## Migration Details

### From M4 to CMake Templates

#### Old M4 System
```m4
dnl selforg-config.m4
SELFORG_INCLUDEDIR=@prefix@/include/selforg
SELFORG_LIBDIR=@prefix@/lib
SELFORG_LIBS="-L${SELFORG_LIBDIR} -lselforg@DEBUG_SUFFIX@"
```

#### New CMake Template
```bash
# selforg-config.sh.in
SELFORG_INCLUDEDIR=@CMAKE_INSTALL_PREFIX@/include/selforg
SELFORG_LIBDIR=@CMAKE_INSTALL_PREFIX@/lib
SELFORG_LIBS="-L${SELFORG_LIBDIR} -lselforg@LPZROBOTS_DEBUG_SUFFIX@"
```

### Library Creation Pattern

#### Before (Makefile)
```makefile
libselforg.a: $(OBJFILES)
	$(AR) $(ARFLAGS) $@ $^
libselforg_opt.a: $(OBJFILES_OPT)
	$(AR) $(ARFLAGS) $@ $^
```

#### After (CMake)
```cmake
lpzrobots_add_library(selforg
    TYPE STATIC
    SOURCES ${SELFORG_SOURCES}
    PUBLIC_HEADERS ${SELFORG_HEADERS}
    INCLUDE_DIRS include
)
```

### Simulation Build Support

#### Traditional Makefile
```makefile
include $(shell ode_robots-config --srcprefix)/Makefile.conf
SOURCES = main.cpp
TARGET = start
include $(shell ode_robots-config --srcprefix)/Makefile.4sim
```

#### Modern CMake
```cmake
lpzrobots_add_simulation(my_simulation
    SOURCES main.cpp
    LIBRARIES lpzrobots::ode_robots lpzrobots::selforg
    OSG_COMPONENTS osg osgDB osgUtil osgGA osgViewer
)
```

## Key Implementation Features

### 1. Unified Library Function

```cmake
# Creates both debug and optimized versions
function(lpzrobots_add_library name)
    # Parse arguments
    cmake_parse_arguments(ARG ...)
    
    # Create main library
    add_library(${name} ${ARG_TYPE} ${ARG_SOURCES})
    
    # Create optimized variant if static
    if(ARG_TYPE STREQUAL "STATIC")
        add_library(${name}_opt STATIC ${ARG_SOURCES})
        target_compile_options(${name}_opt PRIVATE -O3)
    endif()
    
    # Set up installation and properties
    # ...
endfunction()
```

### 2. Config Script Generation

```cmake
# Generate from template
configure_file(
    ${CMAKE_SOURCE_DIR}/cmake/selforg-config.sh.in
    ${CMAKE_BINARY_DIR}/selforg-config
    @ONLY
)

# Make executable
file(CHMOD ${CMAKE_BINARY_DIR}/selforg-config
    PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                GROUP_READ GROUP_EXECUTE
                WORLD_READ WORLD_EXECUTE
)
```

### 3. Platform-Specific Handling

```cmake
# macOS ARM64 detection
if(APPLE AND CMAKE_SYSTEM_PROCESSOR MATCHES "arm64")
    set(LPZROBOTS_MACOS_ARM64 TRUE)
    # Enable NEON optimizations
    add_compile_options(-march=armv8-a+simd)
endif()

# macOS shared library flags
if(APPLE)
    set(CMAKE_SHARED_LIBRARY_CREATE_CXX_FLAGS
        "${CMAKE_SHARED_LIBRARY_CREATE_CXX_FLAGS} -undefined dynamic_lookup")
endif()
```

### 4. Dependency Management

```cmake
# Find Qt with fallback
find_package(Qt6 COMPONENTS Core Widgets OpenGL QUIET)
if(NOT Qt6_FOUND)
    find_package(Qt5 5.12 COMPONENTS Core Widgets OpenGL REQUIRED)
    set(QT_VERSION_MAJOR 5)
endif()

# Handle OpenSceneGraph
find_package(OpenSceneGraph REQUIRED COMPONENTS
    osg osgDB osgUtil osgGA osgViewer osgShadow osgText
)
```

## CMake Presets

### Configuration Presets

```json
{
    "name": "default",
    "displayName": "Default Config",
    "description": "Default build configuration",
    "generator": "Unix Makefiles",
    "binaryDir": "${sourceDir}/build/${presetName}",
    "cacheVariables": {
        "CMAKE_BUILD_TYPE": "Release",
        "BUILD_SHARED_LIBS": "ON",
        "BUILD_EXAMPLES": "ON",
        "BUILD_GUI_TOOLS": "ON"
    }
}
```

### Build Presets

```json
{
    "name": "default",
    "configurePreset": "default",
    "jobs": 8,
    "targets": ["all"]
}
```

## Common Migration Patterns

### 1. Include Directories
```cmake
# Old: -I flags in Makefile
# New: Target include directories
target_include_directories(${target} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include/selforg>
)
```

### 2. Compiler Flags
```cmake
# Old: CFLAGS in Makefile.conf
# New: Target compile options
target_compile_options(${target} PRIVATE
    $<$<CONFIG:Debug>:-g -O0>
    $<$<CONFIG:Release>:-O3>
)
```

### 3. Link Dependencies
```cmake
# Old: LIBS in Makefile
# New: Target link libraries
target_link_libraries(${target} PUBLIC
    lpzrobots::selforg
    ${GSL_LIBRARIES}
    $<$<PLATFORM_ID:Darwin>:-framework OpenGL>
)
```

## Testing Infrastructure

### Unit Tests
```cmake
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
    
    # Auto-discover tests
    lpzrobots_discover_tests(
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        EXTRA_ARGS --no-intro
    )
endif()
```

### Integration Tests
```cmake
lpzrobots_add_test(
    NAME integration_basic
    COMMAND $<TARGET_FILE:test_runner>
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    LABELS "integration"
)
```

## Installation Layout

### Directory Structure
```
${CMAKE_INSTALL_PREFIX}/
├── bin/
│   ├── guilogger
│   ├── ode_robots-config
│   └── selforg-config
├── lib/
│   ├── libselforg.a / .so
│   ├── libselforg_opt.a
│   └── cmake/lpzrobots/
├── include/
│   ├── selforg/
│   └── ode_robots/
└── share/
    ├── lpzrobots/
    └── man/
```

## Troubleshooting Migration Issues

### Common Problems

1. **Duplicate add_subdirectory**
   - Solution: Check if directory already processed
   ```cmake
   if(NOT TARGET selforg)
       add_subdirectory(selforg)
   endif()
   ```

2. **Missing source files**
   - Solution: Use GLOB carefully or list explicitly
   ```cmake
   file(GLOB_RECURSE SOURCES 
       ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
   )
   ```

3. **Config script paths**
   - Solution: Support both installed and build-tree usage
   ```cmake
   if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/../../../CMakeCache.txt")
       # In build tree
   else()
       # Installed
   endif()
   ```

## Future Improvements

### Short Term (3 months)
- Complete Qt6 migration for configurator
- Add CPack packaging rules
- Create FindLPZRobots.cmake module

### Medium Term (6 months)
- Migrate OpenDE to CMake
- Add Conan/vcpkg package recipes
- Remove M4 build system

### Long Term (1 year)
- Full C++20 migration
- GPU acceleration support
- Cloud build infrastructure

## Best Practices

### For Maintainers
1. Always update both CMake and Make systems during transition
2. Test on multiple platforms before committing
3. Keep CMake modules focused and modular
4. Document any platform-specific workarounds

### For Users
1. Use CMake presets for consistency
2. Report any differences from Make behavior
3. Use `lpz-build` script for simple builds
4. Check CMakeCache.txt for configuration issues

## Summary

The CMake migration provides a modern, maintainable build system while preserving full backward compatibility. The modular architecture makes it easy to extend and maintain, while CMake presets provide a consistent user experience across platforms.