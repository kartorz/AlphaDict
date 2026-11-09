#include <QtCore/QTimer>
#include <QtCore/QtMath>
#include <QtGui/QCursor>
#include <QtGui/QGuiApplication>
#include <QtGui/QPainter>
#include <QtGui/QPalette>
#include <QtGui/QScreen>
#include <QtGui/QTextBlockFormat>
#include <QtGui/QTextCursor>
#include <QtGui/QTextDocument>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QVBoxLayout>

#include "capworddialog.h"
#include "Application.h"

namespace {

/* 与 src/gui/qml/Theme.qml 保持一致；QML 切换主题时调用 setDarkTheme()。 */
struct Palette {
    const char *surface;
    const char *surfaceHi;
    const char *border;
    const char *text;
    const char *textDim;
    const char *textMut;
    const char *accent;
    const char *selection;
    const char *scroll;
    const char *scrollHi;
};

const Palette kDark = {
    "#171a22", "#262b37", "#2c3241", "#e7eaf1", "#98a1b3",
    "#6b7385", "#4f8cff", "#2b3a5c", "#39414f", "#4a5468"
};

const Palette kLight = {
    "#ffffff", "#e2e7f1", "#dbe1ec", "#161a22", "#5b6474",
    "#8b93a3", "#4f8cff", "#d6e4ff", "#c6d0e0", "#aab6ca"
};

/* Transparent margin kept for the hand-drawn drop shadow. */
const int kShadow = 12;
const int kRadius = 14;

QString fromUtf8(const std::string &s)
{
    return QString::fromUtf8(s.c_str());
}

} // namespace

// QDialog(owner) causes closing mainwindow when app resides at system tray.
CapWordDialog::CapWordDialog(bool autoclose, int inv, QWidget *parent):
    QDialog(parent),
    m_dark(true),
    m_bAutoClose(autoclose),
    m_autoCloseInterval(inv),
    m_wordLabel(NULL),
    m_phoneticLabel(NULL),
    m_body(NULL),
    m_dictLabel(NULL),
    m_star(NULL)
{
#ifdef WIN32
    setWindowFlags(Qt::FramelessWindowHint | Qt::Popup);
#else
    setWindowFlags(Qt::FramelessWindowHint | Qt::Popup | Qt::Dialog);
#endif
    /* The card paints its own rounded surface. */
    setAttribute(Qt::WA_TranslucentBackground, true);

    buildUi();

    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(close()));
    m_timer->setSingleShot(true);
}

CapWordDialog::~CapWordDialog()
{
    m_timer->stop();
    close();
}

int CapWordDialog::uiFontSize() const
{
    Configure *cfg = g_application.m_configure;
    if (cfg != NULL && cfg->m_setting.fontsize >= UI_FONT_SIZE_MIN)
        return cfg->m_setting.fontsize;
    return UI_FONT_SIZE_DEFAULT;
}

void CapWordDialog::buildUi()
{
    QVBoxLayout *outer = new QVBoxLayout(this);
    outer->setSpacing(0);
    outer->setContentsMargins(0, 0, 0, 0);

    /* Content column, inset by the shadow margin. */
    QWidget *content = new QWidget(this);
    content->setAttribute(Qt::WA_TranslucentBackground, true);
    QVBoxLayout *box = new QVBoxLayout(content);
    box->setContentsMargins(kShadow + 16, kShadow + 12,
                            kShadow + 16, kShadow + 12);
    box->setSpacing(2);

    QHBoxLayout *head = new QHBoxLayout;
    head->setSpacing(8);
    head->setContentsMargins(0, 0, 0, 0);

    m_wordLabel = new QLabel(content);
    m_wordLabel->setTextFormat(Qt::PlainText);

    m_phoneticLabel = new QLabel(content);
    m_phoneticLabel->setTextFormat(Qt::PlainText);

    m_star = new QToolButton(content);
    m_star->setText(QString::fromUtf8("\xe2\x98\x85"));   /* ★ */
    m_star->setToolTip(tr("Add to vocabulary book"));
    m_star->setCursor(Qt::PointingHandCursor);
    m_star->setAutoRaise(true);
    m_star->setFocusPolicy(Qt::NoFocus);
    connect(m_star, SIGNAL(clicked()), this, SLOT(on_addToVbToolButton_clicked()));

    head->addWidget(m_wordLabel);
    head->addWidget(m_phoneticLabel, 0, Qt::AlignBottom);
    head->addStretch(1);
    head->addWidget(m_star, 0, Qt::AlignTop);

    m_body = new QTextEdit(content);
    m_body->setReadOnly(true);
    m_body->setFrameShape(QFrame::NoFrame);
    m_body->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_body->viewport()->setAutoFillBackground(false);

    m_dictLabel = new QLabel(content);
    m_dictLabel->setTextFormat(Qt::PlainText);
    m_dictLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    box->addLayout(head);
    box->addWidget(m_body, 1);
    box->addWidget(m_dictLabel);

    outer->addWidget(content);

    applyTheme();
    fitToContent();
}

void CapWordDialog::setDarkTheme(bool dark)
{
    if (m_dark == dark)
        return;
    m_dark = dark;
    applyTheme();
}

void CapWordDialog::applyTheme()
{
    const Palette &c = m_dark ? kDark : kLight;

    m_wordLabel->setStyleSheet(QString("color:%1;").arg(c.text));
    m_phoneticLabel->setStyleSheet(QString("color:%1;").arg(c.textDim));
    m_dictLabel->setStyleSheet(QString("color:%1;").arg(c.textMut));

    m_star->setStyleSheet(QString(
        "QToolButton{border:none;background:transparent;color:%1;"
        "border-radius:8px;}"
        "QToolButton:hover{background:%2;color:%3;}")
        .arg(c.textDim, c.surfaceHi, c.accent));

    m_body->setStyleSheet(QString(
        "QTextEdit{background:transparent;border:none;padding:0px;color:%1;}"
        "QScrollBar:vertical{background:transparent;width:8px;margin:0px;}"
        "QScrollBar::handle:vertical{background:%2;border-radius:4px;"
        "min-height:28px;}"
        "QScrollBar::handle:vertical:hover{background:%3;}"
        "QScrollBar::add-line:vertical,QScrollBar::sub-line:vertical{height:0px;}"
        "QScrollBar::add-page:vertical,QScrollBar::sub-page:vertical{"
        "background:transparent;}")
        .arg(c.text, c.scroll, c.scrollHi));

    QPalette pal = m_body->palette();
    pal.setColor(QPalette::Text, QColor(c.text));
    pal.setColor(QPalette::Highlight, QColor(c.selection));
    pal.setColor(QPalette::HighlightedText, QColor(c.text));
    m_body->setPalette(pal);

    update();
}

void CapWordDialog::fitToContent()
{
    const int base = uiFontSize();
    const int small = qMax(11, base - 2);
    const int tiny  = qMax(10, base - 4);

    QFont wf = font();
    wf.setPixelSize(base + 4);
    wf.setWeight(QFont::DemiBold);
    m_wordLabel->setFont(wf);

    QFont pf = font();
    pf.setPixelSize(qMax(11, base - 3));
    m_phoneticLabel->setFont(pf);

    QFont bf = font();
    bf.setPixelSize(small);
    m_body->document()->setDefaultFont(bf);
    m_body->document()->setDocumentMargin(0);

    QFont df = font();
    df.setPixelSize(tiny);
    m_dictLabel->setFont(df);

    const int star = qMax(26, base + 6);
    QFont sf = font();
    sf.setPixelSize(qMax(16, base));
    m_star->setFont(sf);
    m_star->setFixedSize(star, star);

    /* Width follows the font size so the card scales with the UI setting. */
    const int width = qBound(380, base * 26, 680);
    const int textWidth = width - 2 * (kShadow + 16) - 10;

    m_body->document()->setTextWidth(textWidth);
    /* Document height for the wrapped width. */
    int bodyH = qCeil(m_body->document()->size().height());

    const int headH = qMax(m_wordLabel->sizeHint().height(),
                           qMax(star, m_phoneticLabel->sizeHint().height()));
    int dictH = 0;
    if (!m_dictLabel->isHidden())
        dictH = m_dictLabel->sizeHint().height();
    const int chrome = 2 * (kShadow + 12) + 16;

    int height = chrome + headH + bodyH + dictH;
    const int minH = base * 4;
    int maxH = base * 22;
    QScreen *screen = QGuiApplication::primaryScreen();
    if (screen != NULL)
        maxH = qMin(maxH, (int)(screen->availableGeometry().height() * 0.6));
    height = qBound(minH, height, qMax(minH, maxH));

    /* Body takes the slack; the card never crowds the shadow margin. */
    m_body->setFixedHeight(qMax(small * 2, height - chrome - headH - dictH));
    setFixedSize(width, height);
}

void CapWordDialog::showAtCursor()
{
    moveToCursor();
    show();
    activateWindow();
    raise();
}

void CapWordDialog::moveToCursor()
{
    QPoint pos = QCursor::pos() + QPoint(CWD_X_OFFSET, CWD_Y_OFFSET);

    /* Keep the whole card on screen: the cursor may sit near an edge. */
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
    QScreen *screen = QGuiApplication::screenAt(QCursor::pos());
#else
    QScreen *screen = NULL;
#endif
    if (screen == NULL)
        screen = QGuiApplication::primaryScreen();
    if (screen != NULL) {
        const QRect avail = screen->availableGeometry();
        if (pos.x() + width() > avail.right())
            pos.setX(avail.right() - width());
        if (pos.y() + height() > avail.bottom())
            pos.setY(avail.bottom() - height());
        pos.setX(qMax(avail.left(), pos.x()));
        pos.setY(qMax(avail.top(),  pos.y()));
    }
    move(pos);
}

void CapWordDialog::setDictItemList(DictItemList *itemList)
{
    if (itemList == NULL || itemList->empty()) {
        delete itemList;
        return;
    }

    m_capword = fromUtf8((*itemList)[0].word);
    m_wordLabel->setText(m_capword);

    QString phonetic = fromUtf8((*itemList)[0].phonetic).trimmed();
    phonetic.replace('\n', "    ");
    m_phoneticLabel->setText(phonetic);
    m_phoneticLabel->setVisible(!phonetic.isEmpty());

    /* Dictionary entries are padded with blank lines; keeping them would
     * only stretch the card. */
    const QStringList lines = fromUtf8((*itemList)[0].expl).split('\n');
    QStringList kept;
    for (int i = 0; i < lines.size(); ++i) {
        const QString line = lines[i].trimmed();
        if (!line.isEmpty())
            kept.append(line);
    }
    m_body->setPlainText(kept.join("\n"));

    /* Breathing room between the explanation lines. */
    QTextCursor cursor(m_body->document());
    cursor.select(QTextCursor::Document);
    QTextBlockFormat blockFmt;
    blockFmt.setLineHeight(135, QTextBlockFormat::ProportionalHeight);
    cursor.mergeBlockFormat(blockFmt);
    m_body->moveCursor(QTextCursor::Start);

    QString dictname = fromUtf8((*itemList)[0].dictname).trimmed();
    m_dictLabel->setText(dictname);
    m_dictLabel->setVisible(!dictname.isEmpty());

    fitToContent();

    if (m_bAutoClose)
        m_timer->start(m_autoCloseInterval);

    delete itemList;
}

void CapWordDialog::paintEvent(QPaintEvent *event)
{
    (void)event;
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);

    const QRectF card = QRectF(rect()).adjusted(kShadow, kShadow,
                                                -kShadow, -kShadow);

    /* Fake soft shadow: the window is translucent, so a few nested rounded
     * rects read as a blur without a compositor effect. */
    p.setPen(Qt::NoPen);
    for (int i = kShadow; i >= 1; --i) {
        p.setBrush(QColor(0, 0, 0, 4 + (kShadow - i)));
        p.drawRoundedRect(card.adjusted(-i, -i, i, i), kRadius + i, kRadius + i);
    }

    const Palette &c = m_dark ? kDark : kLight;
    p.setBrush(QColor(c.surface));
    p.setPen(QPen(QColor(c.border), 1));
    p.drawRoundedRect(card.adjusted(0.5, 0.5, -0.5, -0.5), kRadius, kRadius);
}

bool CapWordDialog::event(QEvent * event)
{
    switch (event->type()) {
    case QEvent::ActivationChange:
        if(QApplication::activeWindow() != this)
            close();
        break;

    case QEvent::Close:
        m_timer->stop();
        break;

    case QEvent::Enter:
        m_timer->stop();
        activateWindow();
        raise();
        break;

    case QEvent::Leave:
        if (m_bAutoClose)
            m_timer->start(m_autoCloseInterval);
        break;

    default:
        break;
    }
    return QWidget::event(event);
}

void CapWordDialog::on_addToVbToolButton_clicked()
{
    if (m_capword != "")
        emit addToVBook(m_capword);
}
