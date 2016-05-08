#-------------------------------------------------
#
# Project created by QtCreator 2016-04-03T13:12:56
#
#-------------------------------------------------

QT       += testlib
QT       -= gui

TARGET = BushyTest
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += \
    tst_splay_map_test.cpp
DEFINES += SRCDIR=\\\"$$PWD/\\\"

DESTDIR = $$OUT_PWD/..

win32-g++: message(win32-g++)
win32-msvc2015: message(win32-msvc2015)

win32-msvc2015 {

CONFIG(debug, debug|release) {
    QMAKE_CXXFLAGS += -D_USE_MATH_DEFINES /arch:AVX /W3 -DDEBUG -DMSVC_COMPILER
} else {
    QMAKE_CXXFLAGS += -FAs -D_USE_MATH_DEFINES /O2 /arch:AVX /W3 -DMSVC_COMPILER
} #config

} #win32-msvc2015

win32-msvc2015 {
    QMAKE_CXXFLAGS += -FAs
}

HEADERS += \
    MapTestAlgorithms.h
