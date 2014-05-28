#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_lineEdit_editingFinished();

    void on_inputLineEdit_editingFinished();

    void on_indexLineEdit_editingFinished();

    void on_dictListWidget_clicked(const QModelIndex &index);

    void on_listWidget_clicked(const QModelIndex &index);

    void on_delToolButton_clicked();

    void on_clearToolButton_clicked();

    void on_preItemTlBtn_clicked();

    void on_nextItemTlBtn_clicked();

    void on_vocabularyListWidget_clicked(const QModelIndex &index);

    void on_saveButton_clicked();

    void on_vocabularyListView_clicked(const QModelIndex &index);

    void on_vocabularyTabWidget_currentChanged(int index);

    void on_indexListView_activated(const QModelIndex &index);

    void on_vbookListView_activated(const QModelIndex &index);

    void on_uilanComboBox_currentIndexChanged(int index);

    void on_uilanComboBox_activated(int index);

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
