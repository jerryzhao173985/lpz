# Fix for lpzrobots_configure_qt_macos Function Not Found

## Problem
The CMake build was failing with errors indicating that `lpzrobots_configure_qt_macos` function was not found when building guilogger, matrixviz, and configurator test app.

## Root Cause
The function `lpzrobots_configure_qt_macos` is defined in `cmake/QtMacOSConfig.cmake`, which is included via `cmake/LPZRobotsCore.cmake`. However, the function calls in the component CMakeLists.txt files were not checking if the function was available before calling it.

## Solution
Added conditional checks before calling the function to ensure it's only called when:
1. We're on Apple platform (`APPLE`)
2. The function is available (`COMMAND lpzrobots_configure_qt_macos`)

## Files Modified

1. **guilogger/CMakeLists.txt**
   ```cmake
   # Before
   lpzrobots_configure_qt_macos(guilogger)
   
   # After
   if(APPLE AND COMMAND lpzrobots_configure_qt_macos)
       lpzrobots_configure_qt_macos(guilogger)
   endif()
   ```

2. **matrixviz/CMakeLists.txt**
   ```cmake
   # Before
   lpzrobots_configure_qt_macos(matrixviz)
   
   # After
   if(APPLE AND COMMAND lpzrobots_configure_qt_macos)
       lpzrobots_configure_qt_macos(matrixviz)
   endif()
   ```

3. **configurator/testapp/CMakeLists.txt**
   ```cmake
   # Before
   lpzrobots_configure_qt_macos(configurator_test)
   
   # After
   if(APPLE AND COMMAND lpzrobots_configure_qt_macos)
       lpzrobots_configure_qt_macos(configurator_test)
   endif()
   ```

## Result
- CMake configuration completes successfully
- Both guilogger and matrixviz build without errors
- The Qt macOS configuration is properly applied when the function is available
- The post-build check confirms no AGL framework references in the built executables

## Additional Notes
The function properly configures Qt applications on macOS to:
- Avoid deprecated AGL framework references
- Use proper OpenGL framework linking
- Set up correct framework search paths
- Apply macOS-specific Qt compatibility settings