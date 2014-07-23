#ifndef CAPWORDDIALOG_H
#define CAPWORDDIALOG_H

#include <QtWidgets/QDialog>

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

    void setText(void *v);

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
