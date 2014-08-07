#include <Windows.h>

#include "alphadict.h"
#include "Util.h"
#include "Log.h"
#include "Application.h"


#if CONFIG_QT5
#include <QtWidgets/QApplication>
#include <QtCore/QTranslator>
#include <QtWidgets/QStyleFactory>
#include <QtWidgets/QMessageBox>
#include "gui/qt/mainwindow.h"
#endif

static void on_exit()
{
}

#ifdef WIN32
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
#else
int main(int argc, char* argv[])
#endif
{
#ifdef _LINUX
    setlocale(LC_ALL, "C.UTF-8");
#endif
    int ret = 0;
    Util::getTimeMS(); // start to timing.
    ret = g_application.initialization();

#if CONFIG_QT5
#ifdef WIN32
    int argc = 0;
    char** argv = 0;
#endif
    QApplication a(argc, argv);
    QTranslator translator;
    if (g_application.m_configure->m_uilanID == UILAN_CN) {
        string trfile = g_application.m_configure->m_dataDir + "/uitr_cn";
        //translator.load(QString::fromUtf8(trfile.c_str()));
        translator.load(QString::fromLocal8Bit(trfile.c_str()));
        a.installTranslator(&translator);
    }
    a.setStyle(QStyleFactory::create("Fusion"));
    //a.setStyle(QStyleFactory::create("windowsvista"));
    if (ret != 0) {
        QMessageBox msgBox;
        QString errmsg = QString("\n\
system init failure, error code:(%1)    \n\n\
please refer README.txt\n").arg(ret);

	msgBox.setText(errmsg);
	msgBox.exec();
        return ret;
    }

    MainWindow w;
    w.raise();
    w.activateWindow();
    w.show();
    w.registerSysExit(on_exit);
    w.initDelay();
    QObject::connect(&a, SIGNAL(aboutToQuit()), &w, SLOT(onAppExit()));
    return a.exec();
#endif
}
