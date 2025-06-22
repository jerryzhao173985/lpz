SUBDIRS += src
TEMPLATE = subdirs 
QT += core gui widgets
CONFIG += warn_on \
          qt \
          thread \
          console \
          c++11 

#CONFIG += debug

CONFIG -= app_bundle
