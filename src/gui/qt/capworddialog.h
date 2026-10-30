#ifndef CAPWORDDIALOG_H
#define CAPWORDDIALOG_H

#include <QtWidgets/QDialog>

#include "iDict.h"

#define CWD_X_OFFSET  8
#define CWD_Y_OFFSET  16

class QTimer;

namespace Ui {
class CapWordDialog;
}

class CapWordDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CapWordDialog(bool autoclose=true, int inv=1000*3.5, QWidget *parent=0);
    virtual ~CapWordDialog();

    //void setDictItemList(DictItemList *v);
    void moveToCursor();
    void setAutoClose(bool autoclose) {m_bAutoClose = autoclose;}
    void setAutoCloseInterval(int inv) {m_autoCloseInterval = inv;}

    void setDictItemList(DictItemList *v);
    void showAtCursor();

signals:
    void addToVBook(const QString &word);

protected:
    bool event(QEvent * event);
   //virtual bool eventFilter( QObject *watched, QEvent *event);

private slots:
//    void on_saveToolButton_clicked();

//    void on_dictToolButton_clicked();
//    void on_textEdit_customContextMenuRequested(const QPoint &pos);
    void on_addToVbToolButton_clicked();

private:
    QString m_capword;
    bool m_bAutoClose;
    int  m_autoCloseInterval;
    QTimer *m_timer;
    Ui::CapWordDialog *ui;
};

#endif // CAPWORDDIALOG_H
