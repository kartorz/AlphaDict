set PATH=%PATH%;C:\Qt\5.9.8\msvc2015\bin

moc.exe  mainwindow.h       -o    moc_mainwindow.cpp


moc.exe  QtMessager.h       -o    moc_QtMessager.cpp


moc.exe  DictIndexModel.h   -o    moc_DictIndexModel.cpp


moc.exe  VBookModel.h       -o    moc_VBookModel.cpp


moc.exe  capworddialog.h       -o    moc_capworddialog.cpp

uic.exe  mainwindow.ui      -o    ui_mainwindow.h

uic.exe  capworddialog.ui      -o    ui_capworddialog.h

rcc.exe  -no-compress -name mainwindow    mainwindow.qrc -o  qrc_mainwindow.cpp

pause