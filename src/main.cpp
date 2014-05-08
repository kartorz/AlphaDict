#include "alphadict.h"
#include "Util.h"
#include "Application.h"

#ifdef _QT
#include "gui/qt/mainwindow.h"
#include <QApplication>
#endif

#include <boost/filesystem.hpp>

static void on_exit()
{
    g_log(LOG_INFO,"main.cpp on_exit()\n");
    printf("on_exit");
}

int main(int argc, char* argv[])
{
	setlocale(LC_ALL, "C.UTF-8");
    Util::getTimeMS(); // start to timing.
    boost::filesystem::path execPath(argv[0]);
    g_application.m_configure->m_execDir = execPath.remove_filename().string();

    g_application.initialization();

#ifdef _QT
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    w.registerSysExit(on_exit);
    QObject::connect(&a, SIGNAL(aboutToQuit()), &w, SLOT(onAppExit()));
    return a.exec();
#endif
}
