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


SOURCES += tst_BushyTest.cpp
DEFINES += SRCDIR=\\\"$$PWD/\\\"

DESTDIR = $$OUT_PWD/..

CONFIG(debug, debug|release) {

win32-msvc
{
    QMAKE_CXXFLAGS += -D_USE_MATH_DEFINES /arch:AVX /W3 -DDEBUG
}

} else {

win32-msvc
{
    QMAKE_CXXFLAGS += -FAs -D_USE_MATH_DEFINES /O2 /arch:AVX /W3
}

} #config

win32-msvc
{
    QMAKE_CXXFLAGS += -FAs
}

HEADERS += \
    MapTestAlgorithms.h
