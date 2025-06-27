
TEMPLATE = app
TARGET = start
DEPENDPATH += . ../.. ../../../selforg/include ../../messagedispatcher ../../qconfigurable
INCLUDEPATH += /$(HOME)/include ../.. ../../../selforg/include  ../../messagedispatcher ../../qconfigurable
LIBS += -L$(HOME)/lib  -lusb -lftdi -lselforg -L../../../selforg
CONFIG += debug 
QMAKE_CXXFLAGS += -Wno-deprecated -Wno-unused-parameter

OBJECTS_DIR = obj
MOC_DIR = moc


#Qt used libraries/functionalities
QT = core gui widgets xml

# Input - Header

# Input - Sources
SOURCES 	+= main.cpp sos.cpp
HEADERS   += sos.h

#include all ecbrobots core files
include(../../ecbrobots.pro.inc)

#include all ecbrobots core resources
RESOURCES   += ecbrobots.qrc

# macOS specific settings
macx {
    # Remove deprecated AGL framework completely
    QMAKE_LFLAGS -= -framework AGL
    QMAKE_LIBS_OPENGL -= -framework AGL
    LIBS -= -framework AGL
    
    # Override any automatic OpenGL detection
    QMAKE_LIBS_OPENGL = -framework OpenGL
    
    # Modern macOS uses Metal/OpenGL via NSOpenGLContext
    LIBS += -framework OpenGL -framework Cocoa
}