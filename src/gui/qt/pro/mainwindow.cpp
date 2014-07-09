#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_lineEdit_editingFinished()
{

}

void MainWindow::on_inputLineEdit_editingFinished()
{

}

void MainWindow::on_indexLineEdit_editingFinished()
{

}

void MainWindow::on_dictListWidget_clicked(const QModelIndex &index)
{

}

void MainWindow::on_listWidget_clicked(const QModelIndex &index)
{

}

void MainWindow::on_delToolButton_clicked()
{

}

void MainWindow::on_clearToolButton_clicked()
{

}

void MainWindow::on_preItemTlBtn_clicked()
{

}

void MainWindow::on_nextItemTlBtn_clicked()
{

}

void MainWindow::on_vocabularyListWidget_clicked(const QModelIndex &index)
{

}

void MainWindow::on_saveButton_clicked()
{

}

void MainWindow::on_vocabularyListView_clicked(const QModelIndex &index)
{

}

void MainWindow::on_vocabularyTabWidget_currentChanged(int index)
{

}

void MainWindow::on_indexListView_activated(const QModelIndex &index)
{

}

void MainWindow::on_vbookListView_activated(const QModelIndex &index)
{

}

void MainWindow::on_uilanComboBox_currentIndexChanged(int index)
{

}

void MainWindow::on_uilanComboBox_activated(int index)
{

}

void MainWindow::on_cwsSelectionCheckBox_clicked()
{

}

void MainWindow::on_cwsClipboardCheckBox_clicked(bool checked)
{

}

void MainWindow::on_cwsSelectionCheckBox_clicked(bool checked)
{

}
