/*****************************************************************************
 *  AlphaDict
 *  Copyright (C) 2004-2026  AlphaDict project
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *****************************************************************************/

#include <QtCore/QEvent>
#include <QtCore/QPoint>
#include <QtCore/QUrl>
#include <QtGui/QClipboard>
#include <QtGui/QIcon>
#include <QtGui/QCursor>
#include <QtGui/QKeyEvent>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QMenu>
#include <QtWidgets/QToolTip>
#include <QtWidgets/QVBoxLayout>
#include <QtQuickWidgets/QQuickWidget>
#include <QtQml/QQmlContext>
#include <QtQml/QQmlError>
#include <QtCore/QDebug>

#include "QmlMainWindow.h"
#include "QmlBridge.h"
#include "QtMessager.h"
#include "DictIndexModel.h"
#include "capworddialog.h"
#include "Configure.h"
#include "MessageQueue.h"
#include "Util.h"
#include "Log.h"

#ifdef _LINUX
#include "X11Util.h"
/* Xlib ships these as macros; they would rewrite the QEvent enum values and
 * any other identifier of the same name used below. */
#ifdef KeyPress
#undef KeyPress
#endif
#ifdef KeyRelease
#undef KeyRelease
#endif
#ifdef None
#undef None
#endif
#endif

QmlMainWindow::QmlMainWindow(QWidget *parent) :
    QWidget(parent),
    m_bridge(NULL),
    m_quick(NULL),
    m_messager(NULL),
    m_capWordDialog(NULL),
    m_systray(NULL),
    m_config(NULL),
    m_trayCwsEnableAct(NULL),
    m_trayCwsClipboardAct(NULL),
    m_trayCwsSelectionAct(NULL),
    m_cwdEnableTemp(true),
    m_nonRepeatSelEn(false),
    onSysExit(NULL)
{
    m_config = g_application.m_configure;

    setWindowTitle(QString::fromUtf8("AlphaDict"));
    setWindowIcon(QIcon(QStringLiteral(":/res/appicon.png")));
    resize(1080, 720);
    setMinimumSize(760, 520);

    m_bridge = new QmlBridge(this);

    m_quick = new QQuickWidget(this);
    m_quick->setResizeMode(QQuickWidget::SizeRootObjectToView);
    m_quick->rootContext()->setContextProperty(QStringLiteral("bridge"), m_bridge);
    m_quick->setSource(QUrl(QStringLiteral("qrc:/qml/Main.qml")));
    if (m_quick->status() == QQuickWidget::Error) {
        foreach (const QQmlError &err, m_quick->errors())
            g_sysLog.e("QML error: %s\n", err.toString().toUtf8().constData());
    }

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(m_quick);

    m_messager = new QtMessager(m_bridge, m_bridge->indexModel(),
                                g_application.uiMessageQ());
    m_messager->start();

    m_capWordDialog = new CapWordDialog(m_config->m_cws.autoCloseEn);
    QObject::connect(m_capWordDialog, SIGNAL(addToVBook(const QString &)),
                     this, SLOT(onCapWordAddToVBook(const QString &)));

    if (QSystemTrayIcon::isSystemTrayAvailable()) {
        m_systray = new QSystemTrayIcon(this);
        QIcon icon;
        icon.addFile(QStringLiteral(":/res/appicon.png"), QSize(), QIcon::Normal, QIcon::Off);
        m_systray->setIcon(icon);
        connect(m_systray, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
                this, SLOT(onSysTrayActivated(QSystemTrayIcon::ActivationReason)));
        m_systray->setContextMenu(creatTrayContextMenu());
        if (m_config->m_setting.bsystemTray)
            m_systray->show();
    }

    QObject::connect(QApplication::clipboard(), SIGNAL(dataChanged()),
                     this, SLOT(onClipboardDataChanged()));
    QObject::connect(QApplication::clipboard(), SIGNAL(selectionChanged()),
                     this, SLOT(onClipboardSelectionChanged()));

    QObject::connect(m_bridge, SIGNAL(requestMove(int, int)),
                     this, SLOT(onRequestMove(int, int)));
    QObject::connect(m_bridge, SIGNAL(requestMinimizeToTray()),
                     this, SLOT(onRequestMinimizeToTray()));
    QObject::connect(m_bridge, SIGNAL(requestQuit()),
                     this, SLOT(onRequestQuit()));
    QObject::connect(m_bridge, SIGNAL(requestSystemTray(bool)),
                     this, SLOT(onRequestSystemTray(bool)));
    QObject::connect(m_bridge, SIGNAL(requestCwsEnable(bool)),
                     this, SLOT(onRequestCwsEnable(bool)));
    QObject::connect(m_bridge, SIGNAL(requestShortcutKey(int, int)),
                     this, SLOT(onRequestShortcutKey(int, int)));
    QObject::connect(m_bridge, SIGNAL(capWordShown()),
                     this, SLOT(onCapWordShown()));

    qApp->installEventFilter(this);
}

QmlMainWindow::~QmlMainWindow()
{
    /* Children are destroyed in creation order, so m_bridge would go away
     * before m_quick and the still-loaded QML tree would re-evaluate every
     * bridge.* binding against a null context property (a screenful of
     * "Cannot read property ... of null" on exit). Drop the QML scene first. */
    delete m_messager;
    m_messager = NULL;
    if (m_quick != NULL)
        m_quick->setSource(QUrl());
    delete m_capWordDialog;
    m_capWordDialog = NULL;
}

QMenu* QmlMainWindow::creatTrayContextMenu()
{
    QMenu *trayMenu = new QMenu(this);

    m_trayCwsEnableAct = new QAction(tr("Enable/Disable Capture Word"), this);
    m_trayCwsEnableAct->setCheckable(true);
    m_trayCwsEnableAct->setChecked(m_config->m_cws.benable);
    connect(m_trayCwsEnableAct, SIGNAL(triggered(bool)), this, SLOT(onTrayCwsEnable(bool)));
    trayMenu->addAction(m_trayCwsEnableAct);

    m_trayCwsSelectionAct = new QAction(tr("Capture Word by Mouse Selection"), this);
    m_trayCwsSelectionAct->setCheckable(true);
    m_trayCwsSelectionAct->setChecked(m_config->m_cws.bselection);
    connect(m_trayCwsSelectionAct, SIGNAL(triggered(bool)), this, SLOT(onTrayCwsSelection(bool)));
    trayMenu->addAction(m_trayCwsSelectionAct);

    m_trayCwsClipboardAct = new QAction(tr("Capture Word by Clipboard"), this);
    m_trayCwsClipboardAct->setCheckable(true);
    m_trayCwsClipboardAct->setChecked(m_config->m_cws.bclipboard);
    connect(m_trayCwsClipboardAct, SIGNAL(triggered(bool)), this, SLOT(onTrayCwsClipboard(bool)));
    trayMenu->addAction(m_trayCwsClipboardAct);

    trayMenu->addSeparator();

    QAction *act = new QAction(tr("Activate"), this);
    connect(act, SIGNAL(triggered()), this, SLOT(onTrayMenuActivated()));
    trayMenu->addAction(act);

    act = new QAction(tr("Exit"), this);
    connect(act, SIGNAL(triggered()), this, SLOT(onTrayMenuClose()));
    trayMenu->addAction(act);

    return trayMenu;
}

void QmlMainWindow::initDelay()
{
    if (m_config->m_cws.benable)
        registerHotkey(m_config->m_cws.shortcutKey);

    if (m_config->m_setting.font == "") {
        QString family = font().family();
        m_config->writeFont(family.toStdString());
    }
}

/*---------------------------------------------------------------------------
 * clipboard capture
 *-------------------------------------------------------------------------*/

void QmlMainWindow::onClipboardDataChanged()
{
    if (m_cwdEnableTemp && m_config->m_cws.benable) {
    #ifdef _LINUX
        if (m_config->m_cws.bclipboard) {
    #elif defined(_WINDOWS)
        if (m_config->m_cws.bclipboard || m_config->m_cws.bselection) {
    #endif
            const QClipboard *clipboard = QApplication::clipboard();
            QString input = clipboard->text(QClipboard::Clipboard).trimmed();
            if (input != "") {
                std::string u8input = std::string(input.toUtf8().data());
                if (Util::isValidInput(u8input))
                    g_application.sysMessageQ()->push(MSG_CAPWORD_QUERY, u8input);
            } else if (m_capWordDialog != NULL && !m_capWordDialog->isHidden()) {
                m_capWordDialog->close();
            }
        }
    }
}

void QmlMainWindow::onClipboardSelectionChanged()
{
    static QString lastInput = "";

    if (m_config->m_cws.bselection && m_cwdEnableTemp && m_config->m_cws.benable) {
        const QClipboard *clipboard = QApplication::clipboard();

        QString input = clipboard->text(QClipboard::Selection).trimmed();
        if (input != "") {
            if (!m_nonRepeatSelEn || input != lastInput) {
                lastInput = input;
                std::string u8input = std::string(input.toUtf8().data());
                if (Util::isValidInput(u8input))
                    g_application.sysMessageQ()->push(MSG_CAPWORD_QUERY, u8input);
            } else {
                lastInput = "";
            }
        } else if (m_capWordDialog != NULL && !m_capWordDialog->isHidden()) {
            m_capWordDialog->close();
        }
    }
}

void QmlMainWindow::onCapWordShown()
{
    if (isActiveWindow())
        return;

    DictItemList *items = m_bridge->takeCapWordItems();
    if (items == NULL)
        return;

    if (items->empty()) {
        delete items;
        return;
    }

    m_capWordDialog->setAutoClose(m_config->m_cws.autoCloseEn);
    m_capWordDialog->setDictItemList(items);   // takes the ownership
    m_capWordDialog->showAtCursor();
}

void QmlMainWindow::onCapWordAddToVBook(const QString &word)
{
    if (word != "" && m_bridge->addToVBookChecked(word))
        showToolTip(tr("Add to vocabulary book,success"));
    else
        showToolTip(tr("Add to vocabulary book,failure"));
}

/*---------------------------------------------------------------------------
 * window behaviour
 *-------------------------------------------------------------------------*/

void QmlMainWindow::onRequestMove(int dx, int dy)
{
    move(pos() + QPoint(dx, dy));
}

void QmlMainWindow::onRequestMinimizeToTray()
{
    if (m_systray != NULL && m_systray->isVisible())
        hide();
    else
        showMinimized();
}

void QmlMainWindow::onRequestQuit()
{
    if (m_systray != NULL)
        m_systray->hide();
    close();
    qApp->quit();
}

void QmlMainWindow::onRequestSystemTray(bool en)
{
    if (m_systray == NULL)
        return;
    if (en)
        m_systray->show();
    else
        m_systray->hide();
}

void QmlMainWindow::onRequestCwsEnable(bool en)
{
    m_cwdEnableTemp = true;
    if (en) {
        registerHotkey(m_config->m_cws.shortcutKey);
    } else {
        unregisterHotkey(m_config->m_cws.shortcutKey);
        if (m_capWordDialog != NULL && !m_capWordDialog->isHidden())
            m_capWordDialog->close();
    }
    if (m_trayCwsEnableAct != NULL)
        m_trayCwsEnableAct->setChecked(en);
}

void QmlMainWindow::onRequestShortcutKey(int oldIndex, int newIndex)
{
    (void)newIndex;
    unregisterHotkey(oldIndex);
    if (m_config->m_cws.benable)
        registerHotkey(m_config->m_cws.shortcutKey);
}

void QmlMainWindow::onSysTrayActivated(QSystemTrayIcon::ActivationReason reason)
{
    if (reason == QSystemTrayIcon::Trigger) {
        showNormal();
        activateWindow();
    }
}

void QmlMainWindow::onTrayCwsEnable(bool checked)
{
    m_bridge->setCwsEnabled(checked);       // writes the config and notifies us back
}

void QmlMainWindow::onTrayCwsClipboard(bool checked)
{
    m_bridge->setCwsClipboard(checked);
}

void QmlMainWindow::onTrayCwsSelection(bool checked)
{
    m_bridge->setCwsSelection(checked);
}

void QmlMainWindow::onTrayMenuActivated()
{
    onSysTrayActivated(QSystemTrayIcon::Trigger);
}

void QmlMainWindow::onTrayMenuClose()
{
    onRequestQuit();
}

bool QmlMainWindow::eventFilter(QObject *watched, QEvent *event)
{
    (void)watched;
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyevent = static_cast<QKeyEvent *>(event);
        if (keyevent->key() == m_config->m_cws.shortcutKey + 'A' &&
            keyevent->modifiers() == (Qt::ControlModifier | Qt::AltModifier)) {
            m_cwdEnableTemp = !m_cwdEnableTemp;
            if (!m_cwdEnableTemp && m_capWordDialog != NULL && !m_capWordDialog->isHidden())
                m_capWordDialog->close();
            return true;
        }
    }
    return false;
}

void QmlMainWindow::closeEvent(QCloseEvent *event)
{
    if (m_config->m_setting.bsystemTray && m_systray != NULL && m_systray->isVisible()) {
        hide();
        event->ignore();
    } else {
        event->accept();
    }
}

void QmlMainWindow::showToolTip(const QString &info, int displayTimeMS)
{
    QRect rect(0, 0, 120, 80);
    QToolTip::showText(QCursor::pos(), info, NULL, rect, displayTimeMS);
}

/*---------------------------------------------------------------------------
 * hotkey and shutdown
 *-------------------------------------------------------------------------*/

void QmlMainWindow::registerHotkey(int key)
{
#ifdef _WINDOWS
    ::RegisterHotKey((HWND)(this->effectiveWinId()), 0x1111,
                     MOD_CONTROL | MOD_ALT, key + 'A');
#elif defined(_LINUX)
    X11Util::registerHotkey(key + 'a');
#endif
}

void QmlMainWindow::unregisterHotkey(int key)
{
#ifdef _WINDOWS
    (void)key;
    UnregisterHotKey((HWND)effectiveWinId(), 0x1111);
#elif defined(_LINUX)
    X11Util::unregisterHotkey(key + 'a');
#endif
}

void QmlMainWindow::onAppExit()
{
    if (m_config->m_cws.benable)
        unregisterHotkey(m_config->m_cws.shortcutKey);

    m_cwdEnableTemp = false;

    if (onSysExit != NULL)
        (*onSysExit)();

    m_messager->abort();
}
