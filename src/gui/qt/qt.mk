gui/qt/moc_mainwindow.cpp: gui/qt/mainwindow.h
	$(MOC) $(DEFINES) $(INCLUDES) gui/qt/mainwindow.h -o gui/qt/moc_mainwindow.cpp

gui/qt/moc_QtMessager.cpp: gui/qt/QtMessager.h
	$(MOC) $(DEFINES) $(INCLUDES) gui/qt/QtMessager.h -o gui/qt/moc_QtMessager.cpp

gui/qt/moc_DictIndexModel.cpp: gui/qt/DictIndexModel.h
	$(MOC) $(DEFINES) $(INCLUDES) gui/qt/DictIndexModel.h -o gui/qt/moc_DictIndexModel.cpp

gui/qt/ui_mainwindow.h: gui/qt/mainwindow.ui
	$(UIC) gui/qt/mainwindow.ui -o gui/qt/ui_mainwindow.h

gui/qt/qrc_mainwindow.cpp:  gui/qt/mainwindow.qrc \
                            gui/qt/res/search.png
	$(RCC) --no-compress -name mainwindow  gui/qt/mainwindow.qrc -o  gui/qt/qrc_mainwindow.cpp
