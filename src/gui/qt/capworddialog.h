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
    explicit CapWordDialog(MainWindow *owner);
    ~CapWordDialog();

    void setDictItemList(DictItemList *v);
    void moveToCursor();

protected:
    bool event(QEvent * event);
   //virtual bool eventFilter( QObject *watched, QEvent *event);

private slots:
    void on_saveToolButton_clicked();

    void on_dictToolButton_clicked();

private:

    Ui::CapWordDialog *ui;
    MainWindow *m_owner;
};

#endif // CAPWORDDIALOG_H
