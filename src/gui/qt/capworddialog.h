#ifndef CAPWORDDIALOG_H
#define CAPWORDDIALOG_H

#include <QtWidgets/QDialog>
#include "iDict.h"

#define CWD_X_OFFSET  8
#define CWD_Y_OFFSET  16

class MainWindow;

namespace Ui {
class CapWordDialog;
}

class CapWordDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CapWordDialog(MainWindow *owner, bool autoclose=true, int inv=1000*15);
    ~CapWordDialog();

    void setDictItemList(DictItemList *v);
    void moveToCursor();
    void setAutoClose(bool autoclose) {m_bAutoClose = autoclose;}
    void setAutoCloseInterval(int inv) {m_autoCloseInterval = inv;}

protected:
    bool event(QEvent * event);
   //virtual bool eventFilter( QObject *watched, QEvent *event);

private slots:
    void on_saveToolButton_clicked();

    void on_dictToolButton_clicked();

    void on_autoClose();

private:
    bool m_bAutoClose;
    int  m_autoCloseInterval;
    bool m_bAutoCloseCancle;   
 
    Ui::CapWordDialog *ui;
    MainWindow *m_owner;
};

#endif // CAPWORDDIALOG_H
