#include "alphadict.h"
#include "Util.h"
#include "Application.h"

#if CONFIG_QT5
#include "gui/qt/mainwindow.h"
#include <QApplication>
#endif

static void on_exit()
{
    g_log(LOG_INFO,"main.cpp on_exit()\n");
    printf("on_exit");
}

int main(int argc, char* argv[])
{
	setlocale(LC_ALL, "C.UTF-8");
    Util::getTimeMS(); // start to timing.
    g_application.initialization();

#if CONFIG_QT5
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    w.registerSysExit(on_exit);
    QObject::connect(&a, SIGNAL(aboutToQuit()), &w, SLOT(onAppExit()));
    return a.exec();
#endif
}
