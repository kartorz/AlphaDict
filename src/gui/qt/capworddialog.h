#ifndef CAPWORDDIALOG_H
#define CAPWORDDIALOG_H

#include <QtWidgets/QDialog>

#include "iDict.h"

#define CWD_X_OFFSET  8
#define CWD_Y_OFFSET  16

class QLabel;
class QTextEdit;
class QTimer;
class QToolButton;

/* Frameless card shown at the cursor by the screen word capture.  It is drawn
 * by hand (rounded surface, no window frame, no focus ring) so it matches the
 * QML front-end instead of the platform widget style. */
class CapWordDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CapWordDialog(bool autoclose=true, int inv=1000*3.5, QWidget *parent=0);
    virtual ~CapWordDialog();

    void moveToCursor();
    void setAutoClose(bool autoclose) {m_bAutoClose = autoclose;}
    void setAutoCloseInterval(int inv) {m_autoCloseInterval = inv;}

    void setDictItemList(DictItemList *v);
    void showAtCursor();

signals:
    void addToVBook(const QString &word);

protected:
    bool event(QEvent * event) override;
    void paintEvent(QPaintEvent *event) override;

private slots:
    void on_addToVbToolButton_clicked();

private:
    int  uiFontSize() const;
    void buildUi();
    void fitToContent();

    QString m_capword;
    bool m_bAutoClose;
    int  m_autoCloseInterval;
    QTimer *m_timer;

    QLabel      *m_wordLabel;
    QLabel      *m_phoneticLabel;
    QTextEdit   *m_body;
    QLabel      *m_dictLabel;
    QToolButton *m_star;
};

#endif // CAPWORDDIALOG_H
