# CMake Race Conditions and File Locking: A Deep Dive

## Executive Summary

This document explores a critical but rarely discussed issue in CMake build systems: **race conditions during parallel configuration**. While most developers are familiar with parallel build issues (`make -j`), fewer realize that CMake itself can run configuration steps in parallel, leading to subtle and hard-to-debug race conditions.

## Table of Contents
1. [The Problem: Hidden Race Conditions](#the-problem-hidden-race-conditions)
2. [Understanding CMake Parallelism](#understanding-cmake-parallelism)
3. [Case Study: LPZRobots Header Symlinks](#case-study-lpzrobots-header-symlinks)
4. [The Solution: File Locking](#the-solution-file-locking)
5. [Implementation Details](#implementation-details)
6. [Best Practices](#best-practices)
7. [Common Pitfalls](#common-pitfalls)
8. [Testing Strategies](#testing-strategies)

## The Problem: Hidden Race Conditions

### What Are CMake Configuration Race Conditions?

When CMake processes a project with multiple subdirectories, it can configure them in parallel. This creates potential race conditions when multiple `CMakeLists.txt` files attempt to:
- Create the same directories
- Write to the same files
- Modify shared cache variables
- Create symbolic links in shared locations

### Why This Matters

Unlike build-time race conditions (which manifest as compilation errors), configuration-time races can cause:
- **Non-deterministic build configurations** - Different results on each run
- **Intermittent failures** - Works on developer machines, fails in CI
- **Silent errors** - Wrong symlinks/files created without warnings
- **Heisenbugs** - Disappear when you try to debug them (serial execution)

### Real-World Example

```cmake
# Component A and Component B both do this:
file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/include/common)
execute_process(
    COMMAND ${CMAKE_COMMAND} -E create_symlink 
    ${CMAKE_CURRENT_SOURCE_DIR}/header.h 
    ${CMAKE_SOURCE_DIR}/include/common/header.h
)
```

**Race Scenario**:
1. Component A checks if directory exists (NO)
2. Component B checks if directory exists (NO)
3. Component A creates directory
4. Component B tries to create directory (may fail or succeed)
5. Both try to create the same symlink (undefined behavior)

## Understanding CMake Parallelism

### When Does CMake Run in Parallel?

1. **Explicit parallel configuration**: `cmake --parallel <N>`
2. **IDE integration**: Many IDEs configure projects with parallelism
3. **CI systems**: Often use parallel configuration for speed
4. **CMake presets**: Can specify parallel configuration

### The Subtlety of the Problem

```cmake
# This looks safe but isn't:
if(NOT EXISTS ${shared_file})
    file(WRITE ${shared_file} "content")
endif()
```

**TOCTOU Race** (Time-Of-Check-Time-Of-Use):
- Thread 1: Checks file doesn't exist ✓
- Thread 2: Checks file doesn't exist ✓
- Thread 1: Creates file
- Thread 2: Overwrites file (data loss!)

## Case Study: LPZRobots Header Symlinks

### The Challenge

LPZRobots needs to create a unified include directory where headers from multiple components are accessible via symlinks:

```
include/
├── selforg/
│   ├── controller.h -> ../../selforg/controller/controller.h
│   └── matrix.h -> ../../selforg/matrix/matrix.h
├── ode_robots/
│   └── robot.h -> ../../ode_robots/robots/robot.h
└── ga_tools/
    └── ga.h -> ../../ga_tools/ga.h
```

### The Race Conditions

1. **Directory Creation Race**
```cmake
file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/include/${component})
# Two components might try to create 'include/' simultaneously
```

2. **Symlink Creation Race**
```cmake
if(NOT EXISTS ${symlink})
    execute_process(COMMAND ln -s ${source} ${symlink})
endif()
# Classic TOCTOU - both threads pass the check, both create
```

3. **Flattened Link Conflicts**
```cmake
# Component A: utils.h -> componentA/utils.h
# Component B: utils.h -> componentB/utils.h
# Who wins? Last one, non-deterministically!
```

## The Solution: File Locking

### CMake's file(LOCK) Command

CMake provides `file(LOCK)` specifically for synchronization:

```cmake
file(LOCK <path> 
    [DIRECTORY] 
    [RELEASE] 
    [GUARD <FUNCTION|FILE|PROCESS>] 
    [RESULT_VARIABLE <variable>] 
    [TIMEOUT <seconds>])
```

### Key Features

1. **Automatic Lock File Creation**: Creates lock file if it doesn't exist
2. **Process-Based Locking**: Default GUARD PROCESS holds lock until CMake exits
3. **Timeout Support**: Prevents infinite waits
4. **Cross-Platform**: Works on Windows, Linux, macOS

## Implementation Details

### The Complete Solution

```cmake
function(lpzrobots_create_header_symlinks component)
    # Lock file in build directory (not source!)
    set(LOCK_FILE "${CMAKE_BINARY_DIR}/.header_symlink.lock")
    set(LOCK_TIMEOUT 30)
    
    # Acquire lock with error checking
    file(LOCK ${LOCK_FILE} 
        TIMEOUT ${LOCK_TIMEOUT} 
        RESULT_VARIABLE lock_result)
    
    if(NOT lock_result EQUAL 0)
        message(FATAL_ERROR 
            "Failed to acquire header symlink lock after ${LOCK_TIMEOUT}s. "
            "Another CMake process may be stuck. "
            "Lock file: ${LOCK_FILE}")
    endif()
    
    # Critical section starts here
    set(INCLUDE_DIR ${CMAKE_SOURCE_DIR}/include/${component})
    file(MAKE_DIRECTORY ${INCLUDE_DIR})
    
    # Track created symlinks globally
    set(CREATED_FLATTENED_LINKS "" 
        CACHE INTERNAL "List of created flattened symlinks")
    
    file(GLOB_RECURSE HEADER_FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/*.h
        ${CMAKE_CURRENT_SOURCE_DIR}/*.hpp)
    
    foreach(header ${HEADER_FILES})
        file(RELATIVE_PATH rel_path 
            ${CMAKE_CURRENT_SOURCE_DIR} ${header})
        get_filename_component(header_name ${header} NAME)
        
        # Create hierarchical symlink
        set(hierarchical_link ${INCLUDE_DIR}/${rel_path})
        if(NOT EXISTS ${hierarchical_link})
            get_filename_component(header_dir ${rel_path} DIRECTORY)
            if(NOT "${header_dir}" STREQUAL "")
                file(MAKE_DIRECTORY ${INCLUDE_DIR}/${header_dir})
            endif()
            
            # Atomic symlink creation
            file(CREATE_LINK ${header} ${hierarchical_link} 
                SYMBOLIC 
                RESULT_VARIABLE link_result)
                
            if(NOT link_result EQUAL 0)
                message(WARNING 
                    "Failed to create symlink: "
                    "${hierarchical_link} -> ${header}")
            endif()
        endif()
        
        # Handle flattened symlinks with conflict detection
        set(flattened_link ${INCLUDE_DIR}/${header_name})
        list(FIND CREATED_FLATTENED_LINKS 
            ${flattened_link} link_index)
            
        if(${link_index} EQUAL -1 AND NOT EXISTS ${flattened_link})
            file(CREATE_LINK ${header} ${flattened_link} 
                SYMBOLIC 
                RESULT_VARIABLE link_result)
                
            if(link_result EQUAL 0)
                list(APPEND CREATED_FLATTENED_LINKS ${flattened_link})
                set(CREATED_FLATTENED_LINKS 
                    ${CREATED_FLATTENED_LINKS} 
                    CACHE INTERNAL 
                    "List of created flattened symlinks" FORCE)
            endif()
        elseif(EXISTS ${flattened_link})
            # Detect conflicts
            file(READ_SYMLINK ${flattened_link} existing_target)
            if(NOT "${existing_target}" STREQUAL "${header}")
                message(WARNING 
                    "Symlink conflict for ${header_name}: "
                    "existing: ${existing_target}, "
                    "skipping: ${header}")
            endif()
        endif()
    endforeach()
    
    # Critical section ends - release lock
    file(LOCK ${LOCK_FILE} RELEASE)
endfunction()
```

### Why This Works

1. **Serialization**: Only one process can hold the lock at a time
2. **Atomic Operations**: `file(CREATE_LINK)` is atomic on most filesystems
3. **Shared State**: CACHE INTERNAL variables are visible across processes
4. **Conflict Detection**: Warns about naming conflicts instead of silent overwrites

## Best Practices

### 1. Always Lock Shared Resources

```cmake
# Bad: Direct modification of shared directory
file(WRITE ${CMAKE_SOURCE_DIR}/generated/config.h "...")

# Good: Lock before modification
file(LOCK ${CMAKE_BINARY_DIR}/.config.lock TIMEOUT 10)
file(WRITE ${CMAKE_SOURCE_DIR}/generated/config.h "...")
file(LOCK ${CMAKE_BINARY_DIR}/.config.lock RELEASE)
```

### 2. Use Build Directory for Lock Files

```cmake
# Bad: Lock file in source directory
set(LOCK_FILE ${CMAKE_SOURCE_DIR}/.lock)

# Good: Lock file in build directory
set(LOCK_FILE ${CMAKE_BINARY_DIR}/.lock)
```

### 3. Always Check Lock Acquisition

```cmake
file(LOCK ${LOCK_FILE} TIMEOUT 30 RESULT_VARIABLE lock_result)
if(NOT lock_result EQUAL 0)
    message(FATAL_ERROR "Failed to acquire lock: ${lock_result}")
endif()
```

### 4. Consider GUARD Scope

```cmake
# GUARD PROCESS (default): Lock held until CMake exits
file(LOCK ${LOCK_FILE} GUARD PROCESS)

# GUARD FILE: Lock released when current file finishes
file(LOCK ${LOCK_FILE} GUARD FILE)

# GUARD FUNCTION: Lock released when function returns
file(LOCK ${LOCK_FILE} GUARD FUNCTION)
```

### 5. Use Appropriate Timeouts

```cmake
# Short operation: 10 seconds
file(LOCK ${LOCK_FILE} TIMEOUT 10)

# Long operation: 60 seconds
file(LOCK ${LOCK_FILE} TIMEOUT 60)

# Critical operation: No timeout (wait forever)
file(LOCK ${LOCK_FILE})
```

## Common Pitfalls

### 1. Forgetting to Release Locks

```cmake
# Bad: No release on error
file(LOCK ${LOCK_FILE})
if(error_condition)
    message(FATAL_ERROR "Error!") # Lock not released!
endif()
file(LOCK ${LOCK_FILE} RELEASE)

# Good: Always release
file(LOCK ${LOCK_FILE})
if(error_condition)
    file(LOCK ${LOCK_FILE} RELEASE)
    message(FATAL_ERROR "Error!")
endif()
file(LOCK ${LOCK_FILE} RELEASE)
```

### 2. Locking Too Much

```cmake
# Bad: Lock entire function
function(process_files)
    file(LOCK ${LOCK_FILE})
    # ... 100 lines of code ...
    file(LOCK ${LOCK_FILE} RELEASE)
endfunction()

# Good: Lock only critical section
function(process_files)
    # ... preparation ...
    file(LOCK ${LOCK_FILE})
    # ... modify shared resource ...
    file(LOCK ${LOCK_FILE} RELEASE)
    # ... rest of processing ...
endfunction()
```

### 3. Deadlocks

```cmake
# Component A:
file(LOCK ${LOCK_A})
file(LOCK ${LOCK_B})  # Waits if B holds this

# Component B:
file(LOCK ${LOCK_B})
file(LOCK ${LOCK_A})  # Deadlock if A holds this
```

## Testing Strategies

### 1. Stress Test Parallel Configuration

```bash
#!/bin/bash
# Test script to expose race conditions
for i in {1..10}; do
    rm -rf build-test-$i
    cmake -B build-test-$i --parallel 16 &
done
wait
# Check if all builds are identical
```

### 2. Add Artificial Delays

```cmake
# Temporarily add delays to expose races
function(lpzrobots_create_header_symlinks component)
    # Add random delay to increase race likelihood
    execute_process(
        COMMAND ${CMAKE_COMMAND} -E sleep 0.${RANDOM}
    )
    # ... rest of function ...
endfunction()
```

### 3. Use Sanitizers

```bash
# Run CMake under ThreadSanitizer (if built with it)
TSAN_OPTIONS=halt_on_error=1 cmake -B build
```

### 4. Verify Determinism

```bash
# Build twice and compare
cmake -B build1 --parallel 16
cmake -B build2 --parallel 16
diff -r build1 build2
```

## Conclusion

File locking in CMake is essential for robust, deterministic builds in modern development environments. While the overhead is minimal (microseconds per lock), the protection against race conditions is invaluable. As projects grow and CI systems become more parallel, proper synchronization becomes not just good practice, but necessary for reliable builds.

### Key Takeaways

1. **CMake configuration can run in parallel** - Don't assume serial execution
2. **Race conditions are real** - They cause intermittent, hard-to-debug failures  
3. **file(LOCK) is the solution** - Purpose-built for CMake synchronization
4. **Lock narrowly** - Only protect actual shared resource access
5. **Test parallel configuration** - Don't just test parallel builds

### Remember

> "In concurrent systems, if something can go wrong due to timing, it will go wrong, 
> but only in production, on Fridays, after 5 PM." - Murphy's Law of Concurrency