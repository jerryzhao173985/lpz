# Sensors sources for ode_robots

# Source files
set(ODE_ROBOTS_SENSORS_SOURCES
    axisorientationsensor.cpp
    camera.cpp
    camerasensor.cpp
    contactsensor.cpp
    derivativesensor.cpp
    irsensor.cpp
    opticalflow.cpp
    rangeFinder.cpp
    raysensor.cpp
    raysensorbank.cpp
    relativepositionsensor.cpp
    soundsensor.cpp
    speedsensor.cpp
    torquesensor.cpp
)

# Add sources to parent scope
lpzrobots_add_sources(${ODE_ROBOTS_SENSORS_SOURCES})