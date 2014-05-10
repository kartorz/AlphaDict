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
