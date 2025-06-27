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
TARGET = bin/guilogger

target.path = /usr/bin
QT += core gui widgets xml
# Platform specific settings
macx {
    # macOS specific: Remove deprecated AGL framework
    # AGL was deprecated in macOS 10.9 and removed in later versions
    QMAKE_LFLAGS -= -framework AGL
    QMAKE_LIBS_OPENGL -= -framework AGL
    LIBS -= -framework AGL
}
INSTALLS += target
