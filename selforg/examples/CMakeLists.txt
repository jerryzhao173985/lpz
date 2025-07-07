# Selforg Examples

if(BUILD_EXAMPLES)
    # Matrix benchmark
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/matrix_benchmark.cpp)
        add_executable(matrix_benchmark matrix_benchmark.cpp)
        target_link_libraries(matrix_benchmark PRIVATE lpzrobots::selforg)
    endif()

    # Controller test
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/test_controller.cpp)
        add_executable(test_controller test_controller.cpp)
        target_link_libraries(test_controller PRIVATE lpzrobots::selforg)
    endif()

    # Other example programs
    file(GLOB EXAMPLE_SOURCES *.cpp)
    list(REMOVE_ITEM EXAMPLE_SOURCES 
        ${CMAKE_CURRENT_SOURCE_DIR}/matrix_benchmark.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/test_controller.cpp
    )
    
    # Remove test_modern_patterns if not using C++20
    if(CMAKE_CXX_STANDARD LESS 20)
        list(REMOVE_ITEM EXAMPLE_SOURCES 
            ${CMAKE_CURRENT_SOURCE_DIR}/test_modern_patterns.cpp
        )
    endif()

    foreach(example_src ${EXAMPLE_SOURCES})
        get_filename_component(example_name ${example_src} NAME_WE)
        add_executable(${example_name} ${example_src})
        target_link_libraries(${example_name} PRIVATE lpzrobots::selforg)
        # Add readline if available (needed for some examples)
        if(READLINE_LIBRARY)
            target_link_libraries(${example_name} PRIVATE ${READLINE_LIBRARY})
        endif()
    endforeach()
endif()