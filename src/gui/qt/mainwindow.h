#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtWidgets/QMainWindow>
#include <QtCore/QModelIndexList>
#include <QtWidgets/QSystemTrayIcon>
#include <QtGui/QClipboard>

#include "Application.h"

namespace Ui {
class MainWindow;
}

class QtMessager;
class DictIndexModel;
class QListWidgetItem;
class VBookModel;
class CapWordDialog;

class MainWindow : public QMainWindow
{
    Q_OBJECT

friend class  QtMessager;

public:
    static void showToolTip(QString info, int displayTimeMS=1500);
    static void showToolTip(QString info, QPoint pos, int displayTimeMS=1500);

    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void registerSysExit(void (*exit)()) { onSysExit = exit; }

    void initDelay();

    QtMessager *m_messager;

public slots:
    void onUpdateExplText(void *v);
    void onUpdateCapWordExplText(void *v);
    void onSetLanComboBox(const QString& src, const QString& det, void *v);

private slots:
    void on_srcLanComboBox_currentIndexChanged(const QString &arg1);

    void on_detLanComboBox_currentIndexChanged(const QString &arg1);

    void on_inputLineEdit_editingFinished();

    void on_inputLineEdit_textChanged(const QString &arg1);

    void on_queryButton_clicked();

    void on_indexListView_clicked(const QModelIndex &index);

    void on_indexListView_activated(const QModelIndex &index);

    void on_pgdownToolButton1_clicked();

    void on_pgdownToolButton2_clicked();

    void on_pgupToolButton1_clicked();

    void on_pgupToolButton2_clicked();
    
    void on_dictListWidget_clicked(const QModelIndex &index);

    void on_dictUpToolButton_clicked();

    void on_dictDownToolButton_clicked();

    void on_uilanComboBox_activated(int index);

    void on_indexLineEdit_editingFinished();

    void on_saveButton_clicked();

    void on_tabWidget_currentChanged(int index);

    void on_vbdelToolButton_clicked();

    void on_vbclearToolButton_clicked();

    void on_vbpreItemTlBtn_clicked();

    void on_vbnextItemTlBtn_clicked();

    void on_vbModeComboBox_currentIndexChanged(int index);

    void on_vbookTableView_clicked(const QModelIndex &index);

    void on_vbookTableView_activated(const QModelIndex &index);
    
    void on_vbInput_editingFinished();

    void on_vbExplHideradioButton_clicked(bool checked);

    void onAppExit();

    void onActionSettingPageAdded();

    void onActionVcbularyPageAdded();

    void onActionHelpPageAdded();

    void onClipboardDataChanged();

    void onClipboardSelectionChanged();

    void on_cwsClipboardCheckBox_clicked(bool checked);

    void on_cwsSelectionCheckBox_clicked(bool checked);

    void on_cwsEnableCheckBox_clicked(bool checked);

    void on_cwsMouseCheckBox_clicked(bool checked);

    void on_cwsShortcutkeyComboBox_activated(int index);

    void on_cwsAutoCloseEnCheckBox_clicked(bool checked);

    void on_fontsizeComboBox_activated(int index);

    void onSysTrayActivated(QSystemTrayIcon::ActivationReason reason);

    void on_systemTrayCheckBox_clicked(bool checked);

    void on_fontComboBox_activated(const QString &arg1);

    void on_resetSettingToolButton_clicked();


    void onTrayCwsEnable(bool checked);
    void onTrayCwsClipboard(bool checked);
    void onTrayCwsSelection(bool checked);
    void onTrayCwsMouse(bool checked);
    void onTrayMenuClose();

protected :
    //bool winEvent( MSG * message, long * result);
    bool nativeEvent(const QByteArray & eventType, void * message, long * result);
    virtual bool eventFilter( QObject *watched, QEvent *event);
    virtual void closeEvent(QCloseEvent * event);

private:
    void registerHotkey(int key);
    void unregisterHotkey(int key);
    int  capwordMode();
    void readHelpText(QString &help);

    inline void showVBookExpl(int row);

    QMenu* creatTrayContextMenu();
    QAction* m_trayCwsEnableAct;
    QAction* m_trayCwsSelectionAct;
    QAction* m_trayCwsMouseAct;
    QAction* m_trayCwsClipboardAct;

    CapWordDialog* m_capWordDialog;
    DictIndexModel* m_dictIndexModel;
    VBookModel*     m_vbookModel;
    bool m_bHideVBookExpl;

    Ui::MainWindow *ui;
    QSystemTrayIcon* m_systray;

    Configure* m_config;
    bool m_initSettingPage;
    bool m_initHelpPage;

    void (*onSysExit)();

    bool m_cwdEnableTemp;
};

#endif // MAINWINDOW_H
