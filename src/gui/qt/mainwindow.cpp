#include <stdio.h>
#include <QtWidgets/QScrollBar>
#include <QtWidgets/QToolTip>
#include <QtWidgets/QShortcut> 
#include <QtCore/QMimeData>
#include <QtCore/QDebug>
#include <QtCore/QEvent>
#include <QtGui/QCursor>

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
    m_capWordDialog(NULL),
    m_cwdEnableTemp(true)
{
    ui->setupUi(this);
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

    m_config = g_application.m_configure;
    
    m_dictIndexModel = new DictIndexModel();
    ui->indexListView->setModel(m_dictIndexModel);

    m_vbookModel = new VBookModel(m_config->getVBPath());
    ui->vbookListView->setModel(m_vbookModel);

    m_messager = new QtMessager(this, m_dictIndexModel, g_application.uiMessageQ());
    m_messager->start();
    
    ui->tabWidget->removeTab(1);
    m_initSettingPage = false;

    ui->tabWidget->removeTab(1);
    
    ui->tabWidget->removeTab(1);
    m_initHelpPage = false;
    //setFocus();
    //QIcon icon("app.ico"); 
    //setWindowIcon(icon);
    qApp->installEventFilter(this);

    m_capWordDialog = new CapWordDialog(this, m_config->m_cws.autoCloseEn);
#if 0
    m_systray = new QSystemTrayIcon(this);
    QIcon icon("trayicon.png"); 
    m_systray->setIcon(icon);
    m_systray->show();
    connect(m_systray,
            SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
            this,
            SLOT(OnSysTrayActivated(QSystemTrayIcon::ActivationReason)));

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
    g_log.d("~MainWindow\n");
}

void MainWindow::initDelay()
{
    if (m_config->m_cws.benable) {
        registerHotkey(m_config->m_cws.shortcutKey);
    #ifdef _WINDOWS
        TextOutHookServer::getReference().inject((HWND)effectiveWinId());
        TextOutHookServer::getReference().captureTextEnable(capwordMode());
    #endif
    }
}

void MainWindow::on_srcLanComboBox_currentIndexChanged(const QString &arg1)
{
    g_application.sysMessageQ()->push(MSG_SET_SRCLAN, std::string(arg1.toUtf8().data()));
}

void MainWindow::on_detLanComboBox_currentIndexChanged(const QString &arg1)
{
    g_application.sysMessageQ()->push(MSG_SET_DETLAN, std::string(arg1.toUtf8().data()));
}

void MainWindow::on_inputLineEdit_editingFinished()
{
    on_queryButton_clicked();
}

void MainWindow::on_inputLineEdit_textChanged(const QString &arg1)
{

}

void MainWindow::on_queryButton_clicked()
{
    QString input = ui->inputLineEdit->text().trimmed();
	g_application.sysMessageQ()->push(MSG_DICT_QUERY, std::string(input.toUtf8().data()));
    ui->dictTextEdit->document()->clear();
}

void MainWindow::on_indexListView_clicked(const QModelIndex &index)
{
    iIndexItem* item = m_dictIndexModel->item(index.row());
    if (item != NULL) {
	    g_application.sysMessageQ()->push(MSG_DICT_INDEX_QUERY, index.row(), (void *)(m_dictIndexModel->item(index.row())));
		ui->dictTextEdit->document()->clear();
		QString text = QString::fromUtf8(item->index.c_str());
		ui->inputLineEdit->setText(text);
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
    //cursor.insertBlock(itemBlock);

    for (unsigned i=0; i< itemList->size(); i++) {
        cursor.insertBlock();
        titleFormat.setFontWeight(QFont::DemiBold);
        text = QString::fromUtf8((*itemList)[i].phonetic.c_str());
        text = text.trimmed();
        if (text != "") {
            cursor.insertBlock();
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
    DictItemList* itemList = (DictItemList*) v;

    //QPoint pos = QCursor::pos();
    m_capWordDialog->moveToCursor();
    m_capWordDialog->setDictItemList(itemList);
    //dlg->exec();
    m_capWordDialog->show();
    m_capWordDialog->raise();
    m_capWordDialog->activateWindow();
    //::SetCapture() /* win32 -- it seems not work */
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

void MainWindow::on_saveButton_clicked()
{
    QString word = ui->inputLineEdit->text();
    
    if (word == "") {
        showToolTip(tr("empty string"));
        return;
    }
    if (m_vbookModel->add(word)) {
        showToolTip(tr("add to vocabulary book,success"));
    } else {
        showToolTip(tr("add to vocabulary book,failure"));
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

            Qt::CheckState ckstate = m_config->m_cws.bselection ? Qt::Checked : Qt::Unchecked;
            ui->cwsSelectionCheckBox->setCheckState(ckstate);

            ckstate = m_config->m_cws.bclipboard ? Qt::Checked : Qt::Unchecked;
            ui->cwsClipboardCheckBox->setCheckState(ckstate);

            ckstate = m_config->m_cws.benable ? Qt::Checked : Qt::Unchecked;
            ui->cwsEnableCheckBox->setCheckState(ckstate);

            ckstate = m_config->m_cws.autoCloseEn ? Qt::Checked : Qt::Unchecked;
            ui->cwsAutoCloseEnCheckBox->setCheckState(ckstate);

        #ifdef _LINUX
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
        ui->vbexplTextEdit->setPlainText("");
        ui->vbExplLabel->setText(m_vbookModel->curExamExpl());
        //ui->vbScoreLabel->setText("0");
        //QIcon icon;
        //icon.addFile(QStringLiteral(":/res/vocabulary.png"), QSize(), QIcon::Normal, QIcon::Off);
        inx = ui->tabWidget->addTab(ui->vocabularyTab, QApplication::translate("MainWindow", "VocabulayBook", 0));
        ui->tabWidget->setCurrentIndex(inx);
    } else {
        if (ui->tabWidget->currentIndex() == inx)
            ui->tabWidget->removeTab(inx);
        else
            ui->tabWidget->setCurrentIndex(inx);
    }
}

void MainWindow::onActionHelpPageAdded()
{
    int inx = ui->tabWidget->indexOf(ui->helpTab);
    if (inx == -1) {
        if (!m_initHelpPage) {
            QString help;
            m_initHelpPage = true;
            readHelpText(help);
            ui->helpTextEdit->setPlainText(help);
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

void MainWindow::on_vocabularyTabWidget_currentChanged(int index)
{
    if (index == 1) {
        ui->vbexplTextEdit->setPlainText("");
        ui->vbExplLabel->setText(m_vbookModel->curExamExpl());
    }
}

void MainWindow::on_vbdelToolButton_clicked()
{
    int currentIndex = ui->vbookListView->currentIndex().row();
    if (currentIndex != -1) {
        QModelIndex current = m_vbookModel->remove(currentIndex);
        ui->vbexplTextEdit->setPlainText("");
        ui->vbookListView->setCurrentIndex(current);
    }
}

void MainWindow::on_vbclearToolButton_clicked()
{
    ui->vbexplTextEdit->setPlainText("");
    m_vbookModel->clear();
}

void MainWindow::on_vbInput_editingFinished()
{
    QString input = ui->vbInput->text();
    int score = 0;
    //if (m_vbookModel->testInput(input, score)) {
    //ui->vbExplLabel->setText(QString("%1").arg(score));
    if (m_vbookModel->testInput(input, score)) {
        on_vbnextItemTlBtn_clicked();
        ui->vbInput->clear();
    } else {
        showToolTip(tr("try again"));        
    }
}

void MainWindow::on_vbpreItemTlBtn_clicked()
{
    QString text;
    if (m_vbookModel->preExamExpl(text)) {
        ui->vbExplLabel->setText(text);
    } else {
        showToolTip(tr("the first item"));
    }
}

void MainWindow::on_vbnextItemTlBtn_clicked()
{
    QString text;
    if (m_vbookModel->nextExamExpl(text)) {
        ui->vbExplLabel->setText(text);
    } else {
        showToolTip(tr("the last item"));
    }
}

void MainWindow::on_vbookListView_clicked(const QModelIndex &index)
{
    QString expl = m_vbookModel->expl(index.row());
    m_vbookModel->setCurrentRow(index.row());
    expl.trimmed();
    ui->vbexplTextEdit->setPlainText(expl);

}

void MainWindow::on_vbookListView_activated(const QModelIndex &index)
{
    on_vbookListView_clicked(index);
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
        if (m_config->m_cws.bclipboard || m_config->m_cws.bselection) {
    #endif
            const QClipboard *clipboard = QApplication::clipboard();
            QString input = clipboard->text(QClipboard::Clipboard).trimmed();
            if (input != "") {
                m_capword = input;
                g_application.sysMessageQ()->push(MSG_CAPWORD_QUERY, std::string(input.toUtf8().data()));
            }
        }
    }
}

void MainWindow::onClipboardSelectionChanged()
{
    if (m_config->m_cws.bselection && m_cwdEnableTemp && m_config->m_cws.benable) {
        const QClipboard *clipboard = QApplication::clipboard();
        
        QString input = clipboard->text(QClipboard::Selection).trimmed();
        if (input != "") {
            m_capword = input;
            g_application.sysMessageQ()->push(MSG_CAPWORD_QUERY, std::string(input.toUtf8().data()));
        } else if (!m_capWordDialog->isHidden()) {
            m_capWordDialog->close();
        }
    }
    //qDebug() <<  clipboard->text(QClipboard::Selection);
    //qDebug() << QCursor::pos().x() << ":" << QCursor::pos().y();
}

void MainWindow::on_cwsClipboardCheckBox_clicked(bool checked)
{
    m_cwdEnableTemp = true;
    m_config->writeCwsClipboard(checked);
}

void MainWindow::on_cwsSelectionCheckBox_clicked(bool checked)
{
    m_cwdEnableTemp = true;
    m_config->writeCwsSelection(checked);
#ifdef _WINDOWS
    TextOutHookServer::getReference().captureTextEnable(capwordMode());
#endif
}

void MainWindow::on_cwsEnableCheckBox_clicked(bool checked)
{
    m_config->writeCwsEnable(checked);
    if (checked) {
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
}

void MainWindow::on_cwsMouseCheckBox_clicked(bool checked)
{
    m_cwdEnableTemp = true;
    m_config->writeCwsMouse(checked);
#ifdef _WINDOWS
    TextOutHookServer::getReference().captureTextEnable(capwordMode());
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
        if (!m_capWordDialog->isHidden()) {
            int x = (int)(((MSG *)msg)->wParam);
            int y = (int)(((MSG *)msg)->lParam);
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
        g_log.d("error when capture word on win32, error code(%d)\n", errcode);
        break;
    }

    case WM_CW_TEXTA:
    {
       char *strbuf = TextOutHookServer::getReference().getCaptureText(true);
       QString input = QString::fromLocal8Bit((char *)strbuf);
       if (input != "") {
           m_capword = input;
           g_application.sysMessageQ()->push(MSG_CAPWORD_QUERY, std::string(input.toUtf8().data()));
       } else {
           m_capWordDialog->close();
       }
       break;
    }

    case WM_CW_TEXTW:
    {
       char *strbuf = TextOutHookServer::getReference().getCaptureText(true);
       QString input = QString::fromWCharArray((const wchar_t *)strbuf);
       if (input != "") {
           m_capword = input;
           g_application.sysMessageQ()->push(MSG_CAPWORD_QUERY, std::string(input.toUtf8().data()));
       } else {
           m_capWordDialog->close();
       }
       break;
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
                    m_capWordDialog->close();
                }
            }
            //g_log.d("win hotkey %d, %d\n", x,y);
        }
        break;
    }

    case WM_CW_DEBUG: 
    {
        int x = (int)(((MSG *)msg)->wParam);
        int y = (int)(((MSG *)msg)->lParam);
        g_log.d("capture word debug message(%d, %d)\n", x, y);
        break;
    }
    // case SIZE_MINIMIZED
    // case WM_CL OSE
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
            if (!m_cwdEnableTemp)
                m_capWordDialog->close();
            //X11Util::forwardHotKey(m_config->m_cws.shortcutKey);
            return true;
        }
    }
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
   }
#else
    pHelpFile = fopen(helpPath.c_str(),"rb");
#endif
    util::ReadFile read;
    bytes = (char *)read(pHelpFile, -1);
    help = QString::fromUtf8(bytes);
    //qDebug() << help;
}

void MainWindow::onAppExit()
{
   m_capWordDialog->close();

   if (m_config->m_cws.benable)
       this->unregisterHotkey(m_config->m_cws.shortcutKey);

#ifdef WIN32
    TextOutHookServer::getReference().uninject();
#endif
    (*onSysExit)();
    // After all the tasks are stopped.
    m_messager->abort();
//    QCoreApplication::quit();
}

#if 0
void MainWindow::OnSysTrayActivated(QSystemTrayIcon::ActivationReason reason)
{
    activateWindow();
    showNormal();
}
#endif
