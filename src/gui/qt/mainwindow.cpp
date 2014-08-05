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
#ifdef WIN32
#include "win32/TextOutHookServer.h"
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
    //setFocus();
    //QIcon icon("app.ico"); 
    //setWindowIcon(icon);
    qApp->installEventFilter(this);
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

    if (m_config->m_cws.benable) {
        X11Util::registerHotkey(m_config->m_cws.shortcutKey+'a');
    }
}

MainWindow::~MainWindow()
{
    m_messager->abort();

    delete m_messager;
    delete ui;
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
    if (m_capWordDialog == NULL) { 
        m_capWordDialog = new CapWordDialog(this);
    } else {
        m_capWordDialog->moveToCursor();
    }

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
            ui->uilanComboBox->setCurrentIndex(m_config->m_uilanID);
            ui->cwsShortcutkeyComboBox->setCurrentIndex(m_config->m_cws.shortcutKey);

            Qt::CheckState ckstate = m_config->m_cws.bselection ? Qt::Checked : Qt::Unchecked;
            ui->cwsSelectionCheckBox->setCheckState(ckstate);

            ckstate = m_config->m_cws.bclipboard ? Qt::Checked : Qt::Unchecked;
            ui->cwsClipboardCheckBox->setCheckState(ckstate);

            ckstate = m_config->m_cws.benable ? Qt::Checked : Qt::Unchecked;
            ui->cswEnableCheckBox->setCheckState(ckstate);
        #ifdef _LINUX
            ui->cwsMouseCheckBox->hide();
        #elif defined(_WINDOWS)
            ckstate = m_config->m_cws.bmouse ? Qt::Checked : Qt::Unchecked;
            ui->cwsMouseCheckBox->setCheckState(ckstate);
        #endif
        }
        //QIcon icon;
        //icon.addFile(QStringLiteral(":/res/setting.png"), QSize(), QIcon::Normal, QIcon::Off);
        inx = ui->tabWidget->addTab(ui->settingTab, QApplication::translate("MainWindow", "setting", 0));
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
        inx = ui->tabWidget->addTab(ui->vocabularyTab, QApplication::translate("MainWindow", "vbook", 0));
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
    ui->vbExplLabel->setText(m_vbookModel->preExamExpl());
}

void MainWindow::on_vbnextItemTlBtn_clicked()
{
    ui->vbExplLabel->setText(m_vbookModel->nextExamExpl());
}

void MainWindow::on_vbookListView_clicked(const QModelIndex &index)
{
    QString expl = m_vbookModel->expl(index.row());
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
    if (m_config->m_cws.bclipboard && m_cwdEnableTemp && m_config->m_cws.benable) {
        const QClipboard *clipboard = QApplication::clipboard();
        
        QString input = clipboard->text(QClipboard::Clipboard).trimmed();
        if (input != "") {
            m_capword = input;
            g_application.sysMessageQ()->push(MSG_CAPWORD_QUERY, std::string(input.toUtf8().data()));
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
        }
    }
    //qDebug() <<  clipboard->text(QClipboard::Selection);
    //qDebug() << QCursor::pos().x() << ":" << QCursor::pos().y();
}

void MainWindow::on_cwsClipboardCheckBox_clicked(bool checked)
{
    m_config->writeCwsClipboard(checked);
}

void MainWindow::on_cwsSelectionCheckBox_clicked(bool checked)
{
    m_config->writeCwsSelection(checked);
}

void MainWindow::on_cswEnableCheckBox_clicked(bool checked)
{
    m_config->writeCwsEnable(checked);
#ifdef _WINDOWS
    if (checked) {
        TextOutHookServer::getReference().inject((HWND)effectiveWinId(), 0);
    } else {
        TextOutHookServer::getReference().uninject();
    }
#elif defined(_LINUX)
    if (checked) {
        X11Util::registerHotkey(m_config->m_cws.shortcutKey+'a');
    } else {
        X11Util::unregisterHotkey(m_config->m_cws.shortcutKey+'a');
    }    
#endif
}

void MainWindow::on_cwsMouseCheckBox_clicked(bool checked)
{
    m_config->writeCwsMouse(checked);
}

void MainWindow::on_cwsShortcutkeyComboBox_activated(int index)
{
    m_config->writeCwsShortcutKey(index);
    if (m_config->m_cws.benable) {
        X11Util::unregisterHotkey(m_config->m_cws.shortcutKey+'a');
        X11Util::registerHotkey(m_config->m_cws.shortcutKey+'a');
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
        if (m_capWordDialog != NULL) {
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

    case WM_CW_TEXTW:
    {
       char *strbuf = TextOutHookServer::getReference().getCaptureText(true);
       QString input = QString::fromWCharArray((const wchar_t *)strbuf);
       if (input != "") {
           m_capword = input;
           g_application.sysMessageQ()->push(MSG_CAPWORD_QUERY, std::string(input.toUtf8().data()));
       } else {
           if (m_capWordDialog != NULL) {
               m_capWordDialog->close();
           }
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
    // case WM_CLOSE
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
        if (key == m_config->m_cws.shortcutKey + 0x41 && 
            modifier == (Qt::ControlModifier|Qt::AltModifier)) {
            m_cwdEnableTemp = !m_cwdEnableTemp;
            //X11Util::forwardHotKey(m_config->m_cws.shortcutKey+'a');
            return true;
        }
    }
    return false;
}

void MainWindow::onAppExit()
{
   if (m_capWordDialog != NULL) 
        m_capWordDialog->close();

   if (m_config->m_cws.benable)
        X11Util::unregisterHotkey(m_config->m_cws.shortcutKey+'a');

#ifdef WIN32
    TextOutHookServer::getReference().uninject();
#endif
    (*onSysExit)();
//    QCoreApplication::quit();
}

#if 0
void MainWindow::OnSysTrayActivated(QSystemTrayIcon::ActivationReason reason)
{
    activateWindow();
    showNormal();
}
#endif
