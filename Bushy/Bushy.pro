#-------------------------------------------------
#
# Project created by QtCreator 2016-04-03T13:10:53
#
#-------------------------------------------------

QT       -= core gui

TARGET = Bushy
TEMPLATE = lib
CONFIG += staticlib

SOURCES += \
    splay_map_instantiation_test.cpp

HEADERS += \
    include/splay_map.h
unix {
    target.path = /usr/lib
    INSTALLS += target
}
