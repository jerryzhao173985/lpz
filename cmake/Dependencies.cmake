# Dependency Management for LPZRobots
# This file manages external dependencies using CMake FetchContent
# This ensures reproducible builds across different systems

include(FetchContent)

# Set download and build options
set(FETCHCONTENT_QUIET OFF)
set(FETCHCONTENT_TRY_FIND_PACKAGE_MODE ALWAYS)

# GSL (GNU Scientific Library) - Required for mathematical functions
# Using GSL-lite as it's header-only and easier to integrate
FetchContent_Declare(
  gsl-lite
  GIT_REPOSITORY https://github.com/gsl-lite/gsl-lite.git
  GIT_TAG        v0.41.0
  GIT_SHALLOW    TRUE
)

# Google Test - For unit testing (only if tests are enabled)
if(LPZROBOTS_BUILD_TESTS)
  FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.14.0
    GIT_SHALLOW    TRUE
  )
  set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
  set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
endif()

# Google Benchmark - For performance testing (optional)
if(LPZROBOTS_BUILD_BENCHMARKS)
  FetchContent_Declare(
    benchmark
    GIT_REPOSITORY https://github.com/google/benchmark.git
    GIT_TAG        v1.8.3
    GIT_SHALLOW    TRUE
  )
  set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
endif()

# fmt - Modern C++ formatting library (useful for logging)
FetchContent_Declare(
  fmt
  GIT_REPOSITORY https://github.com/fmtlib/fmt.git
  GIT_TAG        10.2.1
  GIT_SHALLOW    TRUE
)

# spdlog - Fast C++ logging library (optional but recommended)
option(LPZROBOTS_USE_SPDLOG "Use spdlog for logging" OFF)
if(LPZROBOTS_USE_SPDLOG)
  FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG        v1.13.0
    GIT_SHALLOW    TRUE
  )
  set(SPDLOG_FMT_EXTERNAL ON CACHE BOOL "" FORCE)
endif()

# Make dependencies available
FetchContent_MakeAvailable(gsl-lite fmt)

if(LPZROBOTS_BUILD_TESTS)
  FetchContent_MakeAvailable(googletest)
endif()

if(LPZROBOTS_BUILD_BENCHMARKS)
  FetchContent_MakeAvailable(benchmark)
endif()

if(LPZROBOTS_USE_SPDLOG)
  FetchContent_MakeAvailable(spdlog)
endif()

# System dependencies that still need manual installation
# These cannot be easily managed with FetchContent due to complexity
message(STATUS "")
message(STATUS "System dependencies required (install manually):")
message(STATUS "  - Qt6 (Core, Widgets, OpenGL)")
message(STATUS "  - OpenSceneGraph (3D graphics)")
message(STATUS "  - OpenGL/GLU")
message(STATUS "  - readline (console interaction)")
message(STATUS "")
message(STATUS "On macOS: brew install qt@6 open-scene-graph readline")
message(STATUS "On Ubuntu: apt install qt6-base-dev libopenscenegraph-dev libreadline-dev")
message(STATUS "")