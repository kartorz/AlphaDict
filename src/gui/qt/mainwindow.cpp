#include <QtWidgets/QScrollBar>
#include <QtWidgets/QToolTip>
#include <QtWidgets/QShortcut> 
#include <QtWidgets/QMenu>
#include <QtWidgets/QStyleFactory>
#include <QtCore/QMimeData>
#include <QtCore/QDebug>
#include <QtCore/QEvent>
#include <QtGui/QCursor>
#include <QtGui/QFontDatabase> 
#include <QtGui/QtEvents>

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "capworddialog.h"
#include "DictIndexModel.h"
#include "VBookModel.h"
#include "MessageQueue.h"
#include "QtMessager.h"
#include "iDict.h"
#include "Log.h"
#include "CharUtil.h"
#include "Util.h"
#ifdef WIN32
#include "win32/TextOutHookServer.h"
#define HOTKEY_IDENTIFY   0x1111
#elif defined (_LINUX)
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include "X11Util.h"
#endif

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_cwdEnableTemp(true),
    m_bHideVBookExpl(false),
    m_nonRepeatSelEn(false),
	m_setting(APP_ORGANIZATION, APP_NAME)
{
    ui->setupUi(this);

    ui->vbcmbRepeatCount->setEditable(true);
    ui->vbcmbRepeatCount->lineEdit()->setReadOnly(true);
    ui->vbcmbRepeatCount->lineEdit()->setAlignment(Qt::AlignCenter);
    ui->vbModeComboBox->setEditable(true);
    ui->vbModeComboBox->lineEdit()->setReadOnly(true);
    ui->vbModeComboBox->lineEdit()->setAlignment(Qt::AlignCenter);

	/*if (m_setting.contains(KEY_WIN_GEOMETRY)) {
		restoreGeometry(m_setting.value(KEY_WIN_GEOMETRY).toByteArray());
	}*/

    //ui->tabWidget->setTabsClosable(true);
    //ui->queryButton->setStyleSheet("background-image:url(:/res/search.png)");
    //ui->queryButton->setIconSize(QSize(64,64));
    //ui->inputLineEdit->setAttribute(Qt::WA_InputMethodEnabled,true);
    QObject::connect(ui->actionSetting, SIGNAL(triggered()),this, SLOT(onActionSettingPageAdded()));
    QObject::connect(ui->actionVocabulary, SIGNAL(triggered()), this, SLOT(onActionVcbularyPageAdded()));
    QObject::connect(ui->actionHelp, SIGNAL(triggered()), this, SLOT(onActionHelpPageAdded()));
    //QShortcut  *listViewEnterAccel= new QShortcut(Qt::Key_Return, ui->indexListView);
    //connect(listViewEnterAccel, SIGNAL(activated()), this, SLOT(enterTreeItem()));
    QObject::connect(QApplication::clipboard(), SIGNAL(dataChanged()), this, SLOT(onClipboardDataChanged()));
    QObject::connect(QApplication::clipboard(), SIGNAL(selectionChanged()), this, SLOT(onClipboardSelectionChanged()));
	QObject::connect(ui->inputComboBox->lineEdit(), SIGNAL(returnPressed()), this, SLOT(onInputLineEditReturnPressed()));

    m_config = g_application.m_configure;
    
    m_dictIndexModel = new DictIndexModel();
    ui->indexListView->setModel(m_dictIndexModel);

    m_vbookModel = new VBookModel(m_config->getVBPath());
    ui->vbookTableView->setModel(m_vbookModel);
    int columnWidth[] = {200,100,144};
    for (int i = 0; i < 3; i++)  ui->vbookTableView->setColumnWidth(i,columnWidth[i]);

    m_messager = new QtMessager(this, m_dictIndexModel, g_application.uiMessageQ());
    m_messager->start();
    
    ui->tabWidget->removeTab(1);
    m_initSettingPage = false;

    ui->tabWidget->removeTab(1);
    
    ui->tabWidget->removeTab(1);
    m_initHelpPage = false;

    if (QSystemTrayIcon::isSystemTrayAvailable()) {
        m_systray = new QSystemTrayIcon(this);
        QIcon icon;
        icon.addFile(QStringLiteral(":/res/appicon.png"), QSize(), QIcon::Normal, QIcon::Off);
        m_systray->setIcon(icon);
        connect(m_systray,
            SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
            this,
            SLOT(onSysTrayActivated(QSystemTrayIcon::ActivationReason)));

        QMenu* trayMenu = creatTrayContextMenu();
        m_systray->setContextMenu(trayMenu);
        if (m_config->m_setting.bsystemTray)
            m_systray->show();
    }

    qApp->installEventFilter(this);
    m_capWordDialog = new CapWordDialog(this, m_config->m_cws.autoCloseEn);

#if 0
    QShortcut *shortcut = new QShortcut(QKeySequence(tr("Ctrl+O", "File|Open")),  this, 0, 0, Qt::ApplicationShortcut);
    connect(shortcut,
            SIGNAL(activated()),
            this,
            SLOT(on_pgdownToolButton1_clicked()));

#endif
}

MainWindow::~MainWindow()
{
    delete m_messager;
    delete ui;
    delete m_capWordDialog;
}

QMenu* MainWindow::creatTrayContextMenu()
{
    QMenu* trayMenu = new QMenu(this);

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
#ifdef _WINDOWS
    /*m_trayCwsMouseAct = new QAction(tr("Capture Word by Mouse Over"), this);
    m_trayCwsMouseAct->setCheckable(true);
    m_trayCwsMouseAct->setChecked(m_config->m_cws.bmouse);
    connect(m_trayCwsMouseAct, SIGNAL(triggered(bool)), this, SLOT(onTrayCwsMouse(bool)));
    trayMenu->addAction(m_trayCwsMouseAct);*/
#endif
    m_trayCwsClipboardAct = new QAction(tr("Capture Word by Clipboard"), this);
    m_trayCwsClipboardAct->setCheckable(true);
    m_trayCwsClipboardAct->setChecked(m_config->m_cws.bclipboard);
    connect(m_trayCwsClipboardAct, SIGNAL(triggered(bool)), this, SLOT(onTrayCwsClipboard(bool)));
    trayMenu->addAction(m_trayCwsClipboardAct);

    trayMenu->addSeparator();

    QAction* newAct = new QAction(tr("Activate"), this);
    connect(newAct, SIGNAL(triggered()), this, SLOT(onTrayMenuActivated()));
    trayMenu->addAction(newAct);

    newAct = new QAction(tr("Exit"), this);
    connect(newAct, SIGNAL(triggered()), this, SLOT(onTrayMenuClose()));
    trayMenu->addAction(newAct);

#ifdef _WINDOWS
    trayMenu->setStyle(QStyleFactory::create("windowsvista"));
#endif
    return trayMenu;
}

void MainWindow::initDelay()
{
    if (m_config->m_cws.benable) {
        registerHotkey(m_config->m_cws.shortcutKey);
    #ifdef _WINDOWS
        WId wid = effectiveWinId();
        g_sysLog.i("inject to wid(%d)\n", (int)wid);
        TextOutHookServer::getReference().inject((HWND)wid);
        TextOutHookServer::getReference().captureTextEnable(capwordMode());
    #endif
    }

    QFontDatabase::WritingSystem wsys [] = {QFontDatabase::Latin, QFontDatabase::Latin, QFontDatabase::SimplifiedChinese };
    ui->fontComboBox->setWritingSystem(wsys[m_config->m_setting.uilanID]);
    if (m_config->m_setting.font == "") {
        string curfont = ui->fontComboBox->currentFont().family().toUtf8().data();
        m_config->writeFont(curfont);
    }
}

void MainWindow::on_srcLanComboBox_currentIndexChanged(const QString &arg1)
{
    //g_sysLog.i("on_srcLanComboBox_currentIndexChanged\n");
    g_application.sysMessageQ()->push(MSG_SET_SRCLAN, std::string(arg1.toUtf8().data()));
}

void MainWindow::on_detLanComboBox_currentIndexChanged(const QString &arg1)
{
    //g_sysLog.i("on_detLanComboBox_currentIndexChanged\n");
    g_application.sysMessageQ()->push(MSG_SET_DETLAN, std::string(arg1.toUtf8().data()));
}

void MainWindow::on_inputComboBox_currentIndexChanged(const QString &arg1)
{
	//qDebug() << "inputComboBox_currentIndexChanged,arg1 " << arg1;
	if (!arg1.isEmpty()) {
		if (m_curInput != arg1) {
			lookup(arg1);
			m_curInput = arg1;
		}
	}
}

void MainWindow::onInputLineEditReturnPressed()
{
	QString input = ui->inputComboBox->currentText().trimmed();
	//qDebug()<<"onInputLineEditReturnPressed, currentText:" << input;
    if (!input.isEmpty()) {
		if (m_curInput != input) {
			lookup(input);
			m_curInput = input;
		}
	}
	ui->inputComboBox->clearEditText();
}

void MainWindow::on_queryButton_clicked()
{
    QString input = ui->inputComboBox->currentText().trimmed();
    if (!input.isEmpty() && m_curInput != input) {
        lookup(input);
		m_curInput = input;
    }
}

void MainWindow::on_indexListView_clicked(const QModelIndex &index)
{
    iIndexItem* item = m_dictIndexModel->item(index.row());
    if (item != NULL) {
	    g_application.sysMessageQ()->push(MSG_DICT_INDEX_QUERY, index.row(), (void *)(m_dictIndexModel->item(index.row())));
		ui->dictTextEdit->document()->clear();
		QString text = QString::fromUtf8(item->index.c_str());
		ui->inputComboBox->blockSignals(true);
		ui->inputComboBox->setCurrentText(text);
		if (ui->inputComboBox->findText(text) == -1)
			ui->inputComboBox->addItem(text);
		m_curInput = text;
		ui->inputComboBox->blockSignals(false);
    }
}

void MainWindow::on_indexListView_activated(const QModelIndex &index)
{
    on_indexListView_clicked(index);
}

void MainWindow::on_indexLineEdit_editingFinished()
{
    m_dictIndexModel->onResetIndexList(ui->indexLineEdit->text().toUtf8().data());
}

void MainWindow::onUpdateExplText(void *v)
{
    DictItemList* itemList = (DictItemList*) v;

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
    
    text = QString::fromUtf8((*itemList)[0].dictname.c_str());
    text = text.trimmed();
    cursor.insertText(text, boldFormat);
    cursor.insertBlock();
    cursor.insertText("- - - -");
    cursor.insertBlock();
    //cursor.insertBlock(itemBlock);
    for (unsigned i=0; i< itemList->size(); i++) {
        cursor.insertBlock();
        titleFormat.setFontWeight(QFont::DemiBold);
        int fpsize = titleFormat.fontPointSize();
        if (fpsize == 0) fpsize = m_config->m_setting.fontsize;

        titleFormat.setFontPointSize(fpsize + 2);
        text = QString::fromUtf8((*itemList)[i].word.c_str());
        cursor.insertText(text, titleFormat);

        titleFormat.setFontPointSize(fpsize);
        text = QString::fromUtf8((*itemList)[i].phonetic.c_str());
        text = text.trimmed();
        if (text != "") {
            cursor.insertBlock();
            //qDebug() << text;
            text = text.replace('\n', "    ");
            cursor.insertText(text, titleFormat);
            cursor.insertBlock();
        }

        bodyFormat.setFontWeight(QFont::Light);
        cursor.insertBlock(itemBlock);
        text = QString::fromUtf8((*itemList)[i].expl.c_str());
        text = text.trimmed(); 
     //   qDebug() << text;
        //cursor.insertHtml(text);
        cursor.insertText(text,bodyFormat);
        cursor.insertBlock();
        cursor.insertBlock();
    }

//    ui->dictTextEdit->moveCursor(QTextCursor::Start);

    delete itemList;
}

void MainWindow::onUpdateCapWordExplText(void *v)
{
    if (v == NULL ||
        (ui->inputComboBox->hasFocus() && ui->tabWidget->currentIndex() == 0 ))
        return;

    DictItemList* itemList = (DictItemList*) v;
    if (itemList->size() > 0) {
        //QPoint pos = QCursor::pos();
        m_capWordDialog->moveToCursor();
        m_capWordDialog->setDictItemList(itemList);

        m_capWordDialog->show();
        m_capWordDialog->activateWindow();
        m_capWordDialog->raise();
        //::SetCapture() /* win32 -- it seems not work */
    }
}

void MainWindow::onSetLanComboBox(const QString& src, const QString& det, void *v)
{
    vector<string> *pVec = (vector<string>*) v;
    vector<string>::iterator iter;

    ui->detLanComboBox->blockSignals(true);
    ui->srcLanComboBox->blockSignals(true);

    ui->detLanComboBox->clear();
    ui->srcLanComboBox->clear();

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

    ui->detLanComboBox->blockSignals(false);
    ui->srcLanComboBox->blockSignals(false);
}

void MainWindow::on_saveButton_clicked()
{
    QString word = ui->inputComboBox->currentText();
    
    if (word == "") {
        showToolTip(tr("Empty String"));
        return;
    }
    if (m_vbookModel->add(word)) {
        showToolTip(tr("Add to vocabulary book,success"));
    } else {
        showToolTip(tr("Add to vocabulary book,failure"));
    }
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

void MainWindow::on_dictUpToolButton_clicked()
{
    int currentIndex = ui->dictListWidget->currentRow();
    if (currentIndex != -1 && currentIndex != 0) {
        QListWidgetItem *currentItem = ui->dictListWidget->takeItem(currentIndex);
        ui->dictListWidget->insertItem(currentIndex-1, currentItem);
        ui->dictListWidget->setCurrentRow(currentIndex-1);
        //g_application.m_configure->moveDictItem(currentIndex, false);
		g_application.sysMessageQ()->push(MSG_MOVE_DICTITEM, currentIndex, 0);
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
		g_application.sysMessageQ()->push(MSG_MOVE_DICTITEM, currentIndex, 1);
    }
}

void MainWindow::on_uilanComboBox_activated(int index)
{
    m_config->writeUILanID(index);
}

void MainWindow::onActionSettingPageAdded()
{
    int inx = ui->tabWidget->indexOf(ui->settingTab);
    if (inx == -1) {
        if (!m_initSettingPage) {
            m_initSettingPage = true;
	        for (unsigned i = 0; i < m_config->m_dictNodes.size(); i++) {
                QString name = QString(m_config->m_dictNodes[i].name.c_str());
	        	bool en = m_config->m_dictNodes[i].en;
	        	QListWidgetItem* item = new QListWidgetItem(name, ui->dictListWidget);
	        	item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
	        	if (en)
	        	    item->setCheckState(Qt::Checked);
	        	else
	        	    item->setCheckState(Qt::Unchecked);
	        }
            ui->uilanComboBox->setCurrentIndex(m_config->m_setting.uilanID);
            ui->fontsizeComboBox->setCurrentIndex(m_config->m_setting.fontsize-9);
  
            ui->cwsShortcutkeyComboBox->setCurrentIndex(m_config->m_cws.shortcutKey);

            Qt::CheckState ckstate = m_config->m_setting.bsystemTray ? Qt::Checked : Qt::Unchecked;         
            ui->systemTrayCheckBox->setCheckState(ckstate);

            ckstate = m_config->m_cws.bselection ? Qt::Checked : Qt::Unchecked;
            ui->cwsSelectionCheckBox->setCheckState(ckstate);

            ckstate = m_config->m_cws.bclipboard ? Qt::Checked : Qt::Unchecked;
            ui->cwsClipboardCheckBox->setCheckState(ckstate);

            ckstate = m_config->m_cws.benable ? Qt::Checked : Qt::Unchecked;
            ui->cwsEnableCheckBox->setCheckState(ckstate);

            ckstate = m_config->m_cws.autoCloseEn ? Qt::Checked : Qt::Unchecked;
            ui->cwsAutoCloseEnCheckBox->setCheckState(ckstate);

        #if 1// _LINUX
            ui->cwsMouseCheckBox->hide();
        #elif defined(_WINDOWS)
            ckstate = m_config->m_cws.bmouse ? Qt::Checked : Qt::Unchecked;
            ui->cwsMouseCheckBox->setCheckState(ckstate);
        #endif
        }
        //QIcon icon;
        //icon.addFile(QStringLiteral(":/res/setting.png"), QSize(), QIcon::Normal, QIcon::Off);
        inx = ui->tabWidget->addTab(ui->settingTab, QApplication::translate("MainWindow", "Setting", 0));
        ui->tabWidget->setCurrentIndex(inx);
    } else {
        if (ui->tabWidget->currentIndex() == inx) {
            ui->tabWidget->removeTab(inx);
            /* will reload dictioanry if necessary */
            g_application.sysMessageQ()->push(MSG_RELOAD_DICT, -1, -1);
        } else {
            ui->tabWidget->setCurrentIndex(inx);
        }
    }
}

void MainWindow::onActionVcbularyPageAdded()
{
    int inx = ui->tabWidget->indexOf(ui->vocabularyTab);
    if (inx == -1) {
        ui->vbExplLabel->setText("");
        ui->vbExplLabel->setText(m_vbookModel->expl(m_vbookModel->getCurrentRow()));
        
        //ui->vbExplLabel->setText("");
        //ui->vbExplLabel->setText(m_vbookModel->curExamExpl());
        ui->detLanComboBox->blockSignals(true);

        ui->vbModeComboBox->clear();
        ui->vbModeComboBox->addItem(tr("study"));
        ui->vbModeComboBox->addItem(tr("exam"));

        ui->detLanComboBox->blockSignals(false);

        //ui->vbScoreLabel->setText("0");
        //QIcon icon;
        //icon.addFile(QStringLiteral(":/res/vocabulary.png"), QSize(), QIcon::Normal, QIcon::Off);
        //ui->vbookTableView->
        inx = ui->tabWidget->addTab(ui->vocabularyTab, QApplication::translate("MainWindow", "VocabulayBook", 0));
        ui->tabWidget->setCurrentIndex(inx);
    } else {
        if (ui->tabWidget->currentIndex() == inx)
            ui->tabWidget->removeTab(inx);
        else
            ui->tabWidget->setCurrentIndex(inx);
    }

    ui->vbookTableView->setCurrentIndex(m_vbookModel->curIndex());
}

void MainWindow::onActionHelpPageAdded()
{
    int inx = ui->tabWidget->indexOf(ui->helpTab);
    if (inx == -1) {
        if (!m_initHelpPage) {
            QString help;
            m_initHelpPage = true;
            readHelpText(help);
            ui->helpTextEdit->setText("");
            ui->helpTextEdit->setText(help);
        }
        inx = ui->tabWidget->addTab(ui->helpTab, QApplication::translate("MainWindow", "Help", 0));
        ui->tabWidget->setCurrentIndex(inx);
    } else {
        if (ui->tabWidget->currentIndex() == inx)
            ui->tabWidget->removeTab(inx);
        else
            ui->tabWidget->setCurrentIndex(inx);
    }
}

void MainWindow::on_dictListWidget_clicked(const QModelIndex &index)
{
    int row = index.row();
    QListWidgetItem *item = ui->dictListWidget->item(row);
    if (row != -1) {
        //printf("itemChanged (%d, %d)\n", row, item->checkState());
        if (item->checkState() == Qt::Checked)
		    g_application.sysMessageQ()->push(MSG_SET_DICTEN, row, 1);
        else if (item->checkState() == Qt::Unchecked)
		    g_application.sysMessageQ()->push(MSG_SET_DICTEN, row, 0);
    }

    QString info = QString(m_config->m_dictNodes[index.row()].summary.c_str());
    //ui->dictInfoLabel->setText(info);
    QPoint pos = ui->dictListWidget->pos();
    pos.setX(this->pos().x()+pos.x() + ui->dictListWidget->width() + 20);
    pos.setY(this->pos().y()+pos.y() + 100 + row*20);
    QToolTip::showText(pos, info, ui->dictListWidget);
}

void MainWindow::on_tabWidget_currentChanged(int index)
{
    if (index == 0)
        g_application.sysMessageQ()->push(MSG_RELOAD_DICT, -1, -1);
}

void MainWindow::showVBookExpl(int row)
{
    if (m_bHideVBookExpl == true) {
         ui->vbExplLabel->setText("");
    } else {
         ui->vbExplLabel->setText(m_vbookModel->expl(row));
    }
}

void MainWindow::on_vbExplHideradioButton_clicked(bool checked)
{
    m_bHideVBookExpl = checked;
    showVBookExpl(m_vbookModel->getCurrentRow());
}

void MainWindow::on_vbdelToolButton_clicked()
{
    int currentIndex = ui->vbookTableView->currentIndex().row();
    if (currentIndex != -1) {
        QModelIndex current = m_vbookModel->remove(currentIndex);
        showVBookExpl(current.row());
        ui->vbookTableView->setCurrentIndex(current);
    }
}

void MainWindow::on_vbclearToolButton_clicked()
{
    ui->vbExplLabel->setText("");
    m_vbookModel->clear();
}

void MainWindow::on_vbpreItemTlBtn_clicked()
{
    QModelIndex inx = m_vbookModel->moveUp();
    showVBookExpl(inx.row());
    ui->vbookTableView->setCurrentIndex(inx);

    //    showToolTip(tr("The First Item"));
}

void MainWindow::on_vbnextItemTlBtn_clicked()
{
    QModelIndex inx = m_vbookModel->moveDown();
    showVBookExpl(inx.row());
    ui->vbookTableView->setCurrentIndex(inx);
    //    showToolTip(tr("The Last Item"));
}

void MainWindow::on_vbInput_editingFinished()
{
    QString input = ui->vbInput->text();
    if (input.trimmed().isEmpty())
        return;

    if (m_vbookModel->getMode() == ExamMode) {    
        QString score;
        if (m_vbookModel->exam(input, score)) {
            if (m_vbookModel->isLastRow()) {
                QModelIndex inx = m_vbookModel->moveToFirst();
                showVBookExpl(inx.row());
                ui->vbookTableView->setCurrentIndex(inx);
            } else {
                on_vbnextItemTlBtn_clicked();
            }
            ui->vbInput->clear();
            //ui->vbExamScoreLabel->setText(QString("%1").arg(score));
            //ui->vbExamResultLabel->setPixmap(QPixmap(QString::fromUtf8(":/res/ok.png")));
        } else {
            ui->vbInput->clear();
            //showToolTip(tr("Try Again"));
            //ui->vbExamResultLabel->setPixmap(QPixmap(QString::fromUtf8(":/res/error.png")));
        }

        ui->vbExamScoreLabel->setText(score);
    } else {
        int repeat = ui->vbcmbRepeatCount->currentText().toInt();
        ui->vbInput->clear();
        int ret = m_vbookModel->study(input, repeat);
        if (ret == 0) {
            on_vbnextItemTlBtn_clicked();
        } else if (ret == -1){
            QPoint pos = ui->vbInput->mapToGlobal(QPoint());
            pos.setX(pos.x() + 20);
            pos.setY(pos.y() - ui->vbInput->height() - 20);
            showToolTip(tr("error"), pos, 1000);
        }
    }
}

void MainWindow::on_vbModeComboBox_currentIndexChanged(int index)
{
     /*if (index == 1)
         ui->vbookTableView->hideColumn(0);
     else
         ui->vbookTableView->showColumn(0);*/   
    m_vbookModel->setMode((enum VBookMode)index);
    if(index == 1) { 
        ui->vbdelToolButton->setDisabled(true);
        ui->vbclearToolButton->setDisabled(true);
    } else {
        ui->vbdelToolButton->setDisabled(false);
        ui->vbclearToolButton->setDisabled(false);

        ui->vbExamScoreLabel->setText("");
    }
    ui->vbookTableView->setCurrentIndex(m_vbookModel->curIndex());
}

void MainWindow::on_vbookTableView_clicked(const QModelIndex &index)
{
    QString expl = m_vbookModel->expl(index.row());
    m_vbookModel->setCurrentRow(index.row());
    showVBookExpl(index.row());
}

void MainWindow::on_vbookTableView_activated(const QModelIndex &index)
{
    on_vbookTableView_clicked(index);
}

void MainWindow::showToolTip(QString info, int displayTimeMS)
{
    //showToolTip(info, w, this, displayTimeMS);
    showToolTip(info, QCursor::pos(), displayTimeMS);
}

void MainWindow::showToolTip(QString info, QPoint pos, int displayTimeMS)
{
    QRect rect(0, 0, 120, 80);
    //QFont serifFont("Times", 12, QFont::Bold);
    //QPalette color;
    //color.setColor( QPalette::Inactive,QPalette::QPalette::ToolTipBase, Qt::yellow);
    //QToolTip::setPalette(color);
    //QToolTip::setFont(serifFont);
#if (QT_VERSION >= QT_VERSION_CHECK(5, 2, 1))
    if (displayTimeMS != -1)
        QToolTip::showText(pos, info, NULL, rect, displayTimeMS);
    else
        QToolTip::showText(pos, info, NULL, rect);
#else
    QToolTip::showText(pos, info, NULL, rect);
#endif
}

void MainWindow::onClipboardDataChanged()
{
    if (m_cwdEnableTemp && m_config->m_cws.benable) {
    #ifdef _LINUX
        if (m_config->m_cws.bclipboard) {
    #elif defined(_WINDOWS)
        if ((m_config->m_cws.bclipboard) || 
            (m_config->m_cws.bselection)) {
    #endif
            const QClipboard *clipboard = QApplication::clipboard();
            QString input = clipboard->text(QClipboard::Clipboard).trimmed();
            if (input != "") {
                std::string u8input = std::string(input.toUtf8().data());
                if (Util::isValidInput(u8input)) {
                    //m_capWordDialog->show();
                    g_application.sysMessageQ()->push(MSG_CAPWORD_QUERY, u8input);
                }
            } else if (!m_capWordDialog->isHidden()) {
                m_capWordDialog->close();
            }
        }
    }
}

void MainWindow::onClipboardSelectionChanged()
{
    static QString lastInput = ""; // Sometimes, SelectionChanged occurs when click mouse.

    if (m_config->m_cws.bselection && m_cwdEnableTemp && m_config->m_cws.benable) {
        const QClipboard *clipboard = QApplication::clipboard();

        QString input = clipboard->text(QClipboard::Selection).trimmed();
        if (input != "") {
            if (!m_nonRepeatSelEn || input != lastInput) {
                lastInput = input;
                std::string u8input = std::string(input.toUtf8().data());
                if (Util::isValidInput(u8input)) {
                    //m_capWordDialog->show();
                    g_application.sysMessageQ()->push(MSG_CAPWORD_QUERY, u8input);
                }
	    } else {
                lastInput = ""; // May choose the same word.
            }
        } else if (!m_capWordDialog->isHidden()) {
            m_capWordDialog->close();
        }
        //qDebug() <<  clipboard->text(QClipboard::Selection);
    }
        //qDebug() << QCursor::pos().x() << ":" << QCursor::pos().y();
}

void MainWindow::on_cwsClipboardCheckBox_clicked(bool checked)
{
    m_cwdEnableTemp = true;
    m_config->writeCwsClipboard(checked);
    m_trayCwsClipboardAct->setChecked(checked);
}

void MainWindow::on_cwsSelectionCheckBox_clicked(bool checked)
{
    m_cwdEnableTemp = true;
    m_config->writeCwsSelection(checked);
#ifdef _WINDOWS
    TextOutHookServer::getReference().captureTextEnable(capwordMode());
#endif
    m_trayCwsSelectionAct->setChecked(checked);
}

void MainWindow::on_cwsEnableCheckBox_clicked(bool checked)
{
    m_config->writeCwsEnable(checked);
    if (checked) {
        m_cwdEnableTemp = true;
        this->registerHotkey(m_config->m_cws.shortcutKey);
    } else {
        this->unregisterHotkey(m_config->m_cws.shortcutKey);
    }

#ifdef _WINDOWS
    if (checked) {
        m_cwdEnableTemp = true;
        TextOutHookServer::getReference().inject((HWND)effectiveWinId());
    #ifdef _WINDOWS
        TextOutHookServer::getReference().captureTextEnable(capwordMode());
    #endif
    } else {
        TextOutHookServer::getReference().uninject();
        if (!m_capWordDialog->isHidden()) {
            m_capWordDialog->close();
        }
    }
#endif
    m_trayCwsEnableAct->setChecked(checked);
}

void MainWindow::on_cwsMouseCheckBox_clicked(bool checked)
{
    m_cwdEnableTemp = true;
    m_config->writeCwsMouse(checked);
#ifdef _WINDOWS
    TextOutHookServer::getReference().captureTextEnable(capwordMode());
    m_trayCwsMouseAct->setChecked(checked);
#endif
}

void MainWindow::on_cwsShortcutkeyComboBox_activated(int index)
{
    int old = m_config->m_cws.shortcutKey;
    m_config->writeCwsShortcutKey(index);
    this->unregisterHotkey(old);
    if (m_config->m_cws.benable) {
        this->registerHotkey(m_config->m_cws.shortcutKey);
    }
}

void MainWindow::on_cwsAutoCloseEnCheckBox_clicked(bool checked)
{
    m_config->writeCwsAutoCloseEn(checked);
}

void MainWindow::on_fontsizeComboBox_activated(int index)
{
    m_config->writeFontSize(index+9);   
}

void MainWindow::on_systemTrayCheckBox_clicked(bool checked)
{
    m_config->writeSystemTray(checked);

    if (m_config->m_setting.bsystemTray)
        m_systray->show();
    else
        m_systray->hide();
}

void MainWindow::on_fontComboBox_activated(const QString &arg1)
{
    m_config->writeFont(string(arg1.toUtf8().data()));
    //this->setStatusTip(tr("Need restart to make it valid"));
}

void MainWindow::on_resetSettingToolButton_clicked()
{
	m_setting.clear();
    m_config->reset();
}

void MainWindow::onSysTrayActivated(QSystemTrayIcon::ActivationReason reason)
{
    if (reason == QSystemTrayIcon::Trigger) {
        activateWindow();
        showNormal();
    }
}

void MainWindow::onTrayCwsEnable(bool checked)
{
    on_cwsEnableCheckBox_clicked(checked);
    ui->cwsEnableCheckBox->setCheckState(checked ? Qt::Checked : Qt::Unchecked);
}

void MainWindow::onTrayCwsClipboard(bool checked)
{
    on_cwsClipboardCheckBox_clicked(checked);
    ui->cwsClipboardCheckBox->setCheckState(checked ? Qt::Checked : Qt::Unchecked);
}

void MainWindow::onTrayCwsSelection(bool checked)
{
    on_cwsSelectionCheckBox_clicked(checked);
    ui->cwsSelectionCheckBox->setCheckState(checked ? Qt::Checked : Qt::Unchecked);
}

void MainWindow::onTrayCwsMouse(bool checked)
{
    on_cwsMouseCheckBox_clicked(checked);
    ui->cwsMouseCheckBox->setCheckState(checked ? Qt::Checked : Qt::Unchecked);
}

void MainWindow::onTrayMenuClose()
{
    m_systray->hide();
    close();
}

void MainWindow::onTrayMenuActivated()
{
    onSysTrayActivated( QSystemTrayIcon::Trigger);
}

void MainWindow::keyPressEvent(QKeyEvent *e)
{
    if (e != NULL) {
        if (ui->vbInput->hasFocus()) {
            switch (e->key()) {
            case Qt::Key_Up:
                on_vbpreItemTlBtn_clicked();
                break;
            case Qt::Key_Down:
                on_vbnextItemTlBtn_clicked();
                break;
            /*case Qt::Key_PageUp:
                break;
            case Qt::Key_PageDown:
                break;*/
            }
        }
    }
}

void MainWindow::closeEvent(QCloseEvent * event)
{
	//m_setting.setValue("geometry", saveGeometry());
    if (m_systray->isVisible()) {
        this->hide();
        event->ignore();
    } else {
        event->accept();
    }
}

//bool MainWindow::winEvent(MSG * message, long * result)
bool MainWindow::nativeEvent(const QByteArray & eventType, void * msg, long * result)
{
#ifdef _LINUX
    //XEvent *report = ((XEvent *)msg);
    //int type = report->type;
#elif defined (_WINDOWS)
    //WM_USER : 1024;
    unsigned int message = ((MSG *)msg)->message;
    switch (message) {
    case WM_CW_LBUTTON:
        //g_sysLog.d("WM_CW_LBUTTON\n");
        if (!m_capWordDialog->isHidden()) {
            int x = (int)(((MSG *)msg)->wParam);
            int y = (int)(((MSG *)msg)->lParam);
            //g_sysLog.d("WM_CW_LBUTTON, x,y(%d, %d)\n", x,y);
            QPoint pos(x,y);
            QRect rect = m_capWordDialog->frameGeometry();
            if (!rect.contains(pos)) {
                 m_capWordDialog->close();
            }
            //QMouseEvent event(QEvent::MouseButtonPress, pos, QT::LeftButton, 0, 0);
            //QApplication::sendEvent(m_capWordDialog, &event);      
        }
        break;

    case WM_CW_ERROR:
    {
        DWORD errcode = (DWORD)(((MSG *)msg)->wParam);
        g_sysLog.d("error when capture word on win32, error code(%d)\n", errcode);
        return true;
    }
#if 0
    case WM_CW_SELECTION:
    {
        //m_cwdSelectionMsg = true;
        return true;
    }
#endif
    case WM_CW_TEXTA:
    {
        char strbuf[256];
        TextOutHookServer::getReference().getCaptureText(strbuf, 256, false);

        QString input = QString::fromLocal8Bit((char *)strbuf);
        if (input != "") {
            g_application.sysMessageQ()->push(MSG_CAPWORD_QUERY, std::string(input.toUtf8().data()));
        } else {
            //m_capWordDialog->close();
        }
        return true;
    }

    case WM_CW_TEXTW:
    {
       char strbuf[256];
       TextOutHookServer::getReference().getCaptureText(strbuf, 256, true);
       QString input = QString::fromWCharArray((const wchar_t *)strbuf);
       if (input != "") {
           g_application.sysMessageQ()->push(MSG_CAPWORD_QUERY, std::string(input.toUtf8().data()));
       } else {
           //m_capWordDialog->close();
       }
       return true;
    }

    case WM_HOTKEY:
    {
        if (m_config->m_cws.benable) {
            int identifier = (int)(((MSG *)msg)->wParam);
            //int y = (int)(((MSG *)msg)->lParam); 0x430003
            if (identifier == HOTKEY_IDENTIFY) {
                m_cwdEnableTemp = !m_cwdEnableTemp;
                if (m_cwdEnableTemp && 
                   (m_config->m_cws.bselection || 
                    m_config->m_cws.bmouse)) {
                    TextOutHookServer::getReference().captureTextEnable(capwordMode());
                } else {
                    TextOutHookServer::getReference().captureTextEnable(0);
                    if (!m_capWordDialog->isHidden())
                        m_capWordDialog->close();
                }
            }
        }
        return true;
    }

    case WM_CW_DEBUG: 
    {
        int x = (int)(((MSG *)msg)->wParam);
        int y = (int)(((MSG *)msg)->lParam);
        g_sysLog.d("capture word debug message(%d, %d)\n", x, y);
        return true;
    }
#if 0
    // case SIZE_MINIMIZED
    case WM_CLOSE:
    {
        m_systray->show();
        hide();
        return true;
    }
#endif
    default:
        break;
    }
#endif
    return false;
}

bool MainWindow::eventFilter( QObject * watched, QEvent * event )
{
    if (event->type() == 6/*QEvent::KeyPress*/) {
        QKeyEvent *keyevent = (QKeyEvent *)event;
        int key = keyevent->key();
        Qt::KeyboardModifiers modifier = keyevent->modifiers();
        //printf("native event keypress %d, %d, %d\n", key, m_config->m_cws.shortcutKey, modifier);
        if (key == m_config->m_cws.shortcutKey + 'A' && 
            modifier == (Qt::ControlModifier|Qt::AltModifier)) {
            m_cwdEnableTemp = !m_cwdEnableTemp;
            if (!m_cwdEnableTemp && !m_capWordDialog->isHidden())
                m_capWordDialog->close();
            //X11Util::forwardHotKey(m_config->m_cws.shortcutKey);
            return true;
        }
    }/*else if (event->type() == QEvent::WindowStateChange) {
        if (windowState() == Qt::WindowNoState) {
            static bool windowActive = false;
            if (!windowActive) {
                windowActive = true;
                m_systray->show(); // Have to do this, I don't know why.
                m_systray->hide();
            }
        }
    }*/
    return false;
}

void MainWindow::registerHotkey(int key)
{
#ifdef _WINDOWS
    ::RegisterHotKey((HWND)(this->effectiveWinId()),
                      0x1111, 
                      MOD_CONTROL | MOD_ALT,
                      key+'A');
#elif defined(_LINUX)
    X11Util::registerHotkey(key+'a');
    // QT identifies this key as key+'A'
#endif
    //printf("registerHotkey %d\n", key+'a');
}

void MainWindow::unregisterHotkey(int key)
{
#ifdef _WINDOWS
    UnregisterHotKey((HWND)effectiveWinId(), 0x1111);
#elif defined(_LINUX)
    X11Util::unregisterHotkey(key +'a');
    // QT identifies this key as m_config->m_cws.shortcutKey+'A'
#endif
    //printf("unregisterHotkey %d\n", key+'a');
}

int MainWindow::capwordMode()
{
    int mode = 0;
#ifdef _WINDOWS
    if (m_config->m_cws.bmouse)
        mode |= CAPMODE_MOUSE_OVER;

    if (m_config->m_cws.bselection)
        mode |= CAPMODE_MOUSE_SELECTION;
    
    g_sysLog.d("capwordMode %d\n", mode);
#endif
    return mode;
}

void MainWindow::readHelpText(QString &help)
{
    std::string suffix[] = {".en", ".cn"};
    int lanid = m_config->m_setting.uilanID;
    std::string helpPath = m_config->m_dataDir + "/help" + suffix[lanid];
    FILE *pHelpFile;
    char *bytes;
#ifdef WIN32
   wchar_t *wHelpPath= CharUtil::utf8srtowcs(helpPath.c_str());
   if (wHelpPath != NULL) {
       pHelpFile = _wfopen(wHelpPath, L"rb");
       free(wHelpPath);
   }
#else
    pHelpFile = fopen(helpPath.c_str(),"rb");
#endif
    util::ReadFile read;
    bytes = (char *)read(pHelpFile, -1);
    help = QString::fromUtf8(bytes);
    //qDebug() << help;
}

void MainWindow::lookup(const QString &input)
{
	std::string u8input = std::string(input.toUtf8().data());
	if (Util::isValidInput(u8input)) {
		g_application.sysMessageQ()->push(MSG_DICT_QUERY, u8input);
		ui->dictTextEdit->document()->clear();
	}
}

void MainWindow::onAppExit()
{
   if (m_config->m_cws.benable)
       this->unregisterHotkey(m_config->m_cws.shortcutKey);

#ifdef WIN32
    m_cwdEnableTemp = false;
    TextOutHookServer::getReference().unloadHookLib();
#endif
    (*onSysExit)();
    // After all the tasks are stopped.
    m_messager->abort();
    
    /* May cause crash when dialog on mainwindow, I don't know why, need more check */
    //m_capWordDialog->close();

//    QCoreApplication::quit();
}

void MainWindow::on_nonRepeatSelectionCheckBox_clicked(bool checked)
{
    m_nonRepeatSelEn = checked;
}
