#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "DictIndexModel.h"
#include "MessageQueue.h"
#include "QtMessager.h"
#include "iDict.h"

#include <QDebug>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)    
{
    ui->setupUi(this);
    m_dictIndexModel = new DictIndexModel();
    ui->indexListView->setModel(m_dictIndexModel);

    m_messager = new QtMessager(this, m_dictIndexModel);
    m_messager->start();
}

MainWindow::~MainWindow()
{
    m_messager->abort();

    delete m_messager;
    delete ui;
}

void MainWindow::on_srcLanComboBox_currentIndexChanged(const QString &arg1)
{

}

void MainWindow::on_detLanComboBox_currentIndexChanged(const QString &arg1)
{

}

void MainWindow::on_inputLineEdit_editingFinished()
{
    m_input = ui->inputLineEdit->text();
}

void MainWindow::on_inputLineEdit_textChanged(const QString &arg1)
{

}

void MainWindow::on_queryButton_clicked()
{
	g_sysMessagerQ.push(MSG_DICT_QUERY, std::string(m_input.toUtf8().data()));
    ui->dictTextEdit->document()->clear();
}

void MainWindow::on_indexListView_clicked(const QModelIndex &index)
{
	g_sysMessagerQ.push(MSG_DICT_INDEX_QUERY, index.row(), (void *)(m_dictIndexModel->item(index.row())));
    ui->dictTextEdit->document()->clear();
    iIndexItem* item = m_dictIndexModel->item(index.row());
    QString text = QString::fromWCharArray(item->index, item->inxlen);
    ui->inputLineEdit->setText(text);
}

void MainWindow::onUpdateText(void *v)
{
    iDictItem *i = (iDictItem*) v;

    QTextCursor cursor(ui->dictTextEdit->textCursor());
    QTextFrameFormat itemFrame;
    QTextCharFormat boldFormat;
    QTextBlockFormat itemBlock;
    QString text;

    //itemFrame.setBorder(1);
    //itemFrame.setBorderStyle(QTextFrameFormat::BorderStyle_Inset);
    cursor.insertFrame(itemFrame);

    boldFormat.setFontWeight(QFont::Bold);
    itemBlock.setIndent(1);

    text = QString::fromUtf8(i->identify.c_str());
    text = text.trimmed();
    cursor.insertText(text, boldFormat);
    cursor.insertBlock(itemBlock);

    text = QString::fromUtf8(i->phonetic.c_str());
    text = text.trimmed();
    cursor.insertText(text, boldFormat);
    
    cursor.insertBlock(itemBlock);
    //cursor.insertBlock(itemBlock);
    text = QString::fromUtf8(i->expl.c_str());
    text = text.trimmed(); 
 //   qDebug() << text;
    cursor.insertHtml(text);
    cursor.insertBlock();
    delete i;
}

void MainWindow::onAppExit()
{
	(*onSysExit)();
//    QCoreApplication::quit();
}

