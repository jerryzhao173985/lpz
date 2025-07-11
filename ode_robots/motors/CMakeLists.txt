# Motors sources for ode_robots

# Source files
set(ODE_ROBOTS_MOTORS_SOURCES
    angularmotor.cpp
    oneaxisservo.cpp
    pid.cpp
    twoaxisservo.cpp
)

# Add sources to parent scope
lpzrobots_add_sources(${ODE_ROBOTS_MOTORS_SOURCES})