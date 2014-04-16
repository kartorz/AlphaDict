#include "alphadict.h"
#include "Application.h"

#ifdef _QT
#include "gui/qt/mainwindow.h"
#include <QApplication>
#endif

static void on_exit()
{
    g_log(LOG_INFO,"main.cpp on_exit()\n");
    //delete SysMessager::getInstance();
    //delete TaskManager::getInstance();
    printf("on_exit");
}

int main(int argc, char* argv[])
{
	setlocale(LC_ALL, "C.UTF-8");

    Application::getRefrence().initialization();

#ifdef _QT
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    w.registerSysExit(on_exit);
    QObject::connect(&a, SIGNAL(aboutToQuit()), &w, SLOT(onAppExit()));
    return a.exec();
#endif
}
