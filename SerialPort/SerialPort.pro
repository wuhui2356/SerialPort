#-------------------------------------------------
#
# Project created by QtCreator 2016-09-16T11:47:28
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = SerialPort
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    serialport.cpp \
    thread.cpp

HEADERS  += mainwindow.h \
    serialport.h \
    thread.h

FORMS    += mainwindow.ui

RESOURCES += \
    res.qrc
