##=== config modules ===##

TOPDIR := $(abspath .)
GUI = QT
CC := g++
INCLUDES := -I$(TOPDIR) -I$(TOPDIR)/dict  -I$(TOPDIR)/lib
INCLUDES += -I$(TOPDIR)/gui/qt \
		    -I/usr/share/qt5/mkspecs/linux-g++-64 \
            -I/usr/include/qt5 \
            -I/usr/include/qt5/QtWidgets \
            -I/usr/include/qt5/QtGui \
            -I/usr/include/qt5/QtCore

DEFINES  := -D_LINUX -D_QT
CPPFLAGS := -g -MMD -O0 -fPIC
LDFLAGS  := 

LIBS = -lpthread
LIBS += -L/usr/X11R6/lib64 -lQt5Widgets\
        -L/usr/lib/x86_64-linux-gnu -lQt5Gui\
        -lQt5Core

MODULE.yes := ALDICT

ifeq ($(GUI), QT)
MODULE.yes += QT
MOC = /usr/lib/x86_64-linux-gnu/qt5/bin/moc
UIC = /usr/lib/x86_64-linux-gnu/qt5/bin/uic
endif
