#-------------------------------------------------
#
# Project created by QtCreator 2014-03-31T07:24:05
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = qt
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    ../capworddialog.cpp

HEADERS  += mainwindow.h \
    ../capworddialog.h

FORMS    += ../mainwindow.ui \
    ../capworddialog.ui

RESOURCES += \
    ../mainwindow.qrc
