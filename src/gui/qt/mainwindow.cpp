#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "DictIndexModel.h"
#include "MessageQueue.h"
#include "QtMessager.h"
#include "iDict.h"
#include "Application.h"
//#include <stdio.h>
#include <QScrollBar>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    //ui->tabWidget->setTabsClosable(true);
    
    QObject::connect(ui->actionSetting, SIGNAL(triggered()),this, SLOT(onActionSettingPageAdded()));
    QObject::connect(ui->actionVocabulary, SIGNAL(triggered()), this, SLOT(onActionVcbularyPageAdded()));
    
    m_dictIndexModel = new DictIndexModel();
    ui->indexListView->setModel(m_dictIndexModel);

    m_messager = new QtMessager(this, m_dictIndexModel);
    m_messager->start();
    
    ui->tabWidget->removeTab(1);
    m_initSettingPage = false;
}

MainWindow::~MainWindow()
{
    m_messager->abort();

    delete m_messager;
    delete ui;
}

void MainWindow::on_srcLanComboBox_currentIndexChanged(const QString &arg1)
{
    g_sysMessageQ.push(MSG_SET_SRCLAN, std::string(arg1.toUtf8().data()));
}

void MainWindow::on_detLanComboBox_currentIndexChanged(const QString &arg1)
{
    g_sysMessageQ.push(MSG_SET_DETLAN, std::string(arg1.toUtf8().data()));
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
	g_sysMessageQ.push(MSG_DICT_QUERY, std::string(m_input.toUtf8().data()));
    ui->dictTextEdit->document()->clear();
}

void MainWindow::on_indexListView_clicked(const QModelIndex &index)
{
    iIndexItem* item = m_dictIndexModel->item(index.row());
    if (item != NULL) {
	    g_sysMessageQ.push(MSG_DICT_INDEX_QUERY, index.row(), (void *)(m_dictIndexModel->item(index.row())));
		ui->dictTextEdit->document()->clear();
		QString text = QString::fromUtf8(item->index.c_str());
		ui->inputLineEdit->setText(text);
    }
}

void MainWindow::onUpdateText(void *v)
{
    iDictItem *i = (iDictItem*) v;

    QTextCursor cursor(ui->dictTextEdit->textCursor());
    QTextFrameFormat itemFrame;
    QTextCharFormat boldFormat;
    QTextCharFormat titleFormat;
    QTextCharFormat bodyFormat;
    QTextBlockFormat itemBlock;
    QString text;

    //itemFrame.setBorder(1);
    //itemFrame.setBorderStyle(QTextFrameFormat::BorderStyle_Inset);
    cursor.insertFrame(itemFrame);

    boldFormat.setFontWeight(QFont::Bold);
    itemBlock.setIndent(1);
    text = QString::fromUtf8(i->dictname.c_str());
    text = text.trimmed();
    cursor.insertText(text, boldFormat);
    cursor.insertBlock(itemBlock);

    titleFormat.setFontWeight(QFont::DemiBold);
    text = QString::fromUtf8(i->phonetic.c_str());
    text = text.trimmed();
    cursor.insertText(text, titleFormat);

    bodyFormat.setFontWeight(QFont::Light);
    cursor.insertBlock(itemBlock);
    //cursor.insertBlock(itemBlock);
    text = QString::fromUtf8(i->expl.c_str());
    text = text.trimmed(); 
 //   qDebug() << text;
    //cursor.insertHtml(text);
    cursor.insertText(text,bodyFormat);
    cursor.insertBlock();
    delete i;
}

void MainWindow::onSetLanComboBox(const QString& src, const QString& det, void *v)
{
    vector<string> *pVec = (vector<string>*) v;
    vector<string>::iterator iter;
    ui->detLanComboBox->addItem("any");
	ui->srcLanComboBox->addItem("any");

    for (iter = (*pVec).begin(); iter != (*pVec).end(); iter++) {
        QString item((*iter).c_str());
        ui->srcLanComboBox->addItem(item);
        ui->detLanComboBox->addItem(item);        
    }

    int i = ui->srcLanComboBox->findText(src);
    ui->srcLanComboBox->setCurrentIndex(i);

    i = ui->detLanComboBox->findText(det);
    ui->detLanComboBox->setCurrentIndex(i);
}

void MainWindow::onAppExit()
{
	(*onSysExit)();
//    QCoreApplication::quit();
}


void MainWindow::on_pgdownToolButton1_clicked()
{
    QModelIndex modelIndex = m_dictIndexModel->updateIndexList(1);
	if (modelIndex.row() != -1) {
	    ui->indexListView->scrollTo(modelIndex);
	    ui->indexListView->setCurrentIndex(modelIndex);
	}
}

void MainWindow::on_pgdownToolButton2_clicked()
{
    QModelIndex modelIndex = m_dictIndexModel->updateIndexList(5);
	if (modelIndex.row() != -1) {
	    ui->indexListView->scrollTo(modelIndex);
	    ui->indexListView->setCurrentIndex(modelIndex);
	}
}

void MainWindow::on_pgupToolButton1_clicked()
{
    QModelIndex modelIndex = m_dictIndexModel->updateIndexList(-1);
	if (modelIndex.row() != -1) {
	    ui->indexListView->scrollTo(modelIndex);
	    ui->indexListView->setCurrentIndex(modelIndex);
	}
}

void MainWindow::on_pgupToolButton2_clicked()
{
    QModelIndex modelIndex = m_dictIndexModel->updateIndexList(-5);
	if (modelIndex.row() != -1) {
	    ui->indexListView->scrollTo(modelIndex);
	    ui->indexListView->setCurrentIndex(modelIndex);
	}
}

void MainWindow::on_dictListWidget_itemChanged(QListWidgetItem *item)
{
    int row = ui->dictListWidget->row(item);
    if (row != -1) {
        if (item->checkState() == Qt::Checked)
		    g_sysMessageQ.push(MSG_SET_DICTEN, row, 1);
        else
		    g_sysMessageQ.push(MSG_SET_DICTEN, row, 0);
    }
}

void MainWindow::on_dictUpToolButton_clicked()
{
    int currentIndex = ui->dictListWidget->currentRow();
    if (currentIndex != -1 && currentIndex != 0) {
        QListWidgetItem *currentItem = ui->dictListWidget->takeItem(currentIndex);
        ui->dictListWidget->insertItem(currentIndex-1, currentItem);
        ui->dictListWidget->setCurrentRow(currentIndex-1);
        //g_application.m_configure->moveDictItem(currentIndex, false);
		g_sysMessageQ.push(MSG_MOVE_DICTITEM, currentIndex, 0);
    }
}

void MainWindow::on_dictDownToolButton_clicked()
{
    int currentIndex = ui->dictListWidget->currentRow();
    int count = ui->dictListWidget->count();
    if (currentIndex != -1 && currentIndex < count-1) {
        QListWidgetItem *currentItem = ui->dictListWidget->takeItem(currentIndex);
        ui->dictListWidget->insertItem(currentIndex+1, currentItem);
        ui->dictListWidget->setCurrentRow(currentIndex+1);
        //g_application.m_configure->moveDictItem(currentIndex);
		g_sysMessageQ.push(MSG_MOVE_DICTITEM, currentIndex, 1);
    }
}

void MainWindow::onActionSettingPageAdded()
{
    int inx = ui->tabWidget->indexOf(ui->settingTab);
    if (inx == -1) {
        if (!m_initSettingPage) {
            m_initSettingPage = true;
            Configure* config = g_application.m_configure;
	        for (int i = 0; i < config->m_dictNodes.size(); i++) {
                QString name = QString(config->m_dictNodes[i].name.c_str());
	        	bool en = config->m_dictNodes[i].en;
	        	QListWidgetItem* item = new QListWidgetItem(name, ui->dictListWidget);
	        	item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
	        	if (en)
	        	    item->setCheckState(Qt::Checked);
	        	else
	        	    item->setCheckState(Qt::Unchecked);
	        }
        }
        inx = ui->tabWidget->addTab(ui->settingTab, "setting");
        ui->tabWidget->setCurrentIndex(inx);
    } else {
        ui->tabWidget->removeTab(inx);
    }
}

void MainWindow::onActionVcbularyPageAdded()
{
}

void MainWindow::on_dictListWidget_clicked(const QModelIndex &index)
{
    Configure* config = g_application.m_configure;
	
    QString info = QString(config->m_dictNodes[index.row()].summary.c_str());
    ui->dictInfoLabel->setText(info);
}

void MainWindow::on_tabWidget_currentChanged(int index)
{
    if (index == 0)
        g_sysMessageQ.push(MSG_CURTAB_PGDICT, -1, -1);
}
