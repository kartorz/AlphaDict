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

#ifndef _QMLMAINWINDOW_H_
#define _QMLMAINWINDOW_H_

#include <QtWidgets/QWidget>
#include <QtWidgets/QSystemTrayIcon>

#include "Application.h"

class QQuickWidget;
class QMenu;
class QAction;
class QCloseEvent;
class QmlBridge;
class QtMessager;
class CapWordDialog;

/*! The QML based main window.
 *
 *  It keeps every job the widget window used to do (system tray, global
 *  hotkey, clipboard capture, captured word popup, graceful shutdown) and
 *  lets QML draw the whole user interface.
 */
class QmlMainWindow : public QWidget
{
    Q_OBJECT

public:
    explicit QmlMainWindow(QWidget *parent = 0);
    ~QmlMainWindow();

    void registerSysExit(void (*exit)()) { onSysExit = exit; }
    void initDelay();

public slots:
    void onAppExit();

private slots:
    void onClipboardDataChanged();
    void onClipboardSelectionChanged();
    void onSysTrayActivated(QSystemTrayIcon::ActivationReason reason);
    void onTrayCwsEnable(bool checked);
    void onTrayCwsClipboard(bool checked);
    void onTrayCwsSelection(bool checked);
    void onTrayMenuActivated();
    void onTrayMenuClose();

    void onRequestMove(int dx, int dy);
    void onRequestMinimizeToTray();
    void onRequestQuit();
    void onRequestSystemTray(bool en);
    void onRequestCwsEnable(bool en);
    void onRequestShortcutKey(int oldIndex, int newIndex);
    void onCapWordShown();
    void onCapWordAddToVBook(const QString &word);

protected:
    bool eventFilter(QObject *watched, QEvent *event);
    void closeEvent(QCloseEvent *event);

private:
    void registerHotkey(int key);
    void unregisterHotkey(int key);
    QMenu* creatTrayContextMenu();
    void showToolTip(const QString &info, int displayTimeMS = 1500);

    QmlBridge*      m_bridge;
    QQuickWidget*   m_quick;
    QtMessager*     m_messager;
    CapWordDialog*  m_capWordDialog;
    QSystemTrayIcon* m_systray;
    Configure*      m_config;

    QAction* m_trayCwsEnableAct;
    QAction* m_trayCwsClipboardAct;
    QAction* m_trayCwsSelectionAct;

    bool m_cwdEnableTemp;
    bool m_nonRepeatSelEn;
    void (*onSysExit)();
};

#endif // _QMLMAINWINDOW_H_
