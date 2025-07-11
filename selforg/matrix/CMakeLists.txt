# Matrix benchmarking tool
cmake_minimum_required(VERSION 3.16)

# Check if we're being built as part of the main project
if(NOT TARGET selforg)
    project(matrix_benchmark)
    
    # Find the matrix library
    find_path(MATRIX_INCLUDE_DIR matrix.h
        PATHS ${CMAKE_CURRENT_SOURCE_DIR}
        NO_DEFAULT_PATH
    )
    
    if(NOT MATRIX_INCLUDE_DIR)
        message(FATAL_ERROR "Could not find matrix.h")
    endif()
    
    include_directories(${MATRIX_INCLUDE_DIR})
    
    # Add the source files
    add_library(matrix_lib STATIC
        matrix.cpp
        cpu_features.cpp
        matrixutils.cpp
    )
    
    # Set C++ standard
    set_target_properties(matrix_lib PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
    )
    
    # Add compile definitions for SIMD
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "arm64|aarch64")
        target_compile_definitions(matrix_lib PRIVATE USE_ARM_NEON)
    elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|AMD64")
        target_compile_definitions(matrix_lib PRIVATE USE_AVX2)
    endif()
    
    # Platform-specific flags
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(matrix_lib PRIVATE -march=native -O3)
    endif()
    
    # Create benchmark executable
    add_executable(matrix_benchmark matrix_benchmark.cpp)
    target_link_libraries(matrix_benchmark matrix_lib)
    
    # Set properties for benchmark
    set_target_properties(matrix_benchmark PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
    )
    
else()
    # Being built as part of main project
    if(BUILD_TESTS OR BUILD_BENCHMARKS)
        add_executable(matrix_benchmark matrix_benchmark.cpp)
        target_link_libraries(matrix_benchmark selforg)
        
        # Add to the selforg target directory
        set_target_properties(matrix_benchmark PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        )
    endif()
endif()