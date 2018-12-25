#-------------------------------------------------
#
# Project created by QtCreator 2014-03-31T07:24:05
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = qt
TEMPLATE = app


SOURCES +=\
    ../capworddialog.cpp \
    ../DictIndexModel.cpp \
    ../mainwindow.cpp \
    ../VBookModel.cpp \
    ../QtMessager.cpp

HEADERS  += \
    ../capworddialog.h \
    ../DictIndexModel.h \
    ../mainwindow.h \
    ../QtMessager.h \
    ../VBookModel.h \
    ../ui_mainwindow.h

FORMS    += ../mainwindow.ui \
    ../capworddialog.ui

RESOURCES += \
    ../mainwindow.qrc
