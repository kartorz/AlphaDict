SRCS_GUI_yes = gui/qt/mainwindow.cpp            \
                  gui/qt/moc_mainwindow.cpp        \
                  gui/qt/qrc_mainwindow.cpp        \
                  gui/qt/DictIndexModel.cpp        \
                  gui/qt/moc_DictIndexModel.cpp    \
                  gui/qt/DictListModel.cpp         \
                  gui/qt/moc_DictListModel.cpp     \
                  gui/qt/VBookModel.cpp            \
                  gui/qt/moc_VBookModel.cpp        \
                  gui/qt/QtMessager.cpp            \
                  gui/qt/moc_QtMessager.cpp        \
                  gui/qt/capworddialog.cpp         \
                  gui/qt/moc_capworddialog.cpp

# The QML front-end needs the QtQuickWidgets module, so it is only built when
# configure found that module (QML=yes).
SRCS_QML_yes = gui/qt/QmlBridge.cpp             \
                  gui/qt/moc_QmlBridge.cpp         \
                  gui/qt/QmlMainWindow.cpp         \
                  gui/qt/moc_QmlMainWindow.cpp     \
                  gui/qml/qrc_qml.cpp

QT_H_yes =   gui/qt/ui_mainwindow.h  \
                gui/qt/ui_capworddialog.h
