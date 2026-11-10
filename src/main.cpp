#include "alphadict.h"
#include "Util.h"
#include "Log.h"
#include "Application.h"

#if CONFIG_QT5
#include <QtCore/QTranslator>
#include <QtCore/QLocale>
#include <QtCore/QStringList>
#include <QtCore/QCoreApplication>
#include <QtCore/QDir>
#include <QtNetwork/QLocalSocket>
#include <QtGui/QScreen>
#include <QtGui/QGuiApplication>
#include <QtWidgets/QApplication>
#include <QtWidgets/QStyleFactory>
#include <QtWidgets/QMessageBox>
#if CONFIG_QML
#include "gui/qt/QmlMainWindow.h"
#else
#include "gui/qt/mainwindow.h"
#endif
#endif

static void on_exit()
{
    //printf("on_exit\n");
    g_application.stop();
    Application::delPidFile();
}

#if CONFIG_QT5
inline void showInitErrDialog(int ret)
{
        QMessageBox msgBox;
        QString errmsg = QString("\n\
system init failure, error code:(%1)    \n\n\
read README-win32.txt for more information \n").arg(ret);

        msgBox.setText(errmsg);
        msgBox.exec();
}

/*! Ask a running copy to show itself; true if one answered.  No dialog. */
inline bool wakeupExistingInstance()
{
    if (!Application::existProcess())
        return false;

    QLocalSocket socket;
    socket.connectToServer(
        QString::fromStdString(Application::instanceServerName()));
    if (socket.waitForConnected(1500)) {
        /* The receiver only needs the connection; the line keeps intent in logs. */
        socket.write("show\n");
        socket.flush();
        socket.waitForBytesWritten(1500);
        socket.disconnectFromServer();
        g_sysLog.i("{Application} another instance is running, asked it to show its window\n");
        return true;
    }

    g_sysLog.w("{Application} stale pid file, starting a new instance\n");
    return false;
}
#endif /* CONFIG_QT5 */

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

#if CONFIG_QT5
#ifdef WIN32
    int argc = 0;
    char** argv = 0;
#endif
    QTranslator translator;

    /* Must be set before QApplication is constructed. */
#if QT_VERSION >= QT_VERSION_CHECK(5, 6, 0)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling, true);
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps, true);
#endif
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    /* Keep 125% / 150% instead of rounding to 100% or 200%. */
    QGuiApplication::setHighDpiScaleFactorRoundingPolicy(
        Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
#endif

    QApplication a(argc, argv);

    /* Single instance, before loading any dictionary: returns in ms. */
    if (wakeupExistingInstance())
        return 0;
#endif

    ret = g_application.start();

#if CONFIG_QT5
#if CONFIG_QML
    /* The QML front-end applies the font itself (QmlBridge::applyUiFont). */
#else
    int fontsize = g_application.m_configure->m_setting.fontsize;

    /* fontsize is logical pixels, QFont wants points: convert on 96 dpi. */
    QFont font = a.font();
    if (g_application.m_configure->m_setting.font == "") {
    #ifdef _WINDOWS
        /* Keep the historical Windows default when no family is set. */
        font.setFamily(QString("Courier"));
    #endif
    } else {
        string family = g_application.m_configure->m_setting.font;
        font.setFamily(QString::fromUtf8(family.c_str()));
    }
    font.setPointSizeF(fontsize * 3.0 / 4.0);
    a.setFont(font);
#endif

    if (g_application.m_configure->m_setting.uilanID == UILAN_NONE) {
        if (QLocale::system().language() == QLocale::Chinese)
            g_application.m_configure->writeUILanID(UILAN_CN);
        else
            g_application.m_configure->writeUILanID(UILAN_EN);
    }

    if (g_application.m_configure->m_setting.uilanID == UILAN_CN) {
        // The catalogue is installed next to the data files, but while the
        // program runs straight from the source tree it only exists in
        // gui/qt/.  Try every plausible location instead of a single one,
        // the build tree first: a stale installed .qm must not shadow a
        // fresh one next to the binary.
        QStringList trFiles;
        trFiles << QCoreApplication::applicationDirPath() + "/gui/qt/uitr_cn";
        trFiles << QDir::currentPath() + "/gui/qt/uitr_cn";
        trFiles << QString::fromLocal8Bit(
            (g_application.m_configure->m_dataDir + "/uitr_cn").c_str());
        for (int i = 0; i < trFiles.size(); i++) {
            if (translator.load(trFiles.at(i)))
                break;
        }
        a.installTranslator(&translator);
    }

#ifdef WIN32
    a.setStyle(QStyleFactory::create("windowsvista"));
#else
    a.setStyle(QStyleFactory::create("Fusion"));
#endif

    if (ret != 0 /*application initialization*/) {
        showInitErrDialog(ret);
        g_application.stop();
        return ret;
    }

	/* A copy that failed to initialize never writes this. */
    Application::writePidFile();

#if CONFIG_QML
    QmlMainWindow w;
#else
    MainWindow w;
#endif
    /* Center on the available area of the primary screen (QDesktopWidget is
     * deprecated since Qt 5.11). */
    const QRect avail = QGuiApplication::primaryScreen()->availableGeometry();
    w.move(avail.x() + (avail.width() - w.width()) / 2,
           avail.y() + (avail.height() - w.height()) / 2);
    //w.raise();
    //w.activateWindow();
    w.show();
    w.registerSysExit(on_exit);
    w.initDelay();
    QObject::connect(&a, SIGNAL(aboutToQuit()), &w, SLOT(onAppExit()));
    return a.exec();
#endif

    return ret;
}
