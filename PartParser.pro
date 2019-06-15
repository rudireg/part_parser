#-------------------------------------------------
#
# Project created by QtCreator 2016-07-05T17:42:10
#
#-------------------------------------------------

QT       += core gui network
QMAKE_RPATHDIR += lib
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = PartParser
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    master.cpp \
    partparser.cpp \
    worker.cpp \
    rstring.cpp \
    rhttp.cpp

HEADERS  += mainwindow.h \
    master.h \
    partparser.h \
    worker.h \
    rstring.h \
    rhttp.h

FORMS += mainwindow.ui

win64 {
    LIBS += -llibeay32 -llibssl32 -lssleay32
}
