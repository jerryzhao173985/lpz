# Robots sources for ode_robots

# Source files
set(ODE_ROBOTS_ROBOTS_SOURCES
    amos4legs.cpp
    amosII.cpp
    arm.cpp
    arm2segm.cpp
    ashigaru.cpp
    barrel2masses.cpp
    caterpillar.cpp
    defaultCaterPillar.cpp
    discus.cpp
    forcedsphere.cpp
    formel1.cpp
    fourwheeled.cpp
    hand.cpp
    hexabot.cpp
    hexapod.cpp
    hurlingsnake.cpp
    kuka.cpp
    muscledarm.cpp
    nejihebi.cpp
    nimm2.cpp
    nimm4.cpp
    oderobot.cpp
    plattfussschlange.cpp
    replayrobot.cpp
    robotchain.cpp
    robotfactory.cpp
    schlange.cpp
    schlangeforce.cpp
    schlangeservo.cpp
    schlangeservo2.cpp
    schlangevelocity.cpp
    shortcircuit.cpp
    skeleton.cpp
    sliderwheelie.cpp
    sphererobot.cpp
    sphererobot3masses.cpp
    truckmesh.cpp
    twowheeled.cpp
    uwo.cpp
    vierbeiner.cpp
)

# Add sources to parent scope
lpzrobots_add_sources(${ODE_ROBOTS_ROBOTS_SOURCES})