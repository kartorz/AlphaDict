
gui/qt/moc_mainwindow.cpp: gui/qt/mainwindow.h
	$(MOC) $(DEFINES) $(INCLUDES) gui/qt/mainwindow.h       -o gui/qt/moc_mainwindow.cpp

gui/qt/moc_capworddialog.cpp: gui/qt/capworddialog.h
	$(MOC) $(DEFINES) $(INCLUDES) gui/qt/capworddialog.h    -o gui/qt/moc_capworddialog.cpp

gui/qt/moc_QtMessager.cpp: gui/qt/QtMessager.h
	$(MOC) $(DEFINES) $(INCLUDES) gui/qt/QtMessager.h       -o gui/qt/moc_QtMessager.cpp

gui/qt/moc_DictIndexModel.cpp: gui/qt/DictIndexModel.h
	$(MOC) $(DEFINES) $(INCLUDES) gui/qt/DictIndexModel.h   -o gui/qt/moc_DictIndexModel.cpp

gui/qt/moc_DictListModel.cpp: gui/qt/DictListModel.h
	$(MOC) $(DEFINES) $(INCLUDES) gui/qt/DictListModel.h    -o gui/qt/moc_DictListModel.cpp

gui/qt/moc_VBookModel.cpp: gui/qt/VBookModel.h
	$(MOC) $(DEFINES) $(INCLUDES) gui/qt/VBookModel.h       -o gui/qt/moc_VBookModel.cpp

gui/qt/moc_QmlBridge.cpp: gui/qt/QmlBridge.h
	$(MOC) $(DEFINES) $(INCLUDES) gui/qt/QmlBridge.h        -o gui/qt/moc_QmlBridge.cpp

gui/qt/moc_QmlMainWindow.cpp: gui/qt/QmlMainWindow.h
	$(MOC) $(DEFINES) $(INCLUDES) gui/qt/QmlMainWindow.h    -o gui/qt/moc_QmlMainWindow.cpp

gui/qt/ui_mainwindow.h: gui/qt/mainwindow.ui
	$(UIC) gui/qt/mainwindow.ui -o gui/qt/ui_mainwindow.h

gui/qt/ui_capworddialog.h: gui/qt/capworddialog.ui
	$(UIC) gui/qt/capworddialog.ui -o gui/qt/ui_capworddialog.h

gui/qt/qrc_mainwindow.cpp:  gui/qt/mainwindow.qrc \
                            gui/qt/res/search.png
	$(RCC) --no-compress -name mainwindow  gui/qt/mainwindow.qrc -o  gui/qt/qrc_mainwindow.cpp

gui/qml/qrc_qml.cpp:  gui/qml/qml.qrc  $(wildcard gui/qml/*.qml)
	$(RCC) --no-compress -name qml  gui/qml/qml.qrc -o  gui/qml/qrc_qml.cpp

uitr:
	$(QTTOOL_DIR)/lupdate -no-recursive -locations none -verbose \
		main.cpp *.cpp gui/qt/*.cpp gui/qt/*.ui gui/qml/*.qml -ts gui/qt/uitr_cn.ts
	$(QTTOOL_DIR)/lrelease gui/qt/uitr_cn.ts  -qm  gui/qt/uitr_cn.qm

uitr-edit: uitr
	$(QTTOOL_DIR)/linguist gui/qt/uitr_cn.ts
