# CMakeLists.txt for sphererobot_novel_controllers simulation

# Find lpzrobots simulation build support
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../../../cmake/SimulationBuild.cmake")
    # In-tree build
    include(${CMAKE_CURRENT_SOURCE_DIR}/../../../cmake/SimulationBuild.cmake)
else()
    # Standalone build
    find_package(lpzrobots REQUIRED)
endif()

# Add the simulation executable
lpzrobots_add_simulation(start
    SOURCES main.cpp
)

# Suppress warnings from headers we don't control
if(TARGET start)
    target_compile_options(start PRIVATE
        $<$<CXX_COMPILER_ID:Clang>:-Wno-overloaded-virtual>
        $<$<CXX_COMPILER_ID:Clang>:-Wno-inconsistent-missing-override>
        $<$<CXX_COMPILER_ID:Clang>:-Wno-non-c-typedef-for-linkage>
        $<$<CXX_COMPILER_ID:Clang>:-Wno-sign-compare>
    )
endif()