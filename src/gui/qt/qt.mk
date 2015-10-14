
gui/qt/moc_mainwindow.cpp: gui/qt/mainwindow.h
	$(MOC) $(DEFINES) $(INCLUDES) gui/qt/mainwindow.h       -o gui/qt/moc_mainwindow.cpp

gui/qt/moc_capworddialog.cpp: gui/qt/capworddialog.h
	$(MOC) $(DEFINES) $(INCLUDES) gui/qt/capworddialog.h    -o gui/qt/moc_capworddialog.cpp

gui/qt/moc_QtMessager.cpp: gui/qt/QtMessager.h
	$(MOC) $(DEFINES) $(INCLUDES) gui/qt/QtMessager.h       -o gui/qt/moc_QtMessager.cpp

gui/qt/moc_DictIndexModel.cpp: gui/qt/DictIndexModel.h
	$(MOC) $(DEFINES) $(INCLUDES) gui/qt/DictIndexModel.h   -o gui/qt/moc_DictIndexModel.cpp

gui/qt/moc_VBookModel.cpp: gui/qt/VBookModel.h
	$(MOC) $(DEFINES) $(INCLUDES) gui/qt/VBookModel.h       -o gui/qt/moc_VBookModel.cpp

gui/qt/ui_mainwindow.h: gui/qt/mainwindow.ui
	$(UIC) gui/qt/mainwindow.ui -o gui/qt/ui_mainwindow.h

gui/qt/ui_capworddialog.h: gui/qt/capworddialog.ui
	$(UIC) gui/qt/capworddialog.ui -o gui/qt/ui_capworddialog.h

gui/qt/qrc_mainwindow.cpp:  gui/qt/mainwindow.qrc \
                            gui/qt/res/search.png
	$(RCC) --no-compress -name mainwindow  gui/qt/mainwindow.qrc -o  gui/qt/qrc_mainwindow.cpp

uitr:
	lupdate -no-recursive  -verbose gui/qt -ts gui/qt/uitr_cn.ts
	linguist gui/qt/uitr_cn.ts
	lrelease gui/qt/uitr_cn.ts  -qm  gui/qt/uitr_cn.qm

