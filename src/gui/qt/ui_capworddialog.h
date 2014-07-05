/********************************************************************************
** Form generated from reading UI file 'capworddialog.ui'
**
** Created by: Qt User Interface Compiler version 5.2.1
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
#include <QtWidgets/QToolButton>

QT_BEGIN_NAMESPACE

class Ui_CapWordDialog
{
public:
    QTextEdit *textEdit;
    QToolButton *saveToolButton;
    QToolButton *dictToolButton;

    void setupUi(QDialog *CapWordDialog)
    {
        if (CapWordDialog->objectName().isEmpty())
            CapWordDialog->setObjectName(QStringLiteral("CapWordDialog"));
        CapWordDialog->resize(240, 180);
        CapWordDialog->setAutoFillBackground(false);
        CapWordDialog->setSizeGripEnabled(false);
        CapWordDialog->setModal(false);
        textEdit = new QTextEdit(CapWordDialog);
        textEdit->setObjectName(QStringLiteral("textEdit"));
        textEdit->setGeometry(QRect(0, 0, 240, 150));
        textEdit->setAcceptDrops(false);
#ifndef QT_NO_STATUSTIP
        textEdit->setStatusTip(QStringLiteral(""));
#endif // QT_NO_STATUSTIP
        textEdit->setReadOnly(true);
        saveToolButton = new QToolButton(CapWordDialog);
        saveToolButton->setObjectName(QStringLiteral("saveToolButton"));
        saveToolButton->setGeometry(QRect(167, 150, 32, 32));
        QIcon icon;
        icon.addFile(QStringLiteral(":/res/favorite.png"), QSize(), QIcon::Normal, QIcon::Off);
        saveToolButton->setIcon(icon);
        saveToolButton->setIconSize(QSize(32, 32));
        saveToolButton->setAutoRaise(true);
        dictToolButton = new QToolButton(CapWordDialog);
        dictToolButton->setObjectName(QStringLiteral("dictToolButton"));
        dictToolButton->setGeometry(QRect(208, 148, 32, 32));
        QIcon icon1;
        icon1.addFile(QStringLiteral(":/res/appicon.png"), QSize(), QIcon::Normal, QIcon::Off);
        dictToolButton->setIcon(icon1);
        dictToolButton->setIconSize(QSize(32, 32));
        dictToolButton->setAutoRaise(true);

        retranslateUi(CapWordDialog);

        QMetaObject::connectSlotsByName(CapWordDialog);
    } // setupUi

    void retranslateUi(QDialog *CapWordDialog)
    {
        CapWordDialog->setWindowTitle(QApplication::translate("CapWordDialog", "Capture Word", 0));
        saveToolButton->setText(QString());
        dictToolButton->setText(QString());
    } // retranslateUi

};

namespace Ui {
    class CapWordDialog: public Ui_CapWordDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_CAPWORDDIALOG_H
