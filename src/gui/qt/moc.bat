moc.exe  mainwindow.h       -o    moc_mainwindow.cpp


moc.exe  QtMessager.h       -o    moc_QtMessager.cpp


moc.exe  DictIndexModel.h   -o    moc_DictIndexModel.cpp


moc.exe  VBookModel.h       -o    moc_VBookModel.cpp


uic.exe  mainwindow.ui      -o    ui_mainwindow.h



rcc.exe  -no-compress -name mainwindow    mainwindow.qrc -o  qrc_mainwindow.cpp
pause