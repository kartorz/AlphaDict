/********************************************************************************
** Form generated from reading UI file 'capworddialog.ui'
**
** Created by: Qt User Interface Compiler version 5.3.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_CAPWORDDIALOG_H
#define UI_CAPWORDDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QDialog>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QTextEdit>

QT_BEGIN_NAMESPACE

class Ui_CapWordDialog
{
public:
    QTextEdit *textEdit;

    void setupUi(QDialog *CapWordDialog)
    {
        if (CapWordDialog->objectName().isEmpty())
            CapWordDialog->setObjectName(QStringLiteral("CapWordDialog"));
        CapWordDialog->resize(380, 180);
        CapWordDialog->setToolTipDuration(10000);
        CapWordDialog->setAutoFillBackground(false);
        CapWordDialog->setSizeGripEnabled(false);
        CapWordDialog->setModal(false);
        textEdit = new QTextEdit(CapWordDialog);
        textEdit->setObjectName(QStringLiteral("textEdit"));
        textEdit->setGeometry(QRect(0, 0, 380, 180));
        textEdit->setAcceptDrops(false);
#ifndef QT_NO_STATUSTIP
        textEdit->setStatusTip(QStringLiteral(""));
#endif // QT_NO_STATUSTIP
        textEdit->setReadOnly(true);

        retranslateUi(CapWordDialog);

        QMetaObject::connectSlotsByName(CapWordDialog);
    } // setupUi

    void retranslateUi(QDialog *CapWordDialog)
    {
        CapWordDialog->setWindowTitle(QApplication::translate("CapWordDialog", "Capture Word", 0));
    } // retranslateUi

};

namespace Ui {
    class CapWordDialog: public Ui_CapWordDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_CAPWORDDIALOG_H
