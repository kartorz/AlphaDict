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
//    explicit CapWordDialog(QWidget *parent = 0);
    explicit CapWordDialog(MainWindow *owner);
    ~CapWordDialog();

    void setText(void *v);

protected:
    //virtual void mousePressEvent(QMouseEvent *event);
//    virtual void focusOutEvent(QFocusEvent * event);
    bool event(QEvent * event);

private slots:
    void on_saveToolButton_clicked();

    void on_dictToolButton_clicked();

private:

    Ui::CapWordDialog *ui;
    MainWindow *m_owner;
};

#endif // CAPWORDDIALOG_H
