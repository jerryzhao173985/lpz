# Obstacles sources for ode_robots

# Source files
set(ODE_ROBOTS_OBSTACLES_SOURCES
    abstractground.cpp
    abstractobstacle.cpp
    complexplayground.cpp
    DummyGround.cpp
    DummyObstacle.cpp
    meshground.cpp
    randomobstacles.cpp
    terrainground.cpp
    tracksections/degreesegment.cpp
    tracksections/straightline.cpp
)

# Add sources to parent scope
lpzrobots_add_sources(${ODE_ROBOTS_OBSTACLES_SOURCES})