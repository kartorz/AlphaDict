#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QModelIndexList>
namespace Ui {
class MainWindow;
}

class QtMessager;
class DictIndexModel;
class QListWidgetItem;

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void registerSysExit(void (*exit)()) { onSysExit = exit; }    

    QtMessager *m_messager;

public slots:
    void onUpdateText(void *v);
    void onSetLanComboBox(const QString& src, const QString& det, void *v);

private slots:
    void on_srcLanComboBox_currentIndexChanged(const QString &arg1);

    void on_detLanComboBox_currentIndexChanged(const QString &arg1);

    void on_inputLineEdit_editingFinished();

    void on_inputLineEdit_textChanged(const QString &arg1);

    void on_queryButton_clicked();

    void on_indexListView_clicked(const QModelIndex &index);

    void on_pgdownToolButton1_clicked();

    void on_pgdownToolButton2_clicked();

    void on_pgupToolButton1_clicked();

    void on_pgupToolButton2_clicked();

    void on_dictListWidget_itemChanged(QListWidgetItem *item);

    void on_dictUpToolButton_clicked();

    void on_dictDownToolButton_clicked();

    void onAppExit();

    void on_dictListWidget_clicked(const QModelIndex &index);

    void onActionSettingPageAdded();
    void onActionVcbularyPageAdded();

    void on_tabWidget_currentChanged(int index);

private:
    QString m_input;
    DictIndexModel* m_dictIndexModel;

    Ui::MainWindow *ui;
    void (*onSysExit)();

    bool m_initSettingPage;
};

#endif // MAINWINDOW_H
