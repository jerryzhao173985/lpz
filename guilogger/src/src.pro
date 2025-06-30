# Diese Datei wurde mit dem qmake-Manager von KDevelop erstellt. 
# ------------------------------------------- 
# Unterverzeichnis relativ zum Projektverzeichnis: ./src
# Das Target ist eine Anwendung:  ../bin/guilogger

HEADERS += \ 
           guilogger.h \
           gnuplot.h \
           filelogger.h \
           qdatasource.h \
           qserialreader.h \
           qpipereader.h \
           inifile.h \
           commlineparser.h \
           stl_adds.h \
           plotchannelstablemodel.h \
           plotinfo.h \
           channeldata.h \
    quickmp.h

SOURCES += \
           guilogger.cpp \
           main.cpp \
           gnuplot.cpp \
           filelogger.cpp \
           qserialreader.cpp \
           qpipereader.cpp \
           inifile.cpp \
           stl_adds.cpp \
           plotchannelstablemodel.cpp \
           plotinfo.cpp \
           channeldata.cpp
           

TEMPLATE = app
CONFIG += debug \
warn_on \
thread \
qt \
console \
c++17 \
sdk_no_version_check

# Suppress warnings from system headers and Qt
QMAKE_CXXFLAGS += -Wno-float-equal -Wno-old-style-cast
TARGET = bin/guilogger

target.path = /usr/bin
QT += core gui widgets xml
# Platform specific settings
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
    
    # Note: Post-link processing is handled by the configure script
    
    # Suppress warnings from system frameworks and Qt headers
    QMAKE_CXXFLAGS += -Wno-deprecated-declarations
    # Add system include flags for external libraries
    QMAKE_CXXFLAGS += -isystem /opt/homebrew/include
    QMAKE_CXXFLAGS += -isystem /opt/homebrew/lib
}
INSTALLS += target
