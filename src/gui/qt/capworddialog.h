#ifndef CAPWORDDIALOG_H
#define CAPWORDDIALOG_H

#include <QtWidgets/QDialog>

#include "iDict.h"

#define CWD_X_OFFSET  8
#define CWD_Y_OFFSET  16

class MainWindow;
class QTimer;

namespace Ui {
class CapWordDialog;
}

class CapWordDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CapWordDialog(MainWindow *owner, bool autoclose=true, int inv=1000*10);
    virtual ~CapWordDialog();

    //void setDictItemList(DictItemList *v);
    void moveToCursor();
    void setAutoClose(bool autoclose) {m_bAutoClose = autoclose;}
    void setAutoCloseInterval(int inv) {m_autoCloseInterval = inv;}

    void setDictItemList(DictItemList *v);

protected:
    bool event(QEvent * event);
   //virtual bool eventFilter( QObject *watched, QEvent *event);

private slots:
//    void on_saveToolButton_clicked();

//    void on_dictToolButton_clicked();
//    void on_textEdit_customContextMenuRequested(const QPoint &pos);
    void on_addToVbToolButton_clicked();

private:
    MainWindow *m_owner;
    QString m_capword;
    bool m_bAutoClose;
    int  m_autoCloseInterval;
    QTimer *m_timer;
    Ui::CapWordDialog *ui;
};

#endif // CAPWORDDIALOG_H
