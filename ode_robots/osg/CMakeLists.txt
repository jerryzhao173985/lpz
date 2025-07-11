# OSG-related sources for ode_robots

# Source files
set(ODE_ROBOTS_OSG_SOURCES
    base.cpp
    cameramanipulator.cpp
    cameramanipulatorFollow.cpp
    cameramanipulatorRace.cpp
    cameramanipulatorTV.cpp
    color.cpp
    colorschema.cpp
    dummyprimitive.cpp
    graphline.cpp
    hudstatistics.cpp
    jointprimitive.cpp
    lpzviewer.cpp
    lpzviz.cpp
    osgbox.cpp
    osgboxtex.cpp
    osgcapsule.cpp
    osgcylinder.cpp
    osgdummygeode.cpp
    osghandle.cpp
    osgheightfield.cpp
    osgmesh.cpp
    osgplane.cpp
    osgprimitive.cpp
    osgscene.cpp
    osgsphere.cpp
    osgtext.cpp
    primitive.cpp
    robotcameramanager.cpp
    soundsensor.cpp
    substance.cpp
)

# Add sources to parent scope
lpzrobots_add_sources(${ODE_ROBOTS_OSG_SOURCES})