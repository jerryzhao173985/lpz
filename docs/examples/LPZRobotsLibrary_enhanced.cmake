# Enhanced version of lpzrobots_create_header_symlinks with robust error handling
# This shows the improved implementation based on the deep analysis

# Function to create header symlinks in unified include directory
# This function creates both hierarchical and flattened symlinks to support
# both include styles: <component/subdir/header.h> and <component/header.h>
function(lpzrobots_create_header_symlinks component)
    # Use a lock file to prevent race conditions in parallel configuration
    set(LOCK_FILE "${CMAKE_BINARY_DIR}/.header_symlink.lock")
    set(LOCK_TIMEOUT 30)  # 30 second timeout
    
    # Acquire lock with error checking
    file(LOCK ${LOCK_FILE} 
        GUARD FILE  # Release when this file finishes (safer than PROCESS)
        TIMEOUT ${LOCK_TIMEOUT} 
        RESULT_VARIABLE lock_result)
    
    # Check if lock was acquired successfully
    if(NOT lock_result EQUAL 0)
        message(FATAL_ERROR 
            "Failed to acquire header symlink lock after ${LOCK_TIMEOUT} seconds. "
            "Another CMake process may be stuck or the filesystem doesn't support locking. "
            "Lock result: ${lock_result}, Lock file: ${LOCK_FILE}")
    endif()
    
    # Use a macro to ensure lock release even on errors
    macro(safe_release_lock)
        file(LOCK ${LOCK_FILE} RELEASE RESULT_VARIABLE release_result)
        if(NOT release_result EQUAL 0)
            message(WARNING "Failed to release lock: ${release_result}")
        endif()
    endmacro()
    
    # Create component include directory in source tree's unified include dir
    set(INCLUDE_DIR ${CMAKE_SOURCE_DIR}/include/${component})
    file(MAKE_DIRECTORY ${INCLUDE_DIR})
    
    # Collect all header files
    file(GLOB_RECURSE HEADER_FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/*.h
        ${CMAKE_CURRENT_SOURCE_DIR}/*.hpp
    )
    
    # Filter out certain directories if needed
    list(FILTER HEADER_FILES EXCLUDE REGEX ".*/test/.*")
    list(FILTER HEADER_FILES EXCLUDE REGEX ".*/tests/.*")
    list(FILTER HEADER_FILES EXCLUDE REGEX ".*/obj/.*")
    list(FILTER HEADER_FILES EXCLUDE REGEX ".*/build.*/.*")
    list(FILTER HEADER_FILES EXCLUDE REGEX ".*/include/.*")  # Avoid recursion
    
    # Ensure we have headers to process
    list(LENGTH HEADER_FILES num_headers)
    if(num_headers EQUAL 0)
        message(STATUS "No headers found for component ${component}")
        safe_release_lock()
        return()
    endif()
    
    # Track created symlinks to avoid conflicts
    # Note: CACHE INTERNAL is shared across parallel CMake processes
    set(CREATED_FLATTENED_LINKS "" CACHE INTERNAL "List of created flattened symlinks")
    
    # Statistics for reporting
    set(created_hierarchical 0)
    set(created_flattened 0)
    set(skipped_conflicts 0)
    
    # Create symlinks at configure time
    foreach(header ${HEADER_FILES})
        # Get relative path from component root
        file(RELATIVE_PATH rel_path ${CMAKE_CURRENT_SOURCE_DIR} ${header})
        get_filename_component(header_name ${header} NAME)
        get_filename_component(header_dir ${rel_path} DIRECTORY)
        
        # Create hierarchical symlink (preserving subdirectory structure)
        if(NOT "${header_dir}" STREQUAL "")
            file(MAKE_DIRECTORY ${INCLUDE_DIR}/${header_dir})
            set(hierarchical_link ${INCLUDE_DIR}/${rel_path})
            
            # Check if symlink already exists and points to the correct target
            if(EXISTS ${hierarchical_link})
                if(IS_SYMLINK ${hierarchical_link})
                    file(READ_SYMLINK ${hierarchical_link} existing_target)
                    if("${existing_target}" STREQUAL "${header}")
                        # Symlink already correct, skip
                        continue()
                    else()
                        message(WARNING 
                            "Hierarchical symlink ${hierarchical_link} exists but points to "
                            "${existing_target} instead of ${header}. Removing and recreating.")
                        file(REMOVE ${hierarchical_link})
                    endif()
                else()
                    message(WARNING 
                        "${hierarchical_link} exists but is not a symlink. "
                        "Skipping to avoid data loss.")
                    continue()
                endif()
            endif()
            
            # Create the symlink with error handling
            file(CREATE_LINK ${header} ${hierarchical_link} 
                SYMBOLIC 
                RESULT_VARIABLE link_result
                COPY_ON_ERROR)  # Fall back to copy if symlinks not supported
                
            if(link_result EQUAL 0)
                math(EXPR created_hierarchical "${created_hierarchical} + 1")
            else()
                message(WARNING 
                    "Failed to create hierarchical symlink: ${hierarchical_link} -> ${header}. "
                    "Error: ${link_result}")
            endif()
        endif()
        
        # Create flattened symlink (just the filename in component root)
        set(flattened_link ${INCLUDE_DIR}/${header_name})
        
        # Check if another component already created this flattened link
        list(FIND CREATED_FLATTENED_LINKS ${flattened_link} link_index)
        
        if(${link_index} EQUAL -1)  # Not in our tracking list
            if(EXISTS ${flattened_link})
                # Link exists but not in our list - check what it points to
                if(IS_SYMLINK ${flattened_link})
                    file(READ_SYMLINK ${flattened_link} existing_target)
                    get_filename_component(existing_target_abs ${existing_target} ABSOLUTE 
                        BASE_DIR ${INCLUDE_DIR})
                    get_filename_component(header_abs ${header} ABSOLUTE)
                    
                    if("${existing_target_abs}" STREQUAL "${header_abs}")
                        # Already points to our header, just add to tracking
                        list(APPEND CREATED_FLATTENED_LINKS ${flattened_link})
                        math(EXPR created_flattened "${created_flattened} + 1")
                    else()
                        # Conflict - different target
                        message(WARNING 
                            "Flattened symlink conflict for ${header_name}:\n"
                            "  Existing: ${existing_target}\n"
                            "  Skipping: ${header}\n"
                            "  This is expected if multiple components have headers with the same name.")
                        math(EXPR skipped_conflicts "${skipped_conflicts} + 1")
                    endif()
                else()
                    message(WARNING 
                        "${flattened_link} exists but is not a symlink. "
                        "Skipping to avoid data loss.")
                    math(EXPR skipped_conflicts "${skipped_conflicts} + 1")
                endif()
            else()
                # Link doesn't exist - create it
                file(CREATE_LINK ${header} ${flattened_link} 
                    SYMBOLIC 
                    RESULT_VARIABLE link_result
                    COPY_ON_ERROR)
                    
                if(link_result EQUAL 0)
                    list(APPEND CREATED_FLATTENED_LINKS ${flattened_link})
                    math(EXPR created_flattened "${created_flattened} + 1")
                else()
                    message(WARNING 
                        "Failed to create flattened symlink: ${flattened_link} -> ${header}. "
                        "Error: ${link_result}")
                endif()
            endif()
        endif()
        
        # Update the cache with our tracking list
        set(CREATED_FLATTENED_LINKS ${CREATED_FLATTENED_LINKS} 
            CACHE INTERNAL "List of created flattened symlinks" FORCE)
    endforeach()
    
    # Report statistics
    message(STATUS 
        "Header symlinks for ${component}: "
        "${created_hierarchical} hierarchical, "
        "${created_flattened} flattened, "
        "${skipped_conflicts} conflicts skipped")
    
    # Release the lock
    safe_release_lock()
    
    # Add the unified include directory to this target's interface
    if(TARGET ${component})
        target_include_directories(${component} PUBLIC
            $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
        )
    endif()
endfunction()

# Additional utility function to clean stale symlinks
function(lpzrobots_clean_stale_symlinks)
    set(LOCK_FILE "${CMAKE_BINARY_DIR}/.header_symlink.lock")
    
    # Acquire lock
    file(LOCK ${LOCK_FILE} GUARD FUNCTION TIMEOUT 30 RESULT_VARIABLE lock_result)
    if(NOT lock_result EQUAL 0)
        message(WARNING "Could not acquire lock to clean stale symlinks")
        return()
    endif()
    
    set(INCLUDE_DIR ${CMAKE_SOURCE_DIR}/include)
    if(NOT EXISTS ${INCLUDE_DIR})
        return()
    endif()
    
    # Find all symlinks
    file(GLOB_RECURSE all_symlinks LIST_DIRECTORIES false ${INCLUDE_DIR}/*)
    
    set(removed_count 0)
    foreach(link ${all_symlinks})
        if(IS_SYMLINK ${link})
            file(READ_SYMLINK ${link} target)
            if(NOT IS_ABSOLUTE ${target})
                get_filename_component(target ${target} ABSOLUTE BASE_DIR ${INCLUDE_DIR})
            endif()
            
            # Remove if target doesn't exist
            if(NOT EXISTS ${target})
                file(REMOVE ${link})
                math(EXPR removed_count "${removed_count} + 1")
            endif()
        endif()
    endforeach()
    
    if(removed_count GREATER 0)
        message(STATUS "Removed ${removed_count} stale symlinks")
    endif()
endfunction()

# Function to verify symlink integrity (useful for CI)
function(lpzrobots_verify_symlinks)
    set(INCLUDE_DIR ${CMAKE_SOURCE_DIR}/include)
    if(NOT EXISTS ${INCLUDE_DIR})
        message(STATUS "No include directory to verify")
        return()
    endif()
    
    file(GLOB_RECURSE all_symlinks LIST_DIRECTORIES false ${INCLUDE_DIR}/*)
    
    set(broken_links "")
    set(broken_count 0)
    
    foreach(link ${all_symlinks})
        if(IS_SYMLINK ${link})
            file(READ_SYMLINK ${link} target)
            if(NOT IS_ABSOLUTE ${target})
                get_filename_component(target ${target} ABSOLUTE BASE_DIR ${INCLUDE_DIR})
            endif()
            
            if(NOT EXISTS ${target})
                list(APPEND broken_links ${link})
                math(EXPR broken_count "${broken_count} + 1")
            endif()
        endif()
    endforeach()
    
    if(broken_count GREATER 0)
        string(REPLACE ";" "\n  " broken_list "${broken_links}")
        message(SEND_ERROR 
            "Found ${broken_count} broken symlinks:\n  ${broken_list}")
    else()
        message(STATUS "All symlinks verified successfully")
    endif()
endfunction()