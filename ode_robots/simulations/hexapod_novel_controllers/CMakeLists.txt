# CMakeLists.txt for hexapod_novel_controllers simulation

# Find lpzrobots simulation build support
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../../../cmake/SimulationBuild.cmake")
    # In-tree build
    include(${CMAKE_CURRENT_SOURCE_DIR}/../../../cmake/SimulationBuild.cmake)
else()
    # Standalone build
    find_package(lpzrobots REQUIRED)
endif()

# Add the simulation executables
lpzrobots_add_simulation(hexapod_novel_main
    SOURCES main.cpp
)

lpzrobots_add_simulation(hexapod_novel_behaviors
    SOURCES hexapod_behaviors.cpp
)

lpzrobots_add_simulation(hexapod_multi_demo
    SOURCES main_multi.cpp
)

# Set output names
set_target_properties(hexapod_novel_main PROPERTIES
    OUTPUT_NAME start
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

set_target_properties(hexapod_novel_behaviors PROPERTIES
    OUTPUT_NAME hexapod_behaviors
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

set_target_properties(hexapod_multi_demo PROPERTIES
    OUTPUT_NAME multi_demo
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

# Suppress warnings from headers we don't control
foreach(target hexapod_novel_main hexapod_novel_behaviors hexapod_multi_demo)
    if(TARGET ${target})
        target_compile_options(${target} PRIVATE
            $<$<CXX_COMPILER_ID:Clang>:-Wno-overloaded-virtual>
            $<$<CXX_COMPILER_ID:Clang>:-Wno-inconsistent-missing-override>
            $<$<CXX_COMPILER_ID:Clang>:-Wno-non-c-typedef-for-linkage>
            $<$<CXX_COMPILER_ID:Clang>:-Wno-sign-compare>
        )
        
        # Ensure proper linking with OpenSceneGraph
        find_package(OpenSceneGraph REQUIRED COMPONENTS osg osgDB osgUtil osgGA osgViewer osgShadow osgText)
        if(OpenSceneGraph_FOUND)
            target_link_libraries(${target} PRIVATE ${OPENSCENEGRAPH_LIBRARIES})
        endif()
    endif()
endforeach()