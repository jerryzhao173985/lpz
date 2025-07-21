# GA Tools - Genetic Algorithm Framework

# Use the unified library creation function
lpzrobots_add_component_library(ga_tools
    SUBDIRS
        .  # Root directory sources
        FitnessStrategies
        GenerationSizeStrategies
        MutationFactorStrategies
        MutationStrategies
        RandomStrategies
        SelectionStrategies
        Values
    PUBLIC_DEPS
        lpzrobots::selforg
        Threads::Threads
)

# Disable certain warnings for ga_tools due to legacy code
# TODO: Fix these issues properly instead of suppressing warnings
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
    target_compile_options(ga_tools PRIVATE
        -Wno-old-style-cast
        -Wno-zero-as-null-pointer-constant
        -Wno-sign-conversion
    )
    if(TARGET ga_tools_opt)
        target_compile_options(ga_tools_opt PRIVATE
            -Wno-old-style-cast
            -Wno-zero-as-null-pointer-constant
            -Wno-sign-conversion
        )
    endif()
    if(TARGET ga_tools_dbg)
        target_compile_options(ga_tools_dbg PRIVATE
            -Wno-old-style-cast
            -Wno-zero-as-null-pointer-constant
            -Wno-sign-conversion
        )
    endif()
endif()

# Create header symlinks for development
lpzrobots_create_header_symlinks(ga_tools)

# Add examples if enabled
if(BUILD_EXAMPLES)
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/examples)
        add_subdirectory(examples)
    endif()
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/simulations)
        add_subdirectory(simulations)
    endif()
endif()