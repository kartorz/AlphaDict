/********************************************************************************
** Form generated from reading UI file 'capworddialog.ui'
**
** Created by: Qt User Interface Compiler version 5.9.8
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
    QToolButton *addToVbToolButton;

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
        textEdit->setGeometry(QRect(0, 0, 350, 180));
        textEdit->setContextMenuPolicy(Qt::DefaultContextMenu);
        textEdit->setAcceptDrops(false);
#ifndef QT_NO_STATUSTIP
        textEdit->setStatusTip(QStringLiteral(""));
#endif // QT_NO_STATUSTIP
        textEdit->setReadOnly(true);
        addToVbToolButton = new QToolButton(CapWordDialog);
        addToVbToolButton->setObjectName(QStringLiteral("addToVbToolButton"));
        addToVbToolButton->setGeometry(QRect(351, 7, 28, 28));
        QIcon icon;
        icon.addFile(QStringLiteral(":/res/favorite.png"), QSize(), QIcon::Normal, QIcon::Off);
        addToVbToolButton->setIcon(icon);
        addToVbToolButton->setIconSize(QSize(32, 32));
        addToVbToolButton->setAutoExclusive(false);
        addToVbToolButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
        addToVbToolButton->setAutoRaise(true);

        retranslateUi(CapWordDialog);

        QMetaObject::connectSlotsByName(CapWordDialog);
    } // setupUi

    void retranslateUi(QDialog *CapWordDialog)
    {
        CapWordDialog->setWindowTitle(QApplication::translate("CapWordDialog", "Capture Word", Q_NULLPTR));
        addToVbToolButton->setText(QString());
    } // retranslateUi

};

namespace Ui {
    class CapWordDialog: public Ui_CapWordDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_CAPWORDDIALOG_H
