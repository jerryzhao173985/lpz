# Utils sources for ode_robots

# Source files
set(ODE_ROBOTS_UTILS_SOURCES
    camerahandle.cpp
    colorschema.cpp
    console.cpp
    globaldata.cpp
    grabframe.cpp
    gripper.cpp
    imageppm.cpp
    mathutils.cpp
    odehandle.cpp
    operators.cpp
    osghandle.cpp
    simulationtasksupervisor.cpp
    sound.cpp
    tmpobject.cpp
    tmpprimitive.cpp
)

# Add sources to parent scope
lpzrobots_add_sources(${ODE_ROBOTS_UTILS_SOURCES})