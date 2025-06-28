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
    # Disable Qt's automatic OpenGL configuration to prevent AGL inclusion
    CONFIG -= opengl
    
    # Remove deprecated AGL framework completely
    # AGL was deprecated in macOS 10.9 and removed in later versions
    QMAKE_LFLAGS -= -framework AGL
    QMAKE_LIBS_OPENGL = -framework OpenGL
    LIBS -= -framework AGL
    
    # Note: OpenGL headers are now provided by the SDK, not the framework
    # Don't manually add frameworks - let Qt handle it properly
}