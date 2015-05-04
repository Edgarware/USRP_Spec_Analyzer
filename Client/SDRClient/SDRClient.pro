#-------------------------------------------------
#
# Project created by QtCreator 2015-04-13T18:48:28
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport network

TARGET = SDRClient
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    qcustomplot.cpp \
    networkThread.cpp

HEADERS  += mainwindow.h \
    qcustomplot.h \
    networkThread.h

FORMS    += mainwindow.ui
