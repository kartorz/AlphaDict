#include <QtCore/QTimer>

#include "capworddialog.h"
#include "ui_capworddialog.h"
#include "VBookModel.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "Log.h"

// QDialog(owner) causes closing mainwindow when app resides at system tray. 
CapWordDialog::CapWordDialog(MainWindow *owner, bool autoclose, int inv):
    QDialog(NULL),
    ui(new Ui::CapWordDialog),
    m_bAutoClose(autoclose),
    m_autoCloseInterval(inv)
{
    ui->setupUi(this);
    setWindowFlags(Qt::FramelessWindowHint | Qt::Popup);
    //setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    //setAttribute(Qt::WA_TranslucentBackground, true);
    //move(QCursor::pos().x()+CWD_X_OFFSET, QCursor::pos().y()+CWD_Y_OFFSET);
    //qApp->installEventFilter(this);
    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(close()));
    m_timer->setSingleShot(true);
}

CapWordDialog::~CapWordDialog()
{
    m_timer->stop();
    close();
    g_sysLog.d("~CapWordDialog\n");
    delete ui;
}

void CapWordDialog::moveToCursor()
{
    move(QCursor::pos().x()+CWD_X_OFFSET, QCursor::pos().y()+CWD_Y_OFFSET);
}

void CapWordDialog::setDictItemList(QString &word, DictItemList *itemList)
{
    //DictItemList* itemList = (DictItemList*) v;
    m_capword = word;

    ui->textEdit->clear();

    QTextCursor cursor(ui->textEdit->textCursor());
    QTextCharFormat titleFormat;
    QTextCharFormat bodyFormat;
    QTextBlockFormat itemBlock;
    QString text;

    titleFormat.setFontWeight(QFont::DemiBold);
    //cursor.insertBlock();
    text = QString::fromUtf8((*itemList)[0].word.c_str());
    cursor.insertText(text, titleFormat);
    text = QString::fromUtf8((*itemList)[0].phonetic.c_str());
    text = text.trimmed();
    if (text != "") {
        cursor.insertBlock();
        cursor.insertText(text, titleFormat);
    }
    cursor.insertBlock();

    bodyFormat.setFontWeight(QFont::Light);
    cursor.insertBlock(itemBlock);
    text = QString::fromUtf8((*itemList)[0].expl.c_str());
    text = text.trimmed();
    cursor.insertText(text,bodyFormat);
    ui->textEdit->moveCursor(QTextCursor::Start);
    
    if (m_bAutoClose)
        m_timer->start(m_autoCloseInterval);

    delete itemList;
}

#if 0
void CapWordDialog::on_saveToolButton_clicked()
{
    QString word = m_capword;
    if (word != "") {
    }
}

void CapWordDialog::on_dictToolButton_clicked()
{
    close();
}
#endif

bool CapWordDialog::event(QEvent * event)
{
    switch (event->type()) {
    case QEvent::ActivationChange:
        if(QApplication::activeWindow() != this)
            close();
        break;

    case QEvent::Close:
    case QEvent::Enter:
        m_timer->stop();
        break;

    default:
        break;
    }
    return QWidget::event(event);
}

#if 0
bool CapWordDialog::eventFilter( QObject * watched, QEvent * event )
{
    //g_sysLog.e("event type2 %d\n", event->type());
    return false;
}
#endif
