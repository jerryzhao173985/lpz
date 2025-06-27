SUBDIRS += src
TEMPLATE = subdirs 
QT += core gui widgets xml
CONFIG += warn_on \
          qt \
          thread \
          console \
          c++11 

#CONFIG += debug

CONFIG -= app_bundle

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