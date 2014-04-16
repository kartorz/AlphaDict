
ifeq ($(GUI), QT)

SOURCES_QT = gui/qt/mainwindow.cpp \
             gui/qt/moc_mainwindow.cpp \
             gui/qt/DictIndexModel.cpp \
             gui/qt/moc_DictIndexModel.cpp \
             gui/qt/QtMessager.cpp \
             gui/qt/moc_QtMessager.cpp

endif
