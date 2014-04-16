#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class QtMessager;
class DictIndexModel;

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

private slots:
    void on_srcLanComboBox_currentIndexChanged(const QString &arg1);

    void on_detLanComboBox_currentIndexChanged(const QString &arg1);

    void on_inputLineEdit_editingFinished();

    void on_inputLineEdit_textChanged(const QString &arg1);

    void on_queryButton_clicked();

    void on_indexListView_clicked(const QModelIndex &index);

    void onAppExit();

private:
    QString m_input;
    DictIndexModel* m_dictIndexModel;

    Ui::MainWindow *ui;
    void (*onSysExit)();
};

#endif // MAINWINDOW_H
