#-------------------------------------------------
#
# Project created by QtCreator 2012-05-13T11:14:27
#
#-------------------------------------------------

QT       += core gui network xml sql

TARGET = syncronizator
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    webdav.cpp \
    sqlite.cpp

HEADERS  += mainwindow.h \
    webdav.h \
    sqlite.h

FORMS    += mainwindow.ui
