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
    # CRITICAL: Prevent AGL framework inclusion
    # AGL is deprecated and causes build failures on modern macOS
    
    # Force Qt to not add OpenGL at all (we don't need it)
    CONFIG -= opengl
    CONFIG -= opengl_desktop
    
    # Override Qt's OpenGL library settings completely
    QMAKE_LIBS_OPENGL = -framework OpenGL
    QMAKE_LIBS_OPENGL_QT =
    QMAKE_LIBS_OPENGL_ES2 =
    
    # Explicitly remove AGL from all possible locations
    QMAKE_LFLAGS -= -framework AGL
    LIBS -= -framework AGL
    QMAKE_LIBS -= -framework AGL
    
    # Remove any AGL include paths
    INCLUDEPATH -= /System/Library/Frameworks/AGL.framework/Headers
    QMAKE_INCDIR_OPENGL -= /System/Library/Frameworks/AGL.framework/Headers
    QMAKE_INCDIR -= /System/Library/Frameworks/AGL.framework/Headers
}